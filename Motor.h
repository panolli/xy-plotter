/*
 * Motor.h
 *
 *  Created on: 13.9.2016
 *      Author: Olli
 */

#ifndef MOTOR_H_
#define MOTOR_H_

#define Y_MIN 4
#define Y_MAX 5
#define X_MIN 0
#define X_MAX 1

#define Y_STEP 6
#define Y_DIR  7

#define X_STEP 8
#define X_DIR  9

#include "DigitalIoPin.h"
class Motor {
public:
	Motor(int min, int max, int step, int direction);
	void setDirection(bool dir);
	bool getDirection();
	void invertDirection();
	void stepUp();
	void stepDown();
	bool getLimitMin();
	bool getLimitMax();
	virtual ~Motor();
private:
	bool dir;
	DigitalIoPin min;
	DigitalIoPin max;
	DigitalIoPin step;
	DigitalIoPin direction;

};

#endif /* MOTOR_H_ */
