/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位基站
文件名称: 
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 
其他说明:
功能列表:
******************************************************************/


#include "main.h"

#include "BSP.h"
#include "GPS_Comm.h"
#include "RTK_Comm.h"
#include "BLE_Comm.h"
#include "LORA_Comm.h"
#include "LORA_Base.h"
#include "CORS_Comm.h"
#include "RYDW_StationCard.h"
#include "RYDW_CardInfo.h"
#include "RYDW_Display.h"
#include "RYDW_Locate.h"
#include "RYDW_Init.h"
#include "RYDW_Postion.h"
#include "DTU_Process.h"
#include "DTU_BulkData.h"
#include "APP_STORE.h"
#include "BLE_BaseStation.h"

#include "IMU_Acc.h"

#include "WT588F02b.h"
#include "Drv_W25Qxx_QSPI.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "OLED.h"
#include "Type.h"
#include "IP2365.h"
#include "app_dtu.h"



#include "bsp_dw1000.h"
#include "bsp_dw1000_diag.h"
#include "bsp_dw1000_official.h"
#include "uwb_tdma.h"
#include "deca_device_api.h"
#include "deca_regs.h"



#define USE_DWMAIN	1
uint8_t g_Uploaded=0; 				//数据上传状态 0=无;1=有正常数据上传;2=有报警上传
extern void dw_main(void);
int main(void)
{	

	uint16_t i;
	bool b_alarmflag;
	bool b_posflag;

	uint16_t rtklen;
	
	uint16_t port;
	uint16_t dbglen;
	
	BLEResult bleret;
	uint16_t blelen;
	
	uint16_t adc;
	uint8_t chksum;
	char strbuf[10];
	uint8_t lora_recv;
	uint8_t lora_recvok;

	SCB->VTOR = FLASH_BASE | 0x10000;		//设置偏移量
	
	/* HAL库初始化 */
	HAL_Init();
	SystemPower_Config();
	SystemClock_Config();
	PeripheralsClock_Config();	
	__set_PRIMASK(0);		//开启全局中断	
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);	//SysTick频率为HCLK
	delay_init(72);
	BSPInit();
	/* Debug初始化 */
//	InitTerminalCmdGroup();	
	/* 初始化FLash */
	W25qxx_Init();
//	I2C2_Init();
	I2C3_Init();
	
	RYDW_Init();   /* 模块初始化 */
	e2prom_Init();            /*初始化EEPROM*/
	ChangeVersionFormat();	
	InitFlashPar();
	/* GPS信息初始化 */
	GPS_InitGPSInfo(&g_GPSInfo);
	
	/* 基站定位卡通信模块初始化 */
	RYDW_InitStationWorkingPara(&g_WorkingPara);	
	/* 定位卡信息初始化 */
	RYDW_InitCardTable(&g_CardTable);
	
	/* 批量参数初始化 */
	GenerateCRC32Table();					//生成CRC32表
	g_Uploaded = 0;							//数据上传状态
	g_tmrUploadBasePosDelay = uwTick;		//重置基站定位上传定时器
	g_RtkSurveyInfo.Valid = 2;				//只收敛一次	
	Read_EQUIPNUM_EEPROM();	
	/* IMU初始化 */
//	IMU_InitDriver(&g_IMUDevCtx);			//初始化IMU驱动
//	g_tmrIMUDelay = uwTick;
//	g_tmrIMUDispDelay = uwTick;	
	
	ADC_Init();
	ADC_Acq();
//	if(g_fBatVal < 7)
//		WT588E_PLAY(0x0C);
	/* 显示Logo */
//	ssd1306_maininterface();
	
	
	
   /* 1. GPIO初始化 */
    DW1000_GPIO_Init();

    /* 2. 硬件复位 */
    DW1000_HardReset();

    /* 3. SPI初始化 */
    SPI3_Init();
	
    /* 8. 使能STM32 NVIC中断 (EXTI15_10 for PB14) */
    DW1000_NVIC_Init();
    debug_printf("[DW1000] NVIC interrupt enabled\r\n");
	/* Keep the previous UWB2 power state at boot for comparison. */
	UWB2_SetPower(1);
	
	/* 根据板卡模式配置输出 */
//	DW1000_SetGpioMode();
//	DW1000_SetNormalMode();

#if (UWB_DEVICE_TYPE == UWB_DEVICE_TAG)
//	Ranging_Init(UWB_PAN_ID, UWB_TAG_ID);
//	Ranging_Start();
//	debug_printf("[TAG] Started, PAN=0x%04X, ID=%d\r\n", PAN_ID, tag_id);
#elif (UWB_DEVICE_TYPE == UWB_DEVICE_ANCHOR)
//	Anchor_Init(UWB_ANCHOR_ID, UWB_PAN_ID, UWB_TOTAL_SLOTS);
//	dwt_setrxtimeout(0);
//	dwt_rxenable(DWT_START_RX_IMMEDIATE);		//启动Rx模式
	debug_printf("[ANCHOR] Started, ID=%d, PAN=0x%04X\r\n", UWB_ANCHOR_ID, UWB_PAN_ID);
#endif

	
//    e2prom_Init();
//	Read_EQUIPNUM_EEPROM();
//	Read_HARDWAREVER_EEPROM();
//	ChangeVersionFormat();

#if (USE_DWMAIN)
	dw_main();
	while (1)
	{
		debug_printf("Run failure.\r\n");
	}
#endif	
	
	
	
	
	
	
	
	
	
	
	
	while(1)
	{        	
		/* IMU(陀螺仪) */
		if(g_InitSuccflag & 0x01)
		{
			if (JudgeMilSecondDelayInPro(g_tmrIMUDelay, 100) == true)
			{
				g_tmrIMUDelay = uwTick;
				IMU_GetAccelerationInMMg(&g_IMUDevCtx, &g_IMUVal.AccX, &g_IMUVal.AccY, &g_IMUVal.AccZ);
				IMU_GetAngularRateInMMdps(&g_IMUDevCtx, &g_IMUVal.GyoX, &g_IMUVal.GyoY, &g_IMUVal.GyoZ);
				IMU_GetTempratureInCelsius(&g_IMUDevCtx, &g_IMUVal.TmpC);
			}
			if (JudgeMilSecondDelayInPro(g_tmrIMUDispDelay, 500) == true)
			{
				g_tmrIMUDispDelay = uwTick;
				if (g_DbgEcho.EnableIMUEcho != 0)
				{
					debug_printf("$IMUVAL,%4.4f,%4.4f,%4.4f,%2.1f\r\n", g_IMUVal.AccX, g_IMUVal.AccY, g_IMUVal.AccZ, g_IMUVal.TmpC);
				}
			}
		}
		/* ADC(电池电压) */
		if (JudgeMilSecondDelayInPro(g_tmrAdcDelay, 100) == true)
		{
			g_tmrAdcDelay = uwTick;
			ADC_Acq();
		}	
		ML307C_Process();
		if (g_PostionMode == 0 && g_NeedInitRTK == 0)	
		{
			if (g_ModeInFlash.type == 1)
			{
				/* 使用CORS基站 */				
				g_LocationProg = 0xFF;
				RYDW_StationCommProcess(&g_WorkingPara, &g_CardTable);
			}
			else if (g_ModeInFlash.type == 0)
			{
				if(g_MarkWarnFlag > 0 && g_SelfContCORS == 0)
				{
					RYDW_MarkJudg();				
				}				
				else
				{
					if(g_SelfContCORS == 1 && g_LocationProg !=0xFF)/* 数据定位流程 */					
						g_LocationProg = RYDW_LocateProcess(g_LocationProg, &g_GPSInfo);
				}
				RYDW_StationCommProcess(&g_WorkingPara, &g_CardTable);
			}
		}
		KEYProcess();
				
		if (g_PostionMode == 0 && g_NeedInitRTK == 0)
		{
			/* 检查是否有报警 */
			if (g_NewData == 1)
			{
				/* 卡列表中有定位卡 */
				if (g_CardTable.CardNbr != 0)
				{
					b_alarmflag = RYDW_IsCardAlarm(g_CardTable);
					b_posflag = RYDW_IsCardPosition(g_CardTable);
					/* 检查报警信息 */
					if (b_alarmflag == true)
					{
						/* 有定位卡报警 立刻上传定位数据 */
						g_Uploaded = 2;
					}
					/* 检查定位信息 */
					else if (b_posflag == true)
					{
						/* 有定位信息 立刻上传定位数据 */
						g_Uploaded = 2;
					}
					else
					{
						if (g_CardOfflineNum == g_CardTable.CardNbr)
						{
							/* 所有卡都离线 */
							g_Uploaded = 0;
							g_NewData = 0;
						}
						else
							g_Uploaded = 2;		//演示用 有新数据就发送
					}
					if (g_Uploaded != 0)
					{
						memset(g_UploadRecord, 0x00, sizeof(g_UploadRecord));
						RYDW_GenerateCardRecord(g_CardTable, g_UploadRecord, &g_UploadRecordLen);		//生成定位卡数据
						g_NewData = 0;
					}
				}
				else
				{
					/* 没有定位卡不上传数据 */
					g_NewData = 0;
					g_Uploaded = 0;
				}
			}
		}
		if (g_BLEUploadCard == 1&& g_NeedInitRTK == 0)
		{
			/* 上传蓝牙数据 2s上传一次 */
			if ((JudgeMilSecondDelayInPro(g_tmrBleUploadDelay, 2000) == true) || (g_NewData == 1))
			{
				g_tmrBleUploadDelay = uwTick;
				BLE_SendCardinfo(g_CardTable);
			}
		}		
		TypeProcess();
		if ((CommMode == 1) || (CommMode == 2))
		{
			bleret = BLE_RecvBuf(g_BLEDataBuf, &blelen);
			if (bleret == BLERET_OK)
			{
				memset(g_BLEDataBuf, 0x00, BLE_BUFSIZE);
			}
		}
		RtkMode_Init();
		CheckTool();	
		ToolProcess();
		JudgeUpdateVoice();	
		IP2365_Detect();
		LedStatus();		
	}
}

/******************************************************************
函数名称: void Error_Handler
功能描述: 错误异常处理
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void Error_Handler(void)
{
	__disable_irq();
	while (1)
	{
		/* 异常处理 */
	}
}










