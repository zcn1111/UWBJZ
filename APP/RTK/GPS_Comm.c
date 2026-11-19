
#include "BSP.h"
#include "math.h"

#include "GPS_Comm.h"
#include "gfunc_String.h"

GPS_T		g_tGPS;
GpsLocationInfo_t	g_GPSInfo;
uint8_t		g_GPSSendFlag=0;
char		g_SingleGpsBuf[256];		//GPS单条信息
char		g_GpsNMEAMessageBuf[GPS_NMEA_BUFSIZE];	//GPS-NMEA报文接收缓存
char		g_chGpsSaveInfo[384];

uint8_t		g_LatitudeBCD[4];		//纬度BCD码
uint8_t		g_LongitudeBCD[5];		//经度BCD码
uint8_t		g_SatelliteNum;			//卫星数
uint8_t		g_PosQuality;			//信号质量
uint8_t     g_GpsGGABuf[255];
uint8_t     g_GpsGGABufLen;
Gpscoort_t g_gpsco;
/************************************** GPS数据通信函数 ************************************************/
/******************************************************************
函数名称: GpsResult GPS_RecvBuf
功能描述: 接收Gps数据
参数输入: 
参数输出: 
返 回 值: GpsResult 返回值
其    他: 无
******************************************************************/
GpsResult GPS_RecvBuf(char *OutBuf, uint16_t *BufLen, uint16_t Mask)
{
	uint16_t gpslen;
	
	gpslen = GetRingUsedLen(&g_Uart5RxRing);
	if (gpslen > 0)
	{
		if (gpslen > GPS_RX_BUFSIZE)
		{
			RingBufPop(&g_Uart5RxRing, (uint8_t*)OutBuf, gpslen);
			*BufLen = 0;
			memset(OutBuf, 0x00, GPS_RX_BUFSIZE);
			return GPSRET_LEN_ERR;
		}
		else
		{
			memset(OutBuf, 0x00, GPS_RX_BUFSIZE);
			RingBufPop(&g_Uart5RxRing, (uint8_t*)OutBuf, gpslen);
			*BufLen = gpslen;
			if (g_DbgEcho.EnableGpsEcho == 1) debug_info((uint8_t*)OutBuf, gpslen, "GPS->");
			else if (g_DbgEcho.EnableGpsEcho == 2) debug_printf("%s\r\n", OutBuf);
			return GPSRET_OK;
		}
	}
	
	return GPSRET_NODATA_ERR;
}


/*
*********************************************************************************************************
*	函 数 名: StrToInt
*	功能说明: 将ASCII码字符串转换成十进制
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
int32_t StrToInt(char *_pStr)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* 负数 */
		p++;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < 15; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p++;
			ucTemp = *p;
		}
		if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = (ulInt << 4) + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}
/*
*********************************************************************************************************
*	函 数 名: StrToIntFix
*	功能说明: 将ASCII码字符串转换成十进制, 给定长度
*	形    参: _pStr :待转换的ASCII码串. 可以以逗号或0结束
*			 _ucLen : 固定长度
*	返 回 值: 二进制整数值
*********************************************************************************************************
*/
int32_t StrToIntFix(char *_pStr, uint8_t _ucLen)
{
	uint8_t flag;
	char *p;
	uint32_t ulInt;
	uint8_t i;
	uint8_t ucTemp;

	p = _pStr;
	if (*p == '-')
	{
		flag = 1;	/* 负数 */
		p++;
		_ucLen--;
	}
	else
	{
		flag = 0;
	}

	ulInt = 0;
	for (i = 0; i < _ucLen; i++)
	{
		ucTemp = *p;
		if (ucTemp == '.')	/* 遇到小数点，自动跳过1个字节 */
		{
			p++;
			ucTemp = *p;
		}
		if ((ucTemp >= '0') && (ucTemp <= '9'))
		{
			ulInt = (ulInt << 4) + (ucTemp - '0');
			p++;
		}
		else
		{
			break;
		}
	}

	if (flag == 1)
	{
		return -ulInt;
	}
	return ulInt;
}
void InitGPS(void)
{
	/*
		GPS 模块使用 UART 串口发送定位数据至 STM32, 每秒钟发送一组数据

		因此，只需要配置串口即可。 bsp_uart.c 中已经配置好串口，此处不必再配置
	*/
	g_GPSSendFlag = 2;
	g_tGPS.PositionOk = 0;  /* 数据设置为无效 */
	g_tGPS.TimeOk = 0;  /* 数据设置为无效 */
	g_tGPS.ViewNumber = 0;
	g_tGPS.UartOk = 0;	/* 串口通信正常的标志, 如果以后收到了校验合格的命令串则设置为1 */
}

/************************************** GPS初始化函数 ************************************************/
GpsResult GPS_InitGPSInfo(GpsLocationInfo_t *GpsInfo)
{	
	GpsInfo->uActived = 0;
	GpsInfo->uMode = 0;
	GpsInfo->cMode = ' ';
	GpsInfo->uSatelliteNum = 0;
	GpsInfo->uEWFlag = 0;
	GpsInfo->dLatitudeInDegree = 0.0;
	GpsInfo->dLatitudeInDegMin = 0.0;
	GpsInfo->uNSFlag = 0;
	GpsInfo->dLongitudeInDegree = 0.0;
	GpsInfo->dLongitudeInDegMin = 0.0;
	GpsInfo->fAltitudeInMeter = 0.0;
	GpsInfo->fSpeedInKmh = 0.0;
	GpsInfo->fTrackTrueInDegree = 0.0;
	memset((HexDateTime_t*)&(GpsInfo->DateTimeInUTC), 0x00, sizeof(HexDateTime_t));
	memset((HexDateTime_t*)&(GpsInfo->DateTimeInLocate), 0x00, sizeof(HexDateTime_t));
	GpsInfo->MilSecond = 0;
}

/************************************** GPS数据解析函数 ************************************************/
/******************************************************************
函数名称: bool GPS_IsMatchedMessageHeader
功能描述: 是否有匹配报文头
参数输入: 
参数输出: 
返 回 值: true=找到对应报文头
其    他: 无
******************************************************************/
bool GPS_IsMatchedMessageHeader(char *HeaderStr, uint8_t StrLen, uint32_t Mask)
{
	bool bret;
	
	if (StrLen > 7)
		return false;
	
	/* 检查对应报文头 */
	bret = false;
	if ((Mask & GPSMASK_GGA) == GPSMASK_GGA)
	{
		if ((HeaderStr[3] == 'G') && (HeaderStr[4] == 'G') && (HeaderStr[5] == 'A')) bret = true;		//找到对应报文头
	}
	if ((Mask & GPSMASK_GLL) == GPSMASK_GLL)
	{
		if ((HeaderStr[3] == 'G') && (HeaderStr[4] == 'G') && (HeaderStr[5] == 'L')) bret = true;
	}
	if ((Mask & GPSMASK_GNS) == GPSMASK_GNS)
	{
		if ((HeaderStr[3] == 'G') && (HeaderStr[4] == 'N') && (HeaderStr[5] == 'S')) bret = true;
	}
	if ((Mask & GPSMASK_GRS) == GPSMASK_GRS)
	{
		if ((HeaderStr[3] == 'G') && (HeaderStr[4] == 'R') && (HeaderStr[5] == 'S')) bret = true;
	}
	if ((Mask & GPSMASK_GSA) == GPSMASK_GSA)
	{
		if ((HeaderStr[3] == 'G') && (HeaderStr[4] == 'S') && (HeaderStr[5] == 'A')) bret = true;
	}
	if ((Mask & GPSMASK_GST) == GPSMASK_GST)
	{
		if ((HeaderStr[3] == 'G') && (HeaderStr[4] == 'S') && (HeaderStr[5] == 'T')) bret = true;
	}
	if ((Mask & GPSMASK_GSV) == GPSMASK_GSV)
	{
		if ((HeaderStr[3] == 'G') && (HeaderStr[4] == 'S') && (HeaderStr[5] == 'V')) bret = true;
	}
	if ((Mask & GPSMASK_RMC) == GPSMASK_RMC)
	{
		if ((HeaderStr[3] == 'R') && (HeaderStr[4] == 'M') && (HeaderStr[5] == 'C')) bret = true;
	}
	if ((Mask & GPSMASK_VTG) == GPSMASK_VTG)
	{
		if ((HeaderStr[3] == 'V') && (HeaderStr[4] == 'T') && (HeaderStr[5] == 'G')) bret = true;
	}
	
	return bret;
}

/******************************************************************
函数名称: GpsResult GPS_GetNMEAInfo
功能描述: 获取NMEA数据
参数输入: 
参数输出: 
返 回 值: GpsResult 返回值
其    他: 无
******************************************************************/
GpsResult GPS_GetNMEAInfo(uint8_t *InBuf, uint16_t BufLen, char *GpsStr, uint16_t *StrLen, uint32_t Mask)
{
	uint16_t i, j, stpos, edpos;
	uint8_t step;
	char header[8]={0};
	uint8_t hdrlen;
	uint16_t slen;
	uint16_t findnum;
	
	stpos = 0;
	edpos = 0;
	step = 0;
	j = 0;
	slen = 0;
	findnum = 0;
	memset(GpsStr, 0x00, GPS_NMEA_BUFSIZE);
	for (i=0; i<BufLen; i++)
	{
		switch (step)
		{
			/* Step=0 查找起始标识'$' */
			case 0:
				if (InBuf[i] == '$')
				{
					step = 1;			//找到起始位置
					stpos = i;
					slen = 0;
				}
				break;
			/* Step=1 找到起始位置 查找对应报文头 */
			case 1:
				if (InBuf[i] == ',')
				{
					if ((i - stpos) > 7) hdrlen = 7; else hdrlen = i - stpos;
					memset(header, 0x00, 8);
					memcpy(header, InBuf + stpos, hdrlen);
					if (GPS_IsMatchedMessageHeader(header, hdrlen, Mask) == true)
					{
						/* 找到匹配的报文头 转到下一步 */
						step = 2;
					}
					else
					{
						/* 没有找到报文头 报文不正确重新查找 */
						step = 0;
					}
				}
				break;
			/* Step=2 查找对应报文头结束 */
			case 2:
				if (i <= (BufLen - 2))
				{
					if ((InBuf[i] == 0x0D) && (InBuf[i+1] == 0x0A))
					{
						edpos = i + 1;				//记录结束位置
						slen = edpos - stpos + 1;	//计算数据长度
						step = 3;					//转到下一步
					}
				}
				else
				{
					if (edpos == 0)
					{
						*StrLen = 0;
						return GPSRET_NODATA_ERR;
					}
					else
					{
						*StrLen = strlen(GpsStr);
						return GPSRET_OK;
					}
				}
				break;
			/* Step=3 复制数据 */
			case 3:
				memcpy(GpsStr + j, (char*)InBuf + stpos, slen);
				findnum++;
				j += slen;
				step = 0;
				break;
			default:
				break;
		}
	}
	
	/* 找到至少一条 */
	if (findnum != 0)
	{
		*StrLen = strlen(GpsStr);
		return GPSRET_OK;
	}
}

/******************************************************************
函数名称: GpsResult GPS_AnlysisGGA
功能描述: 处理解析GGA数据
参数输入: 
参数输出: 
返 回 值: GpsResult 返回值
其    他: 无
******************************************************************/
GpsResult GPS_AnlysisGGA(char *GpsStr, uint16_t StrLen, GpsLocationInfo_t *GpsInfo)
{
	char *ptr;
	char cpara[32];
	uint16_t slen;
	uint16_t i;
	uint16_t paranum;
	
	double lat_in_deg, lon_in_deg, alt;
	double deg, minute;
	float time;
	
	
	/* $GNGGA,084607.000,3748.260409,N,11233.256331,E,5,21,1.36,790.0,M,-22.9,M,24.0,1562*4D */
	
	/* 查找数据头 */
	ptr = strstr(GpsStr, "GGA");	
	if (ptr != NULL)
	{
		ptr -=3;
		slen = strlen(ptr);
		for (i=0; i<slen; i++)
		{
			if ((ptr[i] == 0x0D) && (ptr[i+1] == 0x0A))
			{
				break;
			}
		}
		memset(g_SingleGpsBuf, 0x00, 256);
		memcpy(g_SingleGpsBuf, ptr, (i+1));
		memcpy(g_GpsGGABuf,ptr,(i+2));
		g_GpsGGABufLen = i+2;
//		debug_printf("%s", sptr);
		paranum = CalcSpiltChrNum(g_SingleGpsBuf, slen, ',');	//计算总参数个数
		if (paranum != 0)
		{
			/* 解析时间 1 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 1, cpara);	//时间 HHmmss.sss
			GpsInfo->DateTimeInUTC.Hour = Asc2Hex(cpara[0]) * 10 + Asc2Hex(cpara[1]);
			GpsInfo->DateTimeInUTC.Minute = Asc2Hex(cpara[2]) * 10 + Asc2Hex(cpara[3]);
			GpsInfo->DateTimeInUTC.Second = Asc2Hex(cpara[4]) * 10 + Asc2Hex(cpara[5]);
			time = atol(cpara);
			GpsInfo->MilSecond = (uint16_t)((time - floor(time)) * 1000);
			
			/* 解析纬度 2 3 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 2, cpara);	//纬度 ddmm.mmmmmm
			GpsInfo->dLatitudeInDegMin = atof(cpara);
			deg = floor(atof(cpara) / 100);
			minute = (atof(cpara) / 100 - deg) * 100;
			lat_in_deg = deg + minute / 60;
			GpsInfo->dLatitudeInDegree = lat_in_deg;
			g_gpsco.g_Latitude = (uint32_t)(GpsInfo->dLatitudeInDegree * 10000000);
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 3, cpara);	//北纬N 或 南纬S
			if (cpara[0] == 'N') GpsInfo->uNSFlag = 1;
			else if (cpara[0] == 'S') GpsInfo->uNSFlag = 2;
			else GpsInfo->uNSFlag = 0;
			
			/* 解析经度 4 5 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 4, cpara);	//经度 dddmm.mmmmmm
			GpsInfo->dLongitudeInDegMin = atof(cpara);
			deg = floor(atof(cpara) / 100);
			minute = (atof(cpara) / 100 - deg) * 100;
			lon_in_deg = deg + minute / 60;
			GpsInfo->dLongitudeInDegree = lon_in_deg;
			g_gpsco.g_Longitude = (uint32_t)(GpsInfo->dLongitudeInDegree * 10000000);
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 5, cpara);	//东经E 或 西经W
			if (cpara[0] == 'E') GpsInfo->uEWFlag = 1;
			else if (cpara[1] == 'W') GpsInfo->uEWFlag = 2;
			else GpsInfo->uEWFlag = 0;
			
			/* 解析定位模式 6 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 6, cpara);	//定位模式
			GpsInfo->uMode = (uint8_t)(atol(cpara));
			if(g_GPSInfo.uMode != 0 && (uint8_t)(atol(cpara)) == 0)
			{
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13, GPIO_PIN_SET);	
			}			
			/* 解析卫星数 7 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 7, cpara);	//卫星数
			GpsInfo->uSatelliteNum = (uint8_t)(atol(cpara));
			g_SatelliteNum = GpsInfo->uSatelliteNum;
			/* 解析高度 9 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 9, cpara);	//海拔高度
			GpsInfo->fAltitudeInMeter = atof(cpara);
			g_gpsco.g_Altitude = (uint32_t)(GpsInfo->fAltitudeInMeter * 1000);			
			return GPSRET_OK;
		}
	}
	
	return GPSRET_KEYWORD_ERR;
}

/******************************************************************
函数名称: GpsResult GPS_AnlysisRMC
功能描述: 处理解析RMC数据
参数输入: 
参数输出: 
返 回 值: GpsResult 返回值
其    他: 无
******************************************************************/
GpsResult GPS_AnlysisRMC(char *GpsStr, uint16_t StrLen, GpsLocationInfo_t *GpsInfo)
{
	char *ptr;
	char cpara[32];
	uint16_t slen;
	uint16_t i;
	uint16_t paranum;
	
	double lat_in_deg, lon_in_deg, alt;
	double deg, minute;
	float time;
	float spd_in_knot;
	
	/* $GNRMC,025206.00,A,3748.26368000,N,11233.23409498,E,0.006,49.8,041124,4.3,W,M,S*75 */

	/* 查找数据头 */
	ptr = strstr(GpsStr, "RMC");	
	if (ptr != NULL)
	{
		slen = strlen(ptr);
		for (i=0; i<slen; i++)
		{
			if ((ptr[i] == 0x0D) && (ptr[i+1] == 0x0A))
			{
				break;
			}
		}
		memset(g_SingleGpsBuf, 0x00, 256);
		memcpy(g_SingleGpsBuf, ptr, (i+1));
//		debug_printf("%s", sptr);
		paranum = CalcSpiltChrNum(g_SingleGpsBuf, slen, ',');	//计算总参数个数
		if (paranum != 0)
		{
			/* 解析时间 1 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 1, cpara);	//时间 HHmmss.sss
			GpsInfo->DateTimeInUTC.Hour = Asc2Hex(cpara[0]) * 10 + Asc2Hex(cpara[1]);
			GpsInfo->DateTimeInUTC.Minute = Asc2Hex(cpara[2]) * 10 + Asc2Hex(cpara[3]);
			GpsInfo->DateTimeInUTC.Second = Asc2Hex(cpara[4]) * 10 + Asc2Hex(cpara[5]);
			time = atol(cpara);
			GpsInfo->MilSecond = (uint16_t)((time - floor(time)) * 1000);
			
			/* 解析定位标记 2 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 2, cpara);	//状态 A=数据可用 V=未定位
			if ((cpara[0] == 'A') || (cpara[0] == 'D')) GpsInfo->uActived = 1; else GpsInfo->uActived = 0;
			
			/* 解析纬度 3 4 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 3, cpara);	//纬度 ddmm.mmmmmm
			GpsInfo->dLatitudeInDegMin = atof(cpara);
			deg = floor(atof(cpara) / 100);
			minute = (atof(cpara) / 100 - deg) * 100;
			lat_in_deg = deg + minute / 60;
			GpsInfo->dLatitudeInDegree = lat_in_deg;
			g_gpsco.g_Latitude = (uint32_t)(GpsInfo->dLatitudeInDegree * 10000000);
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 4, cpara);	//北纬N 或 南纬S
			if (cpara[0] == 'N') GpsInfo->uNSFlag = 1;
			else if (cpara[0] == 'S') GpsInfo->uNSFlag = 2;
			else GpsInfo->uNSFlag = 0;
			
			/* 解析经度 5 6 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 5, cpara);	//经度 dddmm.mmmmmm
			GpsInfo->dLongitudeInDegMin = atof(cpara);
			deg = floor(atof(cpara) / 100);
			minute = (atof(cpara) / 100 - deg) * 100;
			lon_in_deg = deg + minute / 60;
			GpsInfo->dLongitudeInDegree = lon_in_deg;
			g_gpsco.g_Longitude = (uint32_t)(GpsInfo->dLongitudeInDegree * 10000000);
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 6, cpara);	//东经E 或 西经W
			if (cpara[0] == 'E') GpsInfo->uEWFlag = 1;
			else if (cpara[1] == 'W') GpsInfo->uEWFlag = 2;
			else GpsInfo->uEWFlag = 0;

			/* 解析速度 7 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 7, cpara);	//地面速率 单位节(knot) 1节=1.852Km/h
			spd_in_knot = atof(cpara);
			GpsInfo->fSpeedInKmh = spd_in_knot;
			g_gpsco.g_Speed = (uint32_t)(GpsInfo->fSpeedInKmh * 10);
			/* 解析航向 8 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 8, cpara);	//地面航向 单位度
			GpsInfo->fTrackTrueInDegree = atof(cpara);
			
			/* 解析日期 9 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 9, cpara);	//日期 DDMMYY
			GpsInfo->DateTimeInUTC.Year = Asc2Hex(cpara[4]) * 10 + Asc2Hex(cpara[5]);
			GpsInfo->DateTimeInUTC.Month = Asc2Hex(cpara[2]) * 10 + Asc2Hex(cpara[3]);
			GpsInfo->DateTimeInUTC.Day = Asc2Hex(cpara[0]) * 10 + Asc2Hex(cpara[1]);
			
			/* 解析模式指示 12 */
			memset(cpara, 0x00, sizeof(cpara));
			GetStrAfterSpiltChr(g_SingleGpsBuf, slen, ',', 12, cpara);	//模式指示
			GpsInfo->cMode = cpara[0];
			
			/* 转换当地时间 校时 */
			GpsInfo->DateTimeInLocate = GpsInfo->DateTimeInUTC;
			GpsInfo->DateTimeInLocate.Hour += 8;						//东8时区
			CalcSysTimeBySoftware(&GpsInfo->DateTimeInLocate);
			
			return GPSRET_OK;
		}
	}
	
	return GPSRET_KEYWORD_ERR;
}

/*
*********************************************************************************************************
*	函 数 名: gpsGNGGA
*	功能说明: 分析0183数据包中的 GPGGA 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGGA,092204.999,4250.5589,S,14718.5084,E,1,04,24.4,19.7,M,,,,0000*1F
字段0：$GPGGA，语句ID，表明该语句为Global Positioning System Fix Data（GGA）GPS定位信息
字段1：UTC 时间，hhmmss.sss，时分秒格式
字段2：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段3：纬度N（北纬）或S（南纬）
字段4：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段5：经度E（东经）或W（西经）
字段6：GPS状态，0=未定位，1=非差分定位，2=差分定位，3=无效PPS，6=正在估算
字段7：正在使用的卫星数量（00 - 12）（前导位数不足则补0）
字段8：HDOP水平精度因子（0.5 - 99.9）
字段9：海拔高度（-9999.9 - 99999.9）
字段10：地球椭球面相对大地水准面的高度
字段11：差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
字段12：差分站ID号0000 - 1023（前导位数不足则补0，如果不是差分定位将为空）
字段13：校验值
*/
void gpsGNGGA(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;
	char *ptr;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 UTC 时间，hhmmss.sss，时分秒格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;	
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		g_tGPS.Hour = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Min = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.Sec = StrToIntFix(p, 2);
		p += 2;		
	}	
	/* 字段2 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;	
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		if(g_GPSSendFlag == 2)
			g_GPSSendFlag = 1;
		g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
		p += 2;
		g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) << 16;
		p += 3;
		g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
		p += 4;
	}	
	/* 字段3 纬度N（北纬）或S（南纬） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		if (*p == 'S')
		{
			g_tGPS.NS = 'S';
		}
		else if (*p == 'N')
		{
			g_tGPS.NS = 'N';
		}
		else
		{
			return;
		}
	}	
	/* 字段4  经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		g_tGPS.JingDu_Du = StrToIntFix(p, 3);
		p += 3;
		g_tGPS.JingDu_Fen = StrToIntFix(p, 2) << 16;
		p += 3;
		g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
		p += 4;
	}	


	/* 字段5 经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		if (*p == 'E')
		{
			g_tGPS.EW = 'E';
		}
		else if (*p == 'W')
		{
			g_tGPS.EW = 'W';
		}
	}	
	/* 字段6 GPS状态，0=未定位，1=非差分定位，2=差分定位，3=无效PPS，6=正在估算 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) != 0)
	{
		if ((*p == '1') || (*p == '2'))
		{
			g_tGPS.PositionOk = 1;
		}
		else
		{
			g_tGPS.PositionOk = 0;
		}
	}	
	/* 字段7：正在使用的卫星数量（00 - 12）（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	ptr = strchr(p, ',');
	if ((ptr - p) > 0)
	{
		g_tGPS.ViewNumber = StrToInt(p);
		p += 2;
	}	
	/* 后面的字段信息丢弃 */
}

/*
*********************************************************************************************************
*	函 数 名: gpsGNGSA
*	功能说明: 分析0183数据包中的 GPGSA 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGSA,A,3,01,20,19,13,,,,,,,,,40.4,24.4,32.2*0A
字段0：$GPGSA，语句ID，表明该语句为GPS DOP and Active Satellites（GSA）当前卫星信息
字段1：定位模式，A=自动手动2D/3D，M=手动2D/3D
字段2：定位类型，1=未定位，2=2D定位，3=3D定位
字段3：PRN码（伪随机噪声码），第1信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段4：PRN码（伪随机噪声码），第2信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段5：PRN码（伪随机噪声码），第3信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段6：PRN码（伪随机噪声码），第4信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段7：PRN码（伪随机噪声码），第5信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段8：PRN码（伪随机噪声码），第6信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段9：PRN码（伪随机噪声码），第7信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段10：PRN码（伪随机噪声码），第8信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段11：PRN码（伪随机噪声码），第9信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段12：PRN码（伪随机噪声码），第10信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段13：PRN码（伪随机噪声码），第11信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段14：PRN码（伪随机噪声码），第12信道正在使用的卫星PRN码编号（00）（前导位数不足则补0）
字段15：PDOP综合位置精度因子（0.5 - 99.9）
字段16：HDOP水平精度因子（0.5 - 99.9）
字段17：VDOP垂直精度因子（0.5 - 99.9）
字段18：校验值
*/
void gpsGNGSA(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;
	uint8_t i;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 定位模式，A=自动手动2D/3D，M=手动2D/3D */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.ModeAM = *p;

	/* 字段2 定位类型，1=未定位，2=2D定位，3=3D定位 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Mode2D3D = *p;

	/* 字段3 - 字段14 第1-12信道正在使用的卫星PRN码编号 */
	for (i = 0; i < 12; i++)
	{
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.SateID[i] = StrToInt(p);
	}

	/* 字段15：PDOP综合位置精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.PDOP = StrToInt(p);

	/* 字段16：HDOP水平精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.HDOP = StrToInt(p);

	/* 字段17：VDOP垂直精度因子（0.5 - 99.9） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.VDOP = StrToInt(p);
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPGSV
*	功能说明: 分析0183数据包中的 GPGSV 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGSV,3,1,10,20,78,331,45,01,59,235,47,22,41,069,,13,32,252,45*70

$GPGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
$GPGSV,2,2,07,09,23,313,42,04,19,159,41,15,12,041,42*41

字段0：$GPGSV，语句ID，表明该语句为GPS Satellites in View（GSV）可见卫星信息
字段1：本次GSV语句的总数目（1 - 3）
字段2：本条GSV语句是本次GSV语句的第几条（1 - 3）
字段3：当前可见卫星总数（00 - 12）（前导位数不足则补0）

字段4：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段5：卫星仰角（00 - 90）度（前导位数不足则补0）
字段6：卫星方位角（00 - 359）度（前导位数不足则补0）
字段7：信噪比（00－99）dbHz

字段8：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段9：卫星仰角（00 - 90）度（前导位数不足则补0）
字段10：卫星方位角（00 - 359）度（前导位数不足则补0）
字段11：信噪比（00－99）dbHz

字段12：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0）
字段13：卫星仰角（00 - 90）度（前导位数不足则补0）
字段14：卫星方位角（00 - 359）度（前导位数不足则补0）
字段15：信噪比（00－99）dbHz
字段16：校验值
*/
void gpsGPGSV(uint8_t *_ucaBuf, uint16_t _usLen)
{
//	uint8_t s_total = 0;	/* 语句总数目 */
	uint8_t s_no = 0;		/* 语句序号 */
	uint8_t i;
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1：本次GSV语句的总数目（1 - 3） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
//	s_total = StrToInt(p);

	/* 字段2：本条GSV语句是本次GSV语句的第几条（1 - 3） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	s_no = StrToInt(p);

	/* 字段3：当前可见卫星总数（00 - 12）（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.ViewNumber = StrToInt(p);

	for (i = 0; i < 4; i++)
	{
		/* 字段4：PRN 码（伪随机噪声码）（01 - 32）（前导位数不足则补0） */
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.SateID[(s_no - 1) * 4 + i] = StrToInt(p);

		/* 字段5：卫星仰角（00 - 90）度（前导位数不足则补0）*/
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.Elevation[(s_no - 1) * 4 + i] = StrToInt(p);

		/* 字段6：卫星方位角（00 - 359）度（前导位数不足则补0） */
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.Azimuth[(s_no - 1) * 4 + i] = StrToInt(p);

		/* 字段7：信噪比（00－99）dbHz */
		p = strchr(p, ',');
		if (p == 0)
		{
			return;
		}
		p++;
		g_tGPS.SNR[(s_no - 1) * 4 + i] = StrToInt(p);
	}
}

/*
*********************************************************************************************************
*	函 数 名: gpsGNRMC
*	功能说明: 分析0183数据包中的 GPGSV 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPRMC,024813.640,A,3158.4608,N,11848.3737,E,10.05,324.27,150706,,,A*50
字段0：$GPRMC，语句ID，表明该语句为Recommended Minimum Specific GPS/TRANSIT Data（RMC）推荐最小定位信息
字段1：UTC时间，hhmmss.sss格式
字段2：状态，A=定位，V=未定位
字段3：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段4：纬度N（北纬）或S（南纬）
字段5：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段6：经度E（东经）或W（西经）
字段7：速度，节，Knots
字段8：方位角，度
字段9：UTC日期，DDMMYY格式
字段10：磁偏角，（000 - 180）度（前导位数不足则补0）
字段11：磁偏角方向，E=东W=西
字段12：模式指示 A=自主模式, D=差分模式, E=估算模式, F=RTK浮点解, N=无效, R=RTK固定解
字段16：校验值
*/
void gpsGNRMC(uint8_t *_ucaBuf, uint16_t _usLen)
{  
	uint8_t tmptime1[6]={0};
	char *p;
	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 UTC时间，hhmmss.sss格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Hour = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Min = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Sec = StrToIntFix(p, 2);
	p += 3;
	g_tGPS.mSec = StrToIntFix(p, 3);

	/* 字段2 状态，A=定位，V=未定位 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p != 'A')
	{
		/* 未定位则直接返回 */
		g_tGPS.PositionOk = 0;
		return;
	}
	g_tGPS.PositionOk = 1;
	p += 1;

	/* 字段3 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段4 纬度N（北纬）或S（南纬）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'S')
	{
		g_tGPS.NS = 'S';
	}
	else if (*p == 'N')
	{
		g_tGPS.NS = 'N';
	}
	else
	{
		return;
	}

	/* 字段5 经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.JingDu_Du = StrToIntFix(p, 3);
	p += 3;
	g_tGPS.JingDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段6：经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'E')
	{
		g_tGPS.EW = 'E';
	}
	else if (*p == 'W')
	{
		g_tGPS.EW = 'W';
	}

	/* 字段7：速度，节，Knots  10.05,*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.SpeedKnots = StrToInt(p);

	/* 字段8：方位角，度 ,324.27 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.TrackDegTrue = StrToInt(p);

	/* 字段9：UTC日期，DDMMYY格式  150706 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Day = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Month = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Year = StrToIntFix(p, 2);
	p += 2;
	
	/* 字段10：磁偏角 */
	p = strchr(p, ',');
	if (p == NULL)
	{
		return;
	}
	p++;
	
	/* 字段11：磁偏角方向 */
	p = strchr(p, ',');
	if (p == NULL)
	{
		return;
	}
	p++;
	
	/* 字段12：模式指示 */
	p = strchr(p, ',');
	if (p == NULL)
	{
		return;
	}
	p++;
	g_tGPS.ModeInd = *p;
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPVTG
*	功能说明: 分析0183数据包中的 GPVTG 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPVTG,89.68,T,,M,0.00,N,0.0,K*5F
字段0：$GPVTG，语句ID，表明该语句为Track Made Good and Ground Speed（VTG）地面速度信息
字段1：运动角度，000 - 359，（前导位数不足则补0）
字段2：T=真北参照系
字段3：运动角度，000 - 359，（前导位数不足则补0）
字段4：M=磁北参照系
字段5：水平运动速度（0.00）（前导位数不足则补0）
字段6：N=节，Knots
字段7：水平运动速度（0.00）（前导位数不足则补0）
字段8：K=公里/时，km/h
字段9：校验值
*/
void gpsGPVTG(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1：运动角度，000 - 359，（前导位数不足则补0）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.TrackDegTrue = StrToInt(p);

	/* 字段2：T=真北参照系 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;

	/* 字段3：运动角度，000 - 359，（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.TrackDegMag = StrToInt(p);

	/* 字段4：M=磁北参照系 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;

	/* 字段5：地面速率（000.0~999.9节，前面的0也将被传输） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.SpeedKnots = StrToInt(p);

	/* 字段6：N=节，Knots */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;

	/* 字段7：地面速率（0000.0~1851.8公里/小时，前面的0也将被传输） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.SpeedKM = StrToInt(p);

	/* 字段8：K=公里/时，km/h	 */
}

/*
*********************************************************************************************************
*	函 数 名: gpsGPGLL
*	功能说明: 分析0183数据包中的 GPGLL 命令，结果存放到全局变量
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
/*
例：$GPGLL,4250.5589,S,14718.5084,E,092204.999,A*2D
字段0：$GPGLL，语句ID，表明该语句为Geographic Position（GLL）地理定位信息
字段1：纬度ddmm.mmmm，度分格式（前导位数不足则补0）
字段2：纬度N（北纬）或S（南纬）
字段3：经度dddmm.mmmm，度分格式（前导位数不足则补0）
字段4：经度E（东经）或W（西经）
字段5：UTC时间，hhmmss.sss格式
字段6：状态，A=定位，V=未定位
字段7：校验值
*/
void gpsGPGLL(uint8_t *_ucaBuf, uint16_t _usLen)
{
	char *p;

	p = (char *)_ucaBuf;
	p[_usLen] = 0;

	/* 字段1 纬度ddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.WeiDu_Du = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.WeiDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.WeiDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段2 纬度N（北纬）或S（南纬）*/
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'S')
	{
		g_tGPS.NS = 'S';
	}
	else if (*p == 'N')
	{
		g_tGPS.NS = 'N';
	}
	else
	{
		return;
	}

	/* 字段3 经度dddmm.mmmm，度分格式（前导位数不足则补0） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.JingDu_Du = StrToIntFix(p, 3);
	p += 3;
	g_tGPS.JingDu_Fen = StrToIntFix(p, 2) * 10000;
	p += 3;
	g_tGPS.JingDu_Fen += StrToIntFix(p, 4);
	p += 4;

	/* 字段4：经度E（东经）或W（西经） */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p == 'E')
	{
		g_tGPS.EW = 'E';
	}
	else if (*p == 'W')
	{
		g_tGPS.EW = 'W';
	}

	/* 字段5 UTC时间，hhmmss.sss格式 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	g_tGPS.Hour = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Min = StrToIntFix(p, 2);
	p += 2;
	g_tGPS.Sec = StrToIntFix(p, 2);
	p += 2;

	/* 字段6 状态，A=定位，V=未定位 */
	p = strchr(p, ',');
	if (p == 0)
	{
		return;
	}
	p++;
	if (*p != 'A')
	{
		/* 未定位则直接返回 */
		return;
	}
}

uint16_t glen;


/*
*********************************************************************************************************
*	函 数 名: Analyze0183
*	功能说明: 分析0183数据包
*	形    参:  _ucaBuf  收到的数据
*			 _usLen    数据长度
*	返 回 值: 无
*********************************************************************************************************
*/
void Analyze0183(uint8_t *_ucaBuf, uint16_t _usLen)
{
	uint16_t ggalen;
	
	if (memcmp(_ucaBuf+3, "GGA,", 4) == 0)
	{
		memset((uint8_t*)&g_tGPS,0,sizeof(g_tGPS));
//		debug_printf("%s\r\n", _ucaBuf);					//打印GGA信息
		ggalen = strlen((char*)_ucaBuf) - 2;				//去掉信息末尾的回车换行
		if (glen > 384) glen = 0;
		memcpy(g_chGpsSaveInfo + glen, _ucaBuf, ggalen);	//填入GGA信息
		glen += ggalen;
		memcpy(g_chGpsSaveInfo + glen, " | ", 3);			//增加分割线
		glen += 3;
		gpsGNGGA(_ucaBuf+6, _usLen);
	}
}


/******************************************************************
函数名称: HexDateTime_t CalcTimeZone
功能描述: 计算时区
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
HexDateTime_t CalcTimeZone(HexDateTime_t UTC, int8_t TimeZone)
{
	HexDateTime_t tm;
	
	/* 仅对东8时区有效 西时区需要后面处理 */
	tm.Hour = UTC.Hour + TimeZone;
	CalcSysTimeBySoftware(&tm);
	
	return tm;
}

/******************************************************************
函数名称: uint8_t AnalysisRMC
功能描述: 解析RMC数据
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void AnalysisRMC(uint8_t *_ucaBuf, uint16_t _usLen)
{
	HexDateTime_t datetime;
	BcdDateTime_t gpstime;
	
	uint16_t rmclen;
	
	if (memcmp(_ucaBuf+3, "RMC,", 4) == 0)
	{
		memset((uint8_t*)&g_tGPS,0,sizeof(g_tGPS));
//		debug_printf("%s\r\n", _ucaBuf);					//打印RMC信息
		rmclen = strlen((char*)_ucaBuf) - 2;				//去掉信息末尾的回车换行
		glen = 0;
		gpsGNRMC(_ucaBuf+6, _usLen);						//处理RMC数据
		/* 当前未校时 */
		if ((g_SysTime.Year == 0x00) && (g_SysTime.Month == 0x00) && (g_SysTime.Day == 0x00))
		{
			if ((g_tGPS.Year != 0x00) && (g_tGPS.Month != 0x00))
			{
				gpstime.Year = g_tGPS.Year;
				gpstime.Month = g_tGPS.Month;
				gpstime.Day = g_tGPS.Day;
				gpstime.Hour = g_tGPS.Hour;
				gpstime.Minute = g_tGPS.Min;
				gpstime.Second = g_tGPS.Sec;
				datetime = BcdTime2HexTime(gpstime);
//				datetime = CalcTimeZone(datetime, +8);
				datetime.Hour += 8;
				CalcSysTimeBySoftware(&datetime);
				AdjustSysTime(datetime, &g_SysTime);		//校时
			}
		}
	}
}



int16_t GPSInfoAnlysis(uint8_t *strGPS, uint16_t Size)
{
//	uint16_t i, j;
//	
//	uint8_t findhead, findtail;
//	uint16_t pos;
//	uint16_t rmclen, ggalen, millen, spdlen;
//	
//	char chrmil[10];
//	char chrspd[10];
////	char *strtax;
//	
//	pos = 0;
//	findhead = 0;
//	findtail = 0;
//	glen = 0;
//	
//	int16_t sret;
//	uint32_t elspsec;
//	
//	memset(g_chGpsSaveInfo, 0x00, sizeof(g_chGpsSaveInfo));
//	for (i=0; i<Size; i++)
//	{
//		/* 未找到GPS头 */
//		if (findhead == 0)
//		{
//			if (strGPS[i] == '$')
//			{
//				findhead = 1;
//				pos = 0;
//				memset(g_strGpsSingleInfo, 0x00, sizeof(g_strGpsSingleInfo));
//				g_strGpsSingleInfo[pos] = strGPS[i];
//				pos++;
//			}
//			else
//			{
//				pos = 0;
//			}
//		}
//		/* 找到GPS头 */
//		else
//		{
//			/* 查找当前信息末尾 */
//			g_strGpsSingleInfo[pos] = strGPS[i];
//			pos++;
//			if (findtail == 0)
//			{
//				/* 接到 \r */
//				if (strGPS[i] == '\r')
//				{
//					findtail = 1;
//				}
//				else
//				{
//					if (pos >= 127)
//					{
//						pos = 0;
//						findhead = 0;
//						findtail = 0;
//						memset(g_strGpsSingleInfo, 0x00, sizeof(g_strGpsSingleInfo));
//					}
//				}
//			}
//			else if (findtail == 1)
//			{
//				/* 接到 \r\n */
//				if (strGPS[i] == '\n')
//				{
//					/* 根据头解析 */
//					AnalysisRMC(g_strGpsSingleInfo, pos);	//解析RMC数据
////					Analyze0183(g_strGpsSingleInfo, pos);
//					pos = 0;
//					findhead = 0;
//					findtail = 0;
//					ConvGpsPositionInfo(&g_GPSInfo, &g_tGPS);
//				}
//				else
//				{
//					if (pos >= 127)
//					{
//						pos = 0;
//						findhead = 0;
//						findtail = 0;
//						memset(g_strGpsSingleInfo, 0x00, sizeof(g_strGpsSingleInfo));
//					}
//				}
//			}
//		}
//	}
}

#if 0
void GPSProcess(void)
{
	uint16_t i;
	uint8_t ucData;
	static uint8_t ucGpsHead = 0;
	static uint8_t ucaGpsBuf[512];
	static uint16_t usGpsPos = 0;
	
	if(g_Uart2RxSta == 1)
	{
		g_Uart2RxSta = 0;
		for(i=0;i<g_Uart2.usRxCount;i++)
		{
			ucData = g_DMARxBuf2[i];
			if (ucGpsHead == 0)
			{
				if (ucData == '$')
				{
					ucGpsHead = 1;
					usGpsPos = 0;
				}
			}
			else
			{
				if (usGpsPos <  sizeof(ucaGpsBuf))
				{
					ucaGpsBuf[usGpsPos++] = ucData;

					if ((ucData == '\r') || (ucData == '\n'))
					{	
						Analyze01831(ucaGpsBuf, usGpsPos);
						Analyze0183(ucaGpsBuf, usGpsPos);
						usGpsPos = 0;
						ucGpsHead = 0;
						g_tGPS.UartOk = 1;	/* 接收到正确的命令 */
					}
				}
			}
			
			if(usGpsPos > sizeof(ucaGpsBuf))
			{
				usGpsPos = 0;
				ucGpsHead = 0;
			}
		}
	}
	if(g_GPSSendFlag== 0x01)/*半小时发送GPS数据*/
	{
//		SendGPSData();
		g_GPSSendFlag = 0;
	}

}
#endif
/*
*********************************************************************************************************
*	函 数 名: gps_FenToDu
*	功能说明: 将分转换为度的小数部分，保留6位小数。 将分换算为度。
*	形    参: 无
*	返 回 值: 返回度的小数部分（十进制）
*********************************************************************************************************
*/
uint32_t gps_FenToDu(uint32_t _fen)
{
	uint32_t du;
	
	/* g_tGPS.WeiDu_Fen;	纬度，分. 232475；  小数点后4位  表示 23.2475分 */
	
	du = (_fen * 100) / 60;
	
	return du;
}

/*
*********************************************************************************************************
*	函 数 名: gps_FenToMiao
*	功能说明: 将分的小数部分转化秒
*	形    参: 无
*	返 回 值: 秒 整数部分
*********************************************************************************************************
*/
uint16_t gps_FenToMiao(uint32_t _fen)
{
	uint32_t miao;
	
	/* g_tGPS.WeiDu_Fen;	纬度，分. 232475；  小数点后4位  表示 23.2475分 
		其中小数部分 0.2475 * 60 = 14.85 四舍五入为 15秒	
		
		
		2475 * 60 = 148500
		148500 / 10000 = 14;
		
		if ((148500 % 10000) >= 5000)
		{
			miao = 14 + 1
		}
	*/
	
	miao = ((_fen % 10000) * 60);
	
	if ((miao % 10000) >= 5000)
	{
		miao = miao / 10000 + 1;	/* 5入 */
	}
	else
	{
		miao = miao / 10000;		/* 4舍 */
	}
		
	return miao;
}

void ConvGpsPositionInfo(GpsLocationInfo_t *GInfo, GPS_T *Gps)
{
//	uint32_t tmp;
//	
//	/* 南北纬 */
//	GInfo->NSFlag = 0;
//	if (Gps->NS == 'N')
//	{
//		GInfo->NSFlag = 1;
//	}
//	else if (Gps->NS == 'S')
//	{
//		GInfo->NSFlag = 2;
//	}
//	
//	/* 纬度 */
//	tmp = (uint32_t)Gps->WeiDu_Du * 1000000 + (uint32_t)Gps->WeiDu_Fen / 60;
//	GInfo->LongitudeInDegree = tmp;
//	
//	/* 东西经 */
//	GInfo->EWFlag = 0;
//	if (Gps->EW == 'E')
//	{
//		GInfo->EWFlag = 1;
//	}
//	else if (Gps->EW == 'W')
//	{
//		GInfo->EWFlag = 2;
//	}
//	
//	/* 经度 */
//	tmp = (uint32_t)Gps->JingDu_Du * 1000000 + (uint32_t)Gps->JingDu_Fen / 60;
//	GInfo->LatitudeInDegree = tmp;
//	
//	/* 速度 */
//	GInfo->Speed = (uint16_t)Gps->SpeedKM;
//	
//	/* 定位标记 */
//	GInfo->Actived = Gps->PositionOk;
//	GInfo->SatelliteNum = Gps->ViewNumber;
//	
//	/* 时间 */
//	GInfo->DateTime.Year = Bcd2Hex(g_tGPS.Day);
//	GInfo->DateTime.Month = Bcd2Hex(g_tGPS.Month);
//	GInfo->DateTime.Day = Bcd2Hex(g_tGPS.Day);
//	GInfo->DateTime.Hour = Bcd2Hex(g_tGPS.Hour);
//	GInfo->DateTime.Minute = Bcd2Hex(g_tGPS.Min);
//	GInfo->DateTime.Second = Bcd2Hex(g_tGPS.Sec);
//	
//	/* 定位模式 */
//	GInfo->Mode = g_tGPS.ModeInd;
}

/*
GPS上电后，每隔一定的时间就会返回一定格式的数据，数据格式为：

$信息类型，x，x，x，x，x，x，x，x，x，x，x，x，x

每行开头的字符都是‘$’，接着是信息类型，后面是数据，以逗号分隔开。一行完整的数据如下：

    $GPRMC,080655.00,A,4546.40891,N,12639.65641,E,1.045,328.42,170809,,,A*60



信息类型为：

GPGSV：可见卫星信息

GPGLL：地理定位信息

GPRMC：推荐最小定位信息

GPVTG：地面速度信息

GPGGA：GPS定位信息

GPGSA：当前卫星信息



1、 GPS DOP and Active Satellites（GSA）当前卫星信息

$GPGSA,<1>,<2>,<3>,<3>,,,,,<3>,<3>,<3>,<4>,<5>,<6>,<7>

<1>模式 ：M = 手动， A = 自动。
<2>定位型式 1 = 未定位， 2 = 二维定位， 3 = 三维定位。
<3>PRN 数字：01 至 32 表天空使用中的卫星编号，最多可接收12颗卫星信息。
<4> PDOP位置精度因子（0.5~99.9）
<5> HDOP水平精度因子（0.5~99.9）
<6> VDOP垂直精度因子（0.5~99.9）
<7> Checksum.(检查位).

2、 GPS Satellites in View（GSV）可见卫星信息

$GPGSV, <1>,<2>,<3>,<4>,<5>,<6>,<7>,?<4>,<5>,<6>,<7>,<8>

<1> GSV语句的总数
<2> 本句GSV的编号
<3> 可见卫星的总数，00 至 12。
<4> 卫星编号， 01 至 32。
<5>卫星仰角， 00 至 90 度。
<6>卫星方位角， 000 至 359 度。实际值。
<7>讯号噪声比（C/No）， 00 至 99 dB；无表未接收到讯号。
<8>Checksum.(检查位).
第<4>,<5>,<6>,<7>项个别卫星会重复出现，每行最多有四颗卫星。其余卫星信息会于次一行出现，若未使用，这些字段会空白。


3、Global Positioning System Fix Data（GGA）GPS定位信息
$GPGGA,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,M,<10>,M,<11>,<12>*hh

<1> UTC时间，hhmmss（时分秒）格式
<2> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
<3> 纬度半球N（北半球）或S（南半球）
<4> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
<5> 经度半球E（东经）或W（西经）
<6> GPS状态：0=未定位，1=非差分定位，2=差分定位，6=正在估算
<7> 正在使用解算位置的卫星数量（00~12）（前面的0也将被传输）
<8> HDOP水平精度因子（0.5~99.9）
<9> 海拔高度（-9999.9~99999.9）
<10> 地球椭球面相对大地水准面的高度
<11> 差分时间（从最近一次接收到差分信号开始的秒数，如果不是差分定位将为空）
<12> 差分站ID号0000~1023（前面的0也将被传输，如果不是差分定位将为空）


4、Recommended Minimum Specific GPS/TRANSIT Data（RMC）推荐定位信息
$GPRMC,<1>,<2>,<3>,<4>,<5>,<6>,<7>,<8>,<9>,<10>,<11>,<12>*hh

<1> UTC时间，hhmmss（时分秒）格式
<2> 定位状态，A=有效定位，V=无效定位
<3> 纬度ddmm.mmmm（度分）格式（前面的0也将被传输）
<4> 纬度半球N（北半球）或S（南半球）
<5> 经度dddmm.mmmm（度分）格式（前面的0也将被传输）
<6> 经度半球E（东经）或W（西经）
<7> 地面速率（000.0~999.9节，前面的0也将被传输）
<8> 地面航向（000.0~359.9度，以真北为参考基准，前面的0也将被传输）
<9> UTC日期，ddmmyy（日月年）格式
<10> 磁偏角（000.0~180.0度，前面的0也将被传输）
<11> 磁偏角方向，E（东）或W（西）
<12> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）


5、 Track Made Good and Ground Speed（VTG）地面速度信息
$GPVTG,<1>,T,<2>,M,<3>,N,<4>,K,<5>*hh

<1> 以真北为参考基准的地面航向（000~359度，前面的0也将被传输）
<2> 以磁北为参考基准的地面航向（000~359度，前面的0也将被传输）
<3> 地面速率（000.0~999.9节，前面的0也将被传输）
<4> 地面速率（0000.0~1851.8公里/小时，前面的0也将被传输）
<5> 模式指示（仅NMEA0183 3.00版本输出，A=自主定位，D=差分，E=估算，N=数据无效）


度分秒 换算: 30度 30分 65

3030.6519 = 30度 + 30.6519分， 60进制， 
30.6519 分  --> 30.6519 / 60 = 0.510865度。  30.510865度

*/
