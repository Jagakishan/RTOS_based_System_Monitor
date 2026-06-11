/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "internalTemperature.h"
#include "vrefint.h"
#include "FreeRTOS.h"
#include "task.h"
#include "W25Q32.h"
#include "uart.h"
#include "queue.h"
#include "message_buffer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define TOTAL_COUNT 300000

#define W25Q_START_ADDR 0x000000

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */

int adc_raw;

uint16_t vref_raw;

//Current time
uint32_t hours=10;
uint32_t minutes=50;
uint32_t seconds=0;
uint32_t format=1;

TaskHandle_t temperatureTaskHandle;
TaskHandle_t vrefintTaskHandle;
TaskHandle_t rtcTaskHandle;
TaskHandle_t ramAndLoadStatsTaskHandle;
TaskHandle_t logTaskHandle;
TaskHandle_t readFlashTaskHandle;
TaskHandle_t shellInputTaskHandle;
TaskHandle_t getStatsTaskHandle;
TaskHandle_t getLogTaskHandle;
TaskHandle_t printTaskHandle;

QueueHandle_t shellRxQHandle;

MessageBufferHandle_t printBufHandle;

volatile uint32_t idleCount;

uint8_t writebuffer[256];
char tempMsg[256];

typedef struct{
	uint8_t hr;
	uint8_t min;
	uint8_t sec;
	float actualTemperature;
	float actualVoltage;
	size_t freeHeap;
	float cpuLoad;
}system_state_typedef;

system_state_typedef system_state;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void vApplicationIdleHook(void){
	idleCount++;
}

void temperatureTask(void *parameters){

	while(1){
		vTaskDelay(pdMS_TO_TICKS(500));

		adc_raw=getIntTemp();
		system_state.actualTemperature=scaleTemperature(adc_raw);
	}
}

void vrefintTask(void *parameters){

	while(1){
		vTaskDelay(pdMS_TO_TICKS(500));

		vref_raw=getIntVolt();
		system_state.actualVoltage=scaleVoltage(vref_raw);

	}
}

void rtcTask(void *parameters){

	while(1){
		vTaskDelay(pdMS_TO_TICKS(500));

		system_state.hr=get_hours();
	    system_state.min=get_minutes();
	    system_state.sec=get_seconds();
	}
}

void ramAndLoadStatsTask(void *parameters){

	while(1){
		vTaskDelay(pdMS_TO_TICKS(500));

		system_state.freeHeap=xPortGetFreeHeapSize();

		system_state.cpuLoad=(1.0 - (float)idleCount/(float)TOTAL_COUNT) * 100.0;
		idleCount=0;
	}
}

void logTask(void *parameters){
	static uint32_t currentAddress=0x0;
	static uint32_t nextPageAddress=0x100; //256 bytes

	while(1){
		vTaskDelay(pdMS_TO_TICKS(2000));

		uint16_t len=sprintf((char*)writebuffer,
				"=========================================\r\n"
				"TIME				%u:%u:%u\r\n"
				"INTERNAL TEMPERATURE 	%.2f degrees\r\n"
				"INTERNAL VOLTAGE 		%.3f V\r\n"
				"TOTAL FREE HEAP AVAILABLE:   %u bytes\r\n"
				"CPU Load: 			%.1f\r\n"
				"=========================================\r\n", system_state.hr, system_state.min, system_state.sec,
				system_state.actualTemperature, system_state.actualVoltage, system_state.freeHeap, system_state.cpuLoad);


		if((nextPageAddress-currentAddress) < (len+1)){
			currentAddress=nextPageAddress;
			nextPageAddress+=256;
		}

		W25Q32_Write(currentAddress, writebuffer, (len+1));
		currentAddress+=(len+1);

		xTaskNotify(readFlashTaskHandle, (uint32_t)len, eSetValueWithOverwrite);
	}
}

void readFlashTask(void *parameters){

	static uint32_t readAddress=(uint32_t)W25Q_START_ADDR;
	uint32_t notificationValue;

	while(1){


		xTaskNotifyWait(0, 0, &notificationValue, portMAX_DELAY);

		uint8_t buffer[256];

		W25Q32_Read(readAddress, buffer, (uint16_t)(notificationValue+1));

		readAddress+=256;
	}
}

void shellInputTask(void *parameters){
	char *buffer="\n\nWHAT DO YOU WANT?\r\n\n"
			"GET ELABORATE RUN-TIME STATS:	1\r\n"
			"GET LOG DATA:			2\r\n"
			"CONFIGURE TIME:			3\r\n"
			"FLASH CHIP ERASE		4\r\n\n";

	uint8_t ch;
	uint8_t rxBuf[8];
	uint8_t idx=0;

	sprintf(tempMsg, "Click anything to start\r\n");
	xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);

	while(1){

		xQueueReceive(shellRxQHandle, &ch, portMAX_DELAY);

		if(ch=='\r' || ch=='\n' || ch=='\0' || idx==7){
			rxBuf[idx]='\0';
			idx=0;

			int option=rxBuf[0];
			switch(option){
			case 49:
				xTaskNotifyGive(getStatsTaskHandle);
				break;
			case 50:
				xTaskNotifyGive(getLogTaskHandle);
				break;
			case 52:
				W25Q32_BlockErase(W25Q_START_ADDR);
				sprintf(tempMsg, "\n\tMESSAGE: Winbond Chip erased.\r\n\n\nClick anything to continue...\r\n");
				xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
				break;
			case 51:
				sprintf(tempMsg,"Enter value for HOURS: \r\n");
				xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
				xQueueReceive(shellRxQHandle, &ch, portMAX_DELAY);
				if(ch!=13){
					hours=0;
					hours+=(ch & 0xFF)-'0';

					xQueueReceive(shellRxQHandle, &ch, portMAX_DELAY);
					if(ch!=13){
						hours*=10;
						hours+=(ch & 0xFF)-'0';
					}
				}

				sprintf(tempMsg,"Enter value for MINS: \r\n");
				xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
				xQueueReceive(shellRxQHandle, &ch, portMAX_DELAY);
				if(ch!=13){
					minutes=0;
					minutes+=(ch & 0xFF)-'0';

					xQueueReceive(shellRxQHandle, &ch, portMAX_DELAY);
					if(ch!=13){
						minutes*=10;
						minutes+=(ch & 0xFF)-'0';
					}
				}

				sprintf(tempMsg,"Enter value for SECONDS: \r\n");
				xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
				xQueueReceive(shellRxQHandle, &ch, portMAX_DELAY);
				if(ch!=13){
					seconds=0;
					seconds+=(ch & 0xFF)-'0';

					xQueueReceive(shellRxQHandle, &ch, portMAX_DELAY);
					if(ch!=13){
						seconds*=10;
						seconds+=(ch & 0xFF)-'0';
					}
				}

				rtc_init(seconds, minutes, hours, format);
				sprintf(tempMsg,"\n\tMESSAGE: Time configured to %lu:%lu:%lu\r\n\nClick anything to continue...\r\n"
						, hours, minutes, seconds);
				xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
				break;
			default:
				sprintf(tempMsg, "%s\r\n", buffer);
				xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
				break;
			}

		}else{
			rxBuf[idx++]=ch;
		}

	}
}

void getStatsTask(void *params){

	while(1){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		char stats[256];

		vTaskGetRunTimeStats(stats);


		xMessageBufferSend(printBufHandle, stats, strlen(stats)+1, portMAX_DELAY);

		sprintf(tempMsg, "\nClick anything to continue...\r\n");
		xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
	}
}

void getLogTask(void *param){

	while(1){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);


		xMessageBufferSend(printBufHandle, writebuffer, strlen((char*)writebuffer)+1, portMAX_DELAY);

		sprintf(tempMsg,"\r\n\nClick anything to continue...\r\n");
		xMessageBufferSend(printBufHandle, tempMsg, strlen(tempMsg)+1, portMAX_DELAY);
	}
}

void printTask(void *parameters){
	uint8_t printData[256];

	while(1){
		xMessageBufferReceive(printBufHandle, printData, sizeof(printData), portMAX_DELAY);

		printf("%s", printData);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
  uart_init();
  tim2_init(); //For getting runtime stats

  printf("\nSYSTEM MONITOR SHELL\r\n\n");

  W25Q32_init();
  W25Q32_BlockErase(W25Q_START_ADDR);

  rtc_init(seconds, minutes, hours, format);
  adc_init();


  BaseType_t taskCreationStatus;

  taskCreationStatus = xTaskCreate(temperatureTask, "TEMP", 200, NULL, 1, &temperatureTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(vrefintTask, "VREFINT", 200, NULL, 1, &vrefintTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(rtcTask, "RTC", 200, NULL, 1, &rtcTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(ramAndLoadStatsTask, "RAM&LOAD", 200, NULL, 1, &ramAndLoadStatsTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(logTask, "LOGGER", 300, NULL, 2, &logTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(readFlashTask, "READ LOG", 300, NULL, 2, &readFlashTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(shellInputTask, "SHELL INPUT", 200, NULL, 3, &shellInputTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(getStatsTask, "STATS", 200, NULL, 3, &getStatsTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(getLogTask, "GET LOG", 300, NULL, 3, &getLogTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  taskCreationStatus = xTaskCreate(printTask, "DISPLAY", 300, NULL, 3, &printTaskHandle);
  configASSERT(taskCreationStatus==pdPASS);

  shellRxQHandle= xQueueCreate(16, sizeof(uint8_t));

  printBufHandle= xMessageBufferCreate(256);

  vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
