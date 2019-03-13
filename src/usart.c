/*
 * usart.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */

#include "stm32f103xb.h"
#include "variables.h"
#include "usart.h"

/* Array of temperature values */
extern int16_t temp[][4];

/*source of temperature data*/
enum SOURCE usart_source=source_dht;
/*unit of displayed value */
enum UNIT usart_unit=degC;

uint8_t usart_in=0;

/* Data to send:
 * array size and data array
 * it has contant size and UART will always send USART_DATA bytes
 */
#define USART_DATA 8
uint8_t usart_data[USART_DATA];

void setup_usart(){
	/*Configure USART1: 8N1@9600*/
	USART1->BRR = 8000000/115200; //BaudRate_Register=CPU_Freq / Desired_Baudrate
	/*Default configuration: 8N1 */
	/* Enable: USART, Rx not empty interrupt, Tx, Rx */
	USART1->CR1 = USART_CR1_UE | USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE;
}
void usart_write(){
	/* Send >1 chars by USART
	 * Get temperature from desired source and with desired unit
	 */
	uint16_t n=temp[usart_source][usart_unit];
	/*
	 * Prepare answer:
	 * I want the answer to have constant length:
	 * - 1 char for sign
	 * - 3 chars for value
	 * - 2 chars for unit
	 */
	usart_data[0] = (n>0 ? '+' : '-'); /* Sign */
	/*
	 * Set chars for value:
	 * to get a particular digit of number:
	 *
	 * 1234
	 * (1234 % 10) / 1 = 4
	 * (1234 % 100) / 10 = 3
	 * (1234 % 1000) / 100 = 2
	 * (1234 % 10000) / 1000 = 1
	 *
	 * character_of_digit_n = '0' + n;
	 */
	usart_data[1]='0'+(n%1000)/100;
	usart_data[2]='0'+(n%100)/10;
	usart_data[3]='0'+(n%10);

	/* Set chars for unit symbols */
	switch(usart_unit){
	case degC:
		usart_data[4]='*'; /* Degree. (I couldn't find any better option) */
		usart_data[5]='C';
		break;
	case degF:
		usart_data[4]='*';
		usart_data[5]='F';
		break;
	case K:
		usart_data[4]=' '; /* Space, so that length is constant */
		usart_data[5]='K';
		break;
	default:
		break;
	}
	usart_data[6]=0x0A; /* Line Feed */
	usart_data[7]=0x0D; /* Carriage Return */

	/* Send the char array, byte by byte */
	for(int i=0; i<USART_DATA; ++i) usart_send(usart_data[i]);
}
void usart_send(uint8_t data){
	while(!(USART1->SR & USART_SR_TXE)); /* Loop, if Tx Register NOT empty */
	USART1->DR = data; /* If Tx Register Empty, write data to it */
}

__attribute__((interrupt)) void USART1_IRQHandler(void){
	/* Configure transmission options with received commands */
	if ( USART1->SR & USART_SR_RXNE){ /*Interrupt source: Rx not empty (data received) */
			USART1->SR &= ~USART_SR_RXNE; /* Clear interrupt flag*/
			usart_in= USART1->DR; /*Get value from register*/
			switch(usart_in){
			case 'i': case 'I': /* Set source to [i]nternal sensor */
				usart_source=source_i;
				break;
			case 'n': case 'N':  /* Set source to [N]TC sensor */
				usart_source=source_ntc;
				break;
			case 'l': case 'L':  /* Set source to [L]M35 sensor */
				usart_source=source_lm;
				break;
			case 'd': case 'D': /* Set source to [D]HT11 sensor */
				usart_source=source_dht;
				break;
			case 'c': case 'C': /* Set unit to degree [C]elsius */
				usart_unit=degC;
				break;
			case 'f': case 'F': /* Set unit to degree [F]ahrenheit */
				usart_unit=degF;
				break;
			case 'k': case 'K': /* Set unit to [K]elvin */
				usart_unit=K;
				break;
			default:
				break;
			}
		}
}
