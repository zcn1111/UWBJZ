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
#include "ebyte_core.h"
#include "GPS_Comm.h"
#include "RTK_Comm.h"
#include "RYDW_StationCard.h"

DateTime time_init={0x15,0x05,0x19,0x0E,0x38,0x00};	//初始化实时时间
DateTime sysTime;
uint32_t b;
uint8_t g_LastEvent;

//uint8_t jiao[]={0x10,0x10,0x10,0x13,0x54,0x54,0x55,0x56,0x54,0x54,0x54,0x5C,0x64,0x00,0x00,0x03,
//0x40,0x20,0x20,0xFE,0x00,0x88,0x04,0x02,0x88,0x88,0x50,0x50,0x20,0x50,0x88,0x06,		
//};
//uint8_t si[]={  0x00,0x00,0x3F,0x20,0x20,0x20,0x2F,0x28,0x28,0x2F,0x28,0x28,0x4B,0x52,0x94,0x28,
//0x08,0x7C,0x80,0x80,0xF8,0x80,0xFE,0x82,0xF0,0x84,0x7C,0x00,0xE0,0x22,0x22,0x1E,};//取模用copyleft by horse 2000，


int main(void)
{	

	SCB->VTOR = FLASH_BASE | 0x10000;		//设置偏移量
	
	/* HAL库初始化 */
	WDTFeed();
	HAL_Init();
	SystemClock_Config();
	ENABLE_INT();	
	delay_init(32);
	
	/* 看门狗初始化 */
    IWDG_Init(IWDG_PRESCALER_64, 6000);	//分频数为64,重载值为500,溢出时间为1s(4*2^4*500/32时钟=1000ms)
	WDTFeed();			
	PowerControlInit();										//控制Tool等模块上电InitTimer2();
	
	/* 定时器初始化 */
	WDTFeed();	
	InitTimer2();					//定时器2定时1ms	
	TIM6Init(10000-1, 3200-1);  	//定时器6定时1s
	TIM3Init(50-1, 3200-1);			//设置为125ms中断
    TIM7Init(1000-1, 3200-1);		//设置为100ms中断
	
	/* QSPI初始化 */
	WDTFeed();
	QSPI_Init();
	W25qxx_Init();
	
	/* SPI初始化 */
	WDTFeed();
	SPI1_Init();
	SPI2_Init();
	
    /* 串口初始化 */
	WDTFeed();
	BSP_UartVarInit(&g_Uart1, USART1, g_RxBuf1, UART1_TX_BUF_SIZE, UART1_RX_BUF_SIZE);	//初始化串口变量
	BSP_UartVarInit(&g_Uart2, USART2, g_RxBuf2, UART2_TX_BUF_SIZE, UART2_RX_BUF_SIZE);
	BSP_UartVarInit(&g_Uart3, USART3, g_RxBuf3, UART3_TX_BUF_SIZE, UART3_RX_BUF_SIZE);
	BSP_UartVarInit(&g_LPUart, LPUART1, g_RxBufLP, LPUART_TX_BUF_SIZE, LPUART_RX_BUF_SIZE);
	InitUSART1(115200);				//初始化串口1		RTK通信串口
	InitUSART2(115200);				//初始化串口2		蓝牙通信串口
	InitUSART3(230400);				//初始化串口3		USB转串口(调试串口)
	InitLPUART1(38400);				//初始化低功耗串口	DTU通信串口
//	InitUart();						//初始化串口，USART1-LORA USART2-TOOL 若为接收1号也初始化LPUART1-DTU
	
	/* 按键/指示灯初始化 */
	WDTFeed();	
	BSP_ButtonInit();
	BSP_LEDInit();
	
    /* AD 采样初始化*/
    WDTFeed();                                  
	ADC1Init();
	ADC2Init();
	WDTFeed();
	JudgeVersionChange();
	
	WDTFeed();
	SysInit();
    /* RTC初始化 */
	InitRTC();						//初始化RTC
	
	InitFlashPar();

	/* LoRa驱动初始化 */
	Ebyte_RF.Init();
	LORA_SetupLoraBuffer(&g_LoRaPara, g_LoRaPktBuf, g_LoRaSendBuf, g_LoRaRecvBuf);	//配置LORA
	/* RTK模块初始化 */
	RTK_InitMoudle();
	
	/* 基站定位卡通信模块初始化 */
	RYDW_InitBaseHeaderVar(&g_BaseHeader);
	RYDW_InitCardHeaderVar(&g_CardHeader);
	RYDW_InitStationWorkingPara(&g_WorkingPara);
	RYDW_InitElecFence(&g_ElecFence);
//	/* 雨量计算 */
//    SensorRainInit();
//	/* 蜂鸣器初始化 */
//    BeepInit();
//	ReadPar();
//	WDTFeed();
//	/* 雨量初始化 */			
//	RainGlobalVarInit();
//	WDTFeed(); 
//	InitRDPar();
////	InitUSART3(UART3_BAUD);
//	WDTFeed();
//    TemperGlobalVarInit(); 
//	WDTFeed();
//    WindGlobalVarInit();
//	WDTFeed();	
//	XSGlobalVarInit();
//	WDTFeed();
//	InitLLPar();
	WDTFeed();
    Sys_InitTime();		// 从RTC中读取时间,判断是否有效
	ADC_GetVal();	             
	WDTFeed();
 	g_PowerUpAdjustFlag = true; 
	
	while(1)
	{        	
		WDTFeed();
		SysRunTime1();
		
		WDTFeed();	
        PowerControl();	
		WDTFeed();
		DTUProcess();
		WDTFeed();
		/* DTU 数据包接收 */
//		JudgeToolConnect();
		WDTFeed();
		/* 客服工具通信 */
		ToolProcess();		 
		WDTFeed();
		
		/* LoRa驱动周期任务 */
		WDTFeed();
		Ebyte_RF.StartPollTask();
		
		/* 基站和卡通信 */
		RYDW_StationCommProcess(&g_WorkingPara);
		
        if(g_ADCFlag == true)
		{
    		/* AD采样*/
    		ADC_GetVal(); 
			WDTFeed(); 
    		g_ADCFlag = false;			
		}		
//		if(!g_SysErr)
//		{		
//			RainFunc();         //雨量计算           
//			WDTFeed();
//		}
//		if(g_ShouldUpdateWTRecFlag == 0 && g_ShouldUpdateWTSendFlag == 0)
//		{
//			CommunicateWithGWB();			//这里环境温度用的是轨温板，这个是修改过的轨温模块
//			WDTFeed();
//			rdProcess();
//			WDTFeed(); 
//		}
//		else
//		{
//			TempratrueUpdate();
//			WDTFeed();
//		}
		
//		if(g_GlobalSetSysPara.MonitorOption[0] > 0)   
//		{			
//			TempratrueFunc();   //轨温计算 agj add
//			WDTFeed(); 
//		}		
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












