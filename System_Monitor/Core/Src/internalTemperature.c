/*
 * internalTemperature.c
 *
 *  Created on: Jun 9, 2026
 *      Author: Jagakishan
 */

#include "internalTemperature.h"

float V25=0.76;  //Typical value from datasheet  Unit V
float avg_slope=0.0025; //Typical value from datasheet. Unit mV/*C

void adc_init(){

	  //Initialize and configure ADC1

	  RCC->APB2ENR |= (1U<<8);

	  //ADC1->CR2 |= (1U<<1); //Continuous conversion mode;

	  ADC1->SMPR1 |= (1U<<26);
	  ADC1->SMPR1 &= ~(1U<<25);
	  ADC1->SMPR1 |= (1U<<24);  //Set sampling time as 112 cycles for channel 18

	  ADC1->SMPR1 |= (1U<<23);
	  ADC1->SMPR1 &= ~(1U<<22);
	  ADC1->SMPR1 |= (1U<<21);  //Set sampling time as 112 cycles for channel 17

	  ADC1->SQR1 =0x0; //One conversion

	  return;

}

float scaleTemperature(int adc_raw){
	float Vsense=(adc_raw * 3.3f)/4096.0f;

	float act_temp=0;

	act_temp=((Vsense - V25)/avg_slope) + 25;

	return act_temp;
}

int getIntTemp(){
	int adc_raw;

	  ADC1->SQR3 = 0x12; //18th channel for Temperature sensing

	  ADC->CCR |= (1U<<23); //Turn on temperature sensor

	  ADC1->CR2 |= (1U<<0); //Turn ADC on

	  ADC1->CR2 |= (1U<<30); //Start conversion

	  while(!(ADC1->SR & (1U<<1)));

	  adc_raw=ADC1->DR;


	  return adc_raw;

}






