/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: RYDW_CardInfo.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 定位卡信息处理
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
//#include "ebyte_core.h"
#include "RYDW_Def.h"

#include "RYDW_CardInfo.h"
#include "Lora_Comm.h"

/********* 变量定义 **********/

CardInfoTable_t	g_CardTable;		//定位卡信息表
uint8_t			g_CardOfflineNum;	//离线卡数量



/************************************** 卡数据处理初始化函数 ************************************************/


/******************************************************************
函数名称: void RYDW_InitCardInfo
功能描述: 初始化定位卡数据
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitCardInfo(CardData_t *CardInfo)
{
	uint8_t i;
	
	for (i=0; i<7; i++)
	{
		CardInfo->ID.array[i] = 0x00;
	}
	CardInfo->Latitude = 0;
	CardInfo->Longitude = 0;
	CardInfo->PosQuality = 0;
	memset(CardInfo->Altitude, 0x00, 3);
	CardInfo->Alram.all = 0;
	CardInfo->BatVoltage = 0;
	CardInfo->AlramType.all = 0;
	CardInfo->FenceOrder = 0;
	CardInfo->WordTimeOrder = 0;
	CardInfo->LedOrder = 0;
	memset(CardInfo->Reserver, 0x00, 6);
}

/******************************************************************
函数名称: void RYDW_InitCardTable
功能描述: 初始化定位卡信息表
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitCardTable(CardInfoTable_t *CardTable)
{
	uint8_t i;
	
	for (i=0; i<RYDW_CARDTABLE_MAXNUM; i++)
	{
		RYDW_InitCardInfo(&CardTable->Info[i]);
		CardTable->CardRecvCnt[i] = 0;
		CardTable->Rssi[i] = 0;
		CardTable->Snr[i] = 0;
	}

}

/************************************** 卡数据处理函数 ************************************************/

/******************************************************************
函数名称: bool RYDW_IsCardIDSame
功能描述: 检查定位卡ID是否相同
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
bool RYDW_IsCardIDSame(CardData_t SrcCard, CardData_t DestCard)
{
	bool bret;
	uint8_t i;
	
	for (i=0; i<7; i++)
	{
		if (SrcCard.ID.array[i] != DestCard.ID.array[i])
			return false;
	}
	
	return true;
}

/******************************************************************
函数名称: bool RYDW_IsCardAlarm
功能描述: 检查定位卡ID是否报警
参数输入: 
参数输出: 
返 回 值: bool false=定位卡无报警;true=定位卡有报警
其    他: 无
******************************************************************/
bool RYDW_IsCardAlarm(CardInfoTable_t CardTable)
{
	uint8_t i;
	
	for (i=0; i<CardTable.CardNbr; i++)
	{
		/* 有定位卡报警 */
		if ((g_CardTable.Info[i].Alram.bit.Alram == 1) || \
			(g_CardTable.Info[i].Alram.bit.SOS == 1) || \
			(g_CardTable.Info[i].Alram.bit.LowPower == 1))
		{
			return true;		//报警信息 返回报警信息
		}
	}
	
	return false;
}

/******************************************************************
函数名称: bool RYDW_IsCardPosition
功能描述: 检查定位卡ID是否返回定位信息
参数输入: 
参数输出: 
返 回 值: bool false=无定位信息;true=有定位信息
其    他: 无
******************************************************************/
bool RYDW_IsCardPosition(CardInfoTable_t CardTable)
{
	uint8_t i;
	
	for (i=0; i<CardTable.CardNbr; i++)
	{
		/* 有定位信息 */
		if (CardTable.Info[i].Alram.bit.Position == 1)
		{
			return true;			//返回无定位信息
		}
	}
	
	return false;
}

/******************************************************************
函数名称: bool RYDW_IsAllRecvdElecFence
功能描述: 检查定位卡是否都接收了电子围栏信息
参数输入: 
参数输出: 
返 回 值: bool false=有未接收卡;true=全部接收
其    他: 无
******************************************************************/
bool RYDW_IsAllRecvdElecFence(CardInfoTable_t CardTable, StationPara_t Para)
{
	uint8_t i;
	uint8_t	ord;
	
	ord = Para.ElecFenceOrder;
	
	/* 检查接收过电子围栏信息 */
	if (ord == 0)
		return true;
	
	/* 检查电子围栏编号与接收卡编号是否一致 */
	for (i=0; i<CardTable.CardNbr; i++)
	{
		/* 判断卡在线 */
		if (CardTable.Info[i].Alram.bit.Online == 1)
		{
			/* 判断发送电子围栏编号和接收卡编号不一致 */
			if (CardTable.Info[i].FenceOrder != Para.ElecFenceOrder)
				return false;			//返回有未接收卡信息
		}
	}
	
	return true;
}

/******************************************************************
函数名称: bool RYDW_IsAllRecvdWorkTime
功能描述: 检查定位卡是否都接收了施工时间信息
参数输入: 
参数输出: 
返 回 值: bool false=有未接收卡;true=全部接收
其    他: 无
******************************************************************/
bool RYDW_IsAllRecvdWorkTime(CardInfoTable_t CardTable, StationPara_t Para)
{
	uint8_t i;
	uint8_t	ord;
	
	ord = Para.WorkTimeOrder;
	
	/* 检查接收过施工时间信息 */
	if (ord == 0)
		return true;
	
	/* 检查施工时间编号与接收卡编号是否一致 */
	for (i=0; i<CardTable.CardNbr; i++)
	{
		/* 判断卡在线 */
		if (CardTable.Info[i].Alram.bit.Online == 1)
		{
			/* 判断发送施工时间编号和接收卡编号不一致 */
			if (CardTable.Info[i].WordTimeOrder != Para.WorkTimeOrder)
				return false;			//返回有未接收卡信息
		}
	}
	
	return true;
}

/******************************************************************
函数名称: bool RYDW_IsAllRecvdLedSetup
功能描述: 检查定位卡是否都接收了指示灯设置
参数输入: 
参数输出: 
返 回 值: bool false=有未接收卡;true=全部接收
其    他: 无
******************************************************************/
bool RYDW_IsAllRecvdLedSetup(CardInfoTable_t CardTable, StationPara_t Para)
{
	uint8_t i;
	uint8_t	ord;
	
	ord = Para.LedSetupOrder;
	
	/* 检查接收过指示灯配置信息 */
	if (ord == 0)
		return true;
	
	/* 检查电子围栏编号与接收卡编号是否一致 */
	for (i=0; i<CardTable.CardNbr; i++)
	{
		/* 判断卡在线 */
		if (CardTable.Info[i].Alram.bit.Online == 1)
		{
			/* 判断发送电子围栏编号和接收卡编号不一致 */
			if (CardTable.Info[i].LedOrder != Para.LedSetupOrder)
				return false;			//返回有未接收卡信息
		}
	}
	
	return true;
}

/******************************************************************
函数名称: void RYDW_CopyCardInfo
功能描述: 复制(更新)定位卡信息
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_CopyCardInfo(CardData_t SrcCard, CardData_t *DestCard)
{
	uint8_t i;
	
	for (i=0; i<7; i++)
	{
		DestCard->ID.array[i] = SrcCard.ID.array[i];
	}
	DestCard->Latitude = SrcCard.Latitude;
	DestCard->Longitude = SrcCard.Longitude;
	DestCard->PosQuality = SrcCard.PosQuality;
	memcpy(DestCard->Altitude, SrcCard.Altitude, 3);
	DestCard->Alram.all = SrcCard.Alram.all;
	DestCard->BatVoltage = SrcCard.BatVoltage;
	DestCard->AlramType.all = SrcCard.AlramType.all;
	DestCard->FenceOrder = SrcCard.FenceOrder;
	DestCard->WordTimeOrder = SrcCard.WordTimeOrder;
	DestCard->LedOrder = SrcCard.LedOrder;
	memcpy(DestCard->Reserver, SrcCard.Reserver, 6);
}

/******************************************************************
函数名称: void RYDW_UpdateCardInfoTable
功能描述: 更新定位卡信息表
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_UpdateCardInfoTable(CardData_t NowCardData, uint8_t RecvCnt, CardInfoTable_t *CardTable)
{
	uint16_t i;
	char str_cardid[32]={0};
	
	/* 检查定位卡信息表 */
	for (i=0; i<CardTable->CardNbr; i++)
	{
		/* 接收定位卡数据和卡列表中数据相同 更新列表中卡信息 */
		if (RYDW_IsCardIDSame(CardTable->Info[i], NowCardData) == true)
		{
			NowCardData.Alram.bit.Online = 1;				//接收到定位卡信息 更新定位卡在线
			CardTable->CardRecvCnt[i] = RecvCnt;			//更新定位卡接收编号
			RYDW_CopyCardInfo(NowCardData, &CardTable->Info[i]);
			Arr2AscStr(CardTable->Info[i].ID.array, 7, 0x00, str_cardid);
			debug_printf("CardID=%s, Quality=%d, Lat:%2.7f, Lon:%3.7f\r\n", str_cardid, g_CardTable.Info[i].PosQuality, (double)g_CardTable.Info[i].Latitude / 10000000, (double)g_CardTable.Info[i].Longitude / 10000000);
			if (g_CardTable.Info[i].Alram.bit.Alram == 1)
			{
				/* 有报警信息 显示报警类型 */
				debug_printf("AlarmType=%04X\r\n", g_CardTable.Info[i].AlramType.all);
			}
			break;
		}
	}
	/* 不在信息表里 新定位卡 添加到列表中 */
	if (i == CardTable->CardNbr)
	{
		NowCardData.Alram.bit.Online = 1;			//定位卡在线
		CardTable->CardRecvCnt[i] = RecvCnt;
		RYDW_CopyCardInfo(NowCardData, &CardTable->Info[CardTable->CardNbr]);		//增加
		CardTable->CardNbr++;
		Arr2AscStr(NowCardData.ID.array, 7, 0x00, str_cardid);
		debug_printf("New Card! ID=%s\r\n", str_cardid);
	}
}

/******************************************************************
函数名称: uint8_t RYDW_CheckCardOffline
功能描述: 检查离线卡
参数输入: 
参数输出: 
返 回 值: uint8_t 离线定位卡个数
其    他: 无
******************************************************************/
uint8_t RYDW_CheckCardOffline(uint8_t SendCnt, CardInfoTable_t *CardTable)
{
	uint16_t i;
	uint8_t diff;
	uint8_t offline_num=0;
	
	for (i=0; i<CardTable->CardNbr; i++)
	{
		/* 计算差 */
		if (SendCnt >= CardTable->CardRecvCnt[i])
		{
			diff = SendCnt - CardTable->CardRecvCnt[i];
		}
		else
		{
			diff = 0xFF - CardTable->CardRecvCnt[i] + SendCnt;
		}
		
		/* 连续3个都没有接收到 判断设备离线 */
		if (diff > 3)
		{
			CardTable->Info[i].Alram.bit.Online = 0;
			CardTable->Info[i].Alram.bit.Alram = 0;		//设备离线清除报警状态
			CardTable->Rssi[i] = 0;
			CardTable->Snr[i] = 0;
			offline_num++;
		}
	}
	
	return offline_num;
}





