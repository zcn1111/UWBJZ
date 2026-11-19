/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: RTK_Comm.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: RTK定位模块
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "gfunc_String.h"

#include "RTK_Comm.h"

/******* RTK命令定义 *******/
#if ((USE_LC29HBS) || (USE_LC29HDA))
const char RTKQCMD_GetVerNo[] = "$PQTMVERNO*58\r\n";											//获取当前版本号
const char RTKQCMD_SavePara[] = "$PQTMSAVEPAR*5A\r\n";											//保存配置到NVM

//const char RTKQCMD_SetSureryTime[] = "$PQTMCFGSVIN,W,1,300,30.0,0.0000,0.0000,0.0000*22\r\n";	//收敛时间300s 5分钟
//const char RTKQCMD_SetSureryTime[] = "$PQTMCFGSVIN,W,1,600,30.0,0.0000,0.0000,0.0000*27\r\n";	//收敛时间600s 10分钟
const char RTKQCMD_SetSureryTime[] = "$PQTMCFGSVIN,W,1,900,30.0,0.0000,0.0000,0.0000*28\r\n";	//收敛时间900s 15分钟
//const char RTKQCMD_SetSureryTime[] = "$PQTMCFGSVIN,W,1,1200,30.0,0.0000,0.0000,0.0000*12\r\n";	//收敛时间1200s 20分钟 定位精度30米
//const char RTKQCMD_SetSureryTime[] = "$PQTMCFGSVIN,W,1,3600,30.0,0.0000,0.0000,0.0000*14\r\n";	//收敛时间3600s 1小时

const char RTKQCMD_SetFixedSurery[] = "$PQTMCFGSVIN,W,2,0,0,-1935350.3398,4659848.2551,3888312.0552*2B\r\n";	//收敛固定位置

const char RTKQCMD_ShowSureryInfo[] = "$PQTMCFGMSGRATE,W,PQTMSVINSTATUS,1,1*58\r\n";			//显示测量信息(收敛) 1s
#endif

#if (USE_LC29HDA)
const char RTKQCMD_GetStationMode[] = "$PQTMCFGRCVRMODE,R*32\r\n";								//查询基站模式
const char RTKQCMD_SetBaseStation[] = "$PQTMCFGRCVRMODE,W,2*29\r\n";							//设置基站模式
const char RTKQCMD_SetMovingStation[] = "$PQTMCFGRCVRMODE,W,1*2A\r\n";							//设置移动站模式
const char RTKQCMD_ColdStart[] = "$PQTMCOLD*1C\r\n";											//模块冷启动
const char RTKQCMD_HotStart[] = "$PQTMHOT*4B\r\n";												//模块热启动
const char RTKQCMD_ShowPVTInfo[] = "$PQTMCFGMSGRATE,W,PQTMPVT,1,1*1C\r\n";						//显示PVT(GNSS)信息 1s
const char RTKQCMD_HideGeofenceSta[] = "$PQTMCFGMSGRATE,W,PQTMGEOFENCESTATUS,0,1*5D\r\n";		//隐藏地理围栏信息
#endif

/******* Pair命令定义 *******/

#if (USE_LC29HDA)
const char RTKACMD_SetGGAOutputEnable[] = "$PAIR062,0,1*3F\r\n";			//设置GGA按1s输出
const char RTKACMD_SetRMCOutputEnable[] = "$PAIR062,4,1*3B\r\n";			//设置RMC按1s输出
const char RTKACMD_SetGGAOutputDisable[] = "$PAIR062,0,0*3E\r\n";			//设置GGA不输出
const char RTKACMD_SetRMCOutputDisable[] = "$PAIR062,4,0*3A\r\n";			//设置RMC不输出
const char RTKACMD_SetGSVOutputDisable[] = "$PAIR062,3,0*3D\r\n";			//设置GSV不输出
const char RTKACMD_SetGSAOutputDisable[] = "$PAIR062,2,0*3C\r\n";			//设置GSA不输出
#endif

/******* RTK-UM980 命令定义 *******/
#if (USE_UM980)
/* 模式配置 */
const char RTKMCMD_GetStationMode[] = "MODE\r\n";												//获取接收机模式
const char RTKMCMD_SetRoverMode[] = "MODE ROVER\r\n";											//设置流动站模式
//const char RTKMCMD_SetFixedSurery[] = "MODE BASE -1935350.3398 4659848.2551 3888312.0552\r\n";	//设置基站模式固定点坐标
const char RTKMCMD_SetFixedSurery[] = "MODE BASE 37.8043946667 112.553901583 792.0000\r\n";	//设置基站模式固定点坐标 - 楼顶西南角
const char RTKMCMD_SaveConfig[] = "SAVECONFIG\r\n";											//保存配置

/* NMEA数据输出开启 */
const char RTKMCMD_SetGGAOutputEnable[] = "GPGGA 1\r\n";										//设置输出GGA
const char RTKMCMD_SetRMCOutputEnable[] = "GPRMC 1\r\n";										//设置输出RMC
const char RTKMCMD_SetGSVOutputEnable[] = "GPGSV 1\r\n";
const char RTKMCMD_SetGSAOutputEnable[] = "GPGSA 1\r\n";
/* NMEA数据输出关闭 */
const char RTKMCMD_SetGGAOutputDisable[] = "UNLOG GPGGA\r\n";									//设置不输出GGA
const char RTKMCMD_SetRMCOutputDisable[] = "UNLOG GPRMC\r\n";									//设置不输出RMC

/* RTCM数据输出开启 */
const char RTKMCMD_SetRTCM1033OutputEnable[] = "RTCM1033 COM1 10\r\n";							//设置输出RTCM 1033
const char RTKMCMD_SetRTCM1006OutputEnable[] = "RTCM1006 COM1 10\r\n";							//设置输出RTCM 1006
const char RTKMCMD_SetRTCM1074OutputEnable[] = "RTCM1074 COM1 1\r\n";							//设置输出RTCM 1074
const char RTKMCMD_SetRTCM1124OutputEnable[] = "RTCM1124 COM1 1\r\n";							//设置输出RTCM 1124
const char RTKMCMD_SetRTCM1084OutputEnable[] = "RTCM1084 COM1 1\r\n";							//设置输出RTCM 1084
const char RTKMCMD_SetRTCM1094OutputEnable[] = "RTCM1094 COM1 1\r\n";							//设置输出RTCM 1094

/* RTCM数据输出关闭 */
const char RTKMCMD_SetRTCM1033OutputDisable[] = "UNLOG RTCM1033\r\n";							//设置不输出RTCM 1033
const char RTKMCMD_SetRTCM1006OutputDisable[] = "UNLOG RTCM1006\r\n";							//设置不输出RTCM 1006
const char RTKMCMD_SetRTCM1074OutputDisable[] = "UNLOG RTCM1074\r\n";							//设置不输出RTCM 1074
const char RTKMCMD_SetRTCM1124OutputDisable[] = "UNLOG RTCM1124\r\n";							//设置不输出RTCM 1124
const char RTKMCMD_SetRTCM1084OutputDisable[] = "UNLOG RTCM1084\r\n";							//设置不输出RTCM 1084
const char RTKMCMD_SetRTCM1094OutputDisable[] = "UNLOG RTCM1094\r\n";							//设置不输出RTCM 1094

#endif

/******* 变量定义 *******/
uint8_t				g_RtkRecvBuf[RTK_RX_BUFSIZE];	//RTK接收缓存
uint8_t				g_RtkDiffBuf[RTK_DIFF_BUFSIZE];	//差分数据缓存
RtkSurveyinInfo_t	g_RtkSurveyInfo;				//RTK测量信息
uint16_t			g_RtkDiffLen;

LEDState_t			g_RtkLED;						//RTK指示灯
/************************************** RTK指令通信函数 ************************************************/
/******************************************************************
函数名称: RtkResult RTK_SendBuf
功能描述: 发送RTK数据
参数输入: 
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_SendBuf(uint8_t *InBuf, uint16_t StrLen)
{
	if (g_RtkSurveyInfo.Valid == 2) g_RtkLED.DispType = LEDT_ON;
	HAL_UART_Transmit(&hUSART1, InBuf, StrLen, 1000);
	if (g_DbgEcho.EnableRtkEcho == 1) debug_info((uint8_t*)InBuf, StrLen, "->RTK");
	else if (g_DbgEcho.EnableRtkEcho == 2) debug_ascinfo((uint8_t*)InBuf, StrLen, "->RTK", 0);
	if (g_RtkSurveyInfo.Valid == 2) g_RtkLED.DispType = LEDT_OFF;
	
	return RTKRET_OK;
}

/******************************************************************
函数名称: RtkResult RTK_RecvBuf
功能描述: 接收RTK数据
参数输入: 
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_RecvBuf(uint8_t *OutBuf, uint16_t *StrLen)
{
	uint16_t rlen;
	uint16_t pos;
	
	RtkResult rtkret;
	
	pos = 0;
	/* 接收RTK数据 */
	rlen = GetRingUsedLen(&g_Uart1RxRing);
	if (rlen > 0)
	{
		memset(OutBuf, 0x00, RTK_RX_BUFSIZE);
		if (rlen > RTK_RX_BUFSIZE)
		{
			RingBufClear(&g_Uart1RxRing);
			*StrLen = 0;
			return RTKRET_LEN_ERR;
		}
		else
		{
			RingBufPop(&g_Uart1RxRing, OutBuf, rlen);
			if (g_DbgEcho.EnableRtkEcho == 1) debug_info(OutBuf, rlen, "RTK->");
			else if (g_DbgEcho.EnableRtkEcho == 2) debug_ascinfo(OutBuf, rlen, "RTK->", 0);
			*StrLen = rlen;
			g_RtkLED.Toogle = 1;		//接收到数据翻转指示灯
			if (RtkCommMode == 1)
			{
				printf("%s", OutBuf);
			}
			return RTKRET_OK;
		}
	}
	else
	{
		*StrLen = 0;	//无数据
	}
	
	return RTKRET_NODATA_ERR;
}

/******************************************************************
函数名称: RtkResult RTK_SetupCommand
功能描述: 发送RTK命令
参数输入: char		*SetCmd		发送命令
		 char		*SuccRetStr	成功返回字符
		 uint8_t	RetryCnt	重试次数
		 uint16_t	Timeout		超时时间(ms)
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_SetupCommand(char *SetCmd, char *SuccRetStr, uint8_t RetryCnt, uint16_t Timeout)
{
	uint8_t rcnt;
	uint16_t slen;
	uint32_t tmrrecv;
	
	RtkResult rtkret;
	
	rcnt = 0;
	tmrrecv = uwTick;
	for (rcnt=0; rcnt<=RetryCnt; rcnt++)
	{
		/* 发送命令 */
		tmrrecv = uwTick;
		slen = strlen(SetCmd);
		RTK_SendBuf((uint8_t*)SetCmd, slen);
		printf("[>>RTK] %s", SetCmd);
		
		/* 接收返回值 */
		memset(g_RtkRecvBuf, 0x00, RTK_RX_BUFSIZE);
		while ((uwTick - tmrrecv) < Timeout)
		{
			rtkret = RTK_RecvBuf(g_RtkRecvBuf, &slen);
			if (rtkret == RTKRET_OK)
			{
				printf("[RTK>>] %s", g_RtkRecvBuf);
				if (strstr((char*)g_RtkRecvBuf, SuccRetStr) != NULL)
				{
					/* 匹配到结果字符串 */
					return RTKRET_OK;
				}
			}
		}
	}
	
	return RTKRET_TIMEOUT_ERR;
}

/******************************************************************
函数名称: RtkResult RTK_Reset
功能描述: RTK模块断电复位
参数输入: uint16_t	PowerOffTime	断电时间(ms)
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_Reset(uint16_t PowerOffTime)
{
	debug_printf("RTK Reset...\r\n");
	HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(RTK_RESET_PORT, RTK_RESET_PIN, GPIO_PIN_SET);
	HAL_Delay(PowerOffTime);
	HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(RTK_RESET_PORT, RTK_RESET_PIN, GPIO_PIN_RESET);
	
	return RTKRET_OK;
}

#if (USE_LC29HDA)
/******************************************************************
函数名称: RtkResult RTK_InitMoudle
功能描述: RTK模块初始化
参数输入: 
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_InitMoudle(void)
{
	RtkResult rtkret;
	

	/* 配置基站模式 */
#if (USE_LC29HDA)
	rtkret = RTK_SetupCommand((char*)RTKQCMD_GetStationMode, "OK,2", 3, 2000);	//查询模块基站模式
	if (rtkret != RTKRET_OK)
	{
		rtkret = RTK_SetupCommand((char*)RTKQCMD_SetBaseStation, "OK", 4, 2000);	//配置模块为基站模式
		if (rtkret != RTKRET_OK)
		{
			/* 配置错误时执行 */
		}
	}
#endif
	
//	rtkret = RTK_SetRTKCommand((char*)RTKQCMD_SetMovingStation, "OK", 4, 1000);
	
	/* 配置测量(收敛)时间 */
	rtkret = RTK_SetupCommand((char*)RTKQCMD_SetSureryTime, "OK", 3, 2000);
	if (rtkret != RTKRET_OK)
	{
		/* 配置错误时执行 */
	}
	rtkret = RTK_SetupCommand((char*)RTKQCMD_SetFixedSurery, "OK", 3, 2000);
	
	/* 配置显示测量(收敛)状态 */
	rtkret = RTK_SetupCommand((char*)RTKQCMD_ShowSureryInfo, "OK", 3, 2000);
	if (rtkret != RTKRET_OK)
	{
		/* 配置错误时执行 */
	}

#if (USE_LC29HDA)
	/* 配置显示RMC */
	rtkret = RTK_SetupCommand((char*)RTKACMD_SetRMCOutputEnable, "PAIR001", 3, 2000);
	if (rtkret != RTKRET_OK)
	{
		/* 配置错误时执行 */
	}
	
	/* 配置显示GGA */
	rtkret = RTK_SetupCommand((char*)RTKACMD_SetRMCOutputEnable, "PAIR001", 3, 2000);
	if (rtkret != RTKRET_OK)
	{
		/* 配置错误时执行 */
	}
#endif
	
	/* 配置隐藏地理围栏状态 */
//	rtkret = RTK_SetRTKCommand((char*)RTKQCMD_HideGeofenceSta, "OK", 3, 1000);
//	if (rtkret != RTKRET_OK)
//	{
//		/* 配置错误时执行 */
//	}

	/* 保存配置 */
	rtkret = RTK_SetupCommand((char*)RTKQCMD_SavePara, "OK", 3, 2000);
	if (rtkret != RTKRET_OK)
	{
	}
	
	return RTKRET_OK;
}
#endif

#if (USE_UM980)
/******************************************************************
函数名称: RtkResult RTK_InitRoverModule_UM980
功能描述: RTK模块初始化流动站模式(UM980)
参数输入: 
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_SetRoverModule_UM980(uint8_t SaveCfg)
{
	uint8_t i;
	
	RtkResult rtkret;
	
	/* 模块输出GGA和RMC */
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetGGAOutputEnable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRMCOutputEnable, "OK", 3, 1000);
//	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetGSVOutputEnable, "OK", 3, 1000);
//	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetGSAOutputEnable, "OK", 3, 1000);
//	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRMCOutputEnable, "OK", 3, 1000);
	
	/* 模块关闭输出RTMC数据 */
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1033OutputDisable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1006OutputDisable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1074OutputDisable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1124OutputDisable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1084OutputDisable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1094OutputDisable, "OK", 3, 1000);
		
	/* 检查当前模式 */
	rtkret = RTK_SetupCommand((char*)RTKMCMD_GetStationMode, "MODE ROVER", 3, 1000);
	if (rtkret == RTKRET_OK)
	{
		/* 当前为流动站 */
//		HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_RESET);
//		HAL_Delay(5000);
//		HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_SET);
		return RTKRET_OK;
	}
	
	/* 当前不是流动站 配置流动站模式 */
	for (i=0; i<3; i++)
	{
		rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRoverMode, "OK", 0, 1000);
		if (rtkret == RTKRET_OK)
		{
			rtkret = RTK_SetupCommand((char*)RTKMCMD_GetStationMode, "MODE ROVER", 0, 1000);
			if (rtkret == RTKRET_OK)
			{
				/* 配置成功 */
				if (SaveCfg == 0)
				{
					return RTKRET_OK;
				}
				else
				{
					/* 保存配置到模块 */
					rtkret = RTK_SetupCommand((char*)RTKMCMD_SaveConfig, "OK", 3, 1000);
					if (rtkret == RTKRET_OK)
					{
						return RTKRET_OK;
					}
				}
			}
			else
			{
				/* 配置失败 等待2s重新配置 */
				HAL_Delay(2000);
			}
		}
		else
		{
			HAL_Delay(2000);
		}
	}
	
	/* 命令超时失败 */
	return RTKRET_TIMEOUT_ERR;
}

/******************************************************************
函数名称: RtkResult RTK_SetBaseFixModule_UM980
功能描述: RTK模块初始化基站固定点模式(UM980)
参数输入: double		Lat		固定点纬度
		  double	Lon		固定点经度
		  double	Alt		固定点高度
		  uint8_t	SaveCfg	保存配置到模块 0=不保存;其他=保存
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_SetBaseFixModule_UM980(double Lat, double Lon, double Alt, uint8_t SaveCfg)
{
	RtkResult rtkret;
	char setcmd[64]={0};
	char retcmd[64]={0};
	
	/* 生成命令 */
	memset(setcmd, 0x00, 64);
	sprintf(setcmd, "MODE BASE %2.9f %3.8f %4.4f\r\n", Lat, Lon, Alt);	//设置命令
	memset(retcmd, 0x00, 64);
	sprintf(retcmd, "MODE BASE %2.9f %3.8f %4.4f", Lat, Lon, Alt);		//返回命令	
	/* 模块开启输出RTMC数据 */
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1033OutputEnable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1006OutputEnable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1074OutputEnable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1124OutputEnable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1084OutputEnable, "OK", 3, 1000);
	rtkret = RTK_SetupCommand((char*)RTKMCMD_SetRTCM1094OutputEnable, "OK", 3, 1000);
	/* 检查当前模式 */
	rtkret = RTK_SetupCommand((char*)RTKMCMD_GetStationMode, retcmd, 3, 3000);
	if (rtkret == RTKRET_OK)
	{
		/* 当前是固定点模式 且 固定点坐标正确 */
		return RTKRET_OK;
	}
	
	/* 当前模式不对或固定点不对 配置固定点 */
	rtkret = RTK_SetupCommand(setcmd, "OK", 3, 2000);					//设置固定点
	if (rtkret == RTKRET_OK)
	{
		if (SaveCfg == 0)
		{
			return RTKRET_OK;
		}
		/* 设置成功保存参数 */
		rtkret = RTK_SetupCommand((char*)RTKMCMD_SaveConfig, "OK", 3, 3000);
		if (rtkret == RTKRET_OK)
		{
			return RTKRET_OK;
		}
	}	
	return RTKRET_TIMEOUT_ERR;
}
#endif

/************************************** RTK处理函数 ************************************************/

/******************************************************************
函数名称: RtkResult RTK_GetRTCMInfo
功能描述: 获取RTK差分信息
参数输入: 
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_GetRTCMInfo(uint8_t *InBuf, uint16_t BufLen, uint8_t *DiffBufOut, uint16_t *DiffLen)
{
	uint16_t i;
	uint16_t datlen, difflen;

	i = 0;
	difflen = 0;
	/* RTCM协议数据格式 数据头0xD3(1)+数据长度(2)不超过1023字节+数据体(N)+CRC24(3) */
	do
	{
		/* 检查RTCM协议头 0xD3 */
		if (InBuf[i] == 0xD3)
		{
			datlen = (uint16_t)InBuf[i+1] * 256 + (uint16_t)InBuf[i+2];	//计算数据长度
			if (datlen < 1023)
			{
				/* 检查剩余字节不够剩余数据长度 */
				if ((datlen + 6) > (BufLen - i))
				{
					/* 数据不够 退出循环 */
					break;
				}
				memcpy(DiffBufOut + difflen, InBuf + i, (datlen + 6));	//复制一条D3数据差分缓存
				difflen += datlen + 6;									//数据长度
				i += datlen + 6;										//数据指针向后挪动
			}
			else
			{
				/* 数据长度错误 查找下一个字节 */
				i++;
			}
		}
		else
		{
			/* 数据头错误 查找下一个字节 */
			i++;
		}
	} while (i < BufLen);
	
	/* 返回值 */
	*DiffLen = difflen;
	if (difflen != 0)
	{
		return RTKRET_OK;
	}
	
	return RTKRET_NODATA_ERR;
}

#if (USE_LC29HDA)
/******************************************************************
函数名称: RtkResult RTK_GetSurverInfo
功能描述: 获取RTK测量信息
参数输入: 
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
RtkResult RTK_GetSurverInfo(uint8_t *InBuf, uint16_t BufLen, RtkSurveyinInfo_t *SurveyInfo)
{
	/* $PQTMSVINSTATUS,1,2331,2,,11,300,300,-1935584.8545,4660409.5595,3888796.7980,22.7*03 */
	char *ptr;
	char sptr[128]={0};
	char cpara[20];
	uint8_t paranum;
	uint16_t slen, i;
	
	/* 查找数据头 */
	ptr = strstr((char*)InBuf, "$PQTMSVINSTATUS");	
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
		memcpy(sptr, ptr, (i+1));
		debug_printf("%s", sptr);
		paranum = CalcSpiltChrNum(sptr, slen, ',');	//计算总参数个数
		if (paranum != 0)
		{
			/* 获取测量状态 */
			memset(cpara, 0x00, 20);
			GetStrAfterSpiltChr(sptr, slen, ',', 3, cpara);	//收敛状态 1=收敛中 2=已收敛
			SurveyInfo->Valid = atol(cpara);
			memset(cpara, 0x00, 20);
			GetStrAfterSpiltChr(sptr, slen, ',', 6, cpara);	//观测数
			SurveyInfo->Obs = atol(cpara);
			memset(cpara, 0x00, 20);
			GetStrAfterSpiltChr(sptr, slen, ',', 7, cpara);	//配置观测周期
			SurveyInfo->CfgDur = atol(cpara);
			memset(cpara, 0x00, 20);
			GetStrAfterSpiltChr(ptr, slen, ',', 8, cpara);	//ECEF坐标 X值
			SurveyInfo->MeanX = atof(cpara);
			memset(cpara, 0x00, 20);
			GetStrAfterSpiltChr(sptr, slen, ',', 9, cpara);	//ECEF坐标 Y值
			SurveyInfo->MeanY = atof(cpara);
			memset(cpara, 0x00, 20);
			GetStrAfterSpiltChr(sptr, slen, ',', 10, cpara);//ECEF坐标 Z值
			SurveyInfo->MeanZ = atof(cpara);
			
			
			/* 收敛参数错误处理 */
//			if (SurveyInfo->CfgDur != 900)
//			{
//				HAL_UART_Transmit(&hUSART1, (uint8_t*)RTKQCMD_SetSureryTime, strlen(RTKQCMD_SetSureryTime), 1000); //重新收敛
//			}
			
			if (SurveyInfo->Valid == 0)
			{
				return RTKRET_SURVEY_NOVAILD;
			}
			else if (SurveyInfo->Valid == 1)
			{
				return RTKRET_SURVEY_PROGING;
			}
			else if (SurveyInfo->Valid == 2)
			{
				return RTKRET_SURVEY_OK;
			}
		}
	}
	
	return RTKRET_TYPE_ERR;
}
#endif


