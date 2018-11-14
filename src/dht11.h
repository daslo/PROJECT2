/*
 * dht11.h
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */
#include "stm32f103xb.h"
#include "variables.h"

#ifndef DHT11_H_
#define DHT11_H_

extern int dht_delay_cnt;

void dht(void);
int dht_start(void);
int dht_is_running(void);
int read_dht_crude(void);

#endif /* DHT11_H_ */
