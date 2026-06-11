/*
 * vrefint.h
 *
 *  Created on: Jun 9, 2026
 *      Author: Jagakishan
 */

#ifndef INC_VREFINT_H_
#define INC_VREFINT_H_

#include "main.h"

#define VREFINT_CAL_ADDR ((uint16_t *)0x1FFF7A2A)


float scaleVoltage(uint32_t vref_raw);
uint32_t getIntVolt();


#endif
