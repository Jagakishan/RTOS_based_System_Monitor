/*
 * rtc.h
 *
 *  Created on: Jun 9, 2026
 *      Author: Jagakishan
 */

#ifndef INC_RTC_H_
#define INC_RTC_H_

#include "main.h"

#define ASYNCH_PRE_VAL 127 //128-1
#define SYNCH_PRE_VAL 255 //256-1

void rtc_init(uint32_t seconds, uint32_t minutes, uint32_t hours, uint32_t format);
uint8_t get_seconds(void);
uint8_t get_minutes(void);
uint8_t get_hours(void);


#endif
