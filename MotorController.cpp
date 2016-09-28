/*
 * MotorController.cpp
 *
 *  Created on: 13.9.2016
 *      Author: Olli
 */

#include <MotorController.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stdlib.h"


extern void RIT_start(int count, int us);

MotorController::MotorController(Motor *xMotor, Motor *yMotor, Pencil *pencil, int sizeMMX, int sizeMMY):
xMotor(*xMotor),yMotor(*yMotor),
pencil(*pencil),
sizeMMX(sizeMMX), sizeMMY(sizeMMY){
	// TODO Auto-generated constructor stub

}

int MotorController::initMotor(Motor &motor){
	int size = 0;
	motor.setDirection(1);
	while(!motor.getLimitMax()){
		motor.stepUp();
		RIT_start(SPEED_COUNT, SPEED_US);
		motor.stepDown();
		RIT_start(SPEED_COUNT, SPEED_US);
	}
	motor.invertDirection();
	while(!motor.getLimitMin()){
		motor.stepUp();
		RIT_start(SPEED_COUNT, SPEED_US);
		motor.stepDown();
		RIT_start(SPEED_COUNT, SPEED_US);
		size++;
	}
	motor.invertDirection();
	for(int i = 0; i < 15; i++){
		motor.stepUp();
		RIT_start(SPEED_COUNT, SPEED_US);
		motor.stepDown();
		RIT_start(SPEED_COUNT, SPEED_US);
	}
	return size;
}
void MotorController::init(){
	penUp();
	xTick = yTick = 0;
	sizeTickX = initMotor(xMotor) - 30;
	sizeTickY = initMotor(yMotor) - 30;
	tickPerMMX = sizeTickX / (float) sizeMMX;
	tickPerMMY = sizeTickY / (float) sizeMMY;

}

void MotorController::move(float x, float y){
	x = x / 8;
	y = y / 8;
	int xTick=x * tickPerMMX;
	int yTick=y * tickPerMMY;
	int dx = xTick - this->xTick;
	int dy = yTick - this->yTick;
	int incx = dx > 0 ? -1 : 1;
	int incy= dy > 0 ? -1 : 1;
	int err = (dx>dy ? dx : -dy)/2, e2;
	int countX = 0;
	int countY = 0;

	xMotor.setDirection(incx != 1);
	yMotor.setDirection(incy != 1);
	dx = abs (dx);
	dy = abs (dy);

	int max = dx > dy ? dx : dy;
	int count = 0;
	int currentSpeed = 30;
	int acceleration = 200;
	  for(;;){
	    //setPixel(x,y);
	    if (countX==dx || countY==dy){
	    	if (countX==dx && countY==dy){
	    		break;
	    	}
	    }
	    e2 = err;
	    if (e2 >-dx && !(countX==dx)) {
	    	err -= dy;
	    	countX++;
	    	xMotor.stepUp();
	    }
	    if (e2 < dy && !(countY==dy)) {
	    	err += dx;
	    	countY++;
	    	yMotor.stepUp();
	    }
		RIT_start(SPEED_COUNT * currentSpeed, SPEED_US);
	    xMotor.stepDown();
	    yMotor.stepDown();
		RIT_start(SPEED_COUNT * currentSpeed, SPEED_US);
		if(!(++count % acceleration)){
			if(count < max / 2){
				if(currentSpeed > 1){
					currentSpeed--;
				}
			}
			else if((max - count) <= acceleration * 10){
				if(currentSpeed < 10){
					currentSpeed++;
				}
			}
		}
	  }
	  this->xTick = xTick;
	  this->yTick = yTick;
}

void MotorController::penUp(){
	pencil.set(UP);
}
void MotorController::penDown(){
	pencil.set(DOWN);
}

MotorController::~MotorController() {
	delete &xMotor;
	delete &yMotor;
	delete &pencil;
}

