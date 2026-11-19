#include "main.h"
#include "BSP_spi.h"

/******** SPI句柄变量 ********/
SPI_HandleTypeDef hSPI1;
SPI_HandleTypeDef hSPI2;
SPI_HandleTypeDef hSPI3;

/******** SPI缓存变量 ********/
uint8_t *SpiTxBuf;
uint8_t *SpiRxBuf;

uint16_t SpiDMAHalfCplt=0;
uint16_t SpiDMACplt=0;
uint8_t SpiRecvCplt=0;
uint8_t SpiCSEnable=0;

uint8_t g_SpiRecvCplt=0;
uint8_t g_SpiSendCplt=0;

uint8_t SpiTxAdd;

/******************************************************************
函数名称: void BSP_SpiSetupClkPara(void)
功能描述: 配置SPI时钟参数
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_SpiSetupClkPara(SPI_HandleTypeDef *SpiHandle, uint32_t ClkPre, uint32_t ClkMode)
{
	SpiHandle->Init.BaudRatePrescaler = ClkPre;			//SPI时钟预分频	=(SPI时钟 = PLL2 / 预分频)
	if (ClkMode == SPI_CLOCK_MODE_0)
	{
		SpiHandle->Init.CLKPolarity = SPI_POLARITY_LOW;	//时钟极性		=低空闲
		SpiHandle->Init.CLKPhase = SPI_PHASE_1EDGE;		//时钟相位		=第1边缘
	}
	else if (ClkMode == SPI_CLOCK_MODE_1)
	{
		SpiHandle->Init.CLKPolarity = SPI_POLARITY_LOW;	//时钟极性		=低空闲
		SpiHandle->Init.CLKPhase = SPI_PHASE_2EDGE;		//时钟相位		=第2边缘
	}
	else if (ClkMode == SPI_CLOCK_MODE_2)
	{
		SpiHandle->Init.CLKPolarity = SPI_POLARITY_HIGH;//时钟极性		=高空闲
		SpiHandle->Init.CLKPhase = SPI_PHASE_1EDGE;		//时钟相位		=第1边缘
	}
	else if (ClkMode == SPI_CLOCK_MODE_3)
	{
		SpiHandle->Init.CLKPolarity = SPI_POLARITY_HIGH;//时钟极性		=高空闲
		SpiHandle->Init.CLKPhase = SPI_PHASE_2EDGE;		//时钟相位		=第2边缘
	}
}

/******************************************************************
函数名称: void BSP_SpiSetupDataPara(void)
功能描述: 配置SPI数据参数
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_SpiSetupDataPara(SPI_HandleTypeDef *SpiHandle, uint32_t Mode, uint32_t Direction, uint32_t DataSize, uint32_t FirstBit)
{
	SpiHandle->Init.Mode = Mode;					//SPI工作模式 	
	SpiHandle->Init.Direction = Direction;			//SPI方向		
	SpiHandle->Init.DataSize = DataSize;			//SPI数据大小	
	SpiHandle->Init.FirstBit = FirstBit;			//SPI数据首位		
}

/******************************************************************
函数名称: void BSP_SpiSetupNSSPara(void)
功能描述: 配置SPI片选参数
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_SpiSetupNSSPara(SPI_HandleTypeDef *SpiHandle, uint32_t NssMode, uint32_t NssPhMode)
{
	SpiHandle->Init.NSS = NssMode;					//片选控制模式
	SpiHandle->Init.NSSPMode = NssPhMode;			//片选脉冲控制
}

/******************************************************************
函数名称: void BSP_SpiSetupCrcPara(void)
功能描述: 配置SPICRC参数
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_SpiSetupCrcPara(SPI_HandleTypeDef *SpiHandle, uint32_t CRCEn, uint32_t CRCPoly, uint32_t CRCLength)
{
	SpiHandle->Init.CRCCalculation = CRCEn;			//硬件CRC启用
	SpiHandle->Init.CRCPolynomial = CRCPoly;		//CRC计算多项式(禁用时此项无意义)
	SpiHandle->Init.CRCLength = CRCLength;			//CRC计算数据长度(禁用时此项无意义)
}

/******************************************************************
函数名称: void BSP_SpiInitPara(void)
功能描述: 初始化默认SPI参数配置
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_SpiInitPara(SPI_HandleTypeDef *SpiHandle)
{
	/* SPI参数配置 */
	SpiHandle->Init.Mode = SPI_MODE_MASTER;								//SPI工作模式 	=主机
	SpiHandle->Init.Direction = SPI_DIRECTION_2LINES;					//SPI方向		=双线(全双工同时收发)
	SpiHandle->Init.DataSize = SPI_DATASIZE_8BIT;						//SPI数据大小	=8位
	SpiHandle->Init.FirstBit = SPI_FIRSTBIT_MSB;						//数据首位		=高位在前(MSB)
	/* SPI时钟配置 */
	SpiHandle->Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;		//SPI时钟预分频	=256 (SPI时钟 = PLL2 / 预分频 = 32MHz / 256 = 0.125MHz)
	SpiHandle->Init.CLKPolarity = SPI_POLARITY_HIGH;					//时钟极性		=高空闲
	SpiHandle->Init.CLKPhase = SPI_PHASE_2EDGE;							//时钟相位		=第2边缘
	/* 片选控制 */
	SpiHandle->Init.NSS = SPI_NSS_SOFT;									//片选控制		=软件控制
	SpiHandle->Init.NSSPMode = SPI_NSS_PULSE_DISABLE;					//片选脉冲		=禁用
	SpiHandle->Init.TIMode = SPI_TIMODE_DISABLE;						//TI模式		=禁用
	/* 硬件CRC */
	SpiHandle->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;		//硬件CRC		=禁用
	SpiHandle->Init.CRCPolynomial = 7;									//硬件CRC多项式	=7 (硬件CRC禁用时无意义)
	SpiHandle->Init.CRCLength = SPI_CRC_LENGTH_8BIT;					//硬件CRC长度	=8位 (硬件CRC禁用时无意义)
}


/******************************************************************
函数名称: void BSP_SpiBindInstance(void)
功能描述: 绑定实例
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_SpiBindInstance(SPI_HandleTypeDef *SpiHandle, SPI_TypeDef *Inst)
{
	SpiHandle->Instance = Inst;
	if (HAL_SPI_Init(SpiHandle) != HAL_OK)
	{
		Error_Handler();
	}
}

/******************************************************************
函数名称: void HAL_SPI_MspInit
功能描述: SPI底层管脚初始化
参数输入: SPI_HandleTypeDef	*hspi	SPI句柄
参数输出: 无
返 回 值: 
其    他: HAL库中初始化SPI时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* SPI1 LORA 初始化 */
	if (hspi->Instance == SPI1)
	{
		/* 启用外设时钟 */
		__HAL_RCC_SPI1_CLK_ENABLE();
		
		/* 配置管脚 */
		/* PA7=MOSI PA6=MISO PA5=SCK PA4=NSS */
		GPIO_SetupPin(GPIOA, GPIO_PIN_7, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1);
		GPIO_SetupPin(GPIOA, GPIO_PIN_6, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1);
		GPIO_SetupPin(GPIOA, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1);
		GPIO_SetupPin(GPIOA, GPIO_PIN_4, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, 0);
//		GPIO_SetupPin(GPIOA, GPIO_PIN_4, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI1);
	}
	/* SPI2初始化 */
	else if (hspi->Instance == SPI2)
	{
		/* 启用外设时钟 */
		__HAL_RCC_SPI2_CLK_ENABLE();
		
		/* 配置管脚 */
		/* PD4=MOSI PD3=MISO PD1=SCK PD0=NSS */
		GPIO_SetupPin(GPIOD, GPIO_PIN_4, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI2);
		GPIO_SetupPin(GPIOD, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI2);
		GPIO_SetupPin(GPIOD, GPIO_PIN_1, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI2);
		GPIO_SetupPin(GPIOD, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
//		GPIO_SetupPin(GPIOD, GPIO_PIN_0, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF5_SPI2);
	}
	/* SPI3初始化 */
	else if (hspi->Instance == SPI3)
	{
		/* 启用外设时钟 */
		__HAL_RCC_SPI3_CLK_ENABLE();
		
		/* 配置管脚 */
		/* PB5=MOSI           PB3=SCK PA15=NSS */
		GPIO_SetupPin(GPIOB, GPIO_PIN_5, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF6_SPI3);
		
		GPIO_SetupPin(GPIOB, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF6_SPI3);
//		GPIO_SetupPin(GPIOA, GPIO_PIN_15, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
		GPIO_SetupPin(GPIOB, GPIO_PIN_4, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF6_SPI3);
		GPIO_SetupPin(GPIOD, GPIO_PIN_7, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	}
  

}

/******************************************************************
函数名称: void HAL_UART_MspDeInit
功能描述: 串口底层管脚反初始化
参数输入: UART_HandleTypeDef	*huart	串口句柄
参数输出: 无
返 回 值: 
其    他: HAL库中反初始化串口时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{
	if(hspi->Instance == SPI1)
	{
		/* 关闭SPI1 */
		__HAL_RCC_SPI1_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_4);

	}
	else if (hspi->Instance == SPI3)
	{
		__HAL_RCC_SPI3_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_15);	
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);	
	}
}

/******************************************************************
函数名称: uint16_t SpiWrite
功能描述: SPI写
参数输入: SPI_HandleTypeDef	*hspi	SPI句柄
		  uint8_t			*WBuf	数据输入缓存
		  uint16_t			WByte	输入数据个数
参数输出: 无
返 回 值: uint16_t	已发送的数据个数
其    他: 
******************************************************************/
uint16_t SpiWrite(SPI_HandleTypeDef *hspi, uint8_t *WBuf, uint16_t WByte)
{
	HAL_StatusTypeDef hret;
	uint8_t wbuf, rbuf;
	uint16_t wnbyte;
	uint16_t i;
	
	wnbyte = 0;
	for (i=0; i<WByte; i++)
	{
		wbuf = WBuf[i];
		hret = HAL_SPI_TransmitReceive(hspi, &wbuf, &rbuf, 1, 1000);
		if (hret == HAL_OK)
			wnbyte++;
		else
			break;
	}
	Delay1us(2);		//字节之间延迟至少1.5us
	return wnbyte;
}

/******************************************************************
函数名称: uint16_t SpiRead
功能描述: SPI读
参数输入: SPI_HandleTypeDef	*hspi	SPI句柄
		  uint8_t			*RBuf	数据输出缓存
		  uint16_t			RByte	输出数据个数
参数输出: 无
返 回 值: uint16_t	已读取的数据个数
其    他: 
******************************************************************/
uint16_t SpiRead(SPI_HandleTypeDef *hspi, uint8_t *RBuf, uint16_t RByte)
{
	HAL_StatusTypeDef hret;
	uint8_t wbuf, rbuf;
	uint16_t i;
	uint16_t rnbyte;
	
	rnbyte = 0;
	wbuf = 0xFF;
	for (i=0; i<RByte; i++)
	{
		hret = HAL_SPI_TransmitReceive(hspi, &wbuf, &rbuf, 1, 1000);
		if (hret == HAL_OK)
			rnbyte++;
		else
			break;
		RBuf[i] = rbuf;
	}
	Delay1us(2);		//字节之间至少延迟1.5us
	return rnbyte;
}

/****************************************  中断回调函数 ***********************************************************/

/******************************************************************
函数名称: void HAL_SPI_RxCpltCallback
功能描述: SPI接收完成回调
参数输入: SPI_HandleTypeDef	*hspi	SPI句柄
参数输出: 无
返 回 值: 
其    他: HAL库中产生SPI接收完成中断时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	uint16_t i;
	
//	if (hspi->Instance == SPI4)
//	{
//		g_SpiRecvCplt = 1;
//		HAL_SPI_Receive_DMA(&hSPI4, SpiRxBuf, 256);
//	}
}

/******************************************************************
函数名称: void HAL_SPI_TxCpltCallback
功能描述: SPI发送完成回调
参数输入: SPI_HandleTypeDef	*hspi	SPI句柄
参数输出: 无
返 回 值: 
其    他: HAL库中使用DMA方式SPI发送完成中断时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	uint16_t i;
	
//	if (hspi->Instance == SPI4)
//	{
//		g_SpiSendCplt = 1;
//		HAL_SPI_Transmit_DMA(&hSPI4, SpiTxBuf, 256);
//	}
}

/******************************************************************
函数名称: void HAL_SPI_TxRxCpltCallback
功能描述: SPI收发完成回调
参数输入: SPI_HandleTypeDef	*hspi	SPI句柄
参数输出: 无
返 回 值: 
其    他: HAL库中产生SPI收发完成中断时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
	uint16_t i;
	
//	if (hspi->Instance == SPI4)
//	{
//		for (i=0; i<10; i++)
//		{
//			printf("%02X ", SpiRxBuf[i]);
//		}
//		printf("\r\n");
//		SpiDMACplt++;
//	}
}

/******************************************************************
函数名称: void HAL_SPI_TxHalfCpltCallback
功能描述: SPI发送半完成回调
参数输入: SPI_HandleTypeDef	*hspi	SPI句柄
参数输出: 无
返 回 值: 
其    他: HAL库中使用DMA方式SPI发送半完成中断时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_SPI_TxHalfCpltCallback(SPI_HandleTypeDef *hspi)
{
//	if (hspi->Instance == SPI4)
//	{
//		SpiDMAHalfCplt++;
//		CLEAR_BIT(hspi->Instance->CFG1, SPI_CFG1_TXDMAEN);
//	}
}

void SPI3_Init(void)
{
    hSPI3.Instance				=SPI3;                   //SPI2
    hSPI3.Init.Mode				=SPI_MODE_MASTER;        //设置SPI工作模式，设置为主模式
    hSPI3.Init.Direction		=SPI_DIRECTION_2LINES;   //设置SPI单向或者双向的数据模式:SPI设置为双线模式
    hSPI3.Init.DataSize			=SPI_DATASIZE_8BIT;      //设置SPI的数据大小:SPI发送接收8位帧结构
    hSPI3.Init.CLKPolarity		=SPI_POLARITY_LOW;     	 //串行同步时钟的空闲状态为低电平
    hSPI3.Init.CLKPhase			=SPI_PHASE_1EDGE;        //串行同步时钟的第一个跳变沿（上升或下降）数据被采样
    hSPI3.Init.NSS				=SPI_NSS_SOFT;           //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    hSPI3.Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_32;//定义波特率预分频的值
    hSPI3.Init.FirstBit			=SPI_FIRSTBIT_MSB;       //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始?
    hSPI3.Init.TIMode			=SPI_TIMODE_DISABLE;     //关闭TI模式
    hSPI3.Init.CRCCalculation	=SPI_CRCCALCULATION_DISABLE;//关闭硬件CRC校验
    hSPI3.Init.CRCPolynomial	=10;                  	 //CRC值计算的多项式
    HAL_SPI_Init(&hSPI3);								 //初始化SPI2
    
    __HAL_SPI_ENABLE(&hSPI3);                     //使能SPI2
}