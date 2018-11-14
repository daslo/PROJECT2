/*
 * tm1638.h
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */
#include "stm32f103xb.h"
#include "variables.h"

#ifndef TM1638_H_
#define TM1638_H_



void tm_write();
void tm_set_leds(void);
void shift_out(int);
void tm(long int);
void setup_tm(void);
void tm_commit(void);
void tm_read_buttons(void);
uint8_t tm_read(void);

#endif /* TM1638_H_ */
