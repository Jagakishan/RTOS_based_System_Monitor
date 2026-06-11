#ifndef W25Q32_H
#define W25Q32_H

#include "main.h"
#include "stm32f4xx_hal.h"



extern SPI_HandleTypeDef hspi1;

#ifdef DEBUG_W25Qxx
#define W25_DBG(...) do{printf(__VA_ARGS__); printf("\r\n"); }while(0);
#else
#define W25_DBG(...)
#endif


#define W25Q_READ_ID     0x9F
#define W25Q_WRITE_EN    0x06
#define W25Q_READ        0x03
#define W25Q_PAGE_PROG   0x02
#define W25Q_SECTOR_ERASE 0x20
#define W25Q_RD_STATUS1  0x05
#define W25Q_WIP_BIT     0x01
#define W25Q_CHIP_ERASE  0xC7
#define W25Q_BLOCK_ERASE  0xD8

#define CS_Port GPIOA
#define CS_Pin GPIO_PIN_4

void W25Q32_init();
uint32_t W25Q32_ReadID();
void W25Q32_Write(uint32_t addr, uint8_t *buf, uint16_t len);
void W25Q32_Read(uint32_t addr, uint8_t *buf, uint16_t len);
void W25Q32_SectorErase(uint32_t addr);
void W25Q32_ChipErase(uint32_t addr);
void W25Q32_BlockErase(uint32_t addr);

#endif
