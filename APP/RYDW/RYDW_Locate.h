/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RYDW_LOCATE_H
#define __RYDW_LOCATE_H

#include "GPS_Comm.h"

#include "stdint.h"

//#include "LORA_Comm.h"
//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/

/******* 宏定义 *******/

/******* 外部变量定义 *******/
extern uint8_t	g_LocationProg;
extern uint8_t  g_SelfContCORS;         //自收敛状态收到CORS模式
extern uint32_t g_LocationDispTimer;
extern uint32_t g_SendSignalDelay;
extern uint8_t  g_SendSignalFlag;
extern uint8_t  g_MarkWarnFlag;         //基准合法标记 =1不合法，=0合法

/******* 外部函数定义 *******/

uint8_t RYDW_LocateProcess(uint8_t NowProg, GpsLocationInfo_t *GpsInfo);
void RYDW_LocateDisplay(uint8_t NowProg, GpsLocationInfo_t *GpsInfo, uint32_t *DispTimer);
void SendNetSignal(void);
void RYDW_MarkJudg(void);




#endif /* __RYDW_LOCATE_H */
