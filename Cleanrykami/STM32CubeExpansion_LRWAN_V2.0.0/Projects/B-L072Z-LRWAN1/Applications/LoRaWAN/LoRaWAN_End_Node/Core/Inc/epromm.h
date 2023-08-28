/*
 * epromm.h
 *
 *  Created on: 14 авг. 2023 г.
 *      Author: kosty
 */

#ifndef APPLICATION_CORE_EPROMM_H_
#define APPLICATION_CORE_EPROMM_H_

/* USER CODE BEGIN Includes */
#include "main.h"
/* USER CODE END Includes */
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_flash_ex.h"
#include "stm32l0xx_hal_flash.h"
/* USER CODE BEGIN Private defines */
#define EEPROM_BASE_ADDR 0x08080000UL
#define DATA_EEPROM_ADDR1_END  (0x08080B00UL)
#define DATA_EEPROM_BANK1_END  (0x08080BFFUL) /*!< Program end DATA EEPROM BANK1 address */
#define DATA_EEPROM_BANK2_BASE (0x08080C00UL) /*!< DATA EEPROM BANK2 base address in the alias region */
#define DATA_EEPROM_BANK2_END  (0x080817FFUL) /*!< Program end DATA EEPROM BANK2 address */
#define STM32L072_EEPROM_START_ADDR  0x08080000
#define STM32L072_EEPROM_END_ADDR    0x08080800
//#define EEPROM_PAGE0_ID
#define EEPROM_BYTE_SIZE	0x03FF
uint32_t EEPROM_ReadData(void);
/* USER CODE END Private defines */
//void MX_DMA_Init(void);
void FLASHEx_EEPROM_WRITE(uint16_t BaseAddresseprom, uint8_t *Dataeprom, uint8_t leneerom);
void EEPROM_WRITE_DATA(uint32_t addrepr, void *dataeprom, uint32_t sizeepr);
void EEPROM_CLEAR();
void EEPROM_Read_Data(uint32_t addrepr, void *dataeprom, uint32_t sizeepr);
void *dataeprom;
//uint32_t dataeprom;
/* USER CODE BEGIN Prototypes */
uint32_t sizeepr;
/* USER CODE END Prototypes */


#endif /* APPLICATION_CORE_EPROMM_H_ */
