/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: RYDW_Init.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 人员定位初始化
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "RYDW_Display.h"
#include "RYDW_StationCard.h"
#include "RYDW_Locate.h"
#include "RYDW_Init.h"
#include "RYDW_Postion.h"
/* RTK */
#include "GPS_Comm.h"
#include "RTK_Comm.h"
#include "gfunc_String.h"
/* Lora */
#include "LORA_Comm.h"
#include "LORA_Base.h"

/* OLED */
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
//#include "GT20L16S1Y.h"
/* DTU和CORS */
#include "DTU_Process.h"
#include "DTU_BulkData.h"
#include "CORS_Comm.h"

/* 蓝牙 */
#include "BLE_Comm.h"

#include "string.h"
#include "math.h"
#include "OLED.h"

/* IMU */
#include "IMU_Acc.h"
#include "app_dtu.h"


/********* 变量定义 **********/
//uint8_t	g_LocationProg;
//uint32_t g_DTUResetTimer;
uint8_t g_OledContrast=0xFF;
uint8_t g_OpenOLEDPower;
uint32_t g_OLEDPowerOffDelay;
uint8_t g_InitSuccflag;   //bit0=imu;bit1=蓝牙;bit2=rtk;bit3=dtu;


uint8_t  g_NeedInitDTU;
uint8_t  g_NeedInitRTK;
uint32_t g_NeedInitRtkDelay;
uint32_t g_NeedInitDTUDelay;
uint32_t g_ControlMsgDelay;
uint8_t  g_DataNetOk = 0;
uint8_t  g_CorsNetOk = 0;
/************************************** 初始化函数 ************************************************/



unsigned char DZ_Data[256];

/******************************************************************
函数名称: void RYDW_OLEDInit
功能描述: 显示初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_OLEDInit(void)
{
	g_OpenOLEDPower = 1;
	/* OLED初始化 */
	if(g_OpenOLEDPower == 1)
	{
		HAL_GPIO_WritePin(OLED_PWR_PORT, OLED_PWR_PIN, GPIO_PIN_RESET);		//开启OLED电源 低有效
		HAL_Delay(100);
		ssd1306_Init();			//OLED初始化
		ssd1306_SetContrast(g_OledContrast);
		ssd1306_InitDrawBitmap();
	}
	else 
	{
		LedPowerOFFInit();	
	}
    g_BribeFlag.RouLin = 1; //上
	g_BribeFlag.run = 1;    //运行
	g_OLEDPowerOffDelay = uwTick;	
}

/******************************************************************
函数名称: void RYDW_LoraInit
功能描述: LORA模块初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_LoraInit(void)
{
	ssd1306_ZhengChang(36, 27, White,1);
}

/******************************************************************
函数名称: void RYDW_RtkInit
功能描述: RTK模块初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_RtkInit(void)
{
	RtkResult rtkret;
	
	/* RTK模块初始化 */
//	RYDW_DispString(1, 0, "Init RTK...");
	HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_SET);			//开启RTK电源
	HAL_GPIO_WritePin(RTK_RESET_PORT, RTK_RESET_PIN, GPIO_PIN_RESET);	//RTK复位置高
//	RegisterLEDState(&g_LEDGroup, RTK_LED_PORT, RTK_LED_PIN, &g_RtkLED);//注册RTK指示灯
	g_RtkLED.DispType = LEDT_BLINK_S;
//	HAL_Delay(1000);
	
	/* 配置RTK模块 */
#if (USE_UM980)
	rtkret = RTK_SetRoverModule_UM980(0);	//配置为流动站
//	RTK_InitMoudle();
#endif
	if(g_OpenOLEDPower == 1 && g_NeedInitRTK == 0)
	{
		if (rtkret == RTKRET_OK)
		{
			ssd1306_ZhengChang(36, 48, White,1);
			g_RtkLED.DispType = LEDT_ON;
			g_InitSuccflag |= 0x04;
		}
		else
		{
			ssd1306_ZhengChang(36, 48, White,0);
			g_RtkLED.DispType = LEDT_OFF;
		}
	}
	ReadIDFromFlash();
	g_PostionMode = 0;
	g_PostionOrder = 0;
	/* 打点数据初始化 */
	memset( (uint8_t*)&g_PostionTable, 0x00, sizeof(g_PostionTable));
	g_DispRefreshDelay = uwTick;	//重置刷新定时器
	g_RtkLED.DispType = LEDT_FILP;	//RTK指示灯配置为翻转模式
	g_LocationDispTimer = uwTick;	
}

/******************************************************************
函数名称: void RYDW_DtuInit
功能描述: RTK模块初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_DtuInit(void)
{
	uint8_t cmpipbuf[2][32];												
	memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
	memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));

	HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);			//开启DTU电源
	g_CelluarPowerOn = 1;
	g_CelluarPowerOnDelay = uwTick;
	g_MarkWarnFlag = 2;
	ReadCorsAccFromFlash();      //读CORS账号用户名密码	
	ReadDTUSysPar();	
	InitNet();		
	ssd1306_ZhengChang(102, 48, White,1);
	g_InitSuccflag |= 0x08;		
//	uint8_t retfail=0;
//	uint8_t step=0;
//	uint16_t scode=0xFFFF;
//	char stastr[10];
//	uint8_t cmpipbuf[2][32];												
//	memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
//	memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));	
//	CorsResult corsret;
//	g_MarkWarnFlag = 2;
//	ReadCorsAccFromFlash();
//	ReadDTUSysPar();	
//	/* DTU初始化 */
//	HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);			//开启DTU电源
//	g_CelluarPowerOn = 1;
//	g_CelluarPowerOnDelay = uwTick;;
//	g_DataNetOk = 0;
//	DTU_TimerInit();
//	HAL_Delay(1000);
//	CORS_SetupDataSvr_ML307A();		//配置数据服务器
	if(g_ModeInFlash.type == 1 )
	{
		if( memcmp(g_CorsAccFlash.user,cmpipbuf[0],32) == 0 || memcmp(g_CorsAccFlash.user,cmpipbuf[1],32) == 0 
			||  memcmp(g_CorsAccFlash.pass,cmpipbuf[0],16) == 0 || memcmp(g_CorsAccFlash.pass,cmpipbuf[1],16) == 0)
		{
			g_MarkWarnFlag = 1;
		}
		else
			g_MarkWarnFlag = 0;
	}
//	/* 配置CORS模块 */
//#if (USE_ML307A)
//	/* 等待联网成功 */
//	retfail = 0xFF;
//	step = 0;
//	do
//	{
//		scode = 0;
//		corsret = CORS_GetStatusCode_ML307A(&scode, 1000);
//		if (corsret == CORSRET_OK)
//		{
//			switch (scode)
//			{
//				/* 5001=识别到卡并注册成功 */
//				case 5001:
//					if(g_ModeInFlash.type == 0 && g_SelfContCORS == 0x00)//自收敛模式
//						retfail = 0;
//					break;
//				/* 5002=未识别卡 */
//				case 5002:
//					retfail = 1;
//					break;
//				/* 6001=鉴权成功 */
//				case 6001:
//					retfail = 0;
//					g_DataNetOk = 1;
//				    memset((uint8_t*)&g_Cors, 0x00, sizeof(g_Cors)); 
//					break;
//				/* 6005=鉴权失败 账号或密码错误 */
//				case 6005:
//				case 6007:
//					retfail = 2;
//					CORS_SetupDataSvr_ML307A();		//配置数据服务器
//					if(g_ModeInFlash.type == 1)
//						CORS_SetupNtrip_ML307A(g_CorsAccFlash.user,g_CorsAccFlash.pass);		//配置Ntrip服务器
//					else if(g_ModeInFlash.type == 0 && g_SelfContCORS == 0x01)
//						CORS_SetupNtrip_ML307A(g_Cors.user,g_Cors.pass);
//					retfail = 0xFF;
//					break;
//				/* 6008=未连接Ntrip服务器 */
//				case 6008:
//					retfail = 3;
//					break;
//				default:
//					break;
//			}
//			memset(stastr, 0x00, 10);
//			sprintf(stastr, "STA:%d", scode);
////			RYDW_DispString(6, 0, stastr);
//			if (retfail != 0xFF)
//			{
//				break;
//			}
//		}
//		step++;
//	} while (step < 180);
//#endif
//	if(g_OpenOLEDPower == 1 && g_NeedInitDTU == 0)
//	{
//		if(g_FileSendDataToDtu < 180)
//		{
//			if (retfail == 0)
//			{
//				ssd1306_ZhengChang(102, 48, White,1);
//				g_InitSuccflag |= 0x08;				
//			}
//			else
//			{
//				ssd1306_ZhengChang(102, 48, White,0);
//			}
//		}
//	}
//	g_SendD6Suc = true;
}

/******************************************************************
函数名称: void RYDW_DtuInit
功能描述: RTK模块初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_BleInit(void)
{
	BLEResult bleret;
	
	/* 蓝牙初始化 */
	bleret = BLE_InitModule();
	if(g_OpenOLEDPower == 1)
	{
		if (bleret == BLERET_OK)
		{
			g_InitSuccflag |= 0x02;
			ssd1306_ZhengChang(102, 27, White,1);
		}
		else
		{
			ssd1306_ZhengChang(102, 27, White,0);
		}
	}
}


/******************************************************************
函数名称: void RYDW_IMUInit
功能描述: IMU模块初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_IMUInit(void)
{
	IMU_InitDriver(&g_IMUDevCtx);			//初始化IMU驱动
	g_tmrIMUDelay = uwTick;
	g_tmrIMUDispDelay = uwTick;	
}





/******************************************************************
函数名称: void RYDW_Init
功能描述: 功能初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RYDW_Init(void)
{
//	ReadModeFromFlash();
//	ReadIDFromFlash();
//	memset((uint8_t*)&g_Cors, 0x00, sizeof(g_Cors));       
//	/* 模块初始化 */
//	g_InitSuccflag = 0;   //bit0=imu;bit1=蓝牙;bit2=rtk;bit3=dtu;
//	g_NeedInitRTK = 0;
//	g_NeedInitDTU = 0;
//	RYDW_OLEDInit();
//	RYDW_LoraInit();
//	RYDW_IMUInit();
//	RYDW_BleInit();
//	HAL_Delay(1000);
//	RYDW_RtkInit();
//	HAL_Delay(1000);
//	RYDW_DtuInit();
//	HAL_Delay(1000);	
//	
//	if(g_InitSuccflag == 0x0f)
//	{
//		WT588E_PLAY(0x01);//
//	}
//	else
//		WT588E_PLAY(0x02);
}



void RtkMode_Init(void)
{
	if(g_CtrlMsgFlag == 0 && g_NeedInitRTK ==1 && JudgeMilSecondDelayInPro(g_ControlMsgDelay, 1000) == true)
	{
		/* 关闭RTK电源 */
		HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_RESET);		//关闭RTK电源
		HAL_GPIO_WritePin(RTK_RESET_PORT, RTK_RESET_PIN, GPIO_PIN_SET);		//RTK复位
		g_NeedInitRTK =2;
		g_NeedInitRtkDelay = uwTick;		
	}
	if(g_CtrlMsgFlag == 0 && g_NeedInitDTU ==1 && JudgeMilSecondDelayInPro(g_ControlMsgDelay, 1000) == true)
	{
		/* 关闭DTU电源 */
		HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);		//关闭DTU电源
		g_NeedInitDTU = 2;
		g_NeedInitDTUDelay = uwTick;		
	}
	if( g_NeedInitRTK == 2 &&  JudgeMilSecondDelayInPro(g_NeedInitRtkDelay, 1000) == true)
	{
		RYDW_RtkInit();
		g_NeedInitRTK = 0;
		if(g_OpenOLEDPower == 1)
			ssd1306_maininterface();		
	}
	if( g_NeedInitDTU == 2 &&  JudgeMilSecondDelayInPro(g_NeedInitDTUDelay, 1000) == true)
	{		
		RYDW_DtuInit();	
		g_NeedInitDTU = 0;
		if(g_OpenOLEDPower == 1 && g_ModeInFlash.type == 1)		
			ssd1306_maininterface();
	}
}


