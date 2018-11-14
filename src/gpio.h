/*
 * gpio.h
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */


#ifndef GPIO_H_
#define GPIO_H_

#include "stm32f103xb.h"

void setup_gpio(void);

void set(int);
void clr(int);
int get(int);

#endif /* GPIO_H_ */
