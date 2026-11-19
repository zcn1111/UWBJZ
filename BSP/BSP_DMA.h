#ifndef __BSP_DMA_H
#define __BSP_DMA_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"


/******* 数据定义 *******/
#define DMA_CHANNEL_RX		0x00
#define DMA_CHANNEL_TX		0x01

/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
extern DMA_HandleTypeDef	hDMA_Usart1Rx;	//DMA_USART1-Rx
extern DMA_HandleTypeDef	hDMA_Usart2Rx;	//DMA_USART2-Rx
extern DMA_HandleTypeDef	hDMA_Usart3Rx;	//DMA_USART3-Rx
extern DMA_HandleTypeDef	hDMA_Uart4Rx;	//DMA_UART4-Rx
extern DMA_HandleTypeDef	hDMA_Uart5Rx;	//DMA串口5接收句柄


extern DMA_HandleTypeDef	hDMA_Spi1Rx;	//DMA2-SPI1Rx
extern DMA_HandleTypeDef	hDMA_Spi1Tx;	//DMA2-SPI1Tx


/******* 外部函数声明 *******/
/* DMA初始化 */
void DMA1Init(void);
void DMA2Init(void);


void BSP_DMAInitRxPara(DMA_HandleTypeDef *DMAHandle, uint32_t Request, uint32_t Mode, uint32_t Priority);
void BSP_DMAInitTxPara(DMA_HandleTypeDef *DMAHandle, uint32_t Request, uint32_t Mode, uint32_t Priority);
void BSP_DMASetupFifoPara(DMA_HandleTypeDef *DMAHandle, uint32_t FIFOMode, uint32_t FIFOThre);
void BSP_DMASetupMemoryPara(DMA_HandleTypeDef *DMAHandle, uint32_t MemInc, uint32_t MemAlign);
void BSP_DMASetupPeriphPara(DMA_HandleTypeDef *DMAHandle, uint32_t PerInc, uint32_t PerAlign);
void BSP_DMAUartBindStream(DMA_HandleTypeDef *DMAHandle, DMA_Channel_TypeDef *DMAChannel, UART_HandleTypeDef *UartHandle, uint8_t Channel);
void BSP_DMASpiBindStream(DMA_HandleTypeDef *DMAHandle, DMA_Channel_TypeDef *DMAChannel, SPI_HandleTypeDef *SpiHandle, uint8_t Channel);


//void SetupDMA_Fsmc(void);

#endif

