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
#define DATA_EEPROM_BANK1_END  (0x08080BFFUL) /*!< Program end DATA EEPROM BANK1 address */
#define DATA_EEPROM_BANK2_BASE (0x08080C00UL) /*!< DATA EEPROM BANK2 base address in the alias region */
#define DATA_EEPROM_BANK2_END  (0x080817FFUL) /*!< Program end DATA EEPROM BANK2 address */
//#define EEPROM_PAGE0_ID
#define EEPROM_BYTE_SIZE	0x03FF
uint32_t EEPROM_ReadData(void);
/* USER CODE END Private defines */

//void MX_DMA_Init(void);
void FLASHEx_EEPROM_WRITE(uint16_t BaseAddresseprom, uint8_t *Dataeprom, uint8_t leneerom);
/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */


#endif /* APPLICATION_CORE_EPROMM_H_ */
