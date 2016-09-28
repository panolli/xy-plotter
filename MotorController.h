/*
 * MotorController.h
 *
 *  Created on: 13.9.2016
 *      Author: Olli
 */

#ifndef MOTORCONTROLLER_H_
#define MOTORCONTROLLER_H_

#define SPEED_US    20
#define SPEED_COUNT  1
#include "Motor.h"
#include "Pencil.h"

class MotorController {
public:
	MotorController(Motor *xMotor, Motor *yMotor, Pencil *pencil, int sizeMMX, int sizeMMY);
	void init();
	void move(float x, float y);
	void penUp();
	void penDown();
	virtual ~MotorController();
private:
	Motor &xMotor;
	Motor &yMotor;
	Pencil &pencil;
	int xTick, yTick;
	int sizeTickX, sizeTickY;
	int sizeMMX, sizeMMY;
	float tickPerMMX, tickPerMMY;
	int initMotor(Motor &motor);
};

#endif /* MOTORCONTROLLER_H_ */
