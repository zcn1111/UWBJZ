#ifndef __BLE_BASESTATION_H
#define __BLE_BASESTATION_H

/* 引用头文件 */
#include "stm32l4xx.h"

#include "BSP.h"
#include "g_GlobalValDef.h"
#include "BLE_Comm.h"

/******* 宏定义 *******/


/******* 状态定义 *******/

/******* 数据结构体定义 *******/
/* 蓝牙通信参数 */
#pragma pack(push, 1)	//按1字节对齐
//typedef struct
//{
//} BLEDataPara_t;
#pragma pack(pop)		//对齐恢复

/* 新增蓝牙协议 */


/******* 外部变量声明 *******/
extern uint8_t			g_BLEDataBuf[BLE_BUFSIZE];
extern uint8_t 		g_BLECodeType;
extern uint8_t			g_BLEUploadCard;
extern uint32_t		g_tmrBleUploadDelay;


/******* 外部函数声明 *******/
BLEResult BLE_AnalysisMobileCommProtocol(uint8_t *CodeType, StationPara_t *Para, WorkTime_t *WorkTime, CardInfoTable_t CardTable);
BLEResult BLE_SendCardinfo(CardInfoTable_t CardTable);

#endif /* __BLE_BASESTATION_H */

