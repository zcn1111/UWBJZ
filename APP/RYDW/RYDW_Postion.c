/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: RYDW_Postion.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 人员定位打点
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "Drv_W25Qxx_QSPI.h"
#include "GPS_Comm.h"
#include "RTK_Comm.h"
#include "CORS_Comm.h"
#include "gfunc_String.h"
#include "RYDW_Display.h"
#include "RYDW_StationCard.h"
#include "RYDW_Locate.h"
#include "RYDW_Postion.h"
#include "APP_STORE.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#include "string.h"
#include "math.h"
#include "g_GlobalValDef.h"
#include "DTU_BulkData.h"
#include "RYDW_Init.h"
#include "IP2365.h"
#include "app_dtu.h"
/********* 变量定义 **********/
BaseData_t g_PostionTable;
uint8_t g_PostionOrder;   /*记录基站打点数据条数*/
uint8_t g_PostionMode;			//打点模式
UserCardID_t g_BaseID;    /*基站ID*/ 
uint8_t g_PUNCOORAddnumb=0; /*发送打点数据条数*/

ReadAddrPar  g_ReadAddrPar[20];
uint32_t     g_ReadDataAddr;          /* 读C3的页地址 agj */
uint8_t g_DispPage=1;				//显示页面 ==1主菜单，==0状态页面， ==10每个卡页面
uint8_t g_CursorPos=0;              //光标位置
uint32_t g_SerialNum = 0;
uint32_t g_SerSerialNum;
/************************************** 定位处理函数 ************************************************/

/******************************************************************
函数名称: bool RYDW_GeneratePostionRecord(void)
功能描述: 生成打点数据记录
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
bool RYDW_GeneratePostionRecord(void)
{
	uint16_t i, ord;
	uint16_t sendcnt = 0;
	BcdDateTime_t bcdtime;
	uint16_t pos;
	uint16_t reclen;
	uint8_t chksum_t;
	/* 生成记录数据头 */
	memset(g_ReadAddrPar,0xFF,sizeof(g_ReadAddrPar));
	g_UploadRecord[0] = 0xFA;
	g_UploadRecord[1] = 0x00;
	g_UploadRecord[2] = 0x05;                             /*数据类型：打点数据*/	
	g_UploadRecord[3] = g_MobileInFlash.mobileno[4][0];
	g_UploadRecord[4] = g_MobileInFlash.mobileno[4][1];
	g_UploadRecord[5] = g_MobileInFlash.mobileno[4][2];
	g_UploadRecord[6] = g_MobileInFlash.mobileno[4][3];
	g_UploadRecord[7] = g_MobileInFlash.mobileno[4][4];
	g_UploadRecord[8] = g_MobileInFlash.mobileno[4][5];		
	memcpy(g_UploadRecord + 9,(uint8_t*)&g_IDInFlash.IDnum, 7);
	g_UploadRecord[16] = 0x02;                             /*设备类型：=2基站*/	
	bcdtime = HexTime2BcdTime(g_SysTime);
	memcpy(g_UploadRecord + 17, (uint8_t*)&bcdtime, 6);
	memset(g_UploadRecord + 23,0x00,7);
	reclen = 32;
	sendcnt = g_PUNCOORDatainFlash.needsendcnt;	
	if(g_PUNCOORDatainFlash.needsendcnt >20)
		g_PUNCOORAddnumb = 20;	
	else
		g_PUNCOORAddnumb = g_PUNCOORDatainFlash.needsendcnt;
	for(i=0;i<g_PUNCOORAddnumb;i++)
	{
		if(ReadRecordFromFlash(&g_PUNCOORDatainFlash,g_tmpBuf,i+1) == 1)
		{
			memcpy(g_UploadRecord + 32+32*i,g_tmpBuf,g_PUNCOORDatainFlash.datalen);
			g_ReadAddrPar[i].ReadDataAddr = g_ReadDataAddr;
			g_ReadAddrPar[i].Readaddr = g_PUNCOORDatainFlash.readflagaddr;
			g_ReadAddrPar[i].ReadEmptyFlag = g_SaveSendEmptyFlag;
			g_ReadAddrPar[i].Readset = g_PUNCOORDatainFlash.readflagaddr + g_SaveSendEmptyOffset;			
			reclen+=32; 		
		}
	}
	g_UploadRecord[30] = g_PUNCOORAddnumb*32 & 0xff;
	g_UploadRecord[31] = (g_PUNCOORAddnumb*32 >> 8)& 0xff;					//用户ID按小端模式上传		
	chksum_t = CheckSum(reclen, g_UploadRecord);
	g_UploadRecord[reclen] = chksum_t;
	g_UploadRecord[reclen+1] = g_SerialNum&0xFF;//序列号 
	g_UploadRecord[reclen+2] = (g_SerialNum>>8)&0xFF;//序列号 
	g_UploadRecord[reclen+3] = (g_SerialNum>>16)&0xFF;//序列号 
	g_UploadRecord[reclen+4] = (g_SerialNum>>24)&0xFF;//序列号 	
	tmpsendDataNum = reclen+5;
	sendsocketid =0;
//	HAL_UART_Transmit(&hUART4, g_UploadRecord, reclen+5, 1000);		//转发RTK差分数据给GPS模块/*发送数据*/
//	g_DTUSendDataDelay = uwTick;
	if(g_SerialNum < 0xFFFFFFFF)
		g_SerialNum++;
	else
		g_SerialNum	= 0;	
	return true;
}

/******************************************************************
函数名称: void RYDW_PostionDisp
功能描述: 打点功能流程显示
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_PostionDisp(BaseData_t PosInfo, uint8_t Ord)
{
	char pstr[16];
	uint32_t alt;
	
	memset(pstr, 0x00, 16);
	RYDW_DispString(0, 0, "*** POSTION ***");
	
	memset(pstr, 0x00, 16);
	sprintf(pstr, "ORDER=%02d", Ord);
	RYDW_DispString(1, 0, pstr);
	
	memset(pstr, 0x00, 16);
	sprintf(pstr, "Lat:%2.7f", (double)(PosInfo.Latitude) / 10000000);
	RYDW_DispString(2, 0, pstr);
	
	memset(pstr, 0x00, 16);
	sprintf(pstr, "Lon:%3.7f", (double)(PosInfo.Longitude) / 10000000);
	RYDW_DispString(3, 0, pstr);
	
	memset(pstr, 0x00, 16);
	alt = (uint32_t)(PosInfo.Altitude[0]) + ((uint32_t)(PosInfo.Altitude[1]) << 8) + ((uint32_t)(PosInfo.Altitude[2]) << 16);
	sprintf(pstr, "Alt:%4.3f", ((double)alt / 1000));
	RYDW_DispString(4, 0, pstr);
	
	memset(pstr, 0x00, 16);
	sprintf(pstr, "S=%02d, P=%d", PosInfo.SateOfNum, PosInfo.PosQuality);
	RYDW_DispString(5, 0, pstr);
	
	
}

/******************************************************************
函数名称: void RYDW_PostionProgress
功能描述: 打点功能流程
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_PostionProgress(void)
{
	RtkResult rtkret;
	uint16_t rtklen=0;
	
	GpsResult gpsret;
	uint16_t gpslen_nmea;
	GpsLocationInfo_t gpsinfo;
	uint32_t lat, lon, alt;
	uint16_t speed;
	
	CorsResult corsret;
	uint16_t corslen=0;
	uint8_t DDBuf[32];
	uint8_t t_chksum;
	
	/* 获取CORS数据 */
	memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
	corsret = CORS_RecvBuf(g_CorsRecvBuf, &corslen);
	if (corsret == CORSRET_OK)
	{
		HAL_UART_Transmit(&hUSART1, g_CorsRecvBuf, corslen, 1000);		//发送差分数据数据给RTK
	}
	
	/* 获取GPS数据 */
	memset(g_RtkRecvBuf, 0x00, RTK_RX_BUFSIZE);
	rtkret = RTK_RecvBuf(g_RtkRecvBuf, &rtklen);
	if (rtkret == RTKRET_OK)
	{
		gpsret = GPS_GetNMEAInfo(g_RtkRecvBuf, rtklen, g_GpsNMEAMessageBuf, &gpslen_nmea, GPSMASK_GGA | GPSMASK_RMC);		//提取GGA和RMC
		/* 提取GPS数据 */
		if (gpsret == GPSRET_OK)
		{
			GPS_InitGPSInfo(&gpsinfo);
			
			/* 解析RMC */
			gpsret = GPS_AnlysisRMC(g_GpsNMEAMessageBuf, gpslen_nmea, &gpsinfo);
			
			/* 解析GGA */
			gpsret = GPS_AnlysisGGA(g_GpsNMEAMessageBuf, gpslen_nmea, &gpsinfo);
			if (gpsret == GPSRET_OK)
			{
//				tmpsendDataNum = gpslen_nmea;
//				HAL_UART_Transmit(&hUSART3, (uint8_t*)&g_GpsNMEAMessageBuf, gpslen_nmea, 1000);	//发送GPS数据给DTU
				g_NeedSendGGAFlag =1;
			}
			
			/* 生成数据 */
			memset(&g_PostionTable, 0x00, sizeof(g_PostionTable));
			memset(DDBuf, 0x00, sizeof(DDBuf));
			memcpy(&g_PostionTable.Time, &g_SysTime, 6);/*时间*/
			memcpy(DDBuf, &g_SysTime, 6);/*时间*/
			lat = (uint32_t)(gpsinfo.dLatitudeInDegree * 10000000);
			lon = (uint32_t)(gpsinfo.dLongitudeInDegree * 10000000);
			alt = (uint32_t)(gpsinfo.fAltitudeInMeter * 1000);
			memcpy(DDBuf+6, (uint8_t*)&lat, 4);
			g_PostionTable.Latitude = lat;
			memcpy(DDBuf+10, (uint8_t*)&lon, 4);
			g_PostionTable.Longitude = lon;			
			g_PostionTable.PosQuality = gpsinfo.uMode;			
			g_GPSInfo.uMode = gpsinfo.uMode;			
			DDBuf[14] =  g_PostionTable.PosQuality;
			g_PostionTable.Altitude[0] = (uint8_t)(alt & 0x000000FF);
			g_PostionTable.Altitude[1] = (uint8_t)(alt >> 8);
			g_PostionTable.Altitude[2] = (uint8_t)(alt >> 16);
			memcpy(DDBuf+15,g_PostionTable.Altitude, 3);
			if(g_IP2365Init == 0x01 || g_MainVal > 7.0)
				g_PostionTable.Alram.all = 0x18;                        //报警标记
			else
				g_PostionTable.Alram.all = 0x08;                        //报警标记						
			DDBuf[18] = g_PostionTable.Alram.all;
			DDBuf[19] = (uint8_t)(g_fBatVal*10);/*电池电量*/
			speed = (uint16_t)(gpsinfo.fSpeedInKmh*10);
			memcpy(DDBuf+23,(uint8_t*)&speed, 2);                   //速度
			g_PostionTable.SateOfNum = gpsinfo.uSatelliteNum;		// 放置星数
			DDBuf[25] = g_PostionTable.SateOfNum;
			DDBuf[26] = g_BribeFlag.RouLin;
			if(g_DispPage == 0x11 && g_OpenOLEDPower == 1)
				RYDW_PostionDisp(g_PostionTable, g_PostionOrder); 		
			/*保存打点数据*/
			if ((gpsinfo.uMode == 4 || gpsinfo.uMode == 5 || gpsinfo.uMode == 7) && (gpsinfo.uActived == 1) && g_BribeFlag.run==0)
			{
				SaveRecordToFlash(&g_PUNCOORDatainFlash,(uint8_t*)&DDBuf);			
				g_PostionOrder++;
			}
			
		}
	}
	
	
	
}
