#include "main.h"
#include "BSP.h"
#include "String.h"
#include "app_dtu.h"

/******** 串口句柄变量 ********/
UART_HandleTypeDef hUSART1;
UART_HandleTypeDef hUSART2;
UART_HandleTypeDef hUSART3;
UART_HandleTypeDef hUART4;
UART_HandleTypeDef hUART5;
UART_HandleTypeDef hLPUART1;

/******** 串口参数变量 ********/
UartPara_t		g_pUART2;	//串口2参数
UartPara_t		g_pUART3;	//串口2参数
UartPara_t		g_pUART4;	//串口4参数
UartPara_t		g_pUART5;	//串口4参数
UartPara_t		g_pLPUART1;	//低功耗串口1参数

/******** 接收缓存变量 ********/
uint8_t		g_Uart1RxBuf[UART1_RX_BUFSIZE];		//UART1接收缓存
uint8_t		g_Uart2RxBuf[UART2_RX_BUFSIZE];		//UART2接收缓存
uint8_t		g_Uart3RxBuf[UART3_RX_BUFSIZE];		//UART3接收缓存
uint8_t		g_Uart4TxBuf[UART4_TX_BUFSIZE];		//UART4接收缓存
uint8_t		g_Uart4RxBuf[UART4_RX_BUFSIZE];		//UART4接收缓存
uint8_t		g_Uart5RxBuf[UART5_RX_BUFSIZE];	//UART5接收缓存 RTKGPS
uint8_t		g_LPUart1RxBuf[LPUART1_RX_BUFSIZE];	//LPUART1接收缓存

uint32_t   g_Uart5TxTimer;
uint8_t    g_Uart5DirFlag;	                    /*485方向luo'gu标记 1-接收 2-发送 */
uint16_t   g_TxBuf5[UART5_TX_BUF_BUFSIZE];		/* 发送缓冲区 */
uint32_t   g_Uart4TxTimer;


/******************************************************************
函数名称: void BSP_UartSetupPara
功能描述: 配置串口参数
参数输入: UART_HandleTypeDef		*UartHandle	串口句柄
		  uint32_t				BaudRate	波特率
		  uint32_t				WordLength	数据位
		  uint32_t				Parity		校验位
		  uint32_t				StopBits	停止位
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_UartSetupPara(UART_HandleTypeDef *UartHandle, uint32_t BaudRate, uint32_t WordLength, uint32_t Parity, uint32_t StopBits)
{
	/* 初始化参数 */
	UartHandle->Init.BaudRate = BaudRate;					//波特率		=BaudRate
	UartHandle->Init.WordLength = WordLength;				//数据位 	=WordLength
	UartHandle->Init.StopBits = StopBits;					//停止位 	=SopBits
	UartHandle->Init.Parity = Parity;						//校验位		=Parity
	UartHandle->Init.Mode = UART_MODE_TX_RX;				//串口模式	=收发
	UartHandle->Init.HwFlowCtl = UART_HWCONTROL_NONE;		//硬件流控	=无
	UartHandle->Init.OverSampling = UART_OVERSAMPLING_16;	//过采样		=16位
}

/******************************************************************
函数名称: void BSP_UartBindInstance
功能描述: 串口绑定实例
参数输入: UART_HandleTypeDef		*UartHandle	串口句柄
		  USART_TypeDef			*Inst		实例化
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_UartBindInstance(UART_HandleTypeDef *UartHandle, USART_TypeDef *Inst)
{
	UartHandle->Instance = Inst;							//绑定串口实例
	if (HAL_UART_Init(UartHandle) != HAL_OK)
	{
		Error_Handler();
	}
}

/******************************************************************
函数名称: void HAL_UART_MspInit
功能描述: 串口底层管脚初始化
参数输入: UART_HandleTypeDef	*huart	串口句柄
参数输出: 无
返 回 值: 
其    他: HAL库中初始化串口时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
  
	if (huart->Instance == USART1)
	{
		__HAL_RCC_USART1_CLK_ENABLE();
		
		GPIO_SetupPin(GPIOB, GPIO_PIN_6, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF7_USART1);
		GPIO_SetupPin(GPIOB, GPIO_PIN_7, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF7_USART1);
	}
	else if (huart->Instance == USART2)
	{
		__HAL_RCC_USART2_CLK_ENABLE();
		
		GPIO_SetupPin(GPIOA, GPIO_PIN_2, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF7_USART2);
		GPIO_SetupPin(GPIOA, GPIO_PIN_3, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF7_USART2);
	}
	else if (huart->Instance == USART3)
	{
		__HAL_RCC_USART3_CLK_ENABLE();
		
		GPIO_SetupPin(GPIOD, GPIO_PIN_9, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF7_USART3);
		GPIO_SetupPin(GPIOD, GPIO_PIN_8, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF7_USART3);
	}
	else if (huart->Instance == UART4)
	{
		__HAL_RCC_UART4_CLK_ENABLE();
		
		GPIO_SetupPin(GPIOC, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF8_UART4);
		GPIO_SetupPin(GPIOC, GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF8_UART4);
	}
	else if(huart->Instance==UART5)
	{
		__HAL_RCC_UART5_CLK_ENABLE();
		
		GPIO_SetupPin(GPIOC, GPIO_PIN_12, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF8_UART5);
		GPIO_SetupPin(GPIOD, GPIO_PIN_2, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF8_UART5);
		GPIO_SetupPin(GPIOE, GPIO_PIN_0, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);/*485切换引脚*/
	}
	else if(huart->Instance==LPUART1)
	{
		__HAL_RCC_LPUART1_CLK_ENABLE();
		
		GPIO_SetupPin(GPIOB, GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF8_LPUART1);
		GPIO_SetupPin(GPIOB, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF8_LPUART1);
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
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	if (huart->Instance == USART1)
	{
		/* 关闭时钟 */
		__HAL_RCC_USART1_CLK_DISABLE();

		/* 反初始化管脚 */
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_7);
		
		/* 关闭中断 */
		HAL_NVIC_DisableIRQ(USART1_IRQn);
	}
	else if (huart->Instance == USART2)
	{
		__HAL_RCC_USART2_CLK_DISABLE();
		
		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
		
		HAL_NVIC_DisableIRQ(USART2_IRQn);
	}
	else if (huart->Instance == USART3)
	{
		__HAL_RCC_USART3_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8 | GPIO_PIN_9);

		HAL_NVIC_DisableIRQ(USART3_IRQn);
	}
	else if(huart->Instance==UART4)
	{
		__HAL_RCC_UART4_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10 | GPIO_PIN_11);

		HAL_NVIC_DisableIRQ(UART4_IRQn);
	}
	else if(huart->Instance==UART5)
	{
		__HAL_RCC_UART5_CLK_DISABLE();

		HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);
		HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);

		HAL_NVIC_DisableIRQ(UART5_IRQn);
	}
	else if (huart->Instance == LPUART1)
	{
		__HAL_RCC_LPUART1_CLK_DISABLE();
		
		HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11 | GPIO_PIN_10);
		
		HAL_NVIC_DisableIRQ(LPUART1_IRQn);
	}

}

/******************************************************************
函数名称: void BSP_UartInit
功能描述: 配置串口参数
参数输入: UartPara_t		*Para	串口参数
		  uint32_t		*Timer	定时器变量指针
		  uint32_t		Timeout	超时时间
		  uint8_t		*RxBuf	接收缓存
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_UartParaInit(UartPara_t *Para, uint32_t *TickTimer, uint32_t Timeout, uint8_t *RxBuf)
{
	/* 初始化 */
	Para->Lock = 0;
	Para->ucRxChar = 0x00;
	Para->usRxPos = 0;
	
	/* 配置参数 */
	Para->ulRxTimer = 0xFFFFFFFF;
	Para->plTickTimer = TickTimer;
	Para->usRxIdleTimeout = Timeout;
	Para->pRxBuf = RxBuf;
}

/******************************************************************
函数名称: void BSP_UartRecvByte
功能描述: 串口接收一个字节
参数输入: UartPara_t		*Para	串口参数
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_UartRecvByte(UartPara_t *Para)
{
	Para->ulRxTimer = *(Para->plTickTimer);
	Para->pRxBuf[Para->usRxPos] = (uint8_t)Para->ucRxChar;
	Para->usRxPos++;
}

/******************************************************************
函数名称: void BSP_UartRecvTimeout
功能描述: 串口超时接收
参数输入: UartPara_t		*Para	串口参数
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
uint8_t BSP_UartRecvTimeout_Interrupt(UartPara_t *Para)
{
	uint32_t diff;
	uint8_t ret=0;
	
	/* 判断接收已经超时 */
	if (Para->ulRxTimer != 0xFFFFFFFF)
	{
		if (*(Para->plTickTimer) > Para->ulRxTimer)
		{
			diff = *(Para->plTickTimer) - Para->ulRxTimer;
		}
		else
		{
			diff = 0xFFFFFFFF - Para->ulRxTimer + *(Para->plTickTimer);
		}
		
		if (diff > Para->usRxIdleTimeout)
		{
			/* 空闲超时 */
			ret = 1;
			Para->ulRxTimer = 0xFFFFFFFF;
		}
	}
	
	return ret;
}

/******************************************************************
函数名称: void HAL_UART_ErrorCallback
功能描述: 串口错误回调
参数输入: UART_HandleTypeDef	*huart	串口句柄
参数输出: 无
返 回 值: 
其    他: HAL库中产生串口错误时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	/* 超时错误 OverRunError */
	if (__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE) != RESET)
	{
		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_ORE);	//清溢出标记
	}
	/* 帧错误 FrameError */
	else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_FE) != RESET)
	{
		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_FE);		//清帧错误标记
	}
	/* 奇偶校验错误 ParityError */
	else if (__HAL_UART_GET_FLAG(huart, UART_FLAG_PE) != RESET)
	{
		__HAL_UART_CLEAR_FLAG(huart, UART_FLAG_PE);		//清奇偶错误标记
	}
}

/******************************************************************
函数名称: void HAL_UART_RxCpltCallback
功能描述: 串口接收完成回调
参数输入: UART_HandleTypeDef	*huart	串口句柄
参数输出: 无
返 回 值: 
其    他: HAL库中产生串口接收完成中断时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	int8_t ret;
	
	uint8_t i;
	uint8_t sum;
	
	if (huart->Instance == USART2)
	{
		BSP_UartRecvByte(&g_pUART2);
	}	
	if (huart->Instance == UART4)
	{
		BSP_UartRecvByte(&g_pUART4);
	}
	if (huart->Instance == USART3)
	{
		BSP_UartRecvByte(&g_pUART3);
	}	
	if (huart->Instance == UART5)
	{
		BSP_UartRecvByte(&g_pUART5);
	}	
	if (huart->Instance == LPUART1)
	{
		BSP_UartRecvByte(&g_pLPUART1);
	}
}

/*
*********************************************************************************************************
*	函 数 名: UartSend
*	功能说明: 填写数据到UART发送缓冲区,并启动发送中断。中断处理函数发送完毕后，自动关闭发送中断
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void UartSend(UART_HandleTypeDef *huart, uint8_t *_ucaBuf, uint16_t _usLen)
{	
	UART_HandleTypeDef UartHandle;	

	if(huart->Instance == UART5)
	{
		Tool485_TX_EN();	
		delay_us(2000);		
//		UartHandle =  hUART5;
		g_Uart5DirFlag = 2;	

	}
//	else if(huart->Instance == UART4)
//			g_DTUSendDataDelay = uwTick;
	HAL_UART_Transmit(huart,_ucaBuf,_usLen,1000);
	g_Uart5TxTimer = uwTick;	
}