/**
  ******************************************************************************
  * @file    lora_app.c
  * @author  MCD Application Team
  * @brief   Application of the LRWAN Middleware
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

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "Region.h" /* Needed for LORAWAN_DEFAULT_DATA_RATE */
#include "sys_app.h"
#include "lora_app.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "utilities_def.h"
#include "lora_app_version.h"
#include "lorawan_version.h"
#include "subghz_phy_version.h"
#include "lora_info.h"
#include "LmHandler.h"
#include "stm32_lpm.h"
#include "adc_if.h"
#include "adc.h" // для заряда батареи, но оно в _if
#include "sys_conf.h"
#include "sys_app.h" // для заряда батареи, но оно в _if
#include "CayenneLpp.h"
#include "sys_sensors.h"
#include "stdio.h"
#include "stm32l0xx_hal_lptim.h"
//#include "svoe.h"
//#include "persapp.h"
#include "bmp280.h"//
#include "hts221.h"
#include "rtc.h"
#include "rtc_if.h"
//#include "SVN посмотри"
//#include "SVN посмотри ЕЩЁ"
//#include "BMP280/bmp280.c" dfhkdlzfhdo;fjh;jadpjhzrbyadrnysr
// #include "Counter.c"
// #include "Counter.h"
/* USER CODE BEGIN Includes */
 #include "Counter.h"/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/**
  * @brief LoRa State Machine states
  */
typedef enum TxEventType_e
{
  /**
    * @brief AppdataTransmition issue based on timer every TxDutyCycleTime
    */
  TX_ON_TIMER,
  /**
    * @brief AppdataTransmition external event plugged on OnSendEvent( )
    */
  TX_ON_EVENT
  /* USER CODE BEGIN TxEventType_t */

  /* USER CODE END TxEventType_t */
} TxEventType_t;

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  LoRa endNode send request
  * @param  none
  * @retval none
  */

static void MX_LPTIM1_Init(void);// new

static void SendTxData(void);

/**
  * @brief  TX timer callback function
  * @param  timer context
  * @retval none
  */
static void OnTxTimerEvent(void *context);

/**
  * @brief  LED Tx timer callback function
  * @param  LED context
  * @retval none
  */
static void OnTxTimerLedEvent(void *context);

/**
  * @brief  LED Rx timer callback function
  * @param  LED context
  * @retval none
  */
static void OnRxTimerLedEvent(void *context);

/**
  * @brief  LED Join timer callback function
  * @param  LED context
  * @retval none
  */
static void OnJoinTimerLedEvent(void *context);

/**
  * @brief  join event callback function
  * @param  params
  * @retval none
  */
static void OnJoinRequest(LmHandlerJoinParams_t *joinParams);

/**
  * @brief  tx event callback function
  * @param  params
  * @retval none
  */
static void OnTxData(LmHandlerTxParams_t *params);

/**
  * @brief callback when LoRa endNode has received a frame
  * @param appData
  * @param params
  * @retval None
  */
static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params);

/*!
 * Will be called each time a Radio IRQ is handled by the MAC layer
 *
 */
static void OnMacProcessNotify(void);

/* USER CODE BEGIN PFP */
//static void MX_I2C1_Init(void);

/* USER CODE END PFP */
//static void MX_USART2_UART_Init(void);

//static void I2C1_MspInit(void);
/* Private variables ---------------------------------------------------------*/
/**
  * @brief User application buffer
  */
//static void MX_I2C1_Init(void);
//static void I2C1_MspInit(void);
static uint8_t AppDataBuffer[LORAWAN_APP_DATA_BUFFER_MAX_SIZE];

/**
  * @brief User application data structure
  */
static LmHandlerAppData_t AppData = { 0, 0, AppDataBuffer };

static ActivationType_t ActivationType = LORAWAN_DEFAULT_ACTIVATION_TYPE;

/**
  * @brief LoRaWAN handler Callbacks
  */
static LmHandlerCallbacks_t LmHandlerCallbacks =
{
  .GetBatteryLevel =           GetBatteryLevel,
  .GetTemperature =            GetTemperatureLevel,
  .OnMacProcess =              OnMacProcessNotify,
  .OnJoinRequest =             OnJoinRequest,
  .OnTxData =                  OnTxData,
  .OnRxData =                  OnRxData
};

/**
  * @brief LoRaWAN handler parameters
  */
static LmHandlerParams_t LmHandlerParams =
{
  .ActiveRegion =             ACTIVE_REGION,
  .DefaultClass =             LORAWAN_DEFAULT_CLASS,
  .AdrEnable =                LORAWAN_ADR_STATE,
  .TxDatarate =               LORAWAN_DEFAULT_DATA_RATE,
  .PingPeriodicity =          LORAWAN_DEFAULT_PING_SLOT_PERIODICITY
};

/**
  * @brief Specifies the state of the application LED
  */
static uint8_t AppLedStateOn = RESET;

/**
  * @brief Type of Event to generate application Tx
  */
static TxEventType_t EventType = TX_ON_TIMER;

/**
  * @brief Timer to handle the application Tx
  */
static UTIL_TIMER_Object_t TxTimer;

/**
  * @brief Timer to handle the application Tx Led to toggle
  */
static UTIL_TIMER_Object_t TxLedTimer;

/**
  * @brief Timer to handle the application Rx Led to toggle
  */
static UTIL_TIMER_Object_t RxLedTimer;

/**
  * @brief Timer to handle the application Join Led to toggle
  */
static UTIL_TIMER_Object_t JoinLedTimer;

//BMP280_HandleTypedef bmp280;
BMP280_HandleTypedef bmp280;
//UART_HandleTypeDef huart2;
RTC_HandleTypeDef hrtc;
//UART_HandleTypeDef husart2;
UART_HandleTypeDef husart1;
I2C_HandleTypeDef hi2c1;
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2; ///// Для HTS если вдруг понадобится
LPTIM_HandleTypeDef hlptim1;
union hts221_reg_t;
HTS221_Object_t hts221;
HTS221_Capabilities_t Capabilities;
ADC_HandleTypeDef hadc1;
/* USER CODE END PV */

/* Exported functions ---------------------------------------------------------*/
/* USER CODE BEGIN EF */
// MX_I2C1_Init();
//MX_I2C1_Init(hi2c1);
//MX_USART2_Init();
/* USER CODE END EF */

void LoRaWAN_Init(void)
{
	// UTIL_TIMER_Create(&RunLedTimer, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnJoinTimerLedEvent, NULL); //
	// UTIL_TIMER_SetPeriod(&RunLedTimer, 500); //
  /* USER CODE BEGIN LoRaWAN_Init_1 */
	/* bmp280_init_default_params(&bmp280.params);
	   	bmp280.addr = BMP280_I2C_ADDRESS_0;
	   	bmp280.i2c = &hi2c1;
*/
	   //float pressure1, temperature1, humidity1;
  /* USER CODE END LoRaWAN_Init_1 */
  LED_Init(LED_BLUE);
  LED_Init(LED_RED1);
  LED_Init(LED_RED2);

  /* Get LoRa APP version*/
//  APP_LOG(TS_OFF, VLEVEL_M, "APP_VERSION:        V%X.%X.%X\r\n",
//          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_MAIN_SHIFT),
//          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB1_SHIFT),
//          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB2_SHIFT));
  APP_LOG(TS_OFF, VLEVEL_M, " Версия протокола:        V%X.%X.%X\r\n",
          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_MAIN_SHIFT),
          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB1_SHIFT),
          (uint8_t)(__LORA_APP_VERSION >> __APP_VERSION_SUB2_SHIFT)); // версия приложения (App)
  /* Get MW LoraWAN info */
//  APP_LOG(TS_OFF, VLEVEL_M, "MW_LORAWAN_VERSION: V%X.%X.%X\r\n",
//          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_MAIN_SHIFT),
//          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB1_SHIFT),
//          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB2_SHIFT));
//  APP_LOG(TS_OFF, VLEVEL_M, "MW_LORAWAN_VERSION: V%X.%X.%X\r\n",
//          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_MAIN_SHIFT),
//          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB1_SHIFT),
//          (uint8_t)(__LORAWAN_VERSION >> __APP_VERSION_SUB2_SHIFT)); // версию промежуточного программного обеспечения (Middleware)
  /* Get MW SubGhz_Phy info */
//  APP_LOG(TS_OFF, VLEVEL_M, "MW_RADIO_VERSION:   V%X.%X.%X\r\n",
//          (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_MAIN_SHIFT),
//          (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB1_SHIFT),
//          (uint8_t)(__SUBGHZ_PHY_VERSION >> __APP_VERSION_SUB2_SHIFT)); // версия радио Middleware
  // APP_LOG(TS_OFF, VLEVEL_M, "MW_:   V%X.%X.%X\r\n",
//  APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ==== EEEPROM EST====\r\n");
  //	 (uint8_t) buffer[20]);
  UTIL_TIMER_Create(&TxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerLedEvent, NULL);
  // HAL_LPTIM_Counter_Start_IT(&hlptim,1000);
 // HAL_LPTIM_TimeOut_Start(&TxLedTimer, uint32_t Period, uint32_t Timeout);
  //  HAL_LPTIM_TimeOut_Start(&TxLedTimer,5000,1000);
   UTIL_TIMER_Create(&RxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnRxTimerLedEvent, NULL);
  UTIL_TIMER_Create(&JoinLedTimer, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnJoinTimerLedEvent, NULL);
  UTIL_TIMER_SetPeriod(&TxLedTimer, 500);// 3 стр отрубают светодиоды если их не будет,
  UTIL_TIMER_SetPeriod(&RxLedTimer, 500);
  UTIL_TIMER_SetPeriod(&JoinLedTimer, 500);
 // bmp280_read_float(&bmp280, &temperature1, &pressure1, &humidity1);
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LmHandlerProcess), UTIL_SEQ_RFU, LmHandlerProcess);  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), UTIL_SEQ_RFU, SendTxData);
  /* Init Info table used by LmHandler*/
  LoraInfo_Init();
  /* Init the Lora Stack*/
  LmHandlerInit(&LmHandlerCallbacks);

  LmHandlerConfigure(&LmHandlerParams);

  UTIL_TIMER_Start(&JoinLedTimer);
  // HAL_LPTIM_Counter_Start_IT(&JoinLedTimer,1000); // попытка переделать светодиодом под LPTIM
  LmHandlerJoin(ActivationType);

  if (EventType == TX_ON_TIMER)
  {
    /* send every time timer elapses */

    UTIL_TIMER_Create(&TxTimer,  0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnTxTimerEvent, NULL);
    UTIL_TIMER_SetPeriod(&TxTimer,  APP_TX_DUTYCYCLE);
    //  HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    //  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    //HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI); // NEW
      UTIL_TIMER_Start(&TxTimer);

    // UTIL_TIMER_Stop(&TxTimer); // такой же трай для одиночного сообщения
    //  HAL_LPTIM_Counter_Start_IT(&TxTimer,1000);
  }
  else
  {
    /* send every time button is pushed */
    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);
  }
  /* USER CODE BEGIN LoRaWAN_Init_Last */
 // MX_USART2_Init();
 // MX_I2C1_Init();
  /* bmp280_init_default_params(&bmp280.params);
   	bmp280.addr = BMP280_I2C_ADDRESS_0;
   	bmp280.i2c = &hi2c1;
   	*/
  /* USER CODE END LoRaWAN_Init_Last */
}
/*void LoRaWANWD_Init(void)
{

  /* USER CODE BEGIN LoRaWAN_Init_1 */
	/* bmp280_init_default_params(&bmp280.params);
	   	bmp280.addr = BMP280_I2C_ADDRESS_0;

	   	bmp280.i2c = &hi2c1;gjk
*/
	//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	   //float pressure1, temperature1, humidity1; ЗДЕСЬ РОДИТСЯ СТОРОЖ? а может и не надо
  /* USER CODE END LoRaWAN_Init_1 */

  /* Get LoRa APP version*/


  /* Get MW LoraWAN info */


  /* Get MW SubGhz_Phy info */

  /* USER CODE BEGIN LoRaWAN_Init_Last */
 // MX_USART2_Init();
 // MX_I2C1_Init();
  /* bmp280_init_default_params(&bmp280.params);
   	bmp280.addr = BMP280_I2C_ADDRESS_0;
   	bmp280.i2c = &hi2c1;
   	*/
  /* USER CODE END LoRaWAN_Init_Last */
//}

void BSP_PB_Callback(Button_TypeDef Button)
{
  /* USER CODE BEGIN BSP_PB_Callback_1 */

  /* USER CODE END BSP_PB_Callback_1 */
  switch (Button)
  {
    case  BUTTON_USER:
      UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
      /* USER CODE BEGIN PB_Callback 1 */
      /* USER CODE END PB_Callback 1 */
      break;
    default:
      break;
  }
  /* USER CODE BEGIN BSP_PB_Callback_Last */

  /* USER CODE END BSP_PB_Callback_Last */
}

/* Private functions ---------------------------------------------------------*/
/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

static void OnRxData(LmHandlerAppData_t *appData, LmHandlerRxParams_t *params)
{
  /* USER CODE BEGIN OnRxData_1 */

  /* USER CODE END OnRxData_1 */
  if ((appData != NULL) && (params != NULL))
  {
    LED_On(LED_BLUE);
    static const char *slotStrings[] = { "1", "2", "C", "C Multicast", "B Ping-Slot", "B Multicast Ping-Slot" };

//    APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ========== MCPS-Indication ==========\r\n");
    APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ========== Индикация приёма данных ==========\r\n");
    APP_LOG(TS_OFF, VLEVEL_H, "###### D/L FRAME:%04d | SLOT:%s | PORT:%d | DR:%d | RSSI:%d | SNR:%d\r\n",
            params->DownlinkCounter, slotStrings[params->RxSlot], appData->Port, params->Datarate, params->Rssi, params->Snr);
    switch (appData->Port)
    {
      case LORAWAN_SWITCH_CLASS_PORT:
        /*this port switches the class*/
        if (appData->BufferSize == 1)
        {
          switch (appData->Buffer[0])
          {
            case 0:
            {
              LmHandlerRequestClass(CLASS_A);
              break;
            }
            case 1:
            {
              LmHandlerRequestClass(CLASS_B);
              break;
            }
            case 2:
            {
              LmHandlerRequestClass(CLASS_C);
              break;
            }
            default:
              break;
          }
        }
        break;
      case LORAWAN_USER_APP_PORT:
        if (appData->BufferSize == 1)
        {
          AppLedStateOn = appData->Buffer[0] & 0x01;
          if (AppLedStateOn == RESET)
          {
            APP_LOG(TS_OFF, VLEVEL_H,   "LED OFF\r\n");

            LED_Off(LED_RED1);
          }
          else
          {
            APP_LOG(TS_OFF, VLEVEL_H, "LED ON\r\n");

            LED_On(LED_RED1);
          }
        }
        break;
      /* USER CODE BEGIN OnRxData_Switch_case */

      /* USER CODE END OnRxData_Switch_case */
      default:
        /* USER CODE BEGIN OnRxData_Switch_default */

        /* USER CODE END OnRxData_Switch_default */
        break;
    }
  }

  /* USER CODE BEGIN OnRxData_2 */

  /* USER CODE END OnRxData_2 */
}

static void SendTxData(void)
{
	// uint8_t payload = "textovik";
	 uint16_t pressure = 0;
	 int16_t temperature = 0;
	 // uint16_t humidity = 0;
  	float pressure1, temperature1, humidity1, numberdev, per, time; // номер устройста и период добавил
  	//uint16_t pressure = 0;
  	//int16_t temperature = 0;
  	uint16_t Defbuff;  // для счётчика с охранной кнопки
  sensor_t sensor_data;
  uint16_t size;
  uint8_t Data[256];
  UTIL_TIMER_Time_t nextTxIn = 0;
  bmp280_params_t params;
	RTC_TimeTypeDef sTime1;
  	RTC_DateTypeDef sDate1;
  	uint8_t buffer[20];
  	volatile uint16_t adc = 0;
#ifdef CAYENNE_LPP
  uint8_t channel = 0;
#else
   uint16_t humidity = 0;
  uint32_t i = 0;
  int32_t latitude = 0;
  int32_t longitude = 0;
  uint16_t altitudeGps = 0;
#endif /* CAYENNE_LPP */
  /* USER CODE BEGIN SendTxData_1 */

  /* USER CODE END SendTxData_1 */

  /* EnvSensors_Read(&sensor_data);
  #if defined (SENSOR_ENABLED) && (SENSOR_ENABLED == 1)

    temperature = (int16_t) sensor_data.temperature;

  #else
   temperature = (SYS_GetTemperatureLevel() >> 8);
  #endif  /* SENSOR_ENABLED */
  /*  pressure    = (uint16_t)(sensor_data.pressure * 100 / 10);      /* in hPa / 10 */

    AppData.Port = LORAWAN_USER_APP_PORT;
    /*
#ifdef CAYENNE_LPP
  CayenneLppReset();
  CayenneLppAddBarometricPressure(channel++, pressure);
  CayenneLppAddTemperature(channel++, temperature);
  CayenneLppAddRelativeHumidity(channel++, (uint16_t)(sensor_data.humidity));

  if ((LmHandlerParams.ActiveRegion != LORAMAC_REGION_US915) && (LmHandlerParams.ActiveRegion != LORAMAC_REGION_AU915)
      && (LmHandlerParams.ActiveRegion != LORAMAC_REGION_AS923))
  {
    CayenneLppAddDigitalInput(channel++, GetBatteryLevel());
    CayenneLppAddDigitalOutput(channel++, AppLedStateOn);
  }

  CayenneLppCopy(AppData.Buffer);
  AppData.BufferSize = CayenneLppGetSize();
#else  /* not CAYENNE_LPP */
    /*  humidity    = (uint16_t)(sensor_data.humidity * 10);            /* in %*10     */

  // AppData.Buffer[i++] = AppLedStateOn; //////////////////////////////// ДАЁТ 00 В НАЧАЛЕ
    /*  AppData.Buffer[i++] = (uint8_t)((pressure >> 8) & 0xFF);
  AppData.Buffer[i++] = (uint8_t)(pressure & 0xFF);
  AppData.Buffer[i++] = (uint8_t)(temperature & 0xFF);
  AppData.Buffer[i++] = (uint8_t)((humidity >> 8) & 0xFF);
  AppData.Buffer[i++] = (uint8_t)(humidity & 0xFF);

  //AppData.Buffer[i++] = (uint8_t)(temperature & 0xFF);
   */
  // payload == "text";
  // AppData.Buffer[i++] = payload;
  // BMP280_HandleTypedef bmp280;
  // LmHandlerGetDevAddr = i;
// HAL_I2C_GetState(&hi2c1);
 // HAL_I2C_Master_Receive(&bmp280);
	 // I2C_DMAMasterReceiveCplt(DMA_HandleTypeDef *hdma)
		//HAL_I2C_GetState;
  // __B_L072Z_LRWAN1_BSP_VERSION_MAIN = i;
  // bmp280_init_default_params(&bmp280.params);
  //  bmp280.addr = BMP280_I2C_ADDRESS_0;
  //
  // MX_I2C1_Init();
 // MX_USART2_Init();
  // MX_I2C1_Init();
  // BSP_SPI1_Init();
  BSP_I2C1_Init();
  MX_ADC_Init();
  MX_LPTIM1_Init();

  HAL_LPTIM_Encoder_Start_IT(&hlptim1, 1000);
  // MX_USART1_Init();
  //MX_LPTIM1_Init();
  // calendarValue;
  // HAL_RTC_GetDate(&hrtc, RTC_DateStruct, RTC_FORMAT_BIN);
  //    HAL_RTC_GetTime();
  /*   MX_RTC_Init();
      UTIL_TIMER_GetCurrentTime();
        AppData.Buffer[i++] = UTIL_TIMER_GetCurrentTime; // дата и время
      HAL_RTC_GetTime(&hrtc, &sTime1, RTC_FORMAT_BCD); //
      HAL_RTC_GetDate(&hrtc, &sDate1, RTC_FORMAT_BCD); //
       buffer[0] = (sDate1.Date / 16) + 48; //
      		buffer[1] = (sDate1.Date % 16) + 48; //
    		buffer[2] = '.';
    		buffer[3] = (sDate1.Month / 16) + 48;
    		buffer[4] = (sDate1.Month % 16) + 48;
    		buffer[5] = '.';
    		buffer[6] = '2';
    		buffer[7] = '0';
    		buffer[8] = (sDate1.Year / 16) + 48;
    		buffer[9] = (sDate1.Year % 16) + 48;
    		buffer[10] ='@';
    		buffer[11] = (sTime1.Hours / 16) + 48;
    		buffer[12] = (sTime1.Hours % 16) + 48;
    		buffer[13] = ':';
    		buffer[14] = (sTime1.Minutes / 16) + 48;
    		buffer[15] = (sTime1.Minutes % 16) + 48;
    		buffer[16] = ':';
    		buffer[17] = (sTime1.Seconds / 16) + 48;
    		buffer[18] = (sTime1.Seconds % 16) + 48;
    		 */
  // MX_I2C1_Init();
 // HAL_I2C_GetState;// cntGetValue(&Defbuff);  // здесь будет сбор значения вскрытий// HTS221_Init(pObj);//HTS221_GetCapabilities(pObj, Capabilities);  //  HAL_LPTIM_Counter_Start_IT(&hlptim1,1000);
  // HAL_LPTIM_Counter_Start_IT(&hlptim1,1000);
    //	  HAL_Delay(200);  //	UTIL_TIMER_Create(&ReadLedTimer, 0xFFFFFFFFU, UTIL_TIMER_PERIODIC, OnJoinTimerLedEvent, NULL);  // UTIL_TIMER_SetPeriod(&ReadLedTimer, 5000);  	//UTIL_TIMER_Create(&RxLedTimer, 0xFFFFFFFFU, UTIL_TIMER_ONESHOT, OnRxTimerLedEvent, NULL);
  bmp280_init_default_params(&bmp280.params);
    	bmp280.addr = BMP280_I2C_ADDRESS_0;
    	bmp280.i2c = &hi2c1;
    	bmp280_init(&bmp280, &bmp280.params);
    	 //size = sprintf((char *)Data, "BMP280 initialization failed\n");
    	 // HAL_UART_Transmit(&husart2, Data, size, 1000);
    	 //HAL_Delay(200);
    	//		bool bme280p = bmp280.id == BME280_CHIP_ID;
//    	   	 	size = sprintf((char *)Data, "BMP280: found %s\n", bme280p ? "BME280" : "BMP280");
    	// 	HAL_UART_Transmit(&husart2, Data, size, 1000);
    	// APP_LOG(TS_OFF, VLEVEL_M, "MW_:   V%X.%X.%X\r\n",
    	//		 (uint8_t) buffer[20]);
    	/*	while (!bmp280_init(&bmp280, &bmp280.params)) {
    			size = sprintf((char *)Data, "BMP280 initialization failed\n");
    			 HAL_UART_Transmit(&huart2, Data, size, 1000);
    			HAL_Delay(200);
    		}
        */
    	// HTS221_Init(&hts221);
    	 // HTS221_GetCapabilities(&hts221, temperature);
  // bmp280.i2c = &hi2c1;
   // bmp280_read_float(&bmp280, &temperature1, &pressure1, &humidity1);
  //  bmp280_read_float(&bmp280, &temperature1, &pressure1, &humidity1); старое
    	// HAL_Delay(100);
    	 bmp280_read_float(&bmp280, &temperature1, &pressure1, &humidity1);

    	 if(bmp280.dig_T1>=0)
    	//*/
    	{
//    	  APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ==== Temp, hum sensor Init ====\r\n");
    	  APP_LOG(TS_OFF, VLEVEL_M, "\r\n ==== Датчик измерения активен ====\r\n");
    	} //*/
    	 else
    	 {

//    		 APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ==== Sensor initialization failed ====\r\n");
    		 APP_LOG(TS_OFF, VLEVEL_M, "\r\n ==== Ошибка инициализации датчика  ====\r\n");
    	 }

    	 if(temperature1>=0 && temperature1<=30 && pressure1>=0)
    	     	//*/
    	  {
//    	     	  APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ==== All parameters normal ====\r\n");
    	     	  APP_LOG(TS_OFF, VLEVEL_M, "\r\n ==== Параметры окружающей среды в норме ====\r\n");
    	  } //*/
    	     	 else
    	   {
//    	     		 APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ==== Accident ====\r\n");
    	       		 APP_LOG(TS_OFF, VLEVEL_M, "\r\n ==== Отклонение от нормы ====\r\n");
    	   }
    	 numberdev = 0x01;  // new 0b00010001
    	 per = 0xA; // new


  // bool bme280p = bmp280.id == BME280_CHIP_ID;
 // size = (temperature1, pressure1, humidity1);
//    size = (temperature1, pressure1, humidity1);
//    size = sprintf((char *)Data, "Sensor initialization failed\n");

    //    size = (Capabilities);
    // size = sprintf((char *)Data,
    //	  					"%d:%d:%d\n", (int)temperature,(int)pressure,(int)humidity);
    // printf("IDDDDDDDDDDD 2\n\r");
    //snprintf("IDDDDDDDDDDD 2\n\r");
    // vprintf(size);
    // putchar("IDDDDDDDDDDD 2\n\r");
    // HAL_ADC_Start(&hadc1); // запускаем преобразование сигнала АЦП
    // adc = HAL_ADC_GetValue(&hadc1); // читаем полученное значение в переменную adc
    //	HAL_ADC_Stop(&hadc1); // останавливаем АЦП (не обязательно)
    //   HAL_UART_Transmit(&husart1, Data, size, 1000);
    // HAL_UART_Transmit(&husart1, Data, size, 1000);'
    // HAL_UART_Transmit_DMA(&husart1, Daata, size, 1000);
    // HAL_UART_Transmit_DMA(&husart1, Daata, size, 1000);
    //   HAL_Delay(200);

  // HAL_I2C_Master_Receive(&hi2c1, size);
  //  bmp280_read_float(&bme280p, &temperature1, &pressure1, &humidity1);
    //dev
 // LmHandlerAppData_t *appData, LmHandlerRxParams_t *params):
  // for i = setbuf(AppData.Buffer, 51);
  // AppData.Buffer[i++] = temperature1;// 1  варик
  // AppData.Buffer[i++] = ((pressure1 >> 8) & 0xFF);
  // AppData.Buffer[i++] = 0x02;
  // for (i = 0; i <= 50; i++)  ||Cчётчик
  // pressure1 = (uint16_t)(params.oversampling_pressure * 10);

    //    AppData.Buffer[i++] = ((pressure1 >> 8) & 0xFF);
    // humidity1 = (uint16_t)(params.oversampling_humidity * 10);
    // humidity1 = (uint16_t)(bmp280_params_t).oversampling_humidity * 10;
    // humidity1 = (uint16_t)(params.oversampling_humidity * 10);
    // pressure1 = (uint16_t)(params.oversampling_pressure * 10);
    //temperature1 = (uint16_t)(params.oversampling_temperature * 10);
  // temperature1 = 0x02;
  // AppData.Buffer[i++] = (uint16_t)pressure1 >> 1 ;
  // AppData.Buffer[i++] = (uint8_t)temperature1 >> 8 ;
  //  AppData.Buffer[i++] = (int)humidity1 >> 16;


    //AppData.Buffer[i++] = 0x00;
     AppData.Buffer[i++] = numberdev; // new номер устройства
     AppData.Buffer[i++] = temperature1;
     AppData.Buffer[i++] = humidity1;
     AppData.Buffer[i++] = per; // new период включения прибора
     // AppData.Buffer[i++] = BAT_CR2032; // new заряд батареи, CR2032 старая батарейка, новая LS14500
  // SYS_GetBatteryLevel(); // new заряд батареи
  GetBatteryLevel(); // new заряд батареи
  // AppData.Buffer[i++] = batteryLevel; // new заряд батареи
     AppData.Buffer[i++] = GetBatteryLevel();
  // AppData.Buffer[i++] = Capabilities;
  //AppData.Buffer[i++] = SYS_GetTemperatureLevel();
  // AppData.Buffer[i++] = HAL_ADC_GetState; //  09 что значит
  // AppData.Buffer[i++] = size;
  // AppData.Buffer[i] = Snr;
  // AppData.Buffer[i++] = 0x02;
  //  AppData.Buffer[i++] = cnt; // new период включения прибора
  /* */   // AppData.Buffer[i++] = Capabilities;
  // AppData.Buffer[i] = i;  ||Cчётчик

  // AppData.Buffer[i++] = '\0';

  // HAL_LPTIM_Counter_Start_IT(&hlptim1,10000);
  //HAL_LPTIM_Counter_Start_IT(&hlptim1,1000);

  // HAL_LPTIM_Encoder_Start(&hlptim1,1000); не считает не в какую сторону

 // AppData.Buffer[i++] = (uint8_t)((humidity >> 8) & 0xFF);
 // AppData.Buffer[i++] = (uint8_t)(humidity & 0xFF);

  /*     if ((LmHandlerParams.ActiveRegion == LORAMAC_REGION_US915) || (LmHandlerParams.ActiveRegion == LORAMAC_REGION_AU915)
      || (LmHandlerParams.ActiveRegion == LORAMAC_REGION_AS923))
  {
    AppData.Buffer[i++] = 0;
    AppData.Buffer[i++] = 0;
    AppData.Buffer[i++] = 0;
    AppData.Buffer[i++] = 0;
  }
  else
  {
    latitude = sensor_data.latitude;
    longitude = sensor_data.longitude;

    AppData.Buffer[i++] = GetBatteryLevel();        // 1 (very low) to 254 (fully charged)
    AppData.Buffer[i++] = (uint8_t)((latitude >> 16) & 0xFF);
    AppData.Buffer[i++] = (uint8_t)((latitude >> 8) & 0xFF);
    AppData.Buffer[i++] = (uint8_t)(latitude & 0xFF);
    AppData.Buffer[i++] = (uint8_t)((longitude >> 16) & 0xFF);
    AppData.Buffer[i++] = (uint8_t)((longitude >> 8) & 0xFF);
    AppData.Buffer[i++] = (uint8_t)(longitude & 0xFF);
    AppData.Buffer[i++] = (uint8_t)((altitudeGps >> 8) & 0xFF);
    AppData.Buffer[i++] = (uint8_t)(altitudeGps & 0xFF);

	   }
*/
  AppData.BufferSize = i;
  /*  #endif /* CAYENNE_LPP */

   if (LORAMAC_HANDLER_SUCCESS == LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, &nextTxIn, false))
  {
	  APP_LOG(TS_ON, VLEVEL_L, "Запрос на подключение отправлен\r\n");
//    APP_LOG(TS_ON, VLEVEL_L, "SEND REQUEST\r\n");
  }

  else if (nextTxIn > 0)
  {
//    APP_LOG(TS_ON, VLEVEL_L, "Next Tx in  : ~%d second(s)\r\n", (nextTxIn / 1000));
    APP_LOG(TS_ON, VLEVEL_L, "Следующий запрос на отправку через : ~%d секунды\r\n", (nextTxIn / 1000));
  }
  /* USER CODE BEGIN SendTxData_2 */
  // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  /* USER CODE END SendTxData_2 */
}

static void OnTxTimerEvent(void *context)
{
  /* USER CODE BEGIN OnTxTimerEvent_1 */

  /* USER CODE END OnTxTimerEvent_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LoRaSendOnTxTimerOrButtonEvent), CFG_SEQ_Prio_0);
 // HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
  /*Wait for next tx slot*/
  UTIL_TIMER_Start(&TxTimer);
 // UTIL_TIMER_Stop(&TxTimer); // попытка передать только одно сообщение
//HAL_LPTIM_Counter_Start_IT(&hlptim1,1000);
  /* USER CODE BEGIN OnTxTimerEvent_2 */

  /* USER CODE END OnTxTimerEvent_2 */
}

static void OnTxTimerLedEvent(void *context)
{
  /* USER CODE BEGIN OnTxTimerLedEvent_1 */

  /* USER CODE END OnTxTimerLedEvent_1 */
  LED_Off(LED_RED2);
  /* USER CODE BEGIN OnTxTimerLedEvent_2 */

  /* USER CODE END OnTxTimerLedEvent_2 */
}

static void OnRxTimerLedEvent(void *context)
{
  /* USER CODE BEGIN OnRxTimerLedEvent_1 */

  /* USER CODE END OnRxTimerLedEvent_1 */
  LED_Off(LED_BLUE) ;
  /* USER CODE BEGIN OnRxTimerLedEvent_2 */

  /* USER CODE END OnRxTimerLedEvent_2 */
}

static void OnJoinTimerLedEvent(void *context)
{
  /* USER CODE BEGIN OnJoinTimerLedEvent_1 */

  /* USER CODE END OnJoinTimerLedEvent_1 */
  LED_Toggle(LED_RED1) ;
  /* USER CODE BEGIN OnJoinTimerLedEvent_2 */

  /* USER CODE END OnJoinTimerLedEvent_2 */
}

static void OnTxData(LmHandlerTxParams_t *params)
{
  /* USER CODE BEGIN OnTxData_1 */

  /* USER CODE END OnTxData_1 */
  if ((params != NULL) && (params->IsMcpsConfirm != 0))
  {
    LED_On(LED_RED2) ;
   // LED_Off(LED_RED2);

    UTIL_TIMER_Start(&TxLedTimer);
    //HAL_LPTIM_Counter_Start_IT(&TxTimer,1000);
//    APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### ========== MCPS-Confirmation =============\r\n");
    APP_LOG(TS_OFF, VLEVEL_M, "\r\n MCPS Подтверждение \r\n");
//    APP_LOG(TS_OFF, VLEVEL_M, " Передача информации :%d", params->AppData.Buffer);
//    APP_LOG(TS_OFF, VLEVEL_H, "###### U/L FRAME:%04d | PORT:%d | DR:%d | PWR:%d | Channel:%d", params->UplinkCounter,
//            params->AppData.Port, params->Datarate, params->TxPower,params->Channel);
    APP_LOG(TS_OFF, VLEVEL_M, " Номер посылки:%01d | Порт:%d | Канал скорости передачи данных:%d | Мощность:%d | Канал:%d | Передача информации :%x", params->UplinkCounter,
                params->AppData.Port, params->Datarate, params->TxPower,params->Channel, params->AppData.Buffer);
    APP_LOG(TS_OFF, VLEVEL_H, " | MSG TYPE:");
//    APP_LOG(TS_OFF, VLEVEL_M, " | c:");
    if (params->MsgType == LORAMAC_HANDLER_CONFIRMED_MSG)
    {
      APP_LOG(TS_OFF, VLEVEL_H, "\r\n CONFIRMED [%s]\r\n", (params->AckReceived != 0) ? "ACK" : "NACK");
    }
    else
    {
      APP_LOG(TS_OFF, VLEVEL_H, "\r\n UNCONFIRMED\r\n");
    }
  }

  /* USER CODE BEGIN OnTxData_2 */

  /* USER CODE END OnTxData_2 */
}

static void OnJoinRequest(LmHandlerJoinParams_t *joinParams)
{
  /* USER CODE BEGIN OnJoinRequest_1 */

  /* USER CODE END OnJoinRequest_1 */
  if (joinParams != NULL)
  {
    if (joinParams->Status == LORAMAC_HANDLER_SUCCESS)
    {
      UTIL_TIMER_Stop(&JoinLedTimer);

      LED_Off(LED_RED1) ;

//      APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOINED = ");
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n      Устройство подключено \r\n  ");
      //if (joinParams->Mode == ACTIVATION_TYPE_ABP) //
      if (joinParams->Mode == ACTIVATION_TYPE_OTAA)
      {
//        APP_LOG(TS_OFF, VLEVEL_M, "ABP ======================\r\n");
        APP_LOG(TS_OFF, VLEVEL_M, "  \r\n Активация по персонализации  \r\n");
      }
      else
      {
//        APP_LOG(TS_OFF, VLEVEL_M, "OTAA =====================\r\n");
        APP_LOG(TS_OFF, VLEVEL_M, "   Активация по воздуху  \r\n");
      }
    }
    else
    {
//      APP_LOG(TS_OFF, VLEVEL_M, "\r\n###### = JOIN FAILED\r\n");
      APP_LOG(TS_OFF, VLEVEL_M, "\r\n  Подключение к базовой станции не удалось \r\n");
    }
  }

  /* USER CODE BEGIN OnJoinRequest_2 */

  /* USER CODE END OnJoinRequest_2 */
}
/*void SystemClock_Config(void) //NEW С МЭЙНА
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0}; //NEW
  /* Enable HSE Oscillator and Activate PLL with HSE as source */
/* RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;// new добавил |RCC_OSCILLATORTYPE_LSI
  RCC_OscInitStruct.HSEState            = RCC_HSE_OFF;
  RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState 			= RCC_LSI_ON;	// new
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
/* __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Poll VOSF bit of in PWR_CSR. Wait until it is reset to 0 */
/*  while (__HAL_PWR_GET_FLAG(PWR_FLAG_VOS) != RESET) {};

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
  clocks dividers */
/*  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  // /*   new
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
                                |RCC_PERIPHCLK_I2C1|RCC_PERIPHCLK_RTC
                                |RCC_PERIPHCLK_LPTIM1;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
    PeriphClkInit.LptimClockSelection = RCC_LPTIM1CLKSOURCE_PCLK;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
    {
      Error_Handler();
    }
  }
  */
/*

 */static void MX_LPTIM1_Init(void) // new
{

  /* USER CODE BEGIN LPTIM1_Init 0 */

  /* USER CODE END LPTIM1_Init 0 */

  /* USER CODE BEGIN LPTIM1_Init 1 */

  /* USER CODE END LPTIM1_Init 1 */
	  hlptim1.Instance = LPTIM1;
	  hlptim1.Init.Clock.Source = LPTIM_CLOCKSOURCE_APBCLOCK_LPOSC;
	  hlptim1.Init.Clock.Prescaler = LPTIM_PRESCALER_DIV1;
	  hlptim1.Init.UltraLowPowerClock.Polarity = LPTIM_CLOCKPOLARITY_RISING;
	  hlptim1.Init.UltraLowPowerClock.SampleTime = LPTIM_CLOCKSAMPLETIME_DIRECTTRANSITION;
	  hlptim1.Init.Trigger.Source = LPTIM_TRIGSOURCE_SOFTWARE;
	  hlptim1.Init.OutputPolarity = LPTIM_OUTPUTPOLARITY_HIGH;
	  hlptim1.Init.UpdateMode = LPTIM_UPDATE_IMMEDIATE;
	  hlptim1.Init.CounterSource = LPTIM_COUNTERSOURCE_EXTERNAL;
  if (HAL_LPTIM_Init(&hlptim1) != HAL_OK)
  {
    Error_Handler();
  }
  /*
  /* USER CODE BEGIN LPTIM1_Init 2 */

  /* USER CODE END LPTIM1_Init 2 */

}
  void HAL_LPTIM_MspInit(LPTIM_HandleTypeDef* hlptim)
  {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(hlptim->Instance==LPTIM1)
    {
    /* USER CODE BEGIN LPTIM1_MspInit 0 */

    /* USER CODE END LPTIM1_MspInit 0 */
      /* Peripheral clock enable */
      __HAL_RCC_LPTIM1_CLK_ENABLE();

      __HAL_RCC_GPIOB_CLK_ENABLE();
      /**LPTIM1 GPIO Configuration
      PB5     ------> LPTIM1_IN1
      PB7     ------> LPTIM1_IN2
      */
      GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_7;
      GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
      GPIO_InitStruct.Alternate = GPIO_AF2_LPTIM1;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      /* LPTIM1 interrupt Init */
      HAL_NVIC_SetPriority(LPTIM1_IRQn, 0, 0);
      HAL_NVIC_EnableIRQ(LPTIM1_IRQn);
    /* USER CODE BEGIN LPTIM1_MspInit 1 */

    /* USER CODE END LPTIM1_MspInit 1 */
    }

  }

  /**
  * @brief LPTIM MSP De-Initialization
  * This function freeze the hardware resources used in this example
  * @param hlptim: LPTIM handle pointer
  * @retval None
  */
  void HAL_LPTIM_MspDeInit(LPTIM_HandleTypeDef* hlptim)
  {
    if(hlptim->Instance==LPTIM1)
    {
    /* USER CODE BEGIN LPTIM1_MspDeInit 0 */

    /* USER CODE END LPTIM1_MspDeInit 0 */
      /* Peripheral clock disable */
      __HAL_RCC_LPTIM1_CLK_DISABLE();

      /**LPTIM1 GPIO Configuration
      PB5     ------> LPTIM1_IN1
      PB7     ------> LPTIM1_IN2
      */
      HAL_GPIO_DeInit(GPIOB, GPIO_PIN_5|GPIO_PIN_7);

      /* LPTIM1 interrupt DeInit */
      HAL_NVIC_DisableIRQ(LPTIM1_IRQn);
    /* USER CODE BEGIN LPTIM1_MspDeInit 1 */

    /* USER CODE END LPTIM1_MspDeInit 1 */
    }

  }

static void OnMacProcessNotify(void)
{
  /* USER CODE BEGIN OnMacProcessNotify_1 */

  /* USER CODE END OnMacProcessNotify_1 */
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_LmHandlerProcess), CFG_SEQ_Prio_0);
  /* USER CODE BEGIN OnMacProcessNotify_2 */

  /* USER CODE END OnMacProcessNotify_2 */
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

