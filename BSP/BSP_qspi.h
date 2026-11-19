#ifndef __BSP_QSPI_H
#define __BSP_QSPI_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"

/******* 宏定义 *******/
#define QSPI_FLASHSIZE_32MB		(POSITION_VAL(0x02000000) - 1)

/******* GPIO管脚名称定义 *******/

/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
extern QSPI_HandleTypeDef 	hQSPI;		//QSPI句柄
extern QSPI_CommandTypeDef	hQSPICmd;	//QSPI命令控制句柄


/******* 外部函数声明 *******/
/* 初始化 */
void BSP_SetupQspiPara(QSPI_HandleTypeDef *QspiHandle, uint32_t ClkPre, uint32_t ClkMode, uint32_t FlashSize, uint32_t FlashID);
void BSP_QspiBindInstance(QSPI_HandleTypeDef *QspiHandle, QUADSPI_TypeDef *Inst);

void QSPI_CSEnable(void);
void QSPI_CSDisable(void);

void QSPI_SetupCmdHandle(uint32_t InstMode, uint32_t AddrMode, uint32_t AddrSize, uint32_t DataMode);
void QSPI_SendCmd(uint32_t Inst, uint32_t Addr, uint32_t DummyCyc);
void QSPI_Transmit(uint8_t *Buf, uint32_t Len);
void QSPI_Receive(uint8_t *Buf, uint32_t Len);

void QSPI_DirectCmd(uint32_t Instruction, uint32_t Address, uint32_t DummyCycles, uint32_t InstructionMode, uint32_t AddressMode, uint32_t AddressSize, uint32_t DataMode);
void QSPI_Send_CMD(uint32_t Instruction, uint32_t Address, uint32_t DummyCycles, uint32_t InstructionMode, uint32_t AddressMode, uint32_t AddressSize, uint32_t DataMode);

#endif

