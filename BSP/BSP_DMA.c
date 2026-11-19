#include "main.h"
#include "BSP.h"
#include "String.h"

/******** DMA句柄定义 ********/
DMA_HandleTypeDef	hDMA_Usart1Rx;	//DMA_USART1-Rx
DMA_HandleTypeDef	hDMA_Usart2Rx;	//DMA_USART2-Rx
DMA_HandleTypeDef	hDMA_Usart3Rx;	//DMA_USART3-Rx
DMA_HandleTypeDef	hDMA_Uart4Rx;	//DMA_UART4-Rx
DMA_HandleTypeDef	hDMA_Uart5Rx;	//DMA_UART5-Rx

DMA_HandleTypeDef	hDMA_Spi1Rx;	//DMA2-SPI4Rx
DMA_HandleTypeDef	hDMA_Spi1Tx;	//DMA2-SPI4Tx
/******************************************************************
函数名称: void DMA1Init
功能描述: DMA1初始化
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void DMA1Init(void)
{
	/* 启用DMA1时钟 */
	__HAL_RCC_DMA1_CLK_ENABLE();

}

/******************************************************************
函数名称: void DMA2Init
功能描述: DMA2初始化
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void DMA2Init(void)
{

	/* 启用DMA2时钟 */
	__HAL_RCC_DMA2_CLK_ENABLE();

}

/******************************************************************
函数名称: void BSP_DMAInitRxPara
功能描述: DMA接收配置初始化
参数输入: DMA_HandleTypeDef		*DMAHandle	DMA句柄
		  uint32_t				Request		DMA请求
		  uint32_t				Mode		DMA模式
		  uint32_t				Priority	DMA优先级
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_DMAInitRxPara(DMA_HandleTypeDef *DMAHandle, uint32_t Request, uint32_t Mode, uint32_t Priority)
{
	DMAHandle->Init.Request = Request;							//配置DMA请求		=Request
    DMAHandle->Init.Direction = DMA_PERIPH_TO_MEMORY;			//DMA方向 			=外设-内存	
    DMAHandle->Init.PeriphInc = DMA_PINC_DISABLE;				//DMA外设地址增加	=禁用
    DMAHandle->Init.MemInc = DMA_MINC_ENABLE;					//DMA内存地址增加	=启用
    DMAHandle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//DMA外设数据对齐	=字节
    DMAHandle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		//DMA内存数据对齐	=字节
    DMAHandle->Init.Mode = Mode;								//DMA模式			=Mode
    DMAHandle->Init.Priority = Priority;						//DMA优先级			=Priority
}

/******************************************************************
函数名称: void BSP_DMAInitTxPara
功能描述: DMA发送配置初始化
参数输入: DMA_HandleTypeDef		*DMAHandle	DMA句柄
		  uint32_t				Request		DMA请求
		  uint32_t				Mode		DMA模式
		  uint32_t				Priority	DMA优先级
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_DMAInitTxPara(DMA_HandleTypeDef *DMAHandle, uint32_t Request, uint32_t Mode, uint32_t Priority)
{
	DMAHandle->Init.Request = Request;							//配置DMA请求		=Request
    DMAHandle->Init.Direction = DMA_MEMORY_TO_PERIPH;			//DMA方向 			=内存-外设
    DMAHandle->Init.PeriphInc = DMA_PINC_DISABLE;				//DMA外设地址增加	=禁用
    DMAHandle->Init.MemInc = DMA_MINC_ENABLE;					//DMA内存地址增加	=启用
    DMAHandle->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;	//DMA外设数据对齐	=字节
    DMAHandle->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;		//DMA内存数据对齐	=字节
    DMAHandle->Init.Mode = Mode;								//DMA模式			=Mode
    DMAHandle->Init.Priority = Priority;						//DMA优先级			=Priority
}

/******************************************************************
函数名称: void BSP_DMASetupMemoryPara
功能描述: DMA内存参数配置
参数输入: DMA_HandleTypeDef		*DMAHandle	DMA句柄
		  uint32_t				MemInc		DMA内存地址增加
		  uint32_t				MemAlign	DMA内存数据对齐
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_DMASetupMemoryPara(DMA_HandleTypeDef *DMAHandle, uint32_t MemInc, uint32_t MemAlign)
{
    DMAHandle->Init.MemInc = MemInc;					//DMA内存地址增加	=MemInc
    DMAHandle->Init.MemDataAlignment = MemAlign;		//DMA内存数据对齐	=MemAlign
}

/******************************************************************
函数名称: void BSP_DMASetupPeriphPara
功能描述: DMA外设参数配置
参数输入: DMA_HandleTypeDef		*DMAHandle	DMA句柄
		  uint32_t				PerInc		DMA外设地址增加
		  uint32_t				PerAlign	DMA外设数据对齐
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_DMASetupPeriphPara(DMA_HandleTypeDef *DMAHandle, uint32_t PerInc, uint32_t PerAlign)
{
    DMAHandle->Init.PeriphInc = PerInc;					//DMA外设地址增加	=PerInc
    DMAHandle->Init.PeriphDataAlignment = PerAlign;		//DMA外设数据对齐	=PerAlign
}


/******************************************************************
函数名称: void BSP_DMAUartBindInstance
功能描述: DMA-串口流通道绑定
参数输入: DMA_HandleTypeDef		*DMAHandle	DMA句柄
		  DMA_Channel_TypeDef	*DMAChannel	DMA流通道
		  UART_HandleTypeDef	*UartHandle	串口句柄
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_DMAUartBindStream(DMA_HandleTypeDef *DMAHandle, DMA_Channel_TypeDef *DMAChannel, UART_HandleTypeDef *UartHandle, uint8_t Channel)
{
	/* 初始化 */
	DMAHandle->Instance = DMAChannel;
    if (HAL_DMA_Init(DMAHandle) != HAL_OK)
    {
		Error_Handler();
    }
	
	/* 连接DMA通道 启动DMA */
	if (Channel == DMA_CHANNEL_RX)
	{
		__HAL_LINKDMA(UartHandle, hdmarx, *DMAHandle);
	}
	else if (Channel == DMA_CHANNEL_TX)
	{
		__HAL_LINKDMA(UartHandle, hdmatx, *DMAHandle);
	}
}

/******************************************************************
函数名称: void BSP_DMASpiRxBindStream
功能描述: DMA-SPI流接收通道绑定
参数输入: DMA_HandleTypeDef		*DMAHandle	DMA句柄
		  DMA_Channel_TypeDef	*DMAChannel	DMA流通道
		  SPI_HandleTypeDef		*SpiHandle	SPI句柄
		  uint8_t				Channel		DMA连接通道
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_DMASpiBindStream(DMA_HandleTypeDef *DMAHandle, DMA_Channel_TypeDef *DMAChannel, SPI_HandleTypeDef *SpiHandle, uint8_t Channel)
{
	/* 实例化 */
	DMAHandle->Instance = DMAChannel;
	if (HAL_DMA_Init(DMAHandle) != HAL_OK)
	{
		Error_Handler();
	}
	
	/* 连接DMA通道 启动DMA */
	if (Channel == DMA_CHANNEL_RX)
	{
		__HAL_LINKDMA(SpiHandle, hdmarx, *DMAHandle);
	}
	else if (Channel == DMA_CHANNEL_TX)
	{
		__HAL_LINKDMA(SpiHandle, hdmatx, *DMAHandle);
	}
}
