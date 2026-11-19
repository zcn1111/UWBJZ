/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RYDW_CARDINFO_H
#define __RYDW_CARDINFO_H

//#include "bsp_timer.h"
#include "stdint.h"
//#include "LORA_Comm.h"

#include "RYDW_Def.h"

//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/


/******* 宏定义 *******/

/******* 变量结构体定义 *******/


/* 定位卡列表 */
typedef struct
{
	CardData_t	Info[RYDW_CARDTABLE_MAXNUM];		//定位卡信息
	uint8_t		CardRecvCnt[RYDW_CARDTABLE_MAXNUM];	//定位卡通信接收数据编号
	int16_t		Rssi[RYDW_CARDTABLE_MAXNUM];		//定位卡信号强度
	int8_t		Snr[RYDW_CARDTABLE_MAXNUM];			//定位卡信噪比
	uint8_t		CardNbr;							//定位卡设备数
} CardInfoTable_t;



/******* 外部变量定义 *******/

extern CardInfoTable_t	g_CardTable;		//定位卡信息表
extern uint8_t			g_CardOfflineNum;	//离线卡数量


/******* 外部函数定义 *******/
/* 初始化 */
void RYDW_InitCardInfo(CardData_t *CardInfo);
void RYDW_InitCardTable(CardInfoTable_t *CardTable);

/* 状态检查 */
bool RYDW_IsCardIDSame(CardData_t SrcCard, CardData_t DestCard);
bool RYDW_IsCardAlarm(CardInfoTable_t CardTable);
bool RYDW_IsCardPosition(CardInfoTable_t CardTable);
bool RYDW_IsAllRecvdElecFence(CardInfoTable_t CardTable, StationPara_t Para);
bool RYDW_IsAllRecvdWorkTime(CardInfoTable_t CardTable, StationPara_t Para);
bool RYDW_IsAllRecvdLedSetup(CardInfoTable_t CardTable, StationPara_t Para);

/* 定位卡操作 */
void RYDW_CopyCardInfo(CardData_t SrcCard, CardData_t *DestCard);
void RYDW_UpdateCardInfoTable(CardData_t NowCardData, uint8_t RecvCnt, CardInfoTable_t *CardTable);
uint8_t RYDW_CheckCardOffline(uint8_t SendCnt, CardInfoTable_t *CardTable);


#endif /* __RYDW_CARDINFO_H */
