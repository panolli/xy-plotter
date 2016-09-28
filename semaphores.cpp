/*
 * @brief Blinky example using timers and sysTick
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "DigitalIoPin.h"
#include "semphr.h"
#include "string.h"

#include "time.h"
#include "stdlib.h"

#include "GCodeParser.h"
#include "Motor.h"
#include "MotorController.h"
#include "Pencil.h"

#include "ITM_write.h"

#include "SysLog.h"


#define PINASSIGN7 *((unsigned int *)  0x4003801C)
#define SERVO_PIN    16

volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT;

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

QueueHandle_t syslog_q;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	// initialize RIT (= enable clocking etc.)
	Chip_RIT_Init(LPC_RITIMER);

	// initialize SCT
	Chip_SCTPWM_Init(LPC_SCT0);

	PINASSIGN7 = (PINASSIGN7 & ~(0xff << 8)) | SERVO_PIN << 8;

	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1 );
}


void SCT_Init(void) {
	LPC_SCT0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCT0->CTRL_L |= (72 - 1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz

	LPC_SCT0->MATCHREL[0].L = 120000 - 1; // match 0 @ 1000/1MHz = 1 msec (1 kHz PWM freq)
	LPC_SCT0->MATCHREL[1].L =  1100; // match 1 used for duty cycle (in 10 steps)
	LPC_SCT0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCT0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCT0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCT0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCT0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCT0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCT0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

struct debugEvent {
	char *format;
	uint32_t data[3];
};

extern "C" {
void RIT_IRQHandler(void) {
	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if (RIT_count > 0) {
		RIT_count--;
		// do something useful here...
	} else {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}

void RIT_start(int count, int us) {
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us
			/ 1000000;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);
	// wait for ISR to tell that we're done
	if (xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	} else {
		// unexpected error
	}
}



void debug(char *format, uint32_t d1, uint32_t d2, uint32_t d3){
	debugEvent de;
	de.format = format;
	de.data[0] = d1;
	de.data[1] = d2;
	de.data[2] = d3;
	xQueueSend( syslog_q, ( void * ) &de, portMAX_DELAY );
}


void debugTask(void *pvParameters)
{
	char buffer[64];
	debugEvent e;
	// this is not complete! how do we know which queue to wait on?
	while (1) {
		// read queue
		xQueueReceive(syslog_q, &e, portMAX_DELAY);
		snprintf(buffer, 64, e.format, e.data[0], e.data[1], e.data[2]);
		ITM_write(buffer);
	}
}

void readTask(void *pvParameters)
{
	int count = 0;
	char buffer[100];
	MotorController *motorController = new MotorController(
		new Motor(X_MIN, X_MAX, X_STEP, X_DIR),
		new Motor(Y_MIN, Y_MAX, Y_STEP, Y_DIR),
		new Pencil(PEN),
		310,
		350
	);
	GCodeParser parser(*motorController);
	motorController->init();
	while (1) {
		char c = Board_UARTGetChar();
		if(c != (char) EOF){
			if(c == '\n'){
				if(count != 0){
					buffer[count++] = '\r';
					buffer[count++] = '\n';
					buffer[count++] = '\0';
					parser.parse(buffer);
					count = 0;
				}
			} else {
				buffer[count++] = c;
			}
		}
	}
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void)
{
	prvSetupHardware();

	ITM_init();

	SCT_Init();

	srand(time(NULL));

	syslog_q = xQueueCreate( 20, sizeof(debugEvent));

	sbRIT = xSemaphoreCreateBinary();


	/* Task 1 thread */
	xTaskCreate(debugTask, "debugTask",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* Task 2 thread */
	xTaskCreate(readTask, "readTask",
				configMINIMAL_STACK_SIZE, NULL, (tskIDLE_PRIORITY + 1UL),
				(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}
