/*
 * adc.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */

#include "stm32f103xb.h"
#include "adc.h"

int16_t adc=0;

void setup_adc(){
	/* configure ADC1: order of channels*/
	/* channels:    333332222211111 */
	ADC1->SQR3 |= 0b000000000000000; /*1st: channel 0 (on PA0 pin)*/
	ADC1->SQR3 |= 0b000000000100000; /*2nd: channel 1 (on PA1 pin)*/
	ADC1->SQR3 |= 0b100000000000000; /*3th: channel 16 (internal temperature sensor)*/
	ADC1->SQR1 |= ADC_SQR1_L_1; /* number of channels =0b00010 +1 = 3 */
	/*
	 * Set conversion time
	 * I don't care about speed, so I set the longest possible
	 * (but most precise): 239.5 cycles
	 * (With 8MHz (default) clock it takes ~30us
	 */
	ADC1->SMPR2 |= 0b000000111;
	ADC1->SMPR2 |= 0b000111000;
	ADC1->SMPR2 |= 0b111000000;

	/* Discontinuous mode: 1 channel per conversion */
	ADC1->CR1 |= ADC_CR1_DISCEN;
	/* Start conversion with writing SWSTART bit */
	ADC1->CR2 |= ADC_CR2_EXTTRIG | ADC_CR2_EXTSEL_0 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_2;
	/* Turn on ADC1 and internal temperature sensor */
	ADC1->CR2 |= ADC_CR2_ADON | ADC_CR2_TSVREFE;

	/*
	 * Start calibration procedure
	 * and wait till it ends
	 */
	ADC1->CR2 |= ADC_CR2_CAL;
	while(ADC1->CR2 & ADC_CR2_CAL);
	//ADC1->CR2 |= ADC_CR2_ADON;

}
/*
 * each time the read_adc function is called a conversion of 1 channel occurs
 * next read_adc call will read from next channel
 */

int16_t read_adc(){
	/*
	 * write ADC_CR2_SWSTART to start the conversion of 1 channel
	 * (hardware clears this bit automatically)
	 */
	ADC1->CR2 |= ADC_CR2_SWSTART;
	/* Wait for the end of conversion*/
	while(!(ADC1->SR & ADC_SR_EOC));
	/*
	 * Read data from data register
	 * ADC's resolution is 12bit, so 16bit variable is enough
	 */
	adc=ADC1->DR;

	return adc;
}
