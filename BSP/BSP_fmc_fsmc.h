#ifndef __BSP_FMC_FSMC_H
#define __BSP_FMC_FSMC_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"


/******* GPIO管脚名称定义 *******/

/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
/* NAND句柄 */
extern NAND_HandleTypeDef hNAND1;

/******* 外部函数声明 *******/
void Sram1Init(void);
void Nand1Init(void);

#endif

