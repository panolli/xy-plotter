/*
 * GCodeParser.cpp
 *
 *  Created on: 31 août 2016
 *      Author: Jérémy
 */

#include <stdio.h>
#include "GCodeParser.h"


#include "board.h"
#include "FreeRTOS.h"
#include "task.h"
#include "DigitalIoPin.h"
#include "GCodeParser.h"

extern void debug(char *format, uint32_t d1, uint32_t d2, uint32_t d3);

GCodeParser::GCodeParser(MotorController &motorController): motorController(motorController){
}

int GCodeParser::parse(char *command){
	char variable;
	int code;
	char parameters[50];
	sscanf(command, "%c %d %[^\t\n]", &variable, &code, parameters);
	debug("%c command code %d \r\n", variable, code, 0);
	debug(command, 0, 0, 0);
	switch(variable){
	case 'G':
		return parseG(code, parameters);
	case 'M':
		return parseM(code, parameters);
	default:
		return -1;
	}
	return 0;
}

int GCodeParser::parseG(int code, char *parameters){
	char x, y;
	float xVal, yVal;
	switch(code){
	case 0:
		//move pen
	case 1:
		sscanf(parameters, "%c %f %c %f", &x, &xVal, &y, &yVal);
		motorController.move(xVal, (int)yVal);
		Board_UARTPutSTR("OK \n");
		return 0;
	case 28:
		//move home
		Board_UARTPutSTR("OK \n");
		return 0;
	default:
		return -1;
	}
	return 0;
}

int GCodeParser::parseM(int code, char *parameters){
	switch(code){
	case 1:
		int angle;
		sscanf(parameters, "%d", &angle);
		if(angle == 160)
			motorController.penUp();
		else
			motorController.penDown();
		Board_UARTPutSTR("OK \n");
		return 0;
	case 10:
		Board_UARTPutSTR("M10 XY 3040 2480 0.00 0.00 A0 B0 H0 S80 U160 D90 \n");
	case 4:
		//maybe exit
		Board_UARTPutSTR("OK \n");
		return 0;
	default:
		return -1;
	}
	return 0;
}

GCodeParser::~GCodeParser() {
	delete &motorController;
}

