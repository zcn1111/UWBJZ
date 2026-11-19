/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: RYDW_Display.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 人员定位数据显示
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "ssd1306.h"
#include "ssd1306_fonts.h"

#include "GPS_Comm.h"
#include "RTK_Comm.h"
#include "gfunc_String.h"
#include "RYDW_Display.h"
#include "RYDW_StationCard.h"
#include "RYDW_Init.h"
#include "string.h"
#include "math.h"


/********* 变量定义 **********/
char g_DispBuf[32];
char g_SendProcess[] = {'|', '/', '-', '\\'};

uint32_t g_DispRefreshDelay;		//刷新时间
uint32_t g_DispNoOperationTimer;	//无操作定时器
uint8_t g_SeleWorkMode=0;          //工作模式标记=0正常 =1打点
BribeType g_BribeFlag;
/************************************** 显示函数 ************************************************/

/******************************************************************
函数名称: void RYDW_DispStationStatus
功能描述: 显示基站状态
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispStationStatus(StationStatus_t *Sta, uint8_t PosX, uint8_t PosY)
{
	memset(g_DispBuf, 0x00, sizeof(g_DispBuf));
	memcpy(g_DispBuf, "STA: ", 5);
	
	/* CORS模块 */
	if (Sta->CorsOK == 1)
	{
		g_DispBuf[5] = 'C';
	}
	else
	{
		g_DispBuf[5] = ' ';
	}
	
	/* Lora状态 */
	if (Sta->LoraOK == 1)
	{
		g_DispBuf[6] = 'L';
	}
	else
	{
		g_DispBuf[6] = ' ';
	}
	
	/* GPS定位状态 */
	if (Sta->GPSPosOK == 1)
	{
		g_DispBuf[7] = 'G';
	}
	else
	{
		g_DispBuf[7] = ' ';
	}
	
	/* Dtu状态 */
	if (Sta->DtuOK == 1)
	{
		g_DispBuf[8] = 'D';
	}
	else
	{
		g_DispBuf[8] = ' ';
	}
	
	/* 接收差分数据 */
	if (Sta->RecvDiffOK == 1)
	{
		g_DispBuf[9] = 'R';
	}
	else
	{
		g_DispBuf[9] = ' ';
	}
	
	/* 广播数据条数 */
	g_DispBuf[10] = ' ';
	g_DispBuf[11] = '0';
	g_DispBuf[12] = '0';
	g_DispBuf[13] = '0';
	g_DispBuf[14] = g_SendProcess[ g_DispRtkCnt % 4 ];
	
	/* 显示状态 */
	ssd1306_SetCursor(PosX, PosY);
	ssd1306_WriteString(g_DispBuf, Font_6x8, White);
}

/******************************************************************
函数名称: void RYDW_DispCardInfo
功能描述: 显示定位卡信息
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispCardInfo(CardInfoTable_t *CardTable, uint8_t PosX, uint8_t PosY)
{
	uint8_t i;
	uint16_t id;
	
	memset(g_DispBuf, 0x00, sizeof(g_DispBuf));
	memcpy(g_DispBuf, "Card:", 5);
	
	/* 显示卡数量 */
	g_DispBuf[5] = (CardTable->CardNbr / 10) + '0';
	g_DispBuf[6] = (CardTable->CardNbr % 10) + '0';
	
	/* 显示卡状态 */
	g_DispBuf[7] = ' ';
	for (i=0; i<10; i++)
	{
		g_DispBuf[i + 8] = ' ';
	}
	/* 显示卡编号 */
	for (i=0; i<CardTable->CardNbr; i++)
	{
		id = CardTable->Info[i].ID.byte.UserID % 10;
		g_DispBuf[8 + id] = id + '0';
	}
	
	/* 显示卡状态信息 */
	ssd1306_SetCursor(PosX, PosY);
	ssd1306_WriteString(g_DispBuf, Font_6x8, White);
	
	/* 显示卡报警信息 */
	memset(g_DispBuf, 0x00, sizeof(g_DispBuf));
	memcpy(g_DispBuf, "     ", 5);
	
	/* 显示卡数量 */
	g_DispBuf[5] = ' ';
	g_DispBuf[6] = ' ';
	
	/* 显示卡报警信息 */
	g_DispBuf[7] = ' ';
	for (i=0; i<10; i++)
	{
		g_DispBuf[i + 8] = ' ';
	}
	for (i=0; i<CardTable->CardNbr; i++)
	{
		id = CardTable->Info[i].ID.byte.UserID;
		if (id > 10) id = 10;
	}
	for (i=0; i<CardTable->CardNbr; i++)
	{
		id = CardTable->Info[i].ID.byte.UserID;
		
		/* 判断报警状态 */
		/* 无报警 */
		if ((CardTable->Info[i].Alram.bit.Alram == 0) && (CardTable->Info[i].Alram.bit.Online == 1))
		{
			/* 未定位 */
			if (CardTable->Info[i].PosQuality == 0)
			{
				g_DispBuf[8 + id] = '0';
			}
			/* 单点解 或 差分解 */
			else if (CardTable->Info[i].PosQuality == 1)
			{
				g_DispBuf[8 + id] = '1';
			}
			else if (CardTable->Info[i].PosQuality == 2)
			{
				g_DispBuf[8 + id] = '2';
			}
			/* 浮点解 */
			else if (CardTable->Info[i].PosQuality == 5)
			{
				g_DispBuf[8 + id] = '5';
			}
			/* 固定解 */
			else if (CardTable->Info[i].PosQuality == 4)
			{
				g_DispBuf[8 + id] = '4';
			}
			/* 判断电池状态 */
			if (CardTable->Info[i].BatVoltage & 0x80)
			{
				g_DispBuf[8 + id] = 'V';
			}
		}
		/* 有报警 */
		else if (CardTable->Info[i].Alram.bit.Alram == 1)
		{
			g_DispBuf[8 + id] = '!';
		}
		/* 卡离线 */
		else if (CardTable->Info[i].Alram.bit.Online == 0)
		{
			g_DispBuf[8 + id] = 'X';
		}
		
	}
	
	/* 显示卡状态信息 */
	ssd1306_SetCursor(PosX, PosY + 10);
	ssd1306_WriteString(g_DispBuf, Font_6x8, White);
}

/* GPS数据经纬度格式 转换到 数组 */
void ConvGPSFormat2Arr(double GpsPos, uint8_t *Arr, uint8_t LatOrLon)
{
	double gps, gpsa;
	uint8_t val;
	uint8_t i;
	
	gps = floor(GpsPos * 10000);
	if (LatOrLon == 0)
	{
		/* 转换纬度 */
		gpsa = gps / 1000000;
		Arr[0] = (uint8_t)(floor(gpsa));
		
		gpsa = (gpsa - floor(gpsa)) * 100;
		Arr[1] = (uint8_t)(floor(gpsa));
		
		gpsa = (gpsa - floor(gpsa)) * 100;
		Arr[2] = (uint8_t)(floor(gpsa));
		
		gpsa = (gpsa - floor(gpsa)) * 100;
		Arr[3] = (uint8_t)(floor(gpsa));
	}
	else if (LatOrLon == 1)
	{
		/* 转换经度 */
		gpsa = gps / 100000000;
		Arr[0] = (uint8_t)(floor(gpsa));
		
		gpsa = (gpsa - floor(gpsa)) * 100;
		Arr[1] = (uint8_t)(floor(gpsa));
		
		gpsa = (gpsa - floor(gpsa)) * 100;
		Arr[2] = (uint8_t)(floor(gpsa));
		
		gpsa = (gpsa - floor(gpsa)) * 100;
		Arr[3] = (uint8_t)(floor(gpsa));
		
		gpsa = (gpsa - floor(gpsa)) * 100;
		Arr[4] = (uint8_t)(floor(gpsa));
	}
}
/******************************************************************
函数名称: void RYDW_DispPosition
功能描述: 显示定位信息
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispPosition(uint8_t PosX, uint8_t PosY)
{
	uint16_t satellitenum;
	uint8_t posque;
	
	uint16_t slen;
	uint16_t i;
	uint16_t paranum;
	
	uint8_t posval[5];
	uint16_t clen;
	
	memset(g_DispBuf, 0x00, sizeof(g_DispBuf));
	memcpy(g_DispBuf, "POS: ", 5); 
	
	/* 获取定位状态 */
	/* 获取纬度 */
	memset(posval, 0x00, sizeof(posval));
	ConvGPSFormat2Arr(g_GPSInfo.dLatitudeInDegMin, posval, 0);			//转换纬度
	Hex2BcdInString(posval, 4, g_LatitudeBCD, &clen);
	
	/* 获取经度 */
	ConvGPSFormat2Arr(g_GPSInfo.dLongitudeInDegMin, posval, 1);			//转换经度
	Hex2BcdInString(posval, 5, g_LongitudeBCD, &clen);
	/* 显示经纬度 */
	memset(g_DispBuf, 0x00, sizeof(g_DispBuf));
	sprintf(g_DispBuf, "Lat:%2.7f", g_GPSInfo.dLatitudeInDegree);
	ssd1306_SetCursor(PosX, PosY);
	ssd1306_WriteString(g_DispBuf, Font_6x8, White);
	memset(g_DispBuf, 0x00, sizeof(g_DispBuf));
	sprintf(g_DispBuf, "Lon:%3.7f", g_GPSInfo.dLongitudeInDegree);
	ssd1306_SetCursor(PosX, PosY + 10);
	ssd1306_WriteString(g_DispBuf, Font_6x8, White);
	
	/* 显示卫星数，信号质量 */
	memset(g_DispBuf, 0x00, sizeof(g_DispBuf));
	sprintf(g_DispBuf, "S=%02d, P=%d", g_GPSInfo.uSatelliteNum, g_GPSInfo.uMode);
	ssd1306_SetCursor(PosX, PosY + 20);
	ssd1306_WriteString(g_DispBuf, Font_6x8, White);
}

/******************************************************************
函数名称: void RYDW_DispTime
功能描述: 显示时间
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispTime(HexDateTime_t NowTime, uint8_t PosX, uint8_t PosY)
{
	char ptime[10];
	if(g_OpenOLEDPower == 1)
	{
		/* 显示时间 */
		sprintf(ptime, "%02d:%02d:%02d", NowTime.Hour, NowTime.Minute, NowTime.Second);
		ssd1306_SetCursor(PosX, PosY);
		ssd1306_WriteString(ptime, Font_6x8, White);
		ssd1306_UpdateScreen();	
	}
}

/******************************************************************
函数名称: void RYDW_DispBattery
功能描述: 显示电量
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispBattery(float BatVal)
{
	uint8_t posx, posy;
	
	/* 显示在右上角 */
	if(g_OpenOLEDPower == 1)
	{
		posx = 110;
		posy = 0;
	
		/* 显示电量 */
		ssd1306_FillRectangle(posx, posy, posx + 16, posy + 16, Black);
		ssd1306_UpdateScreen();
		if (BatVal < 6)
		{
			ssd1306_DrawBitmap(posx, posy, Bat_Lev1, 16, 16, White);
		}
		else if ((BatVal >= 6) && (BatVal < 7))
		{
			ssd1306_DrawBitmap(posx, posy, Bat_Lev2, 16, 16, White);
		}
		else if ((BatVal >= 7) && (BatVal < 8))
		{
			ssd1306_DrawBitmap(posx, posy, Bat_Lev3, 16, 16, White);
		}
		else if (BatVal >= 8)
		{
			ssd1306_DrawBitmap(posx, posy, Bat_Lev4, 16, 16, White);
		}
		ssd1306_UpdateScreen();
	}
}

/******************************************************************
函数名称: void RYDW_DispGPS(uint8_t  mark)
功能描述: 显示定位标记
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/

void RYDW_DispGPS(uint8_t  mark)
{
	uint8_t posx, posy;
	
	/* 显示在右上角 */
	if(g_OpenOLEDPower == 1)
	{
		posx = 0;
		posy = 0;
		
		/* 显示定位质量 */
		ssd1306_FillRectangle(posx, posy, posx + 16, posy + 16, Black);
		ssd1306_UpdateScreen();
		if (mark == 0)
		{
			ssd1306_DrawBitmap(posx, posy, GPS_Lev0, 16, 16, White);
		}
		else if (mark == 1)
		{
			ssd1306_DrawBitmap(posx, posy, GPS_Lev1, 16, 16, White);
		}
		else if (mark == 2)
		{
			ssd1306_DrawBitmap(posx, posy, GPS_Lev2, 16, 16, White);
		}
		else if (mark == 4)
		{
			ssd1306_DrawBitmap(posx, posy, GPS_Lev4, 16, 16, White);
		}
		else if (mark == 5)
		{
			ssd1306_DrawBitmap(posx, posy, GPS_Lev5, 16, 16, White);
		}
		else if (mark == 7)
		{
			ssd1306_DrawBitmap(posx, posy, GPS_Lev7, 16, 16, White);
		}	
		ssd1306_UpdateScreen();
	}
}

/******************************************************************
函数名称: void RYDW_DispSignal(uint8_t  num)
功能描述: 显示信号标记
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/

void RYDW_DispSignal(uint8_t  num)
{
	uint8_t posx, posy;
	
	/* 显示在右上角 */
	if(g_OpenOLEDPower == 1)
	{
		posx = 93;
		posy = 0;
		
		/* 显示定位质量 */
		ssd1306_FillRectangle(posx, posy, posx + 16, posy + 16, Black);
		ssd1306_UpdateScreen();
		if (num >=24)
		{
			ssd1306_DrawBitmap(posx, posy, Cors_Lev5, 16, 16, White);
		}
		else if (num >= 18)
		{
			ssd1306_DrawBitmap(posx, posy, Cors_Lev4, 16, 16, White);
		}
		else if (num >= 12)
		{
			ssd1306_DrawBitmap(posx, posy, Cors_Lev3, 16, 16, White);
		}
		else if (num >= 6)
		{
			ssd1306_DrawBitmap(posx, posy, Cors_Lev2, 16, 16, White);
		}
		else if (num >= 1)
		{
			ssd1306_DrawBitmap(posx, posy, Cors_Lev1, 16, 16, White);
		}
		ssd1306_UpdateScreen();
	}
}
/******************************************************************
函数名称: void RYDW_DispClear
功能描述: 清屏
参数输入: 无
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispClear(void)
{
	ssd1306_Fill(Black);			//清除屏幕显示
	ssd1306_UpdateScreen();			//刷新屏幕
}

/******************************************************************
函数名称: void RYDW_DispString
功能描述: 显示字符串(使用默认6*8字体，只能显示ASCII码)
参数输入: uint8_t	Row	显示行位置 最大显示7行
		  uint8_t	Col	显示列位置 最大显示20列
		  char*		Str	显示字符串
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispString(uint8_t Row, uint8_t Col, char *Str)
{
	uint8_t posx, posy;
	
	if ((Col > 20) || (Row > 6))
		return;
	
	posx = Col * 6;
	posy = Row * 9;
	ssd1306_SetCursor(posx, posy);
	ssd1306_WriteString(Str, Font_6x8, White);
	ssd1306_UpdateScreen();
}

/******************************************************************
函数名称: void RYDW_DispCardDetail
功能描述: 显示定位卡详细信息
参数输入: 无
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DispCardDetail(CardInfoTable_t Cardinfo, uint8_t Order)
{
	uint8_t i;
	uint8_t nbr;
	char str[16];
	uint32_t alt;
	
	nbr = Cardinfo.CardNbr;
	if (nbr == 0)
	{
		RYDW_DispString(0, 0, "**** CARDINFO ****");
		RYDW_DispString(1, 0, "NO CARD!");
		return;
	}
	else
	{
		if (Order > (nbr - 1)) return;
	}
	
	RYDW_DispString(0, 0, "**** CARDINFO ****");
	
	memset(str, 0x00, 16);
	sprintf(str, "ID:%05d", Cardinfo.Info[Order].ID.byte.UserID);
	RYDW_DispString(1, 0, str);
	
	memset(str, 0x00, 16);
	sprintf(str, "Lat:%2.7f", (double)(Cardinfo.Info[Order].Latitude) / 10000000);
	RYDW_DispString(2, 0, str);
	
	memset(str, 0x00, 16);
	sprintf(str, "Lon:%3.7f", (double)(Cardinfo.Info[Order].Longitude) / 10000000);
	RYDW_DispString(3, 0, str);
	
	memset(str, 0x00, 16);
	alt = (uint32_t)(Cardinfo.Info[Order].Altitude[2]) + ((uint32_t)(Cardinfo.Info[Order].Altitude[1]) << 8) + ((uint32_t)(Cardinfo.Info[Order].Altitude[0]) << 16);
	sprintf(str, "Alt:%4.3f", ((double)alt / 1000));
	RYDW_DispString(4, 0, str);
	
	memset(str, 0x00, 16);
	if (Cardinfo.Info[Order].Alram.bit.Online == 0)
	{
		RYDW_DispString(5, 0, "OFFLINE     ");
	}
	else
	{
		sprintf(str, "M=%d, Alram=%d", Cardinfo.Info[Order].PosQuality, Cardinfo.Info[Order].AlramType.all);
		RYDW_DispString(5, 0, str);
	}
	
	memset(str, 0x00, 16);
	if (Cardinfo.Info[Order].Alram.bit.Online == 0)
	{
		memset(str, ' ', 10);
		RYDW_DispString(6, 0, str);
	}
	else
	{
		sprintf(str, "RSSI:%d SNR:%d", Cardinfo.Rssi[Order], Cardinfo.Snr[Order]);
		RYDW_DispString(6, 0, str);
	}
}


void  SeleWorkMode(uint8_t flag,uint8_t color)
{//显示
	ssd1306_FillRectangle(68, 48, 92, 60, Black);
	if(flag == 0)
	{//显示正常
		if(color == 0)
		{
			ssd1306_DrawBitmap(68,48,zheng_12x12,12,12,White);	      /*正*/
			ssd1306_DrawBitmap(80,48,chang_12x12,12,12,White);	      /*常*/			
		}
		else if(color == 1)
		{/*反显*/
			ssd1306_DrawBitmap(68,48,zheng1_12x12,12,12,White);	      /*正*/
			ssd1306_DrawBitmap(80,48,chang1_12x12,12,12,White);	      /*常*/			
		}
	}
	else if(flag == 1)
	{//显示打点
		if(color == 0)
		{
			ssd1306_DrawBitmap(68,48,da_12x12,12,12,White);	          /*打*/
			ssd1306_DrawBitmap(80,48,dian_12x12,12,12,White);	      /*点*/			
		}
		else if(color == 1)
		{/*反显*/
			ssd1306_DrawBitmap(68,48,da1_12x12,12,12,White);	      /*打*/
			ssd1306_DrawBitmap(80,48,dian1_12x12,12,12,White);	      /*点*/		
		}
	}
	ssd1306_UpdateScreen();	     	
}

void  ShowLinItrm(uint8_t flag)
{
	ssd1306_FillRectangle(96, 48, 108, 60, Black);
	if(flag == 1)
	{//上行
		ssd1306_DrawBitmap(96,48,shang_12x12,12,12,White);	      /*上*/
	}
	else if(flag == 2)
	{//下行
		ssd1306_DrawBitmap(96,48,xia_12x12,12,12,White);	      /*上*/		
	}
	else if(flag == 3)
	{//单线
		ssd1306_DrawBitmap(96,48,dan_12x12,12,12,White);	      /*上*/		
	}
	ssd1306_UpdateScreen();	  	
}

void  ShowBribeRun(uint8_t flag)
{
	ssd1306_FillRectangle(110, 46, 126, 62, Black);
	if(flag == 1)
	{//暂停
		ssd1306_DrawBitmap(110,46,Stop_Lev,16,16,White);	      /*上*/
	}
	else
	{//上行
		ssd1306_DrawBitmap(110,46,Start_Lev,16,16,White);	      /*上*/		
	}
	ssd1306_UpdateScreen();	  	
}