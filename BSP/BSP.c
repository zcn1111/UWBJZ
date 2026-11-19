
#include "main.h"
#include "BSP.h"
#include "string.h"
#include "stdio.h"
#include "app_tool.h"

/******** 系统电源时钟配置 *********/
/******************************************************************
函数名称: void SystemPower_Config(void)
功能描述: 系统电源配置
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void SystemPower_Config(void)
{
	if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
	{
		while (1);
	}
}

/******************************************************************
函数名称: void SystemClock_Config(void)
功能描述: 系统时钟配置
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void SystemClock_Config(void)
{
	HAL_StatusTypeDef ret=HAL_OK;
	RCC_OscInitTypeDef RCC_OscInitStruct={0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct={0};
    
	/* 时钟源配置 */
	RCC_OscInitStruct.OscillatorType = (RCC_OSCILLATORTYPE_HSE);	//配置外部高速时钟
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;			//外部高速时钟	=开启
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;			//内部高速时钟	=关闭
	
	/* PLL配置 */
	/* 外部晶振f=8MHz */
	/* (PLLCLK = f / M * N / R = 32MHz) */
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;			//PLL			=开启
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;	//PLL时钟源		=HSE
	RCC_OscInitStruct.PLL.PLLM = 1;
	RCC_OscInitStruct.PLL.PLLN = 18;
	RCC_OscInitStruct.PLL.PLLP = 7;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (ret != HAL_OK) while(1);

	/* 时钟分频配置 */
	RCC_ClkInitStruct.ClockType=(RCC_CLOCKTYPE_SYSCLK|\
                                RCC_CLOCKTYPE_HCLK |\
                                RCC_CLOCKTYPE_PCLK1 |\
                                RCC_CLOCKTYPE_PCLK2);

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;		//系统时钟源		=PLL时钟
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;				//AHB总线分频	=1分频	=400/2=200MHz
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1; 				//APB1总线分频	=1分频	=200/2=100MHz
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1; 				//APB2总线分频	=1分频	=200/2=100MHz
	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
	if (ret != HAL_OK) while(1);

	/* 高速IO */
}

/******************************************************************
函数名称: void PeripheralsClock_Config(void)
功能描述: 外设总线时钟配置
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void PeripheralsClock_Config(void)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	HAL_StatusTypeDef ret=HAL_OK;

	
	/* 总线时钟配置 */
	/* 外部晶振f=8MHz */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC;	
	
	/* 配置外设时钟源 */
	PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_SYSCLK;				//ADC时钟		=SYSCLK	=32M
		
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		Error_Handler();
	}
}

/******************************************************************
函数名称: void BSPInit(void)
功能描述: 板级支持包初始化
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSPInit(void)
{
	uint16_t i;
	
	/* GPIO初始化 */
	BSP_GpioInit();
	
	/* 关闭LORA电源 */
	HAL_GPIO_WritePin(LORA_PWR_PORT, LORA_PWR_PIN, GPIO_PIN_RESET);		//关闭Lora电源
	HAL_GPIO_WritePin(LORA_RESET_PORT, LORA_RESET_PIN, GPIO_PIN_SET);	//LORA复位置高
	HAL_GPIO_WritePin(LORA_LED_PORT, LORA_LED_PIN, GPIO_PIN_SET);		//关闭LORA指示灯
	/* 关闭OLED电源 */
	HAL_GPIO_WritePin(OLED_PWR_PORT, OLED_PWR_PIN, GPIO_PIN_SET);		//关闭OLED电源 低有效
	/* 关闭RTK电源 */
	HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_RESET);		//关闭RTK电源
	HAL_GPIO_WritePin(RTK_RESET_PORT, RTK_RESET_PIN, GPIO_PIN_SET);		//RTK复位
	/* 关闭DTU电源 */
	HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);		//关闭DTU电源
	InitCelluarGpio();	
	/*客服工具电源*/
	if(g_CheckTool == 1)   
	{
		HAL_GPIO_WritePin(TOOL_PWR_PORT, TOOL_PWR_PIN, GPIO_PIN_SET);		//关闭客服工具电源
	}
	else if(g_CheckTool == 0)
	{
		HAL_GPIO_WritePin(TOOL_PWR_PORT, TOOL_PWR_PIN, GPIO_PIN_RESET);		//打开客服工具电源
		InitToolVar();
	}
	/* 软件模拟I2C初始化 */
//	SI2C_Init(&Softi2c1);
	
	/* I2C初始化 */
	BSP_I2cSetupPara(&hI2C1, 0x00, I2C_ADDRESSINGMODE_7BIT, I2C_GENERALCALL_DISABLE, I2C_NOSTRETCH_ENABLE);
	BSP_I2cBindInstance(&hI2C1, I2C1);
//	BSP_I2cSetupPara(&hI2C2, 0x00, I2C_ADDRESSINGMODE_7BIT, I2C_GENERALCALL_DISABLE, I2C_NOSTRETCH_ENABLE);
//	BSP_I2cBindInstance(&hI2C2, I2C2);
	BSP_I2cSetupPara(&hI2C3, 0x00, I2C_ADDRESSINGMODE_7BIT, I2C_GENERALCALL_DISABLE, I2C_NOSTRETCH_ENABLE);
	BSP_I2cBindInstance(&hI2C3, I2C3);
	
	/* DMA初始化 */
	DMA1Init();
	DMA2Init();

	/* 串口初始化配置 */	
	/* 串口1初始化 RTK UM980 */
	/* 使用串口空闲中断+DMA */
	BSP_UartSetupPara(&hUSART1, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
	BSP_UartBindInstance(&hUSART1, USART1);
	HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
	__HAL_UART_ENABLE_IT(&hUSART1, UART_IT_IDLE);	//开启空闲中断
	HAL_NVIC_EnableIRQ(USART1_IRQn);				//启用串口1中断
	/* DMA串口接收初始化 */
	BSP_DMAInitRxPara(&hDMA_Usart1Rx, DMA_REQUEST_2, DMA_NORMAL, DMA_PRIORITY_MEDIUM);
	BSP_DMAUartBindStream(&hDMA_Usart1Rx, DMA1_Channel5, &hUSART1, DMA_CHANNEL_RX);
	/* 环形缓存初始化 */
	InitRingBuf(&g_Uart1RxRing, UART1_RX_BUFSIZE, g_Uart1RingBuf);
	HAL_UART_Receive_DMA(&hUSART1, g_Uart1RxBuf, UART1_RX_BUFSIZE);		//开启DMA接收

	/* 串口2初始化 USB转串口 */
	/* 使用串口中断+超时 */
	BSP_UartSetupPara(&hUSART2, 230400, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
	BSP_UartParaInit(&g_pUART2, (uint32_t*)&uwTick, 8, g_Uart2RxBuf);
	BSP_UartBindInstance(&hUSART2, USART2);
	HAL_NVIC_SetPriority(USART2_IRQn, 8, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	/* 环形缓存初始化 */
	InitRingBuf(&g_Uart2RxRing, UART2_RX_BUFSIZE, g_Uart2RingBuf);
	/* 启动接收中断 */
	HAL_UART_Receive_IT(&hUSART2, (uint8_t*)&g_pUART2.ucRxChar, 1);	


	BSP_UartSetupPara(&hUSART3, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
	BSP_UartBindInstance(&hUSART3, USART3);
	HAL_NVIC_SetPriority(USART3_IRQn, 6, 0);
	__HAL_UART_ENABLE_IT(&hUSART3, UART_IT_IDLE);	//开启空闲中断
	HAL_NVIC_EnableIRQ(USART3_IRQn);				//启用串口1中断
	/* DMA串口接收初始化 */
	BSP_DMAInitRxPara(&hDMA_Usart3Rx, DMA_REQUEST_2, DMA_NORMAL, DMA_PRIORITY_MEDIUM);
	BSP_DMAUartBindStream(&hDMA_Usart3Rx, DMA1_Channel3, &hUSART3, DMA_CHANNEL_RX);
	/* 环形缓存初始化 */
	InitRingBuf(&g_Uart3RxRing, UART3_RX_BUFSIZE, g_Uart3RingBuf);
	HAL_UART_Receive_DMA(&hUSART3, g_Uart3RxBuf, UART3_RX_BUFSIZE);		//开启DMA接收



//	/* 串口3初始化 ML307A CORS数据 */
//	/* 使用串口空闲中断+DMA */
//	BSP_UartSetupPara(&hUSART3, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
//	BSP_UartBindInstance(&hUSART3, USART3);
//	HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
//	__HAL_UART_ENABLE_IT(&hUSART3, UART_IT_IDLE);		//超时中断
//	HAL_NVIC_EnableIRQ(USART3_IRQn);
//	/* DMA串口接收初始化 */
//	BSP_DMAInitRxPara(&hDMA_Usart3Rx, DMA_REQUEST_2, DMA_NORMAL, DMA_PRIORITY_MEDIUM);
//	BSP_DMAUartBindStream(&hDMA_Usart3Rx, DMA1_Channel3, &hUSART3, DMA_CHANNEL_RX);
//	/* 环形缓存初始化 */
//	InitRingBuf(&g_Uart3RxRing, UART3_RX_BUFSIZE, g_Uart3RingBuf);
//	HAL_UART_Receive_DMA(&hUSART3, g_Uart3RxBuf, UART3_RX_BUFSIZE);
//	/* 开启DTU电源 */
//	HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);

	/* 串口4初始化 ML307A 透传数据 */
	/* 使用 串口中断+超时模式 需要将中断优先级设高才能保证接收时不被打断 */
	BSP_UartSetupPara(&hUART4, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
//	BSP_UartParaInit(&g_pUART4, (uint32_t*)&uwTick, 8, g_Uart4RxBuf);
	BSP_UartBindInstance(&hUART4, UART4);
	HAL_NVIC_SetPriority(UART4_IRQn, 6, 0);
	__HAL_UART_ENABLE_IT(&hUART4, UART_IT_IDLE);	//开启空闲中断
	HAL_NVIC_EnableIRQ(UART4_IRQn);				//启用串口4中断
	/* DMA串口接收初始化 */
	BSP_DMAInitRxPara(&hDMA_Uart4Rx, DMA_REQUEST_2, DMA_NORMAL, DMA_PRIORITY_MEDIUM);
	BSP_DMAUartBindStream(&hDMA_Uart4Rx, DMA2_Channel5, &hUART4, DMA_CHANNEL_RX);
	/* 环形缓存初始化 */
	InitRingBuf(&g_Uart4RxRing, UART4_RX_BUFSIZE, g_Uart4RingBuf);
	HAL_UART_Receive_DMA(&hUART4, g_Uart4RxBuf, UART4_RX_BUFSIZE);

//    HAL_NVIC_SetPriority(UART4_IRQn, 6, 0);
//	HAL_NVIC_EnableIRQ(UART4_IRQn);
//	/* 环形缓存初始化 */
//	InitRingBuf(&g_Uart4RxRing, UART4_RX_BUFSIZE, g_Uart4RingBuf);
//	/* 启动接收中断 */
//	HAL_UART_Receive_IT(&hUART4, (uint8_t*)&g_pUART4.ucRxChar, 1);
	
	/* 串口5初始化 客服工具345600 */
	BSP_UartSetupPara(&hUART5, 345600, UART_WORDLENGTH_9B, UART_PARITY_NONE, UART_STOPBITS_1);
	BSP_UartBindInstance(&hUART5, UART5);
	HAL_NVIC_SetPriority(UART5_IRQn, 8, 0);
	__HAL_UART_ENABLE_IT(&hUART5, UART_IT_IDLE);	//开启空闲中断
	HAL_NVIC_EnableIRQ(UART5_IRQn);				//启用串口1中断
	/* DMA串口接收初始化 */
	BSP_DMAInitRxPara(&hDMA_Uart5Rx, DMA_REQUEST_2, DMA_NORMAL, DMA_PRIORITY_MEDIUM);
	BSP_DMAUartBindStream(&hDMA_Uart5Rx, DMA2_Channel2, &hUART5, DMA_CHANNEL_RX);
	/* 环形缓存初始化 */
	InitRingBuf(&g_Uart5RxRing, UART5_RX_BUFSIZE, g_Uart5RingBuf);
	HAL_UART_Receive_DMA(&hUART5, g_Uart5RxBuf, UART5_RX_BUFSIZE);		//开启DMA接收
	Tool485_RX_EN();
	g_Uart5DirFlag = 1;	


//	BSP_UartSetupPara(&hUART5, 345600, UART_WORDLENGTH_9B, UART_PARITY_NONE, UART_STOPBITS_1);
//	BSP_UartParaInit(&g_pUART5, (uint32_t*)&uwTick, 50, g_Uart5RxBuf);
//	BSP_UartBindInstance(&hUART5, UART5);
//	HAL_NVIC_SetPriority(UART5_IRQn, 1, 0);
//	HAL_NVIC_EnableIRQ(UART5_IRQn);
//	/* 环形缓存初始化 */
//	InitRingBuf(&g_Uart5RxRing, UART5_RX_BUFSIZE, g_Uart5RingBuf);
//	HAL_UART_Receive_IT(&hUART5, (uint8_t*)&g_pUART5.ucRxChar, 1);
//	Tool485_RX_EN();
//	g_Uart5DirFlag = 1;	

	/* 串口LP1初始化 蓝牙 */
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_4, GPIO_PIN_RESET);
	BSP_UartSetupPara(&hLPUART1, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
	BSP_UartParaInit(&g_pLPUART1, (uint32_t*)&uwTick, 8, g_LPUart1RxBuf);
	BSP_UartBindInstance(&hLPUART1, LPUART1);
	HAL_NVIC_SetPriority(LPUART1_IRQn, 6, 0);
	HAL_NVIC_EnableIRQ(LPUART1_IRQn);
	/* 环形缓存初始化 */
	InitRingBuf(&g_LPUart1RxRing, LPUART1_RX_BUFSIZE, g_LPUart1RingBuf);
	HAL_UART_Receive_IT(&hLPUART1, (uint8_t*)&g_pLPUART1.ucRxChar, 1);

	/* 定时器初始化配置 */
	/* TIM2 TIM3 TIM6 TIM7 挂载APB1上 APB1CLK=32MHz 定时器时钟=APB1CLK*1=32MHz */
	TIM7Init(10000, 7200);	//TIM7定时器 1s
	TIM6Init(100, 7200);	//TIM6定时器 10ms
	TIM2Init();				//TIM2定时器 1us 无中断
	
	/* SPI1初始化 LORA */
	/* SPIclk = 32MHz / 8 = 4Mhz */
//	BSP_SpiInitPara(&hSPI1);
//	BSP_SpiSetupClkPara(&hSPI1, SPI_BAUDRATEPRESCALER_8, SPI_CLOCK_MODE_0);
//	BSP_SpiSetupDataPara(&hSPI1, SPI_MODE_MASTER, SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_FIRSTBIT_MSB);
//	BSP_SpiSetupNSSPara(&hSPI1, SPI_NSS_SOFT, SPI_NSS_PULSE_DISABLE);
//	BSP_SpiBindInstance(&hSPI1, SPI1);
//	__HAL_SPI_ENABLE(&hSPI1);
	
	/* SPI2初始化 IMU */
	/* SPIclk = 32MHz / 128 = 0.25Mhz */
	BSP_SpiInitPara(&hSPI2);
	BSP_SpiSetupClkPara(&hSPI2, SPI_BAUDRATEPRESCALER_128, SPI_CLOCK_MODE_3);
	BSP_SpiSetupDataPara(&hSPI2, SPI_MODE_MASTER, SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_FIRSTBIT_MSB);
	BSP_SpiSetupNSSPara(&hSPI2, SPI_NSS_SOFT, SPI_NSS_PULSE_DISABLE);
	BSP_SpiBindInstance(&hSPI2, SPI2);
	__HAL_SPI_ENABLE(&hSPI2);
	
	/* SPI3初始化 OLED/LCD */
	/* SPIclk = 32MHz / 16 = 2Mhz */
	BSP_SpiInitPara(&hSPI3);
	BSP_SpiSetupClkPara(&hSPI3, SPI_BAUDRATEPRESCALER_16, SPI_CLOCK_MODE_3);
	BSP_SpiSetupDataPara(&hSPI3, SPI_MODE_MASTER, SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_FIRSTBIT_MSB);
	BSP_SpiSetupNSSPara(&hSPI3, SPI_NSS_SOFT, SPI_NSS_PULSE_DISABLE);
	BSP_SpiBindInstance(&hSPI3, SPI3);
	__HAL_SPI_ENABLE(&hSPI3);

	/* QSPI初始化 FLASH */
	/* QSPIclk = 32MHz / 32 = 1Mhz */
	BSP_SetupQspiPara(&hQSPI, 32, QSPI_CLOCK_MODE_0, QSPI_FLASHSIZE_32MB, 0);
	BSP_QspiBindInstance(&hQSPI, QUADSPI);
	
	/* ADC1初始化 */
	BSP_AdcInitPara_ADC1(&hADC1);
	BSP_AdcBindInstance(&hADC1, ADC1);			//绑定ADC1实例
	BSP_AdcSetupChannelPara(&hADC_Ch3, ADC_CHANNEL_3, ADC_REGULAR_RANK_1, ADC_SAMPLETIME_640CYCLES_5, ADC_SINGLE_ENDED, ADC_OFFSET_NONE, 0);
	BSP_AdcInitChannel(&hADC1, &hADC_Ch3);	//初始化通道

	/* ADC1初始化 */
	BSP_AdcInitPara_ADC1(&hADC2);
	BSP_AdcBindInstance(&hADC2, ADC2);			//绑定ADC1实例
	BSP_AdcSetupChannelPara(&hADC_Ch13, ADC_CHANNEL_13, ADC_REGULAR_RANK_1, ADC_SAMPLETIME_640CYCLES_5, ADC_SINGLE_ENDED, ADC_OFFSET_NONE, 0);
	BSP_AdcInitChannel(&hADC2, &hADC_Ch13);	//初始化通道
	/* 配置按键 */	
	BSP_BtnInitPara(&g_BtnUp, KEY1_UP_PORT, KEY1_UP_PIN, 0);
	BSP_BtnInitPara(&g_BtnDown, KEY2_DOWN_PORT, KEY2_DOWN_PIN, 0);
	BSP_BtnInitPara(&g_BtnOK, KEY3_OK_PORT, KEY3_OK_PIN, 0);		//配置按键初始化
}


