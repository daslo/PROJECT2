/*
 * spi.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */

#include "stm32f103xb.h"
#include "spi.h"

void setup_spi(){
	/* bidirectional data line, output */
	SPI2->CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
	/* baudrate divider  F_SPI= CPU_Freq/32 */
	SPI2->CR1 |= SPI_CR1_BR_2;
	/* clock polarity and phase (refer to manual) */
	SPI2->CR1 |= SPI_CR1_CPOL | SPI_CR1_CPHA;
	/*send data Last Significant Bit First */
	SPI2->CR1 |= SPI_CR1_LSBFIRST;
	/* software NSS pin management */
	SPI2->CR1 |= SPI_CR1_SSM;
	SPI2->CR1 |= SPI_CR1_SSI;
	/* SPI in master mode */
	SPI2->CR1 |= SPI_CR1_MSTR;
	/* Enable SPI */
	SPI2->CR1 |= SPI_CR1_SPE;

}
void spi_write(uint8_t data){

	SPI2->DR = data; //write data to data register

	//SPI1->DR = data;
	//while(!(SPI2->SR & SPI_SR_TXE));
	for(int i=0; i<10000; ++i);//wait (TODO)
}
