/*
 * vrefint.c
 *
 *  Created on: Jun 9, 2026
 *      Author: Jagakishan
 */

#include "vrefint.h"


float scaleVoltage(uint32_t vref_raw){

	uint16_t vrefint_cal=*VREFINT_CAL_ADDR;

	float actualVoltage=0.0;

	actualVoltage= 3.3 * ((float)vrefint_cal/(float)vref_raw);

	return actualVoltage;
}

uint32_t getIntVolt(){

	  uint32_t vref_raw;

	  ADC1->SQR3 = 0x11; //17th channel for Voltage sensing

	  ADC->CCR |= (1U<<23); //Turn on temperature sensor

	  ADC1->CR2 |= (1U<<0); //Turn ADC on

	  ADC1->CR2 |= (1U<<30); //Start conversion

	  while(!(ADC1->SR & (1U<<1)));

	  vref_raw=ADC1->DR;


	  return vref_raw;

}
