/*
 * PencilController.cpp
 *
 *  Created on: 16.9.2016
 *      Author: Olli
 */

#include <Pencil.h>
#include "FreeRTOS.h"
#include "task.h"

Pencil::Pencil(int pin): pen(pin, DigitalIoPin::output, false) {
	LPC_SCT0->MATCHREL[1].L =  1100;
	vTaskDelay(configTICK_RATE_HZ / 500);
	status = UP;
}

void Pencil::set(PenPosition value){
	if(value == UP){
		LPC_SCT0->MATCHREL[1].L =  1100;
	}else{
		LPC_SCT0->MATCHREL[1].L =  1500;
	}
	vTaskDelay(configTICK_RATE_HZ / 500);
	this->status = value;
}

PenPosition Pencil::get(){
	return status;
}

Pencil::~Pencil() {
	// TODO Auto-generated destructor stub
}

