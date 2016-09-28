/*
 * Motor.cpp
 *
 *  Created on: 13.9.2016
 *      Author: Olli
 */

#include <Motor.h>


Motor::Motor(int min, int max, int step, int direction):
min(min, DigitalIoPin::pullup, true), max(max, DigitalIoPin::pullup, true),step(step, DigitalIoPin::output, false),direction(direction, DigitalIoPin::output, true){
	// TODO Auto-generated constructor stub

}

void Motor::setDirection(bool dir){
	this->dir = dir;
	direction.write(dir);
}

bool Motor::getDirection(){
	return dir;
}

void Motor::invertDirection(){
	dir=!dir;
	direction.write(dir);
}

void Motor::stepUp(){
	step.write(1);
}

void Motor::stepDown(){
	step.write(0);
}

bool Motor::getLimitMax(){
	return max.read();
}

bool Motor::getLimitMin(){
	return min.read();
}

Motor::~Motor() {
	// TODO Auto-generated destructor stub
}

