#ifndef __BSP_CAN_H
#define __BSP_CAN_H


/* 引用头文件 */
#include "Bsp.h"
//#include "stm32h7xx_hal.h"


/******* GPIO管脚名称定义 *******/

/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
extern FDCAN_HandleTypeDef		hCAN1;

extern FDCAN_RxHeaderTypeDef	CANRxHeader;
extern FDCAN_TxHeaderTypeDef	CANTxHeader;
extern uint8_t					CANRxFlag;

/******* 外部函数声明 *******/
void Can1Init(void);

#endif

