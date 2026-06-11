/*
 * uart.h
 *
 *  Created on: Jun 10, 2026
 *      Author: Jagakishan
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#define SYSTEM_CLOCK 16000000
#define BAUD_RATE 115200

#include "main.h"
#include "FreeRTOS.h"
#include "queue.h"

extern QueueHandle_t shellRxQHandle;

void uart_read(uint8_t* buffer, int numberOfBytes);
void uart_init(void);
void USART2_IRQHandler();

#endif /* INC_UART_H_ */
