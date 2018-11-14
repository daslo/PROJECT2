/*
 * tim.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */

#include "stm32f103xb.h"
#include "tim.h"

/* Data variable used by DHT11 */
uint64_t dht_data=0;
int dht_read=0;

void setup_timer(){
	// f_UEV = f_TIM CLK / (ARR+1)(PSC+1)

	/* TIM1 (for future use)*/
	TIM1->CR1 |=TIM_CR1_ARPE;
	TIM1->DIER |= TIM_DIER_UIE;
	TIM1->CNT=0;
	TIM1->PSC=7;
	TIM1->ARR=0;
	TIM1->CR1 |= TIM_CR1_CEN;

	/* TIM2 (for future use)*/
	TIM2->CR1 |=TIM_CR1_ARPE;
	TIM2->DIER |= TIM_DIER_UIE;
	TIM2->CNT=0;
	TIM2->ARR=8000;
	TIM2->PSC=2000;
	TIM2->CR1 |= TIM_CR1_CEN;

	/*
	 * TIM3 (for dht11)
	 * Input Capture mode on channel 1 (PA6 pin)
	 */
	TIM3->CCMR1 |= TIM_CCMR1_CC1S_0;
	/*
	 * Default configuration (no registers changed):
	 * measure time of HIGH signal
	 * start counting on rising edge
	 * if falling edge detected stop counting and request interrupt
	 */
	TIM3->DIER |= TIM_DIER_CC1IE; /* Enable compare/capture interrupts on channel 1 */
	TIM3->CNT=0; /* Reset CouNT register */
	/*
	 * Set the prescaler, so that timer increments CNT
	 * every 1us (TIM3_Freq = CPU_Freq / (PSC+1)
	 */
	TIM3->PSC=7;
}

__attribute__((interrupt)) void TIM1_UP_IRQHandler(void){
	if(TIM1->SR & TIM_SR_UIF){
		/* (for future use) */
		TIM1->SR &= ~TIM_SR_UIF;
	}
}

__attribute__((interrupt)) void TIM2_IRQHandler(void){
	if(TIM2->SR & TIM_SR_UIF){
		/* (for future use) */
		TIM2->SR &= ~TIM_SR_UIF;
	}
}

__attribute__((interrupt)) void TIM3_IRQHandler(void){
	if(TIM3->SR & TIM_SR_CC1IF){
		/*
		 * Capture/Compare Interrupt
		 * Read value at falling edge
		 *
		 * 70us HIHG signal means '1' bit
		 * 30us HIGH signal means '0' bit
		 *
		 * this interrupt writes whole DHT11 response (40 bit)
		 * to one 64bit variable
		 *
		 * on start is =0. With every bit send function:
		 * 	shifts value left
		 * 	sets last bit if HIGH signal lasts longer than threshold
		 *
		 * 	           1      2     3
		 * 	DHT11 ___------__----__------__
		 *
		 * 	dht_data:
		 *       << 1     |= ...
		 * 	1. ...0000, ...0001
		 * 	2. ...0010, ...0010
		 * 	3. ...0100, ...0101
		 *
		 */
		dht_read=TIM3->CCR1;
			dht_data = dht_data << 1; /*shift data left*/
			if(dht_read > 100){ /*if HIGH signal duration is longer than threshold, it's high bit*/
				dht_data |= 1;
			}
		/* Reset counter and clear interrupt flag */
		TIM3->CNT=0;
		TIM3->SR &= ~TIM_SR_CC1IF;
	}
}
