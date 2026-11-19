#ifndef __BLE_BOARDCAST_H
#define __BLE_BOARDCAST_H

/* 引用头文件 */
#include "stm32h7xx.h"

#include "BSP.h"
#include "DTU_Comm.h"
#include "g_GlobalValDef.h"
#include "Sensor.h"

#include "BLE_Comm.h"

/******* 宏定义 *******/

/******* 状态定义 *******/

/******* 数据结构体定义 *******/

/* 新增蓝牙协议 */


/******* 外部变量声明 *******/
extern uint8_t			g_BLEBoardcastBuf[64];

extern TickTimerPara_t tmrBLEBoardcastTimer;	//蓝牙广播定时器


/******* 外部函数声明 *******/

BLEResult BLE_BoardCastProcess(BLEDataPara_t *BleData);


#endif /* __BLE_BOARDCAST_H */

