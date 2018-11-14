/*
 * dht11.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */
#include "stm32f103xb.h"
#include "dht11.h"

#include "gpio.h"


int dht_delay_cnt;
#define DHT_CNT_MCU1 20
#define pDHT 0xA6
#define MSG_LEN 40

//DHT data is 40bit long
uint64_t dht_data;
//mask: temperature (integral only)
uint64_t t_mask;
int dht_cnt;

int read_dht(){
	//PA3 - out, pp, 2MHz, LOW
	GPIOA->CRL |= GPIO_CRL_MODE3_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF3_0;
	GPIOA->CRL |= GPIO_BSRR_BR3;

	//keep DATA line high
	dht_delay_cnt=20000;

	//PA3 - in, pu
	GPIOA->CRL &= ~GPIO_CRL_MODE3_0;
	GPIOA->CRL &= ~GPIO_CRL_MODE3_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF3_0;
	GPIOA->CRL |= GPIO_CRL_CNF3_1;
	GPIOA->ODR |=GPIO_ODR_ODR3;

	//wait for DHT response
	//pull line high (pull-up)
	while(get(pDHT));
	//dht sets line low
	while(!get(pDHT));
	//dht pulls line high
	while(get(pDHT));

	dht_data=0;
	for(int i=0; i<MSG_LEN; ++i){
		//bit start
		while(!get(pDHT));
		dht_cnt=0;
		//count time of high state
		while(get(pDHT)){
			//count number of microseconds
			dht_delay_cnt=1;
			while(dht_delay_cnt>0);
			++dht_cnt;
		}
		//shift data left
		dht_data = dht_data << 1;
		if(dht_cnt > 50){
			//if duration was longer than 50us, it was high bit
			dht_data |= 1;
		}
	}
	uint8_t t = (dht_data & t_mask) >> 32;
	return t;
}

int read_dht_crude(){
	//PA3 - out, pp, 2MHz, LOW
	GPIOA->CRL |= GPIO_CRL_MODE3_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF3_0;
	GPIOA->CRL |= GPIO_BSRR_BR3;

	//tm_write(-1, 1 );
	//keep DATA line high
	dht_delay_cnt=20000;

	//PA3 - in, pu
	GPIOA->CRL &= ~GPIO_CRL_MODE3_0;
	GPIOA->CRL &= ~GPIO_CRL_MODE3_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF3_0;
	GPIOA->CRL |= GPIO_CRL_CNF3_1;
	GPIOA->ODR |=GPIO_ODR_ODR3;

	//wait for DHT response
	//set line high (pull-up)
	while(get(pDHT));
	//dht sets line low
	while(!get(pDHT));
	//dht pulls line high
	while(get(pDHT));

	int dht_data_t=0;

	for(int i=0; i<8; ++i){
		//bit start
		while(!get(pDHT));
		dht_cnt=0;
		//count time of high state
		while(get(pDHT)){
			//count number of microseconds
			dht_delay_cnt=1;
			while(dht_delay_cnt>0);
			++dht_cnt;
		}
		//shift data left
		dht_data_t = dht_data_t << 1;
		if(dht_cnt > 50){
			//if duration was longer than 50us, it was high bit
			dht_data_t |= 1;
		}
	}
	return dht_data_t;
}

