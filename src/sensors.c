/*
 * sensors.c
 *
 *  Created on: 21.11.2018
 *      Author: DS
 */

#include "stm32f103xb.h"
#include "sensors.h"
/*
 * Calculations of temperature for various sensors
 */

/*
 * NTC thermistor
 *
 * Calculate value in degree C
 *
 *     I             I
 *   +->--[NTC]----+->--[ R ]----+
 *   |             |             |
 *  VCC         ADC_IN1         GND
 *
 *  1st step: calculate resistance of NTC sensor:
 *  VCC - ADC_IN1 = I*R_NTC
 *  ADC_IN1 - GND = I*R
 *
 *  I= (ADC_IN1 - GND) / R
 *  R_NTC = R* (VCC - ADC_IN1)/(ADC_IN1 - GND)
 *
 *  to calculate temperature based on resistance I used the NTC datasheet:
 *
 *  here: I took values from Temperature-Resistance table
 *  (not all, just for temperatures from -15 to +35 deg C)
 *  and approximated them with 2nd degree polynomial
 *  its coefficients are: a2, a1, a0
 *  (for temperature in degree C and resistance in kOhm)
 */

const float a2=0.072557;
const float a1=-3.708759;
const float a0=55.050231;
const float R_series = 2200; //[Ohm]
const uint16_t A_MAX = 4095;


int16_t temp_NTC(int16_t adc){
	/* resistance of NTC [Ohm]*/
	float R_NTC= (float)(A_MAX-adc) * R_series / (float)adc;
	/* temperature [degC] */
	float t= a2*(R_NTC/1000)*(R_NTC/1000) + a1*(R_NTC/1000) + a0;
	return (int16_t)t;
}

/*
 * STM32 internal temperature sensor
 *
 * Calculate temperature in degree Celsius
 * Based on formula from Reference Manual and values from datasheet
*/
 /* voltage at 25 degC: 1.34 - 1.52 (typ. 1.43) [V] */
const float V25=1.43;
/* ADC reading from temperature sensor at 25 degC
int16_t A25=(int16_t)(V25  *4095.0 / 3.3);
*/
const int16_t A25=1775;
/* average slope: 4.0 - 4.6 (typ. 4.3) [mV/degC] */
const float Avg_slope=0.0043; //[V/degC]

int16_t temp_internal(int16_t adc){
	float t= 25.0 + ((A25 - adc)*3.3 / 4095)/Avg_slope;
	return (int16_t)t;
}

/*
 * LM35
 * it's calibrated directly in Celsius scale
 * scale factor = 10 [mV/degC]
 *
 */
const float lm35_scale_factor=0.01; //[V/degC]
int16_t temp_LM35(int16_t adc){

	float t= (adc*3.3/ 4095 )/lm35_scale_factor;
	return (int16_t)t;
}

/*
 * DTH11
 *
 * dht_data contains all 40 bits of DHT11's response,
 * 5x8 bits:
 *
 *   RH i      RH f     T  i     T  f     CS
 * |--------|--------|--------|--------|--------|
 *
 *  RH - Relative Humidity [%]
 *  T - temperature [degree C]
 *  CS - checksum
 *  i - integral part
 *  f - fraction
 *
 *  To get only integral part of temperature:
 *  -mask the variable
 *  -shift the variable
 */
int16_t temp_DHT11(uint64_t data){
	return (data & 0x0000FF0000)>>16;
}
/*
 * Convert temperature to degree Fahrenheit
 * T(degF)= 5/9 * T(degC) + 32
 */

int16_t convertF(int16_t C){
	float F=C*9.0/5.0 + 32;
	return (int16_t)F;
}
/*
 * Convert temperature to Kelvin
 * T(K)= 273.15 + T(degC)
 */

int16_t convertK(int16_t C){
	float K=C+273.15;
	return (int16_t)K;
}
