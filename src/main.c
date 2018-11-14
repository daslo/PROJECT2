/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/
#include "stm32f103xb.h"
#include "variables.h"
#include "gpio.h"
#include "dht11.h"
#include "tm1638.h"
#include "i2c.h"
#include "adc.h"
#include "spi.h"
#include "usart.h"
#include "tim.h"

/*Data variables used by DHT11*/
extern uint64_t dht_data;

//Temporary variable for temperature in degree C
int16_t temp_C; //temporary
//2D array:
//temperature[source][unit]
int16_t temp[][4]={0,0,0, 0,0,0, 0,0,0, 0,0,0};

/*main timer step*/
int systick=0;


void setup(){
	/* Enable peripherals' clocks */
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

	/* Call peripheral setup functions */
	setup_gpio();
	setup_usart();
	setup_spi();
	setup_tm();
	setup_timer();
	setup_adc();

	/*
	 * Enable interrupts in NVIC
	 */
	//NVIC_EnableIRQ(TIM1_UP_IRQn); /* (for future use) */
	//NVIC_EnableIRQ(TIM2_IRQn); /* (for future use) */
	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_EnableIRQ(USART1_IRQn);

	/*
	 * main timer: Period = 0.01s
	 * Number_of_Ticks = CPU_Freq * Period
	 */
	SysTick_Config(8000000 * 0.01);
}

int main(void)
{
	setup();
	tm(12345678);
	for(;;);
}

/*
 * SysTick (main timer) interrupt
 * executes every Period
 * For particular points of time, there are snippets of code
 */
__attribute__((interrupt)) void SysTick_Handler(void){
	//if(systick==5)tm_leds=tm_read();

	switch(systick){
	/*
	 * Start acting at systick=100, to give some time
	 * Read ADC1_IN16 (internal temperature sensor)
	 */
	case 100:
		temp_C=read_adc(); /*Read temperature in deg C */

		/* Calculate temperatures for Internal source, for every unit:
		 *
		 * Degree Celsius: just copy
		 */
		temp[source_i][degC]=temp_C;
		/*
		 * Calculate temperature in degree Fahrenheit
		 * Use approximation, to keep variable type (int16)
		 * T(degF) = 2*T(degC) + 32
		 * precise: T(degF)= 5/9 * T(degC) + 32
		 * error= 1,1%
		 */
		temp[source_i][degF]=temp_C*2 + 32;
		/*
		 * Calculate temperature in Kelvin
		 * Use approximation to keep variable type (int16)
		 * T(K) = 273 + T(degC)
		 * precise: T(K)= 273.15 + T(degC)
		 */
		temp[source_i][K]=temp_C+273;
		/* Blink built-in LED */
		GPIOC->BSRR |=GPIO_BSRR_BS13;
		/* Read buttons from TM1638 and change behaviour of display */
		tm_read_buttons();
		break;
	case 200:
		/*
		 * Set DHT11 pin as output, low
		 * DHT11 will detect low signal and will start sending data after it ends
		 */
		GPIOA->CRL |= GPIO_CRL_MODE6_1;
		GPIOA->CRL |= GPIO_CRL_MODE6_0;
		GPIOA->CRL &= ~GPIO_CRL_CNF6_1;
		GPIOA->CRL &= ~GPIO_CRL_CNF6_0;
		GPIOA->BSRR |= GPIO_BSRR_BR6; /* Reset PA6 pin */
		break;
	case 202:
		/*
		 * Duration of low signal must be at least 18ms
		 * here: 20ms
		 * Set DHT11 pin as input with pull-up
		 */
		GPIOA->CRL &= ~GPIO_CRL_MODE6_0;
		GPIOA->CRL &= ~GPIO_CRL_MODE6_1;
		GPIOA->CRL |= GPIO_CRL_CNF6_1;
		GPIOA->CRL &= ~GPIO_CRL_CNF6_0;
		GPIOA->ODR |= GPIO_ODR_ODR6;

		/*
		 * Enable TIM3 timer (in Input Capture mode on channel 1 (PA6))
		 * It will capture and measure durations of high states
		 * (Refer to TIM3 interrupt)
		 */
		TIM3->CCER |= TIM_CCER_CC1E; /*Enable Input Capture on TIM3_CH1*/
		TIM3->CR1 |= TIM_CR1_CEN; /*Enable TIM3*/
		break;
	case 204:
		/*
		 * Sending data lasts about 4ms
		 * here: waiting for 20ms
		 */

		/* turn off the timer3 */
		TIM3->CCER &= ~TIM_CCER_CC1E;
		TIM3->CR1 &= ~TIM_CR1_CEN;


		GPIOC->BSRR |= GPIO_BSRR_BR13; /*blink built-in LED*/

		/*
		 * dht_data contains 40 bits of DHT11's response,
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
		temp_C=((dht_data & 0x0000FF0000)>>16);
		/*Reset the DHT11's data variable */
		dht_data=0;
		/* Refer to internal sensor part */
		temp[source_d][degC]=temp_C;
		temp[source_d][degF]= temp_C*2 + 32;
		temp[source_d][K]=temp_C+273;

		/* Write value on TM1638 */
		tm_write();
		/* Write value to UART */
		usart_write();

		/* Reset the main timer steps */
		systick=0;
		break;
	default:
		break;
	}
	/* Increment main timer step */
	++systick;
}
