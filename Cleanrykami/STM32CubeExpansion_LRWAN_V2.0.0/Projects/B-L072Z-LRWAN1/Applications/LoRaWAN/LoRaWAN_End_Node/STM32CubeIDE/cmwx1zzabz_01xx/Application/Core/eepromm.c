#include "stm32l0xx.h"
//#include "eeprom.h" //
#include <string.h> // memcpy
#include "stm32l0xx_hal_flash_ex.h"
#include "usart.h"
#include "se-identity.h"// ключи
#include "lora_app.h"// Версия
#include "LmHandler.h"
#include "LmHandlerTypes.h"
#include "Region.h"
#include "lorawan_version.h"
#include "Commissioning.h"
#include "lora_info.h"
#include "epromm.h"
//#include "sx1276.h"// RSSI|SNR
//#define EEPROM_BASE_ADDR 0x08080000UL
//#define DATA_EEPROM_BANK1_END  (0x08080BFFUL) /*!< Program end DATA EEPROM BANK1 address */
//#define DATA_EEPROM_BANK2_BASE (0x08080C00UL) /*!< DATA EEPROM BANK2 base address in the alias region */
//#define DATA_EEPROM_BANK2_END  (0x080817FFUL) /*!< Program end DATA EEPROM BANK2 address */
////#define EEPROM_PAGE0_ID
//#define EEPROM_BYTE_SIZE	0x03FF
CommissioningParams_t CommissioningParams; // Определения ключей соединения
LmHandlerAppData_t AppData; // Полезная нагрузка
LmHandlerCallbacks_t LmHandlerCallbacks; // Измерение заряда батареи
uint16_t BaseAddresseprom;
uint8_t Dataeprom[22]; // Хранит все ключи и параметры размером 22 байта
uint8_t leneerom;
uint8_t i;
//memcpy(Dataeprom, CommissioningParams.DevEui, 8); // Копирование DevEui размером 8 байт
//mibReq.Type = MIB_JOIN_EUI;
//    LoRaMacMibGetRequestConfirm(&mibReq);
//memcpy1(CommissioningParams.JoinEui, mibReq.Param.JoinEui, 8);
//memcpy(Dataeprom + 8, CommissioningParams.JoinEui, 8); // Копирование JoinEui размером 8 байт
//memcpy((Dataeprom + 16), CommissioningParams.AppKey, 16); // Копирование AppKey размером 16 байт
uint8_t lorawanDeviceEUI[8] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
uint8_t lorawanJoinEUI[8] = {0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
uint8_t lorawanAppKey[16] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0x00};
//CommissioningParams;
//HAL_FLASHEx_DATAEEPROM_Unlock();
// /*
 void FLASHEx_EEPROM_WRITE(uint16_t BaseAddresseprom, uint8_t *Dataeprom, uint8_t leneerom)
{
//	uint16_t BaseAddresseprom;
//	uint8_t *Dataeprom;
//	uint8_t leneerom;
	HAL_StatusTypeDef statusOK = HAL_OK;
//	LORAWAN_DEVICE_EUI, LORAWAN_JOIN_EUI, LORAWAN_APP_KEY;LORAWAN_NWK_KEY, LORAWAN_NWK_S_KEY, LORAWAN_APP_S_KEY

	HAL_FLASHEx_DATAEEPROM_Unlock();
	for(i=0;i<leneerom;i++)
	{
		statusOK +=HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, DATA_EEPROM_BANK2_BASE+Dataeprom+i, *Dataeprom);
		Dataeprom++;

	}
	HAL_FLASHEx_DATAEEPROM_Lock();
}
 void EEPROM_WRITE_DATA(uint32_t addrepr, void *dataeprom, uint32_t sizeepr) // метод записи от АКТИВ
 {
	if ((addrepr < STM32L072_EEPROM_START_ADDR) || (addrepr >= STM32L072_EEPROM_END_ADDR))
//			return EEPROM_ADDR_ERROR;

	if ((sizeepr % 4 != 0) || ( sizeepr > STM32L072_EEPROM_END_ADDR - addrepr))
//			return EEPROM_SIZE_ERROR;

	HAL_FLASHEx_DATAEEPROM_Unlock();            // Разблокировка памяти для чтения и записи с EEPROM
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_WRPERR);  // Сброс флага write protect
		for(uint32_t i = 0; i < sizeepr / 4 ; i++)
		{
			if (HAL_FLASHEx_DATAEEPROM_Erase(addrepr) != HAL_OK) {
//				return EEPROM_ERASE_ERROR;
			}
			if (HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, addrepr, ((uint32_t*)dataeprom)[i]) != HAL_OK) {
//				return EEPROM_WRITE_ERROR;
		    }

			addrepr = addrepr + 4;
			}
  	HAL_FLASHEx_DATAEEPROM_Lock();
// 	return EEPROM_SOK;
 }
 uint32_t EEPROM_ReadData(void) {
   uint32_t readData;
   readData = *(__IO uint32_t*)(EEPROM_BASE_ADDR); // Чтение данных из EEPROM
   return readData;
 }

 void EEPROM_Read_Data(uint32_t addrepr, void *dataeprom, uint32_t sizeepr) {  // чтение от АКТИВ

	 if ((addrepr < STM32L072_EEPROM_START_ADDR) || (addrepr >= STM32L072_EEPROM_END_ADDR))
//		 return EEPROM_ADDR_ERROR;
	 if ((sizeepr % 4 != 0) || ( sizeepr > STM32L072_EEPROM_END_ADDR - addrepr))
//		 return EEPROM_SIZE_ERROR;
	for(uint32_t i = 0; i < sizeepr / 4 ; i++) {
		((uint8_t*)dataeprom)[i] = *(uint8_t*)(addrepr + i);
		}
	 // 	return EEPROM_SOK;
  }

 void EEPROM_CLEAR(void) {    // Очистка записи статистики от АКТИВ

	uint32_t address = 0;
	uint32_t Dataepr = 0;
    uint32_t max = 250;
    uint32_t pause = 10;
    	for (uint32_t i = 0; i < max; i++) {
    		address = STM32L072_EEPROM_START_ADDR + i * 4;
    		EEPROM_WRITE_DATA(address, &Dataepr, 4);
    		HAL_Delay(pause);
  }
 }
// */
/*
void FLASHEx_EEPROM_WRITE(uint16_t BiasAddress, uint8_t *Data)
{
	uint8_t i;
	HAL_StatusTypeDef status = HAL_OK;
//	LORAWAN_DEVICE_EUI, LORAWAN_JOIN_EUI, LORAWAN_APP_KEY;LORAWAN_NWK_KEY, LORAWAN_NWK_S_KEY, LORAWAN_APP_S_KEY

	HAL_FLASHEx_DATAEEPROM_Unlock();
	for(i=0;i<len;i++)
	{
		status +=HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_BASE_ADDR+BiasAddress+i, *Data);
		Data++;

	}
	HAL_FLASHEx_DATAEEPROM_Lock();
}
*/
// int eepromm(void) {

//	 Dataeprom = GetDevAddr();
//	 Dataeprom = CommissioningParams.DevAddr;
//	FLASHEx_EEPROM_WRITE(EEPROM_BASE_ADDR, Dataeprom, sizeof(Dataeprom));
//	 FLASHEx_EEPROM_WRITE(0, lorawanDeviceEUI, sizeof(lorawanDeviceEUI));
//	 FLASHEx_EEPROM_WRITE(sizeof(lorawanDeviceEUI), lorawanJoinEUI, sizeof(lorawanJoinEUI));
//	 FLASHEx_EEPROM_WRITE(sizeof(lorawanDeviceEUI) + sizeof(lorawanJoinEUI),
//	                          lorawanAppKey, sizeof(lorawanAppKey));
//	 HAL_FLASHEx_DATAEEPROM_Unlock();
//	 HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_BYTE, EEPROM_BASE_ADDR+BaseAddresseprom+i,*Dataeprom);
//	 HAL_FLASHEx_DATAEEPROM_Lock();
//	 // Вывод в консоль
//	  printf("Hello, STM32CubeIDE!\r\n");
//	 devAddr
//	 FLASHEx_EEPROM_WRITE(EEPROM_BASE_ADDR, LORAWAN_DEVICE_EUI, sizeof(LORAWAN_DEVICE_EUI));
//	 FLASHEx_EEPROM_WRITE(8, LORAWAN_JOIN_EUI, sizeof(LORAWAN_JOIN_EUI));
//	 FLASHEx_EEPROM_WRITE(16, LORAWAN_APP_KEY, sizeof(LORAWAN_APP_KEY));
//}

/*
 * static int write2flash(const void *start, const void *wrdata, uint32_t stor_size){
    int ret = 0; // исходник для записи
    if (FLASH->CR & FLASH_CR_LOCK){ // unloch flash
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
    while(FLASH->SR & FLASH_SR_BSY) IWDG->KR = IWDG_REFRESH;
    FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPERR; // clear all flags
    FLASH->CR |= FLASH_CR_PG;
    const uint16_t *data = (const uint16_t*) wrdata;
    volatile uint16_t *address = (volatile uint16_t*) start;
    USB_sendstr("Start address="); printuhex((uint32_t)start); newline();
    uint32_t i, count = (stor_size + 1) / 2;
    for(i = 0; i < count; ++i){
        IWDG->KR = IWDG_REFRESH;
        *(volatile uint16_t*)(address + i) = data[i];
        while (FLASH->SR & FLASH_SR_BSY) IWDG->KR = IWDG_REFRESH;
        if(*(volatile uint16_t*)(address + i) != data[i]){
            USB_sendstr("DON'T MATCH!\n");
            ret = 1;
            break;
        }
        #ifdef EBUG
        else{ USB_sendstr("Written "); printuhex(data[i]); newline();}
#endif
        if(FLASH->SR & FLASH_SR_PGERR){
            USB_sendstr("Prog err\n");
            ret = 1; // program error - meet not 0xffff
            break;
        }
        FLASH->SR = FLASH_SR_EOP | FLASH_SR_PGERR | FLASH_SR_WRPERR;
    }
    FLASH->CR &= ~(FLASH_CR_PG);
    return ret;
}
 */

/* void FLASHEx_EEPROM_READ(uint16_t BiasAddress,uint8_t *Buffer,uint16_t Length)
{
	uint8_t *wAddr;
	wAddr=(uint8_t *)(EEPROM_BASE_ADDR+BiasAddress);
	while(Length--)
	{
		*Buffer++=*wAddr++;
	}
}
uint16_t a = 5;
uint16_t b = 5;
//FLASHEx_EEPROM_WRITE(4,a);

FLASHEx_EEPROM_WRITE(EEPROM_BASE_ADDR, &Data, Length); */
