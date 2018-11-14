/*
 * adc.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */

#include "stm32f103xb.h"
#include "adc.h"

int16_t t_internal=0;
int16_t a_internal=0;
#define T_ADC_degC 58
#define A_V25 1779

void setup_adc(){
	/* configure ADC1, measure internal temperature sensor (on ADC1_CH16)*/
	ADC1->SQR3 |= 16; /*channel*/
	/*
	 * Select number of cycles to measure,
	 * For internal temperature sensor reading 71,1us is ok
	 * (refer to datasheet)
	 */
	ADC1->SMPR2 |= 0b110;

	/* Turn on ADC1 and internal temperature sensor */
	ADC1->CR2 |= ADC_CR2_ADON | ADC_CR2_TSVREFE;

	/*
	 * Start calibration procedure
	 * and wait till it ends
	 */
	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);
}

int16_t read_adc(){
	/*
	 * write ADC_CR2_ADON again to start the conversion
	 *
	 */
	ADC1->CR2 |= ADC_CR2_ADON;
	ADC1->CR2 |= ADC_CR2_SWSTART;
	//ADC1->CR2 |= ADC_CR2_CONT;

	a_internal=ADC1->DR; /* Read data from data register */
	/*
	 * Calculate temperature in degree Celsius
	 * This formula is based on formula in Reference Manual
	 * and values in datasheet
	 * 	(Note: it's not exactly the formula from RM
	 * 	V25 value [in V] is changed to analog equivalent analog value
	 * 	and Avg_slope is already used
	 */
	t_internal= 25+(A_V25 - a_internal)*T_ADC_degC/4096;
	return t_internal;
}
