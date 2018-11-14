/*
 * i2c.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */
#include "stm32f103xb.h"
#include "i2c.h"

void setup_i2c(){
	//I2C1, slave
	I2C1->CR2 |= I2C_CR2_FREQ_1;
	I2C1->CCR |= 0x04;
	I2C1->TRISE |= 0;
	I2C1->CR1 |= I2C_CR1_PE;
}

