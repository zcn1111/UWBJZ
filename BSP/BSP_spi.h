#ifndef __BSP_SPI_H
#define __BSP_SPI_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"


/******* GPIO管脚名称定义 *******/

/******* 数据结构体定义 *******/
#define SPI_CLOCK_MODE_0		0x00	//SPI时钟模式0
#define SPI_CLOCK_MODE_1		0x01	//SPI时钟模式1
#define SPI_CLOCK_MODE_2		0x02	//SPI时钟模式2
#define SPI_CLOCK_MODE_3		0x03	//SPI时钟模式3

/******* 外部变量声明 *******/
extern SPI_HandleTypeDef	hSPI1;		//SPI1句柄变量
extern SPI_HandleTypeDef	hSPI2;		//SPI2句柄变量
extern SPI_HandleTypeDef	hSPI3;		//SPI3句柄变量

extern uint8_t *SpiTxBuf;
extern uint8_t *SpiRxBuf;

extern uint8_t g_SpiRecvCplt;
extern uint8_t g_SpiSendCplt;
extern uint8_t SpiCSEnable;
extern uint8_t SpiTxAdd;

/******* 外部函数声明 *******/
void Spi1Init(void);
void SPI3_Init(void);
void BSP_SpiInitPara(SPI_HandleTypeDef *SpiHandle);
void BSP_SpiSetupClkPara(SPI_HandleTypeDef *SpiHandle, uint32_t ClkPre, uint32_t ClkMode);
void BSP_SpiSetupDataPara(SPI_HandleTypeDef *SpiHandle, uint32_t Mode, uint32_t Direction, uint32_t DataSize, uint32_t FirstBit);
void BSP_SpiSetupNSSPara(SPI_HandleTypeDef *SpiHandle, uint32_t NssMode, uint32_t NssPhMode);
void BSP_SpiSetupCrcPara(SPI_HandleTypeDef *SpiHandle, uint32_t CRCEn, uint32_t CRCPoly, uint32_t CRCLength);
void BSP_SpiBindInstance(SPI_HandleTypeDef *SpiHandle, SPI_TypeDef *Inst);


uint16_t SpiWrite(SPI_HandleTypeDef *hspi, uint8_t *WBuf, uint16_t WByte);
uint16_t SpiRead(SPI_HandleTypeDef *hspi, uint8_t *RBuf, uint16_t RByte);


void MX_SPI4_Init(void);

#endif

