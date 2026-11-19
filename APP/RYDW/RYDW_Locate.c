/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: RYDW_Locate.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 人员定位定位流程
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"

#include "GPS_Comm.h"
#include "RTK_Comm.h"
#include "CORS_Comm.h"
#include "gfunc_String.h"
#include "RYDW_Display.h"
#include "RYDW_StationCard.h"
#include "RYDW_Locate.h"

#include "ssd1306.h"
#include "ssd1306_fonts.h"

#include "string.h"
#include "math.h"
#include "ssd1306_tests.h"
#include "app_tool.h"
#include "g_GlobalValDef.h"
#include "app_dtu.h"
#include "RYDW_Init.h"
/********* 变量定义 **********/
uint8_t	g_LocationProg;
uint8_t g_FixedCnt;
uint8_t  g_SelfContCORS = 0;         //自收敛状态收到CORS模式
uint8_t  g_MarkWarnFlag = 2;         //基准合法标记 =1不合法，=0合法,=2开机初始化
uint32_t g_LocationDispTimer;
uint32_t g_DTUResetTimer;
uint32_t g_SendSignalDelay;
uint8_t  g_SendSignalFlag=1;
/************************************** 定位处理函数 ************************************************/
#define EARTH_RADIUS                6371000.0 // 地球半径（米）
double haversine_distance(double lat1, double lon1, double lat2, double lon2)
{
	// 使用更精确的弧度转换系数
    const double DEG_TO_RAD = 0.01745;
    
    // 转换为弧度（
    double phi1 = lat1 * DEG_TO_RAD;
    double phi2 = lat2 * DEG_TO_RAD;
    double delta_phi = (lat2 - lat1) * DEG_TO_RAD;
    double delta_lambda = (lon2 - lon1) * DEG_TO_RAD;

    // Haversine公式计算（使用双精度数学函数）
    double a = sin(delta_phi / 2.0) * sin(delta_phi / 2.0) +
              cos(phi1) * cos(phi2) *
              sin(delta_lambda / 2.0) * sin(delta_lambda / 2.0);

    // 防止计算误差导致a超出[0,1]范围
    a = fmax(0.0, fmin(a, 1.0));

    // 计算大圆距离
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    
    // 最终计算结果
    double distance = EARTH_RADIUS * c;

    // 添加溢出保护（虽然正常情况下不需要）
    return isfinite(distance) ? distance : -1.0; // 返回-1表示计算错误
}
/******************************************************************
函数名称: void RYDW_MarkJudg
功能描述: 定位处理流程
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_MarkJudg(void)
{
	RtkResult rtkret=0xFFFF;	
	CorsResult corsret=0xFFFF;

	if(g_gpsco.g_Latitude != 0 && g_gpsco.g_Latitude != 0xFFFFFFFF && 
		g_gpsco.g_Longitude != 0 && g_gpsco.g_Longitude !=0xFFFFFFFF)   
	{//当前有经纬度
		ReadMarkFromFlash();                                                                                    //读取标记的经纬度
		double Longitude_0 = (double)g_MarkInfo.longitude / 10000000;
		double Latitude_0 = (double)(g_MarkInfo.latitude) / 10000000;
		double fAltitude_0 = (double)(g_MarkInfo.fAltitude) / 1000;
		double Longitude = (double)g_gpsco.g_Longitude /10000000;
		double Latitude = (double)g_gpsco.g_Latitude /10000000;		
		
		if(haversine_distance(Latitude_0,Longitude_0,Latitude,Longitude) > 10 )       //比较当前位置和基准点距离
		{//若超范围，提示且不再自检
			g_LocationProg = 0x00;  
			if(g_MarkWarnFlag != 0x01)
			{
				g_MarkWarnFlag = 0x01;    //报警
				ssd1306_CORSBenchmarkface();
			}
		}
		else
		{				
			rtkret = RTK_SetBaseFixModule_UM980(Latitude_0,Longitude_0,fAltitude_0, 0);
			if (rtkret == RTKRET_OK)
			{		
				g_LocationProg = 0xFF;
				g_MarkWarnFlag = 0;		
				ssd1306_Fill(Black);			//清除屏幕显示
				ssd1306_UpdateScreen();			//刷新屏幕
				/* 完成配置 */
				ssd1306_maininterface();									
			}			
		}
	}
	else
	{//定位异常
	}
	
                      
	
	
}


/******************************************************************
函数名称: void RYDW_LocateProcess
功能描述: 定位处理流程
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
uint8_t RYDW_LocateProcess(uint8_t NowProg, GpsLocationInfo_t *GpsInfo)
{
	GpsResult gpsret=0xFFFF;
	uint16_t gpslen;
	
	RtkResult rtkret=0xFFFF;
	uint16_t rtklen;
	
	CorsResult corsret=0xFFFF;
	uint16_t corslen;
	uint8_t rssi;
	
	char stanum[10]={0};
	uint8_t	ret;

	ret = NowProg;
	/* Step 0:定位开始 */
	if (NowProg == 0)
	{
		ssd1306_Fill(Black);
		ssd1306_UpdateScreen();			//刷新屏幕
		RYDW_DispString(0, 0, "Set ROVER...");
		debug_printf("Set RTK\r\n");
		return 1;
	}
	/* Step 1:配置RTK流动站模式 接收差分数据 */
	if (NowProg == 1)
	{
		rtkret = RTK_SetRoverModule_UM980(0);		//初始化RTK流动站模式
		if (rtkret == RTKRET_OK)
		{
			debug_printf("Set RTK OK!\r\n");
			g_DTUResetTimer = uwTick;
			RYDW_DispString(0, 13, "OK!");
			RYDW_DispString(1, 0, "Set CORS...");
			/* 配置成功 进入步骤2 */
			return 2;
		}
	}
	/* Step 2:配置CORS模块 连接CORS账号 */
	else if (NowProg == 2)
	{
		/* 接收RTK数据 内含GPS数据 */
		rtkret = RTK_RecvBuf(g_RtkRecvBuf, &rtklen);
		if (rtkret == RTKRET_OK)
		{
			/* 解析RMC数据 */
			gpsret = GPS_AnlysisRMC((char*)g_RtkRecvBuf, rtklen, GpsInfo);
			if (gpsret == GPSRET_OK)
			{
				/* 解析成功 */
				if ((g_SysTime.Year == 0x00) && (g_SysTime.Month == 0x00))
				{
					AdjustSysTime(GpsInfo->DateTimeInLocate, &g_SysTime);
				}
			}
			/* 解析GGA数据 */

			gpsret = GPS_AnlysisGGA((char*)g_RtkRecvBuf, rtklen, GpsInfo);
			if (gpsret == GPSRET_OK)
			{
				/* 接收到GGA数据 判断固定解 连续接收5个固定解 */
				if ((GpsInfo->uMode == 4) && (GpsInfo->uActived == 1) && (g_FixedCnt >= 5))
				{
					RYDW_DispString(2, 13, "OK!");
					RYDW_DispString(3, 0, "Set FIXED...");
					/* 获得固定解 进入步骤4 */
					return 3;
				}
				else
				{
					if (GpsInfo->uMode == 4)
					{
						g_FixedCnt++;
					}
					else
					{
						g_FixedCnt = 0;
					}
					g_NeedSendGGAFlag =1;
				}
			}
			
			RYDW_DispString(6, 0, "            ");
			sprintf(stanum, "S=%02d", GpsInfo->uSatelliteNum);
			RYDW_DispString(6, 8, stanum);
			if (GpsInfo->uMode == 0)
			{
				RYDW_DispString(6, 0, "NONE");
			}
			else if (GpsInfo->uMode == 1)
			{
				RYDW_DispString(6, 0, "SINGLE");
			}
			else if (GpsInfo->uMode == 2)
			{
				RYDW_DispString(6, 0, "DIFF");
			}
			else if (GpsInfo->uMode == 5)
			{
				RYDW_DispString(6, 0, "FLOAT");
			}
			else if (GpsInfo->uMode == 4)
			{
				RYDW_DispString(6, 0, "FIXED");
			}
			memset(g_RtkRecvBuf, 0x00, RTK_RX_BUFSIZE);
		}
	}
	else if (NowProg == 3)
	{
		rtkret = RTK_SetBaseFixModule_UM980(GpsInfo->dLatitudeInDegree, GpsInfo->dLongitudeInDegree, GpsInfo->fAltitudeInMeter, 0);
		if (rtkret == RTKRET_OK)
		{
			if(g_SelfContCORS == 0x01)		
			{
				g_MarkInfo.longitude = g_gpsco.g_Longitude;	
				g_MarkInfo.latitude = g_gpsco.g_Latitude;
				g_MarkInfo.fAltitude = g_gpsco.g_Altitude;
				WriteMarkToFlash();					
			}			
			return 4;				
		}
	}
	/* Step 5:完成 */
	else if (NowProg == 4)
	{
		ssd1306_Fill(Black);			//清除屏幕显示
		ssd1306_UpdateScreen();			//刷新屏幕
		ssd1306_maininterface();
		debug_printf("Locate successfully!\r\n");
		g_SelfContCORS = 0;
		g_MarkWarnFlag = 0;	
		CelluarStep = STA_CLOSE_TCP;
		g_SocketId = corssocketid;
		g_ChangeCORSACCFlag = 1;
		g_CorsNetOk = 0;		
		return 0xFF;
	}
	
	return ret;
	
}






/*索要信号强度*/
void SendNetSignal(void)
{
//	CorsResult corsret=0xFFFF;
//	uint8_t rssi;
//	if(JudgeMilSecondDelayInPro(g_SendSignalDelay, 30000) == true || g_SendSignalFlag == 1)
//	{/*多长时间索要一次信号强度和连接状态30s*/
//		g_SendSignalFlag = 0;
//		g_SendSignalDelay = uwTick;
//		corsret = CORS_GetNetSignalQuality_ML307A(&rssi);
//		if (corsret == CORSRET_OK)
//		{
//			g_Signal = rssi;
//			if ((rssi != 99) && (rssi > 10))
//			{
//				debug_printf("Set CORS OK RSSI=%d\r\n", rssi);					
//				/*显示信号强度*/
////				corsret = CORS_SetupCommand((char*)CORSMCMD_GetGSMStatus, "6001", 3, 1000);
////				if (corsret == CORSRET_OK)
////				{
////					debug_printf("Connect Server OK\r\n");
////					g_FixedCnt = 0;
////					/*连接正常*/
////				}	
//			}
//		}
//		else
//		{
//			/* 连接失败 断电重新连接 */
//	//			HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);
//	//			HAL_Delay(2000);
//	//			HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);
//			
//	//			corsret = CORS_InitModule_ML307A();
//			if (corsret == CORSRET_OK)
//			{
//				/* 配置成功 进入步骤3 */
//				memset(g_RtkRecvBuf, 0x00, RTK_RX_BUFSIZE);
//				memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
//			}
//		}
//	}
}

