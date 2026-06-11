/*
 * rtc.c
 *
 *  Created on: Jun 9, 2026
 *      Author: Jagakishan
 */

#include "rtc.h"

static void enter_init_mode(void){
    RTC->ISR |= (1U<<7); //Init mode

    while(!(RTC->ISR & (1U<<6))){} //Wait for init mode confirmation

    return;
}

static void exit_init_mode(void){
    RTC->ISR &= ~(1U<<7); // Exit Init mode

    while((RTC->ISR & (1U<<6))){} //Wait for free run mode confirmation

    return;
}

static void set_asynch_prescale_value(void){
    RTC->PRER &= ~(127<<16);
    RTC->PRER |= (ASYNCH_PRE_VAL<<16);
}

static void set_synch_prescale_value(void){
    RTC->PRER &= ~(32767<<0);
    RTC->PRER |= (SYNCH_PRE_VAL<<0);
}

static uint8_t bcd2dec(uint8_t bcdVal){
    return (uint8_t)(((bcdVal&0xF0)>>4)*10U + (bcdVal&0x0F));
}

static uint8_t dec2bcd(uint32_t decVal){
	uint8_t ones=decVal%10;
	decVal=decVal/10;
	uint8_t tens=decVal;

	return (uint8_t)(((tens)<<4) | ((ones)<<0));
}

static void set_time(uint32_t seconds, uint32_t minutes, uint32_t hours, uint32_t format){
	uint8_t s=dec2bcd(seconds);
	uint8_t m=dec2bcd(minutes);
	uint8_t h=dec2bcd(hours);

    uint32_t temp=0;

    temp = (((uint32_t)s<<0) | ((uint32_t)m<<8) | ((uint32_t)h<<16) | (format<<22));

    RTC->TR=temp;

    return;
}

uint8_t get_hours(void){
    uint8_t temp;
    temp=(uint8_t)((RTC->TR & 0x003F0000)>>16);

    return bcd2dec(temp);
}

uint8_t get_minutes(void){
    uint8_t temp;
    temp=(uint8_t)((RTC->TR & 0x00007F00)>>8);

    return bcd2dec(temp);
}

uint8_t get_seconds(void){
    uint8_t temp;
    temp=(uint8_t)((RTC->TR & 0x0000007F)>>0);

    return bcd2dec(temp);
}

void rtc_init(uint32_t seconds, uint32_t minutes, uint32_t hours, uint32_t format){
    RCC->APB1ENR |= (1U<<28); //PWR peripheral enable

    PWR->CR |= (1U<<8); //Disable backup domain write protection to configure RTC

    RCC->BDCR |= (1U<<16);
    RCC->BDCR &= ~(1U<<16); //Reset the backup domain and again start for proper working.

    RCC->BDCR |= (1U<<0); //LSE on
    while(!(RCC->BDCR & (1U<<1))){} //wait for LSE to be ready

    RCC->BDCR |= (1U<<8);
    RCC->BDCR &= ~(1U<<9); //RTC clock sel->LSE

    RCC->BDCR |= (1U<<15); //Enable RTC

    RTC->WPR = 0xCAU;
    RTC->WPR = 0x53U; //Disable RTC write protection

    enter_init_mode();

    set_asynch_prescale_value();

    set_synch_prescale_value();

    set_time(seconds, minutes, hours, format);

    exit_init_mode();

    RTC->WPR = 0xFFU; //enable RTC write protection

    return;
}

