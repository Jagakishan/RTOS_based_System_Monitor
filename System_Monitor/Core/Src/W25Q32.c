#include "W25Q32.h"
#include "string.h"

static inline void CSlow(){
	HAL_GPIO_WritePin(CS_Port, CS_Pin, GPIO_PIN_RESET);
}

static inline void CShigh(){
	HAL_GPIO_WritePin(CS_Port, CS_Pin, GPIO_PIN_SET);
}

static void tx(uint8_t data){
	uint8_t dummy=0xFF;
	HAL_SPI_TransmitReceive(&hspi1, &data, &dummy, 1, HAL_MAX_DELAY);
}

static void tx_addr(uint32_t addr){
	tx((addr>>16) & 0xFF);
	tx((addr>>8) & 0xFF);
	tx(addr & 0xFF);
}

static uint8_t rx(){
	uint8_t dummy=0xFF, rxData;
	HAL_SPI_TransmitReceive(&hspi1, &dummy, &rxData, 1, HAL_MAX_DELAY);

	return rxData;
}

static void W25Q_writeEnable(){
	CSlow();
	tx(W25Q_WRITE_EN);
	CShigh();
}

static uint8_t W25Q_waitBusy(){
	uint8_t status;
	do{
		CSlow();
		tx(W25Q_RD_STATUS1);
		status=rx();
		CShigh();
	}while(status & W25Q_WIP_BIT);

	return status;
}

void W25Q32_init(){
	CShigh();
	W25Q_waitBusy();
}

uint32_t W25Q32_ReadID(){
	uint32_t JEDEC_ID=0;

	CSlow();
	tx(W25Q_READ_ID);
	JEDEC_ID |= ((uint32_t)rx()<<16);
	JEDEC_ID |= ((uint32_t)rx()<<8);
	JEDEC_ID |= (uint32_t)rx();
	CShigh();

	return JEDEC_ID;	// Should be 0xEF4016
}

void W25Q32_Read(uint32_t addr, uint8_t *buf, uint16_t len){
	uint8_t txDummy[256];
	memset(txDummy, 0xFF, len);

	CSlow();
	tx(W25Q_READ);
	tx_addr(addr);
	HAL_SPI_TransmitReceive(&hspi1, txDummy, buf, len, HAL_MAX_DELAY);
	CShigh();
}

void W25Q32_Write(uint32_t addr, uint8_t *buf, uint16_t len){
	if (len > 256) len = 256;

	W25Q_waitBusy();
	W25Q_writeEnable();
	W25Q_waitBusy();
	CSlow();
	tx(W25Q_PAGE_PROG);
	tx_addr(addr);
	HAL_SPI_Transmit(&hspi1, buf, len, HAL_MAX_DELAY);
	CShigh();
	W25Q_waitBusy();
}

void W25Q32_SectorErase(uint32_t addr){
    W25Q_writeEnable();
    W25Q_waitBusy();
    CSlow();
    tx(W25Q_SECTOR_ERASE);
    tx_addr(addr);
    CShigh();
    W25Q_waitBusy();
}

void W25Q32_ChipErase(uint32_t addr){
    W25Q_writeEnable();
    W25Q_waitBusy();
    CSlow();
    tx(W25Q_CHIP_ERASE);
    tx_addr(addr);
    CShigh();
    W25Q_waitBusy();
}

void W25Q32_BlockErase(uint32_t addr){
    W25Q_writeEnable();
    W25Q_waitBusy();
    CSlow();
    tx(W25Q_BLOCK_ERASE);
    tx_addr(addr);
    CShigh();
    W25Q_waitBusy();
}







