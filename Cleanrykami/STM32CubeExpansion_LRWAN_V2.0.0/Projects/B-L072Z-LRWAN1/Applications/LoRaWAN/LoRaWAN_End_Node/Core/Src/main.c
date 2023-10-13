/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "app_lorawan.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "bmp280.h"
#include "stm32_timer.h"
//#include "epromm.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define BUFFER_SIZE
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
LPTIM_HandleTypeDef hlptim1;
/*RTC_HandleTypeDef hrtc;
UART_HandleTypeDef husart2;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
BMP280_HandleTypedef bmp280;
float pressure, temperature, humidity;
uint16_t size;
uint8_t Data[256];
*/
//uint8_t transmitBuffer[BUFFER_SIZE];
//SPI_HandleTypeDef hspi2;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
//static UTIL_TIMER_Object_t SleepTimer; // vkr
/*static void MX_RTC_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);

*/
//static void MX_SPI2_Init(void);
/*void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if (hspi->Instance == SPI2)
  {
    // Передача завершена
  }
}*/
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_LoRaWAN_Init();
  /* USER CODE BEGIN 2 */
//  FLASHEx_EEPROM_WRITE(STM32L072_EEPROM_START_ADDR, dataeprom, sizeepr);
  /* bmp280_init_default_params(&bmp280.params);
      	bmp280.addr = BMP280_I2C_ADDRESS_0;
      	bmp280.i2c = &hi2c1;
      	bmp280_init(&bmp280, &bmp280.params);
      	   			size = sprintf((char *)Data, "BMP280 initialization failed\n");
      	   			 HAL_UART_Transmit(&husart2, Data, size, 1000);
      	   			HAL_Delay(200);

      	   		bool bme280p = bmp280.id == BME280_CHIP_ID;
      	   		size = sprintf((char *)Data, "BMP280: found %s\n", bme280p ? "BME280" : "BMP280");
      	 	HAL_UART_Transmit(&husart2, Data, size, 1000);
      	 	*/
  //      uint16_t eepromBaseAddress = 0x08080000; // Пример адреса EEPROM
  	  #define eepromBaseAddress 0x08080000UL
        uint8_t dataToWrite[] = {0x01, 0x02, 0x03}; // Пример данных для записи
        uint8_t dataToWrite1 = 10; // Пример данных для записи
        uint8_t dataSize = sizeof(dataToWrite1); // Размер данных
//        dataeprom = dataToWrite;
//        FLASHEx_EEPROM_WRITE(STM32L072_EEPROM_START_ADDR, dataToWrite, dataSize);
  //      HAL_Delay(1000);
//        EEPROM_CLEAR();
//        EEPROM_WRITE_DATA(STM32L072_EEPROM_START_ADDR, dataToWrite1, dataSize);
  //    FLASHEx_EEPROM_WRITE(0, params->DownlinkCounter, sizeof(params->DownlinkCounter));
  //      dataeprom = dataToWrite;
//        EEPROM_CLEAR();      // возможный вариант
//        EEPROM_WRITE_DATA(eepromBaseAddress, dataToWrite1, dataSize); // возможный вариант
        uint8_t datareadepr = 0; // Прочитанные данные
//        EEPROM_ReadData();
        uint8_t datareadeprerror = 2; // Буфер ошибки
//        EEPROM_Read_Data(eepromBaseAddress, datareadepr, dataSize); //  возможный вариант
//        datareadepr =  EEPROM_ReadData();
//        datareadepr = EEPROM_Read_Data();
//        memcmp(datareadepr,dataToWrite);
         if (datareadepr == dataToWrite1)
        {
        	datareadeprerror = 100;
        }
        else {
            // Прочитанные данные не соответствуют записанным данным
            // Выполняйте необходимые действия здесь
//               printf("Ошибка при записи или чтении данных из EEPROM\n");
            datareadeprerror =  0;
        }
//        int isEqual = memcmp(dataToWrite1, datareadepr, dataSize);
//        if (isEqual == 0) {
//               // Прочитанные данные равны записанным данным
//               // Выполняйте необходимые действия здесь
////               printf("Данные успешно записаны и прочитаны из EEPROM\n");
//              datareadeprerror ++;
//           } else {
//               // Прочитанные данные не соответствуют записанным данным
//               // Выполняйте необходимые действия здесь
////               printf("Ошибка при записи или чтении данных из EEPROM\n");
//               datareadeprerror --;


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
	  // HAL_LPTIM_Counter_Start_IT(&hlptim1,1000);
	  // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	  // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    MX_LoRaWAN_Process();

    /* USER CODE BEGIN 3 */
//    EEPROM_WRITE_DATA(eepromBaseAddress, dataToWrite1, dataSize);
    // HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    //  HAL_PWR_EnterSTANDBYMode();
    // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); // vkr
        // UTIL_TIMER_SetPeriod(&SleepTimer, 1000); // vkr
        // UTIL_TIMER_Start(&SleepTimer) */
        /* HAL_Delay(100);
   		  bmp280_read_float(&bmp280, &temperature, &pressure, &humidity);
   		  // bmp280_read_float(&bmp280, &temperature, &pressure, &humidity);
   		  //dev

   		  size = sprintf((char *)Data,
   		  					"%d:%d:%d\n", (int)temperature,(int)pressure,(int)humidity);

   		  HAL_UART_Transmit(&husart2, Data, size, 1000);
   		  			HAL_Delay(200);
   		  			*/
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) //NEW С МЭЙНА
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
  /* Enable HSE Oscillator and Activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSEState            = RCC_HSE_OFF;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;// n
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL          = RCC_PLLMUL_6;
  RCC_OscInitStruct.PLL.PLLDIV          = RCC_PLLDIV_3;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Set Voltage scale1 as MCU will run at 32MHz */
__HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
 while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  clocks dividers */
 RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
		 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RTC
                              |RCC_PERIPHCLK_LPTIM1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  PeriphClkInit.LptimClockSelection = RCC_LPTIM1CLKSOURCE_PCLK;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
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
  while (1)
  {
  }
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
