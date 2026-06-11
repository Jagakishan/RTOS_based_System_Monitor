/*
 * tim2.c
 *
 *  Created on: Jun 11, 2026
 *      Author: Jagakishan
 */

#include "tim2.h"

uint32_t tim2GetCount(){
	return (TIM2->CNT);
}

void tim2_init(){
	RCC->APB1ENR |= (1U<<0);

	TIM2->PSC = 15;
	TIM2->ARR = 0xFFFFFFFF; //1MHz or 1us timer

	TIM2->CR1 &= ~(1U<<4);  //Upcounting mode

	TIM2->CNT=0; //Initial count to 0.

	TIM2->EGR |= (1U<<0);

	TIM2->CR1 |= (1U<<0); //Start timer

	return;
}
