/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RYDW_DISPLAY_H
#define __RYDW_DISPLAY_H

#include "BSP_timer.h"

#include "stdint.h"
#include "RYDW_StationCard.h"
//#include "LORA_Comm.h"
//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/

/******* 宏定义 *******/


/******* 外部变量定义 *******/
extern uint32_t g_DispRefreshDelay;		//刷新时间
extern uint32_t g_DispNoOperationTimer;	//无操作定时器
extern uint8_t g_SeleWorkMode;          //工作模式标记=0正常 =1打点
typedef struct
{
    uint8_t RouLin;          //=1上行;=2下行;=3单线
	uint8_t run;             //=0运行；=1暂停
}BribeType;
extern BribeType g_BribeFlag;
/******* 外部函数定义 *******/

void RYDW_DispStationStatus(StationStatus_t *Sta, uint8_t PosX, uint8_t PosY);
void RYDW_DispCardInfo(CardInfoTable_t *CardTable, uint8_t PosX, uint8_t PosY);
void RYDW_DispPosition(uint8_t PosX, uint8_t PosY);
void RYDW_DispTime(HexDateTime_t NowTime, uint8_t PosX, uint8_t PosY);
void RYDW_DispBattery(float BatVal);

void RYDW_DispCardDetail(CardInfoTable_t Cardinfo, uint8_t Order);

void RYDW_DispString(uint8_t Row, uint8_t Col, char *Str);
void RYDW_DispClear(void);
void RYDW_DispGPS(uint8_t  mark);
void RYDW_DispSignal(uint8_t  num);
void  SeleWorkMode(uint8_t flag,uint8_t color);
void  ShowLinItrm(uint8_t flag);
void  ShowBribeRun(uint8_t flag);



#endif /* __RYDW_DISPLAY_H */
