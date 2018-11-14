/*
 * usart.h
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */

#include "stm32f103xb.h"
#include "variables.h"

#ifndef USART_H_
#define USART_H_

void setup_usart(void);
void usart_write(void);
void usart_send(uint8_t);

#endif /* USART_H_ */
