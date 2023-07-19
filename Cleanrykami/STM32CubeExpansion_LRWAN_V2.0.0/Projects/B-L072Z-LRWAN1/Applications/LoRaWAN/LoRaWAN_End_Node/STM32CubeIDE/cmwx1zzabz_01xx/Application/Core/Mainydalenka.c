#include <MDR32F9Qx_port.h>
#include <MDR32F9Qx_rst_clk.h>
#include <MDR32F9Qx_eeprom.h>
#include <MDR32F9Qx_uart.h>
#include "pll.h"
#include "uart.h"
#include "flash.h"
#include "gpio.h"

#define EEPROM_PAGE_SIZE    0x00001000
#define EEPROM_FLASH_SIZE   0x00020000
#define EEPROM_ADDR_START   0x08000000
#define EEPROM_ADDR_END  	0x08020000

#define COMMAND_BAUD 				0x62 	// Установка скорости обмена
#define COMMAND_WRITE_FLASH  		0x6C 	// Запись данных во flash память
#define COMMAND_ERASE_FLASH  		0x79	// Очистка Flash памяти
#define COMMAND_ERASE_PAGE			0x60 	// Очистка  страницы flash памяти

#define REQUEST_STATUS 				0x04	// Запрос статуса
#define REQUEST_SERVICE_DATA 		0x13 	// Запрос сервисных данных
#define REQUEST_DATA_FLASH 			0x69 	// Запрос данных из flash памяти
#define REQUEST_CR					0x6D 	// Запрос на приглашение

#define RESPONSE_STATUS 			0x05 	// Статус устройства

#define DATA_EMPTY 				0xFFFFFFFF
#define WORD					0x04  		// длина слова в байтах
 typedef enum {
	COMMAND_SOK    			= 0, // Статус ОК
	UART_RECEIVE_ERROR		= 1, // Аппаратная ошибка UART
	PARAMETR_ERROR			= 2, // Неправильный параметр
	ERROR_BAUD				= 3, // Не правильный параметр скорости
	COMMAND_ERROR			= 4, // Ошибка выполения команды


 } CommandStatus; // статус выполнения команд

 // Команды подтверждения
 uint8_t serviceData [5]	= {REQUEST_SERVICE_DATA, 0x01, 0x02,0x03,0x04}; // Сервисные данные
 uint8_t responseCr [3]		= {0x6D, 0x0A, 0x3E}; 		// Ответ на приглашение
 uint8_t ackCommandOk		= 0x4B; 					// Потверждение выполнения команды
 uint8_t ackComWriteFLASH 	=  COMMAND_WRITE_FLASH; 	// Потверждение приема команды
 uint8_t ackRequestData 	=  REQUEST_DATA_FLASH;		// Потверждение приема команды
 uint8_t ackCommandBaud		=  COMMAND_BAUD;			// Потверждение приема команды
 uint8_t ackEraseFlash		=  COMMAND_ERASE_FLASH;		// Потверждение приема команды
 uint8_t ackErasePage		=  COMMAND_ERASE_PAGE;		// Потверждение приема команды

 // Сообщения об ошибках
 uint8_t errChn [2]		= {0x45,0x69};	// Аппаратная ошибка UART
 uint8_t errCmd [2]		= {0x45,0x63};	// Принята неизвестная команда или параметр
 uint8_t errBaud [2]	= {0x45,0x62};	// Не правильный параметр скорости
 uint8_t errMake [2]	= {0x45,0x64};	// Ошибка выполения команды

 CommandStatus status;
 UartDrvStatus uartStatus;
SUartStatusReceive stRec; // Статус приема по UART

 uint8_t dataRx [4096];// буфер для хранения входных данных

 CommandStatus writeDataFlashUart();
 CommandStatus sendDataFlashUart();
 CommandStatus baudChange();
 CommandStatus sendStatus();
 CommandStatus comErasePage();
 CommandStatus comEraseAll();

 // отладка

 void mcuDelay(int waitTicks)
 {
   int i;
   for (i = 0; i < waitTicks; i++)
   {
     __NOP();
   }
 }
 // конец отладки

 int main(void) {

 uint8_t commandUart;

 // Настройка pll
 SPllHwSetup pllHwSetup;
 pllHwSetup.pllMultiplier = 8; ///9
 pllHwSetup.cpuPrescaler = CPU_CLK_DIV1;

 //	Настройка UART2_CTRL
SGpioHwSetup setupGpio_uart2;
setupGpio_uart2.pinNum = PORT_Pin_2;
setupGpio_uart2.pinPort = GPIO_PORT_F;
setupGpio_uart2.pinDir = GPIO_MODE_OUT;

 // Настройка uart
 SUartHwSetup uartHwSetup;
 uartHwSetup.uartChannel = UART_CHANNEL_2;
 uartHwSetup.baudRate = UART_BAUD_RATE_9600;
 uartHwSetup.wordLength =UART_WORD_LENGTH_8B;
 uartHwSetup.stopBits = UART_STOP_BITS_1;
 uartHwSetup.parity = UART_PARITY_NO;

// //	 Настройка GPIO
//	SGpioHwSetup setupGpio;
//	setupGpio.pinNum = PORT_Pin_12;
//	setupGpio.pinPort =  GPIO_PORT_B;
//	setupGpio.pinDir = GPIO_MODE_OUT;
//
//    SGpioHwSetup setupGpio_2;
//    setupGpio_2.pinNum = PORT_Pin_14;
//  	setupGpio_2.pinPort =  GPIO_PORT_B;
//  	setupGpio_2.pinDir = GPIO_MODE_OUT;

 // Инициализация pll
 pllInit(&pllHwSetup);

 //	Инициализация gpio для UART_2
 gpioInit(&setupGpio_uart2);

// //	Инициализация gpio для СД
// gpioInit(&setupGpio);
// gpioInit(&setupGpio_2);

 // Инициализация uart
 uartHwInit (&uartHwSetup);

 //	Инициализация flash
 flashHwInit();

 while (1) {
//	 // отладка
//	 //		 Мигание светодиодом
//	 		PORT_SetBits(MDR_PORTB, PORT_Pin_12 | PORT_Pin_14);
//	 		mcuDelay(5000000);
//	 		PORT_ResetBits(MDR_PORTB, PORT_Pin_12 | PORT_Pin_14);
//	 		mcuDelay(5000000);
//	 // конец отладки

	uartStatus = uartReceiveData(UART_CHANNEL_2, &commandUart,  1, &stRec);

	if (uartStatus == UART_DRV_SOK) {

		switch(commandUart) {

			case REQUEST_CR:
				uartSendData(UART_CHANNEL_2, responseCr, 3);
				break;

			case COMMAND_ERASE_FLASH:
				status = comEraseAll();
				break;

			case COMMAND_ERASE_PAGE:
				status = comErasePage();
				break;

			case COMMAND_BAUD :
				status = baudChange();
				break;

			case COMMAND_WRITE_FLASH:
				status = writeDataFlashUart();
				break;

			case REQUEST_STATUS:
				status = sendStatus();
				break;

			case REQUEST_SERVICE_DATA:
				uartSendData (UART_CHANNEL_2, serviceData, 5);
				break;

			case REQUEST_DATA_FLASH:
				status = sendDataFlashUart();
				break;

			default:
				uartSendData (UART_CHANNEL_2, errCmd, 2);
				break;
		} // end switch
	} // end if
	if (uartStatus == UART_DRV_RECEIVE_ERROR) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
	}
 } // end while
} // end main

 // Запись данных
 CommandStatus writeDataFlashUart () {

	uint32_t addrData, size;

	// Получение адреса
	if (uartReceiveData (UART_CHANNEL_2, &addrData,WORD,&stRec) != UART_DRV_SOK) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	// Проверка на корректность адреса
	if (!((addrData >= EEPROM_ADDR_START) && (addrData < EEPROM_ADDR_END))){
		uartSendData (UART_CHANNEL_2, errCmd, 2);
		return COMMAND_ERROR;
	}
	// Получение размера принимаемых данных
	if (uartReceiveData (UART_CHANNEL_2, &size,WORD,&stRec) != UART_DRV_SOK) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	// проверка корректности размера
	if (size > EEPROM_PAGE_SIZE) {
		uartSendData (UART_CHANNEL_2, errCmd, 2);
		return COMMAND_ERROR;
	}
	// Отправка подверждения приема команды
	uartSendData (UART_CHANNEL_2, &ackComWriteFLASH, 1);

	// Прием данных
	if (uartReceiveData(UART_CHANNEL_2, dataRx, size,&stRec) != UART_DRV_SOK) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	if (flashWrite (addrData, dataRx, size ) == FLASH_DRV_SOK) {
		//Отправка подтвеждения выполнения команды
		uartSendData (UART_CHANNEL_2, &ackCommandOk, 1);
		return COMMAND_SOK;
	}
	else {
		uartSendData (UART_CHANNEL_2, errMake, 2);
		return COMMAND_ERROR;
	}
 }

 // Отправка данных из flash памяти
 CommandStatus sendDataFlashUart() {

	uint8_t data;
	uint32_t i, addrData, size;

	// Получение адреса
	if (uartReceiveData (UART_CHANNEL_2, &addrData,WORD,&stRec) != UART_DRV_SOK) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	// Проверка на корректность адреса
	if (!((addrData >= EEPROM_ADDR_START) && (addrData < EEPROM_ADDR_END))){
		uartSendData (UART_CHANNEL_2, errCmd, 2);
		return COMMAND_ERROR;
	}
	// Получение размера принимаемых данных
	if (uartReceiveData (UART_CHANNEL_2, &size,WORD,&stRec) != UART_DRV_SOK) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	// проверка корректности размера
	if (size > EEPROM_PAGE_SIZE) {
		uartSendData (UART_CHANNEL_2, errCmd, 2);
		return COMMAND_ERROR;
	}
	// Отправка подверждения приема команды
	uartSendData (UART_CHANNEL_2, &ackRequestData, 1);

	for (i = 0; i < size; i++) {
		flashReadData (addrData+i,&data,1);
		uartSendData (UART_CHANNEL_2, &data, 1);
	}
	//Отправка подтвеждения выполения команды
	uartSendData (UART_CHANNEL_2, &ackCommandOk, 1);
	return COMMAND_SOK;
 }

 // Cмена скорости
 CommandStatus baudChange() {

	uint32_t baud;

	// Получение значения скорости
	if (uartReceiveData(UART_CHANNEL_2, &baud,WORD,&stRec) != UART_DRV_SOK) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	// Проверка значения скорости
	switch(baud) {
		case UART_BAUD_RATE_9600:
			break;
		case UART_BAUD_RATE_14400:
			break;
		case UART_BAUD_RATE_28800:
			break;
		case UART_BAUD_RATE_56000:
			break;
		case UART_BAUD_RATE_128000:
			break;
		case UART_BAUD_RATE_256000:
			break;
		default:
			uartSendData (UART_CHANNEL_2, errBaud, 2);
			return ERROR_BAUD;
	}
	// Отправка подверждения приема команды
	uartSendData (UART_CHANNEL_2, &ackCommandBaud, 1);

	//Смена скорости
	uartBaudChange (UART_CHANNEL_2,baud);

	return COMMAND_SOK;
 }

 // Отправка статуса
 CommandStatus sendStatus () {
	uint8_t data, st;
	// Получение параметров
	if (uartReceiveData(UART_CHANNEL_2, &data,  1,&stRec) != UART_DRV_SOK) {
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	st = status << 2;
	// Отправка статуса
	uartSendData (UART_CHANNEL_2, &st, 1);
 }

 // Очистка страницы flash памяти
 CommandStatus comErasePage () {
	 uint32_t addrData;
	// Получение адреса
	if (uartReceiveData (UART_CHANNEL_2, &addrData,WORD,&stRec) != UART_DRV_SOK) {
		//Отправка ошибки
		uartSendData (UART_CHANNEL_2, errChn, 2);
		return UART_RECEIVE_ERROR;
	}
	// Проверка на корректность адреса
	if ((addrData >= EEPROM_ADDR_START) && (addrData < EEPROM_ADDR_END)){
		//Отправка подтверждения приема команды
		uartSendData (UART_CHANNEL_2, &ackErasePage, 1);
	}
	else {
		// Отправка ошибки
		uartSendData (UART_CHANNEL_2, errCmd, 2);
		return PARAMETR_ERROR;
	}
	//Очитска страницы
	if (flashErasePage(addrData) == FLASH_DRV_SOK) {
		//Отправка подтвеждения выполения команды
		uartSendData (UART_CHANNEL_2, &ackCommandOk, 1);
		return COMMAND_SOK;
	}
	else {
		// Отправка ошибки
		uartSendData (UART_CHANNEL_2, errMake, 2);
		return COMMAND_ERROR;
	}
 }

  // Очитска всей памяти
  CommandStatus comEraseAll () {
	//Отправка подтверждения приема команды
	uartSendData (UART_CHANNEL_2, &ackEraseFlash, 1);
	// Очистка всей памяти
	if (flashEraseAllPage() == FLASH_DRV_SOK) {
		//Отправка подтвеждения выполения команды
		uartSendData (UART_CHANNEL_2, &ackCommandOk, 1);
		return COMMAND_SOK;
	}
	else {
		// Отправка ошибки
		uartSendData (UART_CHANNEL_2, errMake, 2);
		return COMMAND_ERROR;
	}
 }
