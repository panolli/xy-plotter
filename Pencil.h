/*
 * PencilController.h
 *
 *  Created on: 16.9.2016
 *      Author: Olli
 */

#ifndef PENCIL_H_
#define PENCIL_H_

#include "DigitalIoPin.h"

#define PEN 3

enum PenPosition {UP, DOWN};

class Pencil {
public:
	Pencil(int pin);
	void set(PenPosition value);
	PenPosition get();
	virtual ~Pencil();
private:
	DigitalIoPin pen;
	PenPosition status;
};

#endif /* PENCIL_H_ */
