/*
 * SysLog.h
 *
 *  Created on: 2 sept. 2016
 *      Author: Jérémy
 */

#include "semphr.h"

#ifndef SYSLOG_H_
#define SYSLOG_H_

class Syslog {
public:
	Syslog();
	virtual ~Syslog();
	void write(char *description);
private:
	SemaphoreHandle_t syslogMutex;
};
Syslog::Syslog() {
	syslogMutex = xSemaphoreCreateMutex();
}
Syslog::~Syslog() {
	vSemaphoreDelete(syslogMutex);
}
void Syslog::write(char *description) {
	if (xSemaphoreTake(syslogMutex, portMAX_DELAY) == pdTRUE) {
		Board_UARTPutSTR(description);
		xSemaphoreGive(syslogMutex);
	}
}

#endif /* SYSLOG_H_ */
