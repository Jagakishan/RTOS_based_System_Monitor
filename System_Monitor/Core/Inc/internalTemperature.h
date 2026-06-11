#ifndef INT_TEMP
#define INT_TEMP

#include "main.h"

void adc_init();
float scaleTemperature(int adc_raw);
int getIntTemp();

#endif
