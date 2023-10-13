/////////////////////////////////// либо через if в lora.app либо через while в main|app.lorawan
/*или switch ?
либо сначала cntGetValue  и там исходя от данных
либо приходит прерывание на пин и уже в if заводится условие для проверки этого пина либо прерывания
Допустим имеем значение регистра и там далее если он равен 1 и более, то в переменную записывается
 значение вскрытий и при включении передаёт данные  в общий буф payload
 При 0 кол-ве вскрытий соответсвенно инфа передаётся так же при просыпе
   2 варианта исполнения:
   1. При срабатывании кнопки выводит из спящего режима и отсылает данные о взломе дополнительно
   с измеренными параметрами, без них нет смысла выводить, типо тогда же придётся создать отдельный
  буфер с данными для охранной кнопки, а я эти данные запихиваю в общую payload.
  uint8_t Data[256];
   2. Как и реализовано в структурке и дипломе, в спящем режиме считывать значение кнопки а при
  просыпе передавать в общем буфере значение вскрытий, как-то явно помечая.
  В случае прекращения передачи после вскрытия на веб морде тогда нужно ввести отслеживание
 по devEU | numberdev

while (REG != 0)
  {
	}

while (REG = 0)
  {

	}
	while ((REG = 0()) != EOF)
  void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
/*  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
/*}

#ifdef  USE_FULL_ASSERT
/*
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
/*void assert_failed(uint8_t *file, uint32_t line)
{
//  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
 /* while (1)
  {
  }
//  /* USER CODE END 6 */
/*}
//#endif /* USE_FULL_ASSERT
 */
