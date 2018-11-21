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
#include "sensors.h"

/*Data variables used by DHT11*/
extern uint64_t dht_data;

//Temporary variable for temperature in degree C
int16_t adc_value; //temporary
//2D array:
//temperature[source][unit]
int16_t temp[][4]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};


/*main timer step, -50 to give some time*/
int systick=-50;


void setup(){
	/* Enable peripherals' clocks */
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
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
	switch(systick){
	/*
	 * Read ADC1_IN16 (internal temperature sensor)
	 */
	case 60:
		/* LM35 (CH0)*/
		/*
		 * Read ADC and store it (I keep it viewable for debug purposes)
		 * Every of the 3 read_adc calls returns conversion results
		 * from 3 different channels
		 */
		temp[source_lm][NONE]=read_adc();
		/*
		 * Convert ADC reading to temperature in degree C
		 * (every sensor has own function)
		 */
		temp[source_lm][degC]=temp_LM35(temp[source_lm][NONE]);
		/* Convert temperature in degree C to degree F */
		temp[source_lm][degF]=convertF(temp[source_lm][degC]);
		/* Convert temperature in degree C to Kelvins*/
		temp[source_lm][K]=convertK(temp[source_lm][degC]);
		break;
	case 70:
		/*NTC (CH1)*/
		temp[source_ntc][NONE]=read_adc();
		temp[source_ntc][degC]=temp_NTC(temp[source_ntc][NONE]);
		temp[source_ntc][degF]=convertF(temp[source_ntc][degC]);
		temp[source_ntc][K]=convertK(temp[source_ntc][degC]);
		break;
	case 80:
		/*internal sensor (CH16)*/
		temp[source_i][NONE]=read_adc();
		temp[source_i][degC]=temp_internal(temp[source_i][NONE]);
		temp[source_i][degF]=convertF(temp[source_i][degC]);
		temp[source_i][K]=convertK(temp[source_i][degC]);
		break;
	case 90:
		/* Blink built-in LED */
		GPIOC->BSRR |=GPIO_BSRR_BR13;
		break;
	case 100:
		/*
		 * Datasheet says, sampling period should be at least 1 second
		 * and for 1 second since power-up chip is unstable.
		 * Therefore DHT11 related code executes at 100th (1s) step.
		 * Set DHT11 pin as output, low
		 * DHT11 will detect low signal and will start sending data after it ends
		 */
		GPIOA->CRL |= GPIO_CRL_MODE6_1;
		GPIOA->CRL |= GPIO_CRL_MODE6_0;
		GPIOA->CRL &= ~GPIO_CRL_CNF6_1;
		GPIOA->CRL &= ~GPIO_CRL_CNF6_0;
		GPIOA->BSRR |= GPIO_BSRR_BR6; /* Reset PA6 pin */
		break;
	case 102:
		/*
		 * Duration of low signal must be at least 18ms
		 * here: 20ms
		 *
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
	case 103:
		/*
		 * Sending data lasts about 4ms (DTH11 datasheet)
		 * here: waiting for 10ms - the shortest time
		 * with that SysTick configuration
		 */
		/* turn off the timer3 */
		TIM3->CCER &= ~TIM_CCER_CC1E;
		TIM3->CR1 &= ~TIM_CR1_CEN;
		/*
		 * DHT11 sensor has no ADC data to store, so
		 * I put a totally random value into NONE
		 * Well, not that random... it has to be negative
		 * to see if "-" symbol works :D
		 */
		temp[source_dht][NONE]=-2137;
		temp[source_dht][degC]=temp_DHT11(dht_data);
		temp[source_dht][degF]= convertF(temp[source_dht][degC]);
		temp[source_dht][K]=convertK(temp[source_dht][degC]);
		/*Reset the DHT11's data variable */
		dht_data=0;
		/* Refer to internal sensor part */

		GPIOC->BSRR |= GPIO_BSRR_BS13; /*blink built-in LED*/
		/* Read buttons from TM1638 and change behaviour of display */
		tm_read_buttons();
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
