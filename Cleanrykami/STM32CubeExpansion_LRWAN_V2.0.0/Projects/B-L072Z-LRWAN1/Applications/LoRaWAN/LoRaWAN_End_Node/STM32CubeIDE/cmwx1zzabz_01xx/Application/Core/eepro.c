#include "MDR32F9Qx_config.h"
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_eeprom.h>
#include "flash.h"

#define			 EEPROM_PAGE_SIZE  	 0x00001000

const uint32_t EEPROM_FLASH_SIZE 	=  0x00020000;
const uint32_t EEPROM_ADDR_START	=  0x08000000;
const uint32_t EEPROM_ADDR_END		=  0x08020000;
const uint32_t DATA_EMPTY 			=  0xFFFFFFFF;

uint32_t PAGE_ADDR_MASK  		= ~(EEPROM_PAGE_SIZE -1);

uint8_t bufData [4096];
  void memcpy8(void* dst, const void* src, unsigned int cnt);

// Инициализация flash
FlashDrvStatus flashHwInit() {
  // Тактирование
  RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);

  return FLASH_DRV_SOK;
}

// Очистка все памяти
FlashDrvStatus flashEraseAllPage() {
	uint32_t i;
	EEPROM_EraseAllPages (EEPROM_Main_Bank_Select);

	for (i = 0; i < EEPROM_FLASH_SIZE; i += 4) {

		if (EEPROM_ReadWord (EEPROM_ADDR_START + i, EEPROM_Main_Bank_Select) != DATA_EMPTY) {
			return FLASH_DRV_ERROR;
		}
	}
	return FLASH_DRV_SOK ;
}

// Очистка одной страницы
FlashDrvStatus flashErasePage(uint32_t addr) {
	uint32_t i;
	if ((addr >= EEPROM_ADDR_START) && (addr < EEPROM_ADDR_END)) {
		EEPROM_ErasePage (addr, EEPROM_Main_Bank_Select);

		for (i = 0; i < EEPROM_PAGE_SIZE; i += 4) {
			if (EEPROM_ReadWord (addr + i, EEPROM_Main_Bank_Select) != DATA_EMPTY) {
				return FLASH_DRV_ERROR;
			}
		}
	}
	else return FLASH_DRV_ADDR_ERROR;
	return FLASH_DRV_SOK;
}

//Чтение данных по байтам
FlashDrvStatus flashReadData(uint32_t addr, uint8_t *data, uint32_t size ) {
	uint32_t i;
	for (i = 0; i < size; i++) {
		*(data++)= EEPROM_ReadByte (addr + i, EEPROM_Main_Bank_Select);
	}
	return FLASH_DRV_SOK;
}

// Запись данных по байтам
FlashDrvStatus flashWriteData(uint32_t addr, uint8_t *data, uint32_t size ) {
	uint32_t i;
	for (i = 0; i < size; i++) {
		EEPROM_ProgramByte (addr+i, EEPROM_Main_Bank_Select,data[i]);
	}
	return FLASH_DRV_SOK;
}

// Чтение страницы из flash в буфер
static FlashDrvStatus flashReadPage(uint32_t addr) {
	return flashReadData (addr & PAGE_ADDR_MASK, bufData, EEPROM_PAGE_SIZE);
}

// Запись страницы из буфера во flash
static FlashDrvStatus flashWritePage(uint32_t addr) {
	return flashWriteData (addr & PAGE_ADDR_MASK,bufData, EEPROM_PAGE_SIZE);
}

// Запись данных с кэшированием
FlashDrvStatus flashWrite(uint32_t addr, uint8_t *data, uint32_t size ) {

	uint32_t bytesWrited = 0;
	uint32_t endAddr = addr + size;
	uint32_t pageStart; // Начальный адрес страницы
	uint32_t pageEnd; // Конечный адрес страницы
	uint32_t changeStart; // Начальный адрес заменяемых данных в странице
	uint32_t changeEnd; // Конечный адрес заменяемых данных в странице
	uint32_t changeSize; // Колличество заменяемых данных в странице

	while (bytesWrited < size) {
		pageStart = (addr + bytesWrited) & PAGE_ADDR_MASK;
		pageEnd = pageStart + EEPROM_PAGE_SIZE;

		changeStart = (addr + bytesWrited) - pageStart;
		changeEnd = (endAddr > pageEnd) ? pageEnd - pageStart : endAddr - pageStart;
		changeSize = changeEnd - changeStart;

		flashReadPage (pageStart);

		memcpy8(&bufData[changeStart], data + bytesWrited, changeSize);

		if (flashErasePage (pageStart) != FLASH_DRV_SOK) {
			return FLASH_DRV_ERROR;
		}

		flashWritePage (pageStart);
		bytesWrited += changeSize;
	}
	return FLASH_DRV_SOK;
}

  void memcpy8(void* dst, const void* src, unsigned int cnt)
  {
    char* pszDest = (char*)dst;
    const char* pszSource = (const char*)src;

    while (cnt)
    {
      *(pszDest++) = *(pszSource++);
      --cnt;
    }
  }










