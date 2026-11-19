/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RYDW_POSTION_H
#define __RYDW_POSTION_H

#include "GPS_Comm.h"
#include "RYDW_Def.h"
#include "RYDW_CardInfo.h"

#include "stdint.h"

//#include "LORA_Comm.h"
//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/

/******* 宏定义 *******/

/******* 外部变量定义 *******/
extern BaseData_t g_PostionTable;

extern uint8_t g_PostionOrder;
extern uint8_t g_PUNCOORAddnumb; /*发送打点数据条数*/
extern uint8_t g_PostionMode;			//打点模式
typedef struct
{
	uint32_t ReadDataAddr;
	uint32_t Readaddr;
	uint32_t Readset;
	uint8_t  ReadEmptyFlag;	
} ReadAddrPar;
extern ReadAddrPar  g_ReadAddrPar[20];
extern uint32_t g_ReadDataAddr;          /* 读C3的页地址 agj */
extern UserCardID_t g_BaseID;
extern uint8_t g_DispPage;
extern uint8_t g_CursorPos;              //光标位置
extern uint32_t g_SerialNum;
extern uint32_t g_SerSerialNum;
/******* 外部函数定义 *******/

bool RYDW_GeneratePostionRecord(void);
void RYDW_PostionDisp(BaseData_t PosInfo, uint8_t Ord);
void RYDW_PostionProgress(void);









#endif /* __RYDW_POSTION_H */
