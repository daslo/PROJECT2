/*
 * sensors.h
 *
 *  Created on: 21.11.2018
 *      Author: DS
 */

#ifndef SENSORS_H_
#define SENSORS_H_

int16_t temp_NTC(int16_t);
int16_t temp_internal(int16_t);
int16_t temp_LM35(int16_t);
int16_t temp_DHT11(uint64_t);

int16_t convertF(int16_t);
int16_t convertK(int16_t);


#endif /* SENSORS_H_ */
