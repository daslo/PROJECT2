/*
 * gpio.c
 *
 *  Created on: 05.10.2018
 *      Author: DS
 */
#include "stm32f103xb.h"
#include "gpio.h"

/*PORT, PIN DESCRIPTION:
 *
 * 0x(port)(pin)
 *
 * port = {0xA,0xB,0xC}
 * pin = {0x0, 0x1, ..., 0xf}
 *
 * 0xA1 = PA1
 * then:
 * pin = 0xA1 mod 0x10 = 0x1
 * port = 0xA1 div 0x10 = 0xA
 */
/* pin descriptions:
 *
 *  PA9 (USART1 TX) = out, alt, 2MHz
 *  port-pin (pin function) = direction, configuration, maximum output speed
 *
 *  direction: out, in
 *  configuration: (GPIO_CRx_CNFx_x)
 *  	pp = push-pull
 *  	pu = pull-up
 *  	pd = pull-down
 *  	float = floating
 *
 *  	analog = analog
 *  	alt = alternative function
 *
 * maximum output speed is configured by GPIO_CRx_MODEx_x bits
 *
 *
 */
void setup_gpio(){
	/* PC13 (built-in LED) = out, pp, 2MHz */
	GPIOC->CRH |= GPIO_CRH_MODE13_1;
	GPIOC->CRH &= ~GPIO_CRH_CNF13_0;

	/* PA8 (PWM1/1) - alt, pp, 50MHz */
	GPIOA->CRH |= GPIO_CRH_CNF8_1;
	GPIOA->CRH &= ~GPIO_CRH_CNF8_0;
	GPIOA->CRH |= GPIO_CRH_MODE8_1;
	GPIOA->CRH |= GPIO_CRH_MODE8_0;

	/* PA9 (USART1 TX) = out, alt, 2MHz */
	GPIOA->CRH |= GPIO_CRH_CNF9_1;
	GPIOA->CRH &= ~GPIO_CRH_CNF9_0;
	GPIOA->CRH |= GPIO_CRH_MODE9_1;
	GPIOA->CRH &= ~GPIO_CRH_MODE9_0;

	/* PA10 (USART1 RX) = in, float*/
	/* default configuration */

	/* PA6 (DHT11 data pin)*/
	/* configured in SysTick interrupt */

	/* PA0 (LM35) = analog */
	GPIOA->CRL &= ~GPIO_CRL_MODE0_0;
	GPIOA->CRL &= ~GPIO_CRL_MODE0_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF0_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF0_1;

	/* PA1 (NTC) = analog */
	GPIOA->CRL &= ~GPIO_CRL_MODE1_0;
	GPIOA->CRL &= ~GPIO_CRL_MODE1_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF1_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF1_1;

	//----------------------------------
	/*
	 * SPI1 configuration, with NSS
	//PA4 (SPI1 NSS) = out, pp, 50MHz
	GPIOA->CRL &= ~GPIO_CRL_CNF4_0;
	GPIOA->CRL &= ~GPIO_CRL_CNF4_0;
	GPIOA->CRL |= GPIO_CRL_MODE4_1;
	GPIOA->CRL |= GPIO_CRL_MODE4_0;

	//PA5 (SPI1 SCK) = out, alt, 50MHz
	GPIOA->CRL |= GPIO_CRL_CNF5_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF5_0;
	GPIOA->CRL |= GPIO_CRL_MODE5_1;
	GPIOA->CRL |= GPIO_CRL_MODE5_0;

	//PA7 (SPI1 MOSI) = out, alt, 50MHz
	GPIOA->CRL |= GPIO_CRL_CNF7_1;
	GPIOA->CRL &= ~GPIO_CRL_CNF7_0;
	GPIOA->CRL |= GPIO_CRL_MODE7_1;
	GPIOA->CRL |= GPIO_CRL_MODE7_0;
	*/
	//------------------------------------
	/* PB12 (SPI2 NSS) = out, pp, 50MHz */
	GPIOB->CRH &= ~GPIO_CRH_CNF12_0;
	GPIOB->CRH &= ~GPIO_CRH_CNF12_0;
	GPIOB->CRH |= GPIO_CRH_MODE12_1;
	GPIOB->CRH |= GPIO_CRH_MODE12_0;

	/* PB13 (SPI2 SCK) = out, ALT, 50MHz */
	GPIOB->CRH |= GPIO_CRH_CNF13_1;
	GPIOB->CRH &= ~GPIO_CRH_CNF13_0;
	GPIOB->CRH |= GPIO_CRH_MODE13_1;
	GPIOB->CRH |= GPIO_CRH_MODE13_0;

	/* PB15 (SPI2 MOSI) = out, ALT, 50MHz */
	GPIOB->CRH |= GPIO_CRH_CNF15_1;
	GPIOB->CRH &= ~GPIO_CRH_CNF15_0;
	GPIOB->CRH |= GPIO_CRH_MODE15_1;
	GPIOB->CRH |= GPIO_CRH_MODE15_0;

}
void set(int n){
	uint8_t pin = n%0x10;
	uint8_t port= n/0x10;
	switch(port){
	case 0xA:
		GPIOA->BSRR |= (GPIO_BSRR_BS0 << pin);
		break;
	case 0xB:
		GPIOB->BSRR |= (GPIO_BSRR_BS0 << pin);
		break;
	case 0xC:
		GPIOC->BSRR |= (GPIO_BSRR_BS0 << pin);
		break;
	default:
		break;
	}
}
void clr(int n){
	uint8_t pin = n%(0x10);
	uint8_t port = n/(0x10);
	switch(port){
	case 0xA:
		GPIOA->BSRR |= (GPIO_BSRR_BR0 << pin);
		break;
	case 0xB:
		GPIOB->BSRR |= (GPIO_BSRR_BR0 << pin);
		break;
	case 0xC:
		GPIOC->BSRR |= (GPIO_BSRR_BR0 << pin);
		break;
	default:
		break;
	}
}
int get(int n){
	uint8_t pin = n%(0x10);
	uint8_t port = n/(0x10);
	uint8_t result=0;
	switch(port){
	case 0xA:
		result = (GPIOA->IDR & (GPIO_IDR_IDR0 << pin));
		break;
	case 0xB:
		result = (GPIOB->IDR & (GPIO_IDR_IDR0 << pin));
		break;
	case 0xC:
		result = (GPIOC->IDR & (GPIO_IDR_IDR0 << pin));
		break;
	}
	return result;
}
