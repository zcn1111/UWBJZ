/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: RYDW_StationCard.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 基站与定位卡通信
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "LORA_Comm.h"
#include "LORA_Base.h"
#include "RTK_Comm.h"
#include "GPS_Comm.h"
#include "CORS_Comm.h"

#if (LORA_USE_E22x)
#include "ebyte_core.h"
#elif (LORA_USE_RA01S)
#endif
#include "RYDW_StationCard.h"
#include "RYDW_CardInfo.h"
#include "Drv_W25Qxx_QSPI.h"
#include "g_GlobalValDef.h"
#include "DTU_BulkData.h"
#include "RYDW_Postion.h"
#include "IP2365.h"
#include "RYDW_Locate.h"
#include "app_dtu.h"
#include "RYDW_Init.h"




StationPara_t	g_WorkingPara;		//工作站运行参数

uint8_t			g_UploadRecord[1024];	//上传信息记录缓存
uint16_t		g_UploadRecordLen;		//上传信息记录长度
uint8_t			g_NewData;				//新数据标记
uint8_t			g_RecvRTKCnt;			//接收RTK数据次数
uint8_t			g_DispRtkCnt;
uint32_t		g_RTKRecvIntervalTimer;	//RTK接收间隔时间
uint32_t		g_LastIntervalTimer;	//上次接收间隔时间

StationStatus_t	g_StationStatus;		//基站状态
uint32_t		g_tmrTrainingWarning;	//来车报警定时器
uint8_t         g_recvdiff = 0;     //差分数据是否完整

/************************************** 基站和定位卡通信函数 ************************************************/

/******************************************************************
函数名称: void RYDW_InitHeaderVar
功能描述: 初始化Lora数据头变量参数
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitHeaderVar(LoraHeader_t *Header)
{
	Header->Type = 0x00;
	Header->DestOrder = 0x00;		//默认是基站
	Header->SrcOrder = 0x00;
	Header->PositionSta = 0;		//收敛标记=未收敛
	Header->WorkingSta = 0x00;		//上下道标记=无
	Header->TrainingSta = 0x00;		//来车标记=无
}

/******************************************************************
函数名称: void RYDW_InitStationWorkingPara
功能描述: 初始化工作站运行参数
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitStationWorkingPara(StationPara_t *Para)
{
	Para->WorkingSta = 0;
	Para->PositionSta = 0;
	Para->TrainingSta = 0;
	Para->AlramSta.all = 0;
	
	Para->Latitude = 0;
	Para->Longitude = 0;
	Para->Altitude = 0;
	
	Para->BatVoltage = 0;
	Para->BoardcastTimer = uwTick;
	Para->BoardcastCnt = 0;
	
	Para->ElecFenceOrder = 0;
	Para->WorkTimeOrder = 0;
	Para->LedSetupOrder = 0;
}

/******************************************************************
函数名称: void RYDW_InitElecFence
功能描述: 初始化电子围栏
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitElecFence(EFence_t *Fence)
{
	uint8_t i, j;
	
	/* 初始化围栏数据 */
	memset(Fence->Fence, 0x00, 123);//清数据
	
	/* 初始化围栏编号 */
	Fence->Order = 0;
}

/******************************************************************
函数名称: void RYDW_InitWorkTime
功能描述: 初始化施工时间
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_InitWorkTime(WorkTime_t *Stime)
{
	/* 初始化施工开始时间 */
	Stime->StartTime.Year = 0;
	Stime->StartTime.Month = 0;
	Stime->StartTime.Day = 0;
	Stime->StartTime.Hour = 0;
	Stime->StartTime.Minute = 0;
	Stime->StartTime.Second = 0;
	
	/* 初始化施工结束时间 */
	Stime->FinishTime.Year = 0;
	Stime->FinishTime.Month = 0;
	Stime->FinishTime.Day = 0;
	Stime->FinishTime.Hour = 0;
	Stime->FinishTime.Minute = 0;
	Stime->FinishTime.Second = 0;
	
	/* 初始化施工时间编号 */
	Stime->Order = 0;
	
	memset(Stime->Reserve, 0x00, 39);
}

/******************************************************************
函数名称: void RYDW_UpdateHeaderFromPara
功能描述: 从参数中更新数据头
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_UpdateHeaderFromPara(StationPara_t *Para, LoraHeader_t *Header)
{
	Header->PositionSta = Para->PositionSta;
	Header->WorkingSta = Para->WorkingSta;
	Header->TrainingSta = Para->TrainingSta;
}

/******************************************************************
函数名称: void RYDW_UpdateParaFromCardInfo
功能描述: 从数据头中更新运行参数
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_UpdateParaFromHeader(LoraHeader_t *Header, StationPara_t *Para)
{
	/* 更新来车标记和上下道 */
	Para->TrainingSta = Header->TrainingSta;
	Para->WorkingSta = Header->WorkingSta;	
}


/******************************************************************
函数名称: bool RYDW_IsWorkTimerOver
功能描述: 检查施工时间是否已结束
参数输入: HexDataTime_t	NowTime		当前时间
		 CstrTime_t		Stime		施工时间段
		 int32_t		DelaySec	延迟时间(秒)
参数输出: 
返 回 值: bool false=未结束;true=已结束
其    他: 无
******************************************************************/
bool RYDW_IsWorkTimerOver(HexDateTime_t NowTime, WorkTime_t Stime, int32_t DelaySec)
{
	uint32_t now_in_sec;
	uint32_t ctr_in_sec;
	
	if ((Stime.StartTime.Year == 0x00) && (Stime.StartTime.Month == 0x00))
	{
		return true;
	}
	now_in_sec = HexTime2ElapsedSecond(NowTime);
	ctr_in_sec = HexTime2ElapsedSecond(Stime.FinishTime) + DelaySec;
	if (now_in_sec <= ctr_in_sec)
	{
		return false;				//当前施工时间未结束
	}
	
	return true;
}	

/******************************************************************
函数名称: bool RYDW_IsNewConstructionTime
功能描述: 检查是否是新施工时间
参数输入: 
参数输出: 
返 回 值: bool false=已有施工时间;true=新施工时间
其    他: 无
******************************************************************/
bool RYDW_IsNewConstructionTime(WorkTime_t OldTime, WorkTime_t NewTime)
{
	/* 比较施工开始时间 */
	if (IsHexTimeSame(OldTime.StartTime, NewTime.StartTime) == true)
	{
		/* 比较施工结束时间 */
		if (IsHexTimeSame(OldTime.FinishTime, NewTime.FinishTime) == true)
		{
			return false;
		}
	}
	
	return true;
}


/******************************************************************
函数名称: void RYDW_ConvConstructionTime2DispStr
功能描述: 施工时间转换为显示字符串
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_ConvConstructionTime2DispStr(WorkTime_t Stime, char *Str, uint16_t* StrLen)
{
	uint16_t slen;
	uint16_t pos;
	
	char tmpstr[32];
	
	pos = 0;
	memset(tmpstr, 0x00, 32);
	sprintf(tmpstr, "StTime %02d/%02d/%02d %02d:%02d:%02d\r\n",	Stime.StartTime.Year,		\
																Stime.StartTime.Month,		\
																Stime.StartTime.Day,		\
																Stime.StartTime.Hour,		\
																Stime.StartTime.Minute,		\
																Stime.StartTime.Second);
	slen = strlen(tmpstr);
	memcpy(Str + pos, tmpstr, slen);
	pos += slen;
	
	memset(tmpstr, 0x00, 32);
	sprintf(tmpstr, "EdTime %02d/%02d/%02d %02d:%02d:%02d\r\n",	Stime.FinishTime.Year,		\
																Stime.FinishTime.Month,		\
																Stime.FinishTime.Day,		\
																Stime.FinishTime.Hour,		\
																Stime.FinishTime.Minute, 	\
																Stime.FinishTime.Second);
	slen = strlen(tmpstr);
	memcpy(Str + pos, tmpstr, slen);
	pos += slen;
	
	*StrLen = pos;
}

/******************************************************************
函数名称: void RYDW_GenerateCardRecord
功能描述: 生成差分数据记录
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_GenerateCardRecord(CardInfoTable_t CardTable, uint8_t *RecordBuf, uint16_t *RecordLen)
{
	uint16_t i, ord;
	BcdDateTime_t bcdtime;
	uint16_t pos;
	uint16_t reclen;
	uint8_t chksum_t;
	
	/* 生成记录数据头 */

	RecordBuf[0] = 0xFA;
	RecordBuf[1] = 0x00;
	/* 本机号 */
	RecordBuf[2] = g_MobileInFlash.mobileno[4][0];
	RecordBuf[3] = g_MobileInFlash.mobileno[4][1];
	RecordBuf[4] = g_MobileInFlash.mobileno[4][2];
	RecordBuf[5] = g_MobileInFlash.mobileno[4][3];
	RecordBuf[6] = g_MobileInFlash.mobileno[4][4];
	RecordBuf[7] = g_MobileInFlash.mobileno[4][5];	
//	RecordBuf[2] = 0x18;
//	RecordBuf[3] = 0x40;
//	RecordBuf[4] = 0x65;
//	RecordBuf[5] = 0x03;
//	RecordBuf[6] = 0x52;
//	RecordBuf[7] = 0x4F;
//	memcpy(RecordBuf + 2, g_DtuPara.ConPhoneNum.MobNum, 6);
	bcdtime = HexTime2BcdTime(g_SysTime);
	memcpy(RecordBuf + 8, (uint8_t*)&bcdtime, 6);
	RecordBuf[14] = 3;   //差分数据
	reclen = 15;
	
	/* 生成卡数据 */
	ord = 0;
	for (i=0; i<CardTable.CardNbr; i++)
	{
		/* 在线的定位卡发送 */
		if (CardTable.Info[i].Alram.bit.Online == 1)
		{
			pos = 15 + ord * 32;
			memcpy(RecordBuf + (pos + 0), CardTable.Info[i].ID.array, 5);
//			ConvUint16ToArray(CardTable.Info[i].ID.byte.UserID, RecordBuf + 5, M_BIG_ENDIAN);
			RecordBuf[pos + 5] = (uint8_t)(CardTable.Info[i].ID.byte.UserID & 0xFF);						//用户ID按小端模式上传
			RecordBuf[pos + 6] = (uint8_t)(CardTable.Info[i].ID.byte.UserID >> 8);
			ConvUint32ToArray(CardTable.Info[i].Latitude, RecordBuf + (pos + 7), M_LITTLE_ENDIAN);
			ConvUint32ToArray(CardTable.Info[i].Longitude, RecordBuf + (pos + 11), M_LITTLE_ENDIAN);
			RecordBuf[(pos + 15)] = CardTable.Info[i].PosQuality;
			memcpy(RecordBuf + (pos + 16), CardTable.Info[i].Altitude, 3);
			RecordBuf[(pos + 19)] = CardTable.Info[i].Alram.all;
			RecordBuf[(pos + 20)] = CardTable.Info[i].BatVoltage;
			ConvUint16ToArray(CardTable.Info[i].AlramType.all, RecordBuf + (pos + 21), M_LITTLE_ENDIAN);
			RecordBuf[(pos + 23)] = CardTable.Info[i].FenceOrder;
			RecordBuf[(pos + 24)] = CardTable.Info[i].WordTimeOrder;
			RecordBuf[(pos + 25)] = CardTable.Info[i].LedOrder;
			memcpy(RecordBuf + (pos + 26), CardTable.Info[i].Reserver, 6);		//其他6个预留字节
			reclen += 32;
			ord++;
		}
	}
//	RecordBuf[22] = reclen << 8;
//	RecordBuf[23] = reclen & 0xff;
	chksum_t = CheckSum(reclen, RecordBuf);
	RecordBuf[reclen] = chksum_t;
	tmpsendDataNum = reclen + 1;
//	HAL_UART_Transmit(&hUART4, RecordBuf, reclen + 1, 1000);		//转发RTK差分数据给GPS模块
//	g_DTUSendDataDelay = uwTick;
	g_FileSendDataToDtu++;
	*RecordLen = reclen + 1;
}

/******************************************************************
函数名称: void RYDW_GenerateBaseStationPosRecord(uint8_t *RecordBuf, uint16_t *RecordLen,uint8_t num)
功能描述: 生成基站定位数据
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void RYDW_GenerateBaseStationPosRecord(uint8_t *RecordBuf, uint16_t *RecordLen,uint8_t num)
{
	BcdDateTime_t bcdtime;
	uint16_t len;
	uint8_t chksum_t,i;
	/* 生成记录数据头 */
	RecordBuf[0] = 0xFA;
	RecordBuf[1] = 0x00;
	RecordBuf[2] = 2;                                            /*基站数据*/		
	RecordBuf[3] = g_MobileInFlash.mobileno[4][0];
	RecordBuf[4] = g_MobileInFlash.mobileno[4][1];
	RecordBuf[5] = g_MobileInFlash.mobileno[4][2];
	RecordBuf[6] = g_MobileInFlash.mobileno[4][3];
	RecordBuf[7] = g_MobileInFlash.mobileno[4][4];
	RecordBuf[8] = g_MobileInFlash.mobileno[4][5];		
	memcpy(RecordBuf + 9, g_IDInFlash.IDnum, 7);                /*ID号*/	
	RecordBuf[16] = 0x02;                                       /*基站*/	
	bcdtime = HexTime2BcdTime(g_SysTime);
	memcpy(RecordBuf + 17, (uint8_t*)&bcdtime, 6);              /*时间*/
	memset(RecordBuf+23,0x00,7);                                /*预留*/	
	RecordBuf[30] = (32*num)&0xFF;                              /*数据长度*/
	RecordBuf[31] = (32*num>>8)&0xFF;	
	/* 生成定位数据 */
	for(i=0;i<num;i++)
	{
		RecordBuf[32] = g_SysTime.Year;
		RecordBuf[33] = g_SysTime.Month;
		RecordBuf[34] = g_SysTime.Day;
		RecordBuf[35] = g_SysTime.Hour;
		RecordBuf[36] = g_SysTime.Minute;
		RecordBuf[37] = g_SysTime.Second;
		ConvUint32ToArray(g_gpsco.g_Latitude,RecordBuf + 38,M_LITTLE_ENDIAN);    //小端模式  //纬度
		ConvUint32ToArray(g_gpsco.g_Longitude,RecordBuf + 42,M_LITTLE_ENDIAN);   //小端模式  //经度
		RecordBuf[46] = g_GPSInfo.uMode;                                         //定位质量
		RecordBuf[47] = (uint8_t)((g_gpsco.g_Altitude >> 16) & 0xFF);            //大端
		RecordBuf[48] = (uint8_t)((g_gpsco.g_Altitude >> 8) & 0xFF);
		RecordBuf[49] = (uint8_t)(g_gpsco.g_Altitude & 0xFF);   
		RecordBuf[50] = 0x00;
		if(g_IP2365Init == 0x01 || g_MainVal > 7.0)
			RecordBuf[50] |= 0x10;	                                             /*充电标记*/
		if(g_MarkWarnFlag == 1)
			RecordBuf[50] |= 0x20;	                                             /*基站CORS/基准未设标记*/
		RecordBuf[51] = (uint8_t)(g_fBatVal*10);                                 /*电池电量*/	
		RecordBuf[52] = 0x00;                                                    /*报警类型*/
		RecordBuf[53] = 0x00;		
		RecordBuf[54] = 0x00;	                                                 //电子围栏编
		RecordBuf[55] = g_gpsco.g_Speed&0xFF;	//速度
		RecordBuf[56] = (g_gpsco.g_Speed>>8)&0xFF;									
		RecordBuf[57] = g_SatelliteNum;					//星数
		RecordBuf[58] = 0x00;							//区域类型
		memset(RecordBuf + 59, 0, 5);		//其他6个预留字节
	}
	chksum_t = CheckSum(64, RecordBuf);
	RecordBuf[64] = chksum_t;	
	RecordBuf[65] = g_SerialNum&0xFF;//序列号 
	RecordBuf[66] = (g_SerialNum>>8)&0xFF;//序列号 
	RecordBuf[67] = (g_SerialNum>>16)&0xFF;//序列号 
	RecordBuf[68] = (g_SerialNum>>24)&0xFF;//序列号 
	tmpsendDataNum = 69;
	sendsocketid =0;
//	HAL_UART_Transmit(&hUART4, RecordBuf, 69, 1000);		//转发RTK差分数据给GPS模块
//	g_DTUSendDataDelay = uwTick;
	g_FileSendDataToDtu++;
	if(g_SerialNum < 0xFFFFFFFF)
		g_SerialNum++;
	else
		g_SerialNum	= 0;	  
}

/******************************************************************
函数名称: void RYDW_HeaderProcess
功能描述: 数据头处理
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
void RYDW_HeaderPrcoess(LoraHeader_t *RecvHeader, LoraHeader_t *SendHeader, StationPara_t *Para)
{
	/* 非报警状态 检查定位卡发上下道标记和来车标记 */
	if (Para->BoardcastType.Emergent == 0)
	{
		g_tmrTrainingWarning = uwTick;
		RYDW_UpdateParaFromHeader(RecvHeader, Para);
		if ((Para->WorkingSta != 0) || (Para->TrainingSta != 0))
		{
			Para->BoardcastType.Emergent = 1;
		}
	}
	
	/* 检查当前上下道和来车标记 */
	if (Para->BoardcastType.Emergent == 1)
	{
		if (JudgeMilSecondDelayInPro(g_tmrTrainingWarning, 180000) == true)
		{
			/* 等待3分钟时间已过 清除当前状态 */
			g_tmrTrainingWarning = uwTick;
			Para->BoardcastType.Emergent = 0;
			if (Para->WorkingSta != 0) Para->WorkingSta = 0;
			if (Para->TrainingSta != 0) Para->TrainingSta = 0;
			SendHeader->WorkingSta = 0;
			SendHeader->TrainingSta = 0;
		}
		else
		{
			/* 连续发送3分钟 */
			Para->BoardcastType.Emergent = 1;
			Para->BoardcastCnt = 0;				//每次都发送
			RYDW_UpdateHeaderFromPara(Para, SendHeader);
		}
	}
}

/******************************************************************
函数名称: void RYDW_CardsProcess
功能描述: 卡信息处理
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_CardsProcess(CardInfoTable_t *CardTable, StationPara_t *Para)
{
	uint8_t i;
	bool receipt=true;
}

/******************************************************************
函数名称: void RYDW_RecvInfoProcess
功能描述: 接收数据信息处理
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_RecvInfoProcess(uint8_t TypeCode, uint8_t *InfoBuf, uint16_t BufLen, StationPara_t *Para)
{
	uint8_t e_ord;
	uint8_t i;
	uint16_t e_pos;
}


/******************************************************************
函数名称: void RYDW_StationCommProcess
功能描述: 
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
void RYDW_StationCommProcess(StationPara_t *Para, CardInfoTable_t *CardTable)
{
	LoraResult	loraret;
	uint8_t		i, j;
	
	RtkResult 	rtkret=RTKRET_NODATA_ERR;
	uint16_t	rtk_len;
	uint16_t	rtk_difflen=0;
	uint8_t		recvdiff;
	uint32_t	rtk_interval;
	
	GpsResult	gpsret=GPSRET_NODATA_ERR;
	uint16_t	gps_nmea_len;
	
	CorsResult	corsret=CORSRET_NODATA_ERR;
	uint16_t	cors_len=0;
	
	uint8_t		cardnum;
	uint8_t		recvcnt;
	
	uint16_t displen;
	
	/* 接收处理差分数据 */
	recvdiff = 0;
	/* 测试模式 */
//	if (WorkMode == 1)
//	{
//		memset(g_RtkDiffBuf, 0x00, RTK_DIFF_BUFSIZE);
////		memcpy(g_RtkDiffBuf+2, TestDiffBuf, 241);
//		g_RtkDiffBuf[0] = 0x01;
//		g_RtkDiffBuf[1] = 0x00;
//		g_RtkDiffLen = 237;
//		Para->PositionSta = 2;
//		Para->BoardcastType.Location = 1;
//		g_RTKRecvIntervalTimer = uwTick;
//		if (JudgeMilSecondDelayInPro(Para->BoardcastTimer, RYDW_BOARDCAST_INTERVAL * 1000) == true)
//		{
//			Para->BoardcastTimer = uwTick;
//			g_RecvRTKCnt = RYDW_BOARDCAST_INTERVAL;
//			recvdiff = 1;	
//		}
//		
//		corsret = CORS_RecvBuf(g_CorsRecvBuf, &cors_len);
//		memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
//	}
	
	/* 正常工作模式 */
	if (WorkMode == 0)
	{
//		if (g_ModeInFlash.type == 1)
//		{
//			/* 直接接收CORS数据 */
//			memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
//			cors_len = 0;
//			corsret = CORS_RecvBuf(g_CorsRecvBuf, &cors_len);
//			if (corsret == CORSRET_OK && strstr((char*)g_CorsRecvBuf,"$STATUS")== NULL && strstr((char*)g_CorsRecvBuf,"+CSIM")== NULL 
//				&&strstr((char*)g_CorsRecvBuf,"4G LTE DTU")== NULL && strstr((char*)g_CorsRecvBuf,"+EIND")== NULL
//				&&strstr((char*)g_CorsRecvBuf,"OK")== NULL)
//			{
//				memset(g_RtkDiffBuf, 0, RTK_DIFF_BUFSIZE);
//				memcpy(g_RtkDiffBuf, g_CorsRecvBuf, cors_len);
//				g_RtkDiffLen = cors_len;
//				Para->PositionSta = 2;
//				recvdiff = 1;
//				g_recvdiff = 0;
//				g_RTKRecvIntervalTimer = uwTick;
//			}
//		}
		if (g_ModeInFlash.type == 0)
		{
			/* 接收RTK数据 */
			rtkret = RTK_RecvBuf(g_RtkRecvBuf, &rtk_len);

			/* 处理差分数据 */
			if (rtkret == RTKRET_OK)
			{
				memset(g_RtkDiffBuf, 0x00, RTK_DIFF_BUFSIZE);
				g_RtkDiffLen = 0;
				HAL_UART_Transmit(&hUSART2, g_RtkRecvBuf, rtk_len, 1000);/*打印收到的D3数据*/
				rtkret = RTK_GetRTCMInfo(g_RtkRecvBuf, rtk_len, g_RtkDiffBuf, &g_RtkDiffLen);
				if (rtkret == RTKRET_OK)
				{
					Para->PositionSta = 2;			
					g_recvdiff = 0;
					recvdiff = 1;
					g_RTKRecvIntervalTimer = uwTick;
				}
			}
		}
	}
	/* 处理差分数据 */
	if (recvdiff == 1)
	{
		if (g_LastIntervalTimer <= g_RTKRecvIntervalTimer)
		{
			rtk_interval = g_RTKRecvIntervalTimer - g_LastIntervalTimer; 
		}
		else
		{
			rtk_interval = 0xFFFFFFFF - g_LastIntervalTimer + g_RTKRecvIntervalTimer;
		}
		if (rtk_interval > 700)
		{
			/* 计算两次接收差分数据间隔超过700ms 认为是分开的两次 */
			g_RecvRTKCnt++;
			g_DispRtkCnt++;
			g_recvdiff = 1;
			g_LastIntervalTimer = g_RTKRecvIntervalTimer;
		}
		if (g_RecvRTKCnt >= RYDW_BOARDCAST_INTERVAL) g_RecvRTKCnt = RYDW_BOARDCAST_INTERVAL;
		g_StationStatus.CorsOK = 1;
		g_StationStatus.RecvDiffOK = 1;
		if (g_RecvRTKCnt == RYDW_BOARDCAST_INTERVAL)
		{
			if (g_DbgEcho.EnableRTCMEcho == 1) debug_info(g_RtkDiffBuf, g_RtkDiffLen, "DIFF->");
			else if (g_DbgEcho.EnableRTCMEcho == 2) debug_ascinfo(g_RtkDiffBuf, g_RtkDiffLen, "DIFF->", 0);
			Para->BoardcastType.Location = 1;
			if (g_ModeInFlash.type == 1)
			{
			}
		}
	}
	else
	{
		g_StationStatus.RecvDiffOK = 0;
	}
#if ((USE_UM980) || (USE_LC29HDA))
	/* 接收GPS数据 */
	if (g_ModeInFlash.type == 1)
	{
		rtk_len = 0;
		rtkret = RTK_RecvBuf(g_RtkRecvBuf, &rtk_len);
		if (rtkret == RTKRET_OK)
		{
//			debug_printf("%s", g_RtkRecvBuf);
			gps_nmea_len = 0;
			gpsret = GPS_GetNMEAInfo(g_RtkRecvBuf, rtk_len, g_GpsNMEAMessageBuf, &gps_nmea_len, GPSMASK_GGA | GPSMASK_RMC);   
		}
	}
	else if (g_ModeInFlash.type == 0)
	{
		gps_nmea_len = 0;
		gpsret = GPS_GetNMEAInfo(g_RtkRecvBuf, rtk_len, g_GpsNMEAMessageBuf, &gps_nmea_len, GPSMASK_GGA | GPSMASK_RMC);
	}
	/* 提取GPS数据 */
	if (gpsret == GPSRET_OK)
	{
		/* 解析RMC */
		gpsret = GPS_AnlysisRMC(g_GpsNMEAMessageBuf, gps_nmea_len, &g_GPSInfo);
		if (gpsret == GPSRET_OK)
		{
			if (g_GPSInfo.uActived == 1)
			{
				g_StationStatus.GPSPosOK = 1;
			}
			else
			{
				g_StationStatus.GPSPosOK = 0;
			}
			
			if ((g_SysTime.Year == 0x00) && (g_SysTime.Month == 0x00))
			{
				if (g_GPSInfo.DateTimeInLocate.Year != 0x00)
				{
					AdjustSysTime(g_GPSInfo.DateTimeInLocate, &g_SysTime);
				}
			}
		}
		
		/* 解析GGA */
		gpsret = GPS_AnlysisGGA(g_GpsNMEAMessageBuf, gps_nmea_len, &g_GPSInfo);

		if (g_ModeInFlash.type == 1)
		{
			if (gpsret == GPSRET_OK)
			{
				g_NeedSendGGAFlag =1;
//				tmpsendDataNum = gps_nmea_len;
//				HAL_UART_Transmit(&hUSART3, (uint8_t*)g_GpsNMEAMessageBuf, gps_nmea_len, 1000);		//给CORS转发定位数据
			}
		}
		else
		{
			if(g_CorsNetOk == 1 && gpsret == GPSRET_OK)
				g_NeedSendGGAFlag =1;
		}
	}
#endif
	/* 处理卡状态 */
	RYDW_CardsProcess(&g_CardTable, Para);	
}


void	RYDW_GenerateDiffData(void)
{
	BcdDateTime_t bcdtime;
	uint8_t g_Mobile_Sim[6];
	memset(tmpsendbuf, 0x00, sizeof(tmpsendbuf));
	/* 生成记录数据头 */
	tmpsendbuf[0] = 0xFA;
	tmpsendbuf[1] = 0x00;
	tmpsendbuf[2] = 3;                                                       /*差分数据*/
	tmpsendbuf[3] = g_MobileInFlash.mobileno[4][0];
	tmpsendbuf[4] = g_MobileInFlash.mobileno[4][1];
	tmpsendbuf[5] = g_MobileInFlash.mobileno[4][2];
	tmpsendbuf[6] = g_MobileInFlash.mobileno[4][3];
	tmpsendbuf[7] = g_MobileInFlash.mobileno[4][4];
	tmpsendbuf[8] = g_MobileInFlash.mobileno[4][5];	
	memcpy(tmpsendbuf + 9, g_IDInFlash.IDnum, 7);
	tmpsendbuf[16] = 0x02;                                                   /*设备类型：=2基站*/
	bcdtime = HexTime2BcdTime(g_SysTime);
	memcpy(tmpsendbuf + 17, (uint8_t*)&bcdtime, 6);
	tmpsendbuf[30] = g_RtkDiffLen & 0xff;
	tmpsendbuf[31] = (g_RtkDiffLen >> 8)& 0xff;					//用户ID按小端模式上传
	memcpy(tmpsendbuf +32, g_RtkDiffBuf, g_RtkDiffLen);
	tmpsendbuf[g_RtkDiffLen+32] = CheckSum(g_RtkDiffLen+32, tmpsendbuf);
	tmpsendbuf[g_RtkDiffLen+33] = g_SerialNum&0xFF;//序列号 
	tmpsendbuf[g_RtkDiffLen+34] = (g_SerialNum>>8)&0xFF;//序列号 
	tmpsendbuf[g_RtkDiffLen+35] = (g_SerialNum>>16)&0xFF;//序列号 
	tmpsendbuf[g_RtkDiffLen+36] = (g_SerialNum>>24)&0xFF;//序列号 	
	tmpsendDataNum = g_RtkDiffLen+37;
	sendsocketid =0;
	HAL_UART_Transmit(&hUSART2, tmpsendbuf, g_RtkDiffLen+37, 1000);
//	g_DTUSendDataDelay = uwTick;
	g_FileSendDataToDtu++;
	if(g_SerialNum < 0xFFFFFFFF)
		g_SerialNum++;
	else
		g_SerialNum	= 0;	
}	
