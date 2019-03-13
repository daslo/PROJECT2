/*
 * tm1638.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */
#include "stm32f103xb.h"
#include "tm1638.h"
#include "variables.h"
#include "gpio.h"
#include "spi.h"

/* Array of temperature values
 * External, defined in main.c
 */
extern int16_t temp[4][4];
/*
 * State of LEDS
 * when sending data about LED, only last bit of byte matters
 * therefore only 8bits are needed
 */
uint8_t tm_leds=0x00;
/* State of 7 segment displays, each define by 8 bits */
uint8_t tm_seg[]={0, 0 ,0 ,0, 0, 0, 0, 0};
/* Source of temperature data */
enum SOURCE tm_source=source_dht;
/* Unit of displayed value */
enum UNIT tm_unit=degC;


/*
 * NSS pin
 * SPI NSS is software-controlled, so it acts as ordinary GPIO pin
 * it must be low during communication with TM1638
 */
#define pSTROBE 0xBC /*PB12*/

/*
 * to set state of  7-seg display, the uC has to send
 * a 8 bits, each defining one segment
 *   _____
 *  |  a  |
 * f|     |b
 *  |_____|
 * e|  g  |c
 *  |     |
 *  |_____| o [dp]
 *     d
 *
 */
/* Following constants contain states of segments (in order: [dp]gfedcba)
 * equivalent to symbols: */

/*digits: 0,1,2,3,4,5,6,7,8,9*/
const int DIGITS[]={
		0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110,
		0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111 };
/* other set of digits, not used here*/
const int DIGITS2[]={
		0b00111111, 0b00000110, 0b01010011, 0b01001111, 0b01000110,
		0b01001001, 0b01111001, 0b00000111, 0b01111111, 0b01001111 };

/* define also other symbols */
const int SYM_BLANK = 0b00000000; /*blank */
const int SYM_MINUS = 0b01000000; /*minus */
const int SYM_DEG = 0b01100011; /*degree */
const int SYM_C = 0b00111001; /*C */
const int SYM_F = 0b01110001; /*F */
const int SYM_K1 = 0b01110010; /*K - 1st part */
const int SYM_K2 = 0b00010000; /*K - 2nd part */

void tm_commit(){
	/* Send states of LEDs and 7Seg displays */
	tm_set_leds();

	clr(pSTROBE); /* NSS=0 */
	spi_write(0x40); /* address auto-increment mode */
	set(pSTROBE); /* NSS=1 */

	clr(pSTROBE); /* NSS=0 */
	spi_write(0xc0); /* first address */
	for(int i=0; i<8; ++i){
		spi_write(tm_seg[i]); /* send state of i-th 7Seg display */
		//spi_write((uint8_t) (tm_leds & (1<<i))); /* send state of i-th LED */
		spi_write( (tm_leds & (1<<i)) ? 0xff : 0x00);
	}
	set(pSTROBE); /* NSS=1 */
}

/* Show variable 'n' on TM1638 */
void tm(long int n){
	/*set variables, so that they represents 'n' in decimal base */
	tm_seg[7]=DIGITS[(n%10)/1];
	tm_seg[6]=DIGITS[(n%100)/10];
	tm_seg[5]=DIGITS[(n%1000)/100];
	tm_seg[4]=DIGITS[(n%10000)/1000];
	tm_seg[3]=DIGITS[(n%100000)/10000];
	tm_seg[2]=DIGITS[(n%1000000)/100000];
	tm_seg[1]=DIGITS[(n%10000000)/1000000];
	tm_seg[0]=DIGITS[(n%100000000)/10000000];
	tm_commit(); /* send data to TM1638*/
}
void tm_set_leds(void){
	//(TODO) <?>
	//set leds based on source of tm data
	tm_leds= 1<<tm_source;
}
void tm_write(){
	/*
	 * Get temperature from desired source and with desired unit
	 * Set the 7Segs as follows:
	 *
	 * - sign (optional)
	 * - 1000s' digit
	 * - 100s' digit
	 * - 10s' digit
	 * - 1s' digit
	 * - always blank
	 * - unit (1st part)
	 * - unit (2nd part)
	 *
	 */
	int16_t n=temp[tm_source][tm_unit];
	tm_seg[0]=(n>=0 ? SYM_BLANK : SYM_MINUS); /* set '-' sing if needed */

	if(n<0) n=-n; /*get absolute value, because sign is already handled */
	/* set digits */
	tm_seg[1]=DIGITS[(n%10000)/1000];
	tm_seg[2]=DIGITS[(n%1000)/100];
	tm_seg[3]=DIGITS[(n%100)/10];
	tm_seg[4]=DIGITS[n%10];
	tm_seg[5]=SYM_BLANK;

	/* set 2 displays for unit symbol */
	switch(tm_unit){
	case degC: /*[degree C]*/
		tm_seg[7]=SYM_C;
		tm_seg[6]=SYM_DEG;
		break;
	case degF: /*[degree F]*/
		tm_seg[7]=SYM_F;
		tm_seg[6]=SYM_DEG;
		break;
	case K: /* [K] (well, sort of) (TODO?)*/
		tm_seg[7]=SYM_K2;
		tm_seg[6]=SYM_K1;
		break;
	default:
		tm_seg[7]=SYM_BLANK;
		tm_seg[6]=SYM_BLANK;
		break;
	}
	tm_commit(); /*Send data to TM1638 */
}


uint8_t tm_read(){
	/* Read state of buttons */
	clr(pSTROBE); /* NSS=0 */
	spi_write(0x42); /* Read buttons mode */
	for(int i=0; i<1000; ++i); //wait (TODO: check TXNE flag?)
	SPI2->CR1 &= ~SPI_CR1_BIDIOE; /* set SPI bidirectional line as input */

	while(!(SPI2->SR & SPI_SR_RXNE)); /*wait for data*/
	for(int i=0; i<1000; ++i);//wait (TODO: check TXNE flag?)
	uint8_t buttons=SPI2->DR; /*get data*/

	set(pSTROBE); /*NSS=1*/

	SPI2->CR1 |= SPI_CR1_BIDIOE; /* Set SPI bidirectional line as output */
	return buttons;
}

void tm_read_buttons(){
	/*
	 * Change which sensor's temperatue user want to read
	 * and what unit it should have
	 *
	 * Normally I would have each button for one source or one unit
	 *
	 * TM1638 board is very cheap hardware and can easily break
	 * if some of the buttons don't work (like mine :D) you can
	 * toggle through values by pressing one button
	 */
	uint8_t buttons=tm_read();
	if(buttons == 1){
		/* button1 pressed => change source of data */
		++tm_source; /* increment enumerated variable */
		/*
		 * Last possible value of my enum types are not a valid values
		 * They tell, that last possible valid option was changed,
		 * so the variable needs to be reset
		 *
		 * button  |
		 *         |____-____-_____-___-
		 *         |    .    .     .   .
		 *         |    .    .     .   .
		 *    (end)|    .    .     .   .
		 *        3|    .     ____ /   .
		 *        2|     ____/     |    ___
		 *        1|____/          |___/
		 *         +-----------------------
		 *
		 * last value (end) reached => reset variable
		 */
		if(tm_source == source_END) tm_source=0;
		//tm_leds = (1 << tm_source);
	}
	if(buttons == 16){
		/* button5 pressed => change unit */
		++tm_unit;
		if(tm_unit == END) tm_unit=0;
	}
	if(buttons == 17){
		/*do nothing */
	}
}
void setup_tm(){
	/* Activate display and set brightness */
	clr(pSTROBE); /* NSS=0 */
	spi_write(0x88); /* bits: 000abbbb, a=activate, b=brightness */
	set(pSTROBE); /* NSS=1 */
}

