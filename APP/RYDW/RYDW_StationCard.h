/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RYDW_STATIONCARD_H
#define __RYDW_STATIONCARD_H

//#include "bsp_timer.h"
#include "stdint.h"
//#include "LORA_Comm.h"

#include "RYDW_Def.h"
#include "RYDW_CardInfo.h"

//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/


/******* 宏定义 *******/


/******* 变量结构体定义 *******/

void	RYDW_GenerateDiffData(void);

extern StationPara_t	g_WorkingPara;		//工作站运行参数

extern uint8_t			g_UploadRecord[1024];	//上传信息记录缓存
extern uint16_t		g_UploadRecordLen;		//上传信息记录长度



extern uint8_t			g_NewData;			//新数据

extern StationStatus_t	g_StationStatus;	//基站状态
extern uint8_t			g_DispRtkCnt;			

extern uint8_t          g_recvdiff;     //差分数据是否完整
extern uint8_t			g_RecvRTKCnt;			//接收RTK数据次数

extern uint32_t			g_RTKRecvIntervalTimer;	//RTK接收间隔时间
extern uint32_t		    g_LastIntervalTimer;	//上次接收间隔时间
/******* 外部函数定义 *******/

void RYDW_InitHeaderVar(LoraHeader_t *Header);
void RYDW_InitStationWorkingPara(StationPara_t *Para);
void RYDW_InitElecFence(EFence_t *Fence);
void RYDW_InitWorkTime(WorkTime_t *Stime);


void RYDW_StationCommProcess(StationPara_t *Para, CardInfoTable_t *CardTable);

void RYDW_GenerateCardRecord(CardInfoTable_t CardTable, uint8_t *RecordBuf, uint16_t *RecordLen);
void RYDW_GenerateBaseStationPosRecord(uint8_t *RecordBuf, uint16_t *RecordLen,uint8_t num);





#endif /* __RYDW_STATIONCARD_H */
