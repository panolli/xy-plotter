/*
 * GCodeParser.h
 *
 *  Created on: 31 août 2016
 *      Author: Jérémy
 */


#ifndef GCODEPARSER_H_
#define GCODEPARSER_H_

#include "MotorController.h"

class GCodeParser {
public:
	GCodeParser(MotorController &motorController);
	virtual ~GCodeParser();
	int parse(char *command);
private:
	int parseG(int code, char *parameters);
	int parseM(int code, char *parameters);
	MotorController &motorController;
};

#endif /* GCODEPARSER_H_ */
