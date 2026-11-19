#include "main.h"
#include "ssd1306.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>  // For memcpy
#include "OLED.h"
#include "RYDW_Init.h"
#include "RYDW_Display.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "RYDW_Postion.h"
#include "RYDW_Locate.h"
#include "DTU_BulkData.h"
#include "g_GlobalValDef.h"
uint8_t g_DispCardOrder=0;
uint8_t lastpostion=0;
uint32_t g_UpInterFaceTimer;
uint8_t g_UpInterFaceNum=0;
uint8_t g_RTKLedFlag=1;             /*rtk指示灯状态切换标记*/
uint32_t g_RTKLedDelay;           /*rtk指示灯状态切换延时*/
uint32_t g_VoiceAnnouTimer;
void LedPowerONInit(void)
{
//	GPIO_SetupPin(OLED_RES_PORT, OLED_RES_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);		//OLED复位
//	HAL_GPIO_WritePin(SSD1306_Reset_Port, SSD1306_Reset_Pin, GPIO_PIN_RESET);
//	GPIO_SetupPin(OLED_DC_PORT, OLED_DC_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);		//OLED数据/命令切换 0=命令;1=数据						
//	BSP_SpiInitPara(&hSPI3);
//	BSP_SpiSetupClkPara(&hSPI3, SPI_BAUDRATEPRESCALER_16, SPI_CLOCK_MODE_3);
//	BSP_SpiSetupDataPara(&hSPI3, SPI_MODE_MASTER, SPI_DIRECTION_2LINES, SPI_DATASIZE_8BIT, SPI_FIRSTBIT_MSB);
//	BSP_SpiSetupNSSPara(&hSPI3, SPI_NSS_SOFT, SPI_NSS_PULSE_DISABLE);
//	BSP_SpiBindInstance(&hSPI3, SPI3);
//	__HAL_SPI_ENABLE(&hSPI3);

//	HAL_GPIO_WritePin(OLED_PWR_PORT, OLED_PWR_PIN, GPIO_PIN_RESET);		//开启OLED电源 低有效
//	HAL_Delay(100);
//	ssd1306_Init();			//OLED初始化
//	ssd1306_SetContrast(g_OledContrast);
////	if(g_ModeInFlash.type == 1)
//	if(g_LocationProg == 0xFF)	
//		LedPowerONShow();	  
////	else if(g_ModeInFlash.type == 0)
////	{
////		if(g_OpenOLEDPower == 1)
////		{
////			ssd1306_Fill(Black);
////		}
////		else
////		{
////			LedPowerONShow();	
////		}
////		
////	}
//	g_OLEDPowerOffDelay = uwTick;
//	g_RTKLedDelay = uwTick;      /*rtk指示灯状态切换延时*/
}

void LedPowerOFFInit(void)
{
//	HAL_SPI_DeInit(&hSPI3);
//	HAL_GPIO_WritePin(OLED_PWR_PORT, OLED_PWR_PIN, GPIO_PIN_SET);		//关OLED电源 低有效
//	GPIO_SetupPin(OLED_RES_PORT, OLED_RES_PIN, GPIO_MODE_ANALOG, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);		//OLED复位
//	GPIO_SetupPin(OLED_DC_PORT, OLED_DC_PIN, GPIO_MODE_ANALOG, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);		//OLED数据/命令切换 0=命令;1=数据
}


void KEYProcess(void)
{
	/* 按键 */
	BSP_BtnEventProcess(&g_BtnOK);
	BSP_BtnEventProcess(&g_BtnUp);
	BSP_BtnEventProcess(&g_BtnDown);
	/* 打点模式 长按OK键进入打点模式 再长按OK键退出打点模式 */
	if (g_BtnOK.EventPara.eEvent == BTN_EVT_LONG)
	{
			if(g_OpenOLEDPower == 0)
			{
				g_OpenOLEDPower = 1;
				LedPowerONInit();	
				g_OLEDPowerOffDelay = uwTick;
			}
			else
			{
				g_BtnOK.EventPara.eEvent = BTN_EVT_NONE;
				g_BtnOK.EventPara.eLastEvent = BTN_EVT_LONG;
			}
	}
	if (g_PostionMode == 1 && g_NeedInitRTK == 0)
	{
		RYDW_PostionProgress();
		if (g_PostionOrder == 5)
		{
			g_PostionOrder = 0;
		}
	}
	
	/* 显示翻页 按上翻页或下翻页显示定位卡信息 OK键返回主界面 */
	if (g_BtnUp.EventPara.eEvent == BTN_EVT_CLICK)
	{
		if(g_OpenOLEDPower == 0)
		{
			g_OpenOLEDPower = 1;
			LedPowerONInit();	
			g_OLEDPowerOffDelay = uwTick;
		}
		else
		{				
			if(g_DispPage == 1)  
			{/*主界面*/
				if(g_CursorPos == 1)
				{
					ssd1306_FillRectangle(2, 51, 10, 58, Black);					
					ssd1306_DrawBitmap(2,30,Arrow_Lev,8,7,White);	          /*→*//*移动光标*/
					ssd1306_UpdateScreen();	
					g_CursorPos = 0;
				}
			}
			else if(g_DispPage == 2)
			{
				if(g_SeleWorkMode==0)
					g_SeleWorkMode = 1;
				else
					g_SeleWorkMode = 0;
				SeleWorkMode(g_SeleWorkMode,0x01);  /*正常高亮*/
			}
			else if (g_DispPage == 0)
			{
				g_DispPage = 0x10;			//切换显示页面
				RYDW_DispClear();
			}
			else if (g_DispPage == 0x10)
			{
				/* 上翻页 */
				if (g_DispCardOrder == 0)
				{
					g_DispCardOrder = g_CardTable.CardNbr - 1;
				}
				else
				{
					g_DispCardOrder--;
				}
				RYDW_DispClear();
			}
			g_BtnUp.EventPara.eEvent = BTN_EVT_NONE;
			g_BtnUp.EventPara.eLastEvent = BTN_EVT_CLICK;
			g_DispNoOperationTimer = uwTick;
		}
	}
	else if(g_BtnUp.EventPara.eEvent == BTN_EVT_DOUBLE)
	{//双击上键，打点模式下切换上行、下行、单线数据类型
		if(g_OpenOLEDPower == 0)
		{
			g_OpenOLEDPower = 1;
			LedPowerONInit();	
			g_OLEDPowerOffDelay = uwTick;
		}
		else
		{				
			if(g_DispPage == 1 && g_SeleWorkMode == 1) /*主界面，且工作模式为打点*/
			{
				if(g_BribeFlag.RouLin<3) //默认是上行，=1上行；=2下行；=3单线
				{
					g_BribeFlag.RouLin++;
					ShowLinItrm(g_BribeFlag.RouLin);/*界面显示*/
				}
				else 
				{
					g_BribeFlag.RouLin=1;
					ShowLinItrm(g_BribeFlag.RouLin);/*界面显示*/
				}
			}
		}
	}
	else if (g_BtnDown.EventPara.eEvent == BTN_EVT_CLICK)
	{
		if(g_OpenOLEDPower == 0)
		{
			g_OpenOLEDPower = 1;
			LedPowerONInit();	
			g_OLEDPowerOffDelay = uwTick;
		}
		else
		{
			if(g_DispPage == 1)  
			{/*主界面*/
				if(g_CursorPos == 0)
				{
					ssd1306_FillRectangle(2, 30, 10, 37, Black);
					ssd1306_DrawBitmap(2,51,Arrow_Lev,8,7,White);	          /*→*//*移动光标*/
					ssd1306_UpdateScreen();	
					g_CursorPos = 1;
				}
			}
			else if(g_DispPage == 2)
			{
				if(g_SeleWorkMode==0)
					g_SeleWorkMode = 1;
				else
					g_SeleWorkMode = 0;
				SeleWorkMode(g_SeleWorkMode,0x01);  /*正常高亮*/
			}			
			else if (g_DispPage == 0)
			{
				g_DispPage = 0x10;
				RYDW_DispClear();
			}
			else if (g_DispPage == 0x10)
			{
				/* 下翻页 */
				if (g_DispCardOrder == (g_CardTable.CardNbr - 1))
				{
					g_DispCardOrder = 0;
				}
				else
				{
					g_DispCardOrder++;
				}
				RYDW_DispClear();
			}
			g_BtnDown.EventPara.eEvent = BTN_EVT_NONE;
			g_BtnDown.EventPara.eLastEvent = BTN_EVT_CLICK;
			g_DispNoOperationTimer = uwTick;
		}			
	}
	else if(g_BtnDown.EventPara.eEvent == BTN_EVT_DOUBLE)
	{//双击下键，打点开始暂停
		if(g_OpenOLEDPower == 0)
		{
			g_OpenOLEDPower = 1;
			LedPowerONInit();	
			g_OLEDPowerOffDelay = uwTick;
		}
		else
		{
			if(g_DispPage == 1 && g_SeleWorkMode == 1) /*主界面，且工作模式为打点*/
			{
				if(g_BribeFlag.run==0) //默认是暂停，=0运行；=1暂停
				{
					g_BribeFlag.run=1;
					ShowBribeRun(g_BribeFlag.run);/*界面显示*/
				}
				else if(g_BribeFlag.run == 1)
				{
					g_BribeFlag.run=0;
					ShowBribeRun(g_BribeFlag.run);/*界面显示*/
				}			
			}
		}
	}		
	else if (g_BtnOK.EventPara.eEvent == BTN_EVT_CLICK)
	{
		if(g_OpenOLEDPower == 0)
		{
			g_OpenOLEDPower = 1;
			LedPowerONInit();	
			g_OLEDPowerOffDelay = uwTick;
		}
		else
		{
			if(g_DispPage == 1)  
			{/*主界面*/									
				if(g_CursorPos == 1)
				{ //模式
					SeleWorkMode(g_SeleWorkMode,0x01);  /*正常高亮*/
					g_DispPage = 2;								
				}
				else if(g_CursorPos == 0)
				{ //状态
					if(g_SeleWorkMode == 0)
					{//正常模式
						g_DispPage = 0;	
					}
					else
					{//打点模式
						g_DispPage = 0x11;	
					}					
					RYDW_DispClear();
				}
			}			
			else if(g_DispPage == 2)
			{
				SeleWorkMode(g_SeleWorkMode,0x00);  /*正常高亮*/
				g_DispPage = 1;
				g_PostionMode = g_SeleWorkMode;
				if(g_SeleWorkMode == 1)
				{
					ShowLinItrm(g_BribeFlag.RouLin);/*界面显示*/
					ShowBribeRun(g_BribeFlag.run);/*界面显示*/
				}
				else
				{
					g_BribeFlag.RouLin = 1; //上
					g_BribeFlag.run = 1;    //暂停
					ssd1306_FillRectangle(96, 48, 108, 60, Black);
					ssd1306_FillRectangle(110, 46, 126, 62, Black);
				}
			}
			else if(g_DispPage == 0 || g_DispPage == 0x11)
			{
				ssd1306_maininterface();
				g_DispPage = 1;
				g_CursorPos = 0;
				if(g_SeleWorkMode == 1)
				{
					ShowLinItrm(g_BribeFlag.RouLin);/*界面显示*/
					ShowBribeRun(g_BribeFlag.run);/*界面显示*/
				}
				else
				{
					g_BribeFlag.RouLin = 1; //上
					g_BribeFlag.run = 1;    //暂停
					ssd1306_FillRectangle(96, 48, 108, 60, Black);
					ssd1306_FillRectangle(110, 46, 126, 62, Black);
				}				
			}
			else if(g_DispPage == 0x10)
			{
				g_DispPage = 0;
				RYDW_DispClear();
			}
			g_BtnOK.EventPara.eEvent = BTN_EVT_NONE;
			g_BtnOK.EventPara.eLastEvent = BTN_EVT_CLICK;
		}
	}
	
	/* 启动后刷新显示 */
	if(g_OpenOLEDPower == 1)
	{
		if ((g_LocationProg == 0xFF || g_LocationProg == 0x00) && (g_PostionMode == 0)  )
		{
			if (g_DispPage == 0x00)  /**/
			{
				/* 主页面 */
				if (JudgeMilSecondDelayInPro(g_DispRefreshDelay, 500) == true)
				{
					g_DispRefreshDelay = uwTick;
					RYDW_DispStationStatus(&g_StationStatus, 0, 0);	//显示设备状态	占一行
					RYDW_DispCardInfo(&g_CardTable, 0, 10);			//显示卡信息		占两行
					RYDW_DispPosition(0, 30);						//显示基站定位	占三行
					RYDW_DispTime(g_SysTime, 80, 50);				//显示时间
					ssd1306_UpdateScreen();
					RYDW_DispBattery(g_fBatVal);					//显示电池
					if (lastpostion == 0)
					{
						if ((g_LatitudeBCD[0] != 0) && (g_LongitudeBCD[0] != 0))
						{
							lastpostion = 1;		//已定位标记
							g_BasePosUploaded = 1;
						}
					}
				}
			}
			else if (g_DispPage == 0x10 || g_DispPage == 0x11)
			{
				/* 定位卡显示页面 */
				if (JudgeMilSecondDelayInPro(g_DispNoOperationTimer, 20000) == true)
				{
					/* 20秒无操作返回主页面 */
					g_DispNoOperationTimer = uwTick;
					RYDW_DispClear();
					g_DispPage = 0x00;
				}
				else
				{
					if (JudgeMilSecondDelayInPro(g_DispRefreshDelay, 500) == true)
					{
						g_DispRefreshDelay = uwTick;
						RYDW_DispCardDetail(g_CardTable, g_DispCardOrder);
						RYDW_DispBattery(g_fBatVal);
					}
				}
			}
		}
	}
	else
	{
			if (lastpostion == 0)
			{
				if ((g_LatitudeBCD[0] != 0) && (g_LongitudeBCD[0] != 0))
				{
					lastpostion = 1;		//已定位标记
					g_BasePosUploaded = 1;
				}
			}		
	}	
	if((g_DispPage==1 || g_DispPage==2) && JudgeMilSecondDelayInPro(g_UpInterFaceTimer, 1000) == true  ) //
	{			

		g_UpInterFaceNum++;
		if(g_UpInterFaceNum % 10 == 0)
		{
			SendNetSignal();                       /*信号显示*/	
		}
		if((g_ModeInFlash.type == 0 && g_LocationProg == 0xFF)|| g_ModeInFlash.type == 1)
		{		
			RYDW_DispTime(g_SysTime, 47, 5);	   /*时间*/	
			g_UpInterFaceTimer= uwTick;
			if(g_UpInterFaceNum % 10 == 0)
			{
				RYDW_DispSignal(g_Signal);
				RYDW_DispGPS(g_GPSInfo.uMode);		   /*定位模块*/ 
			}
			if(g_UpInterFaceNum == 60)
			{
				RYDW_DispBattery(g_fBatVal);		   /*电池*/	
				g_UpInterFaceNum = 0;
			}
			if (lastpostion == 0)
			{
				if ((g_GPSInfo.dLatitudeInDegMin != 0) && (g_GPSInfo.dLongitudeInDegMin != 0))
				{
					lastpostion = 1;		//已定位标记
					g_BasePosUploaded = 1;
				}
			}
		}		
	}
	if((g_DispPage == 1 || g_DispPage == 0 || g_DispPage == 10 ) && g_SeleWorkMode == 1 && g_BribeFlag.run==0)
	{
		if(JudgeMilSecondDelayInPro(g_VoiceAnnouTimer, 5000) == true)
		{
			WT588E_PLAY(0x0E);
			g_VoiceAnnouTimer =  uwTick;
		}
	}
	if(g_OpenOLEDPower == 1 && g_LocationProg == 0xFF && JudgeMilSecondDelayInPro(g_OLEDPowerOffDelay, 300000) == true)
	{
		g_OpenOLEDPower = 0;
		LedPowerOFFInit();			
		g_OLEDPowerOffDelay = uwTick;
	}		
}
void LedStatus(void) 
{
	if(g_GPSInfo.uMode	> 0 && g_GPSInfo.uMode	< 3)
	{
		if(JudgeMilSecondDelayInPro(g_RTKLedDelay,500) == true)
		{	
			HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);           
			g_RTKLedDelay = uwTick;
		}		
	}
	else if(g_GPSInfo.uMode == 5)    /*浮点解*/
	{
		if(g_RTKLedFlag == 0 && JudgeMilSecondDelayInPro(g_RTKLedDelay,1000) == true)
		{	
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13, GPIO_PIN_SET);				
			g_RTKLedDelay = uwTick;
			g_RTKLedFlag = 1;
		}	
		if(g_RTKLedFlag == 1 && JudgeMilSecondDelayInPro(g_RTKLedDelay,2000) == true)
		{	
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13, GPIO_PIN_RESET);
			g_RTKLedDelay = uwTick;
			g_RTKLedFlag = 0;				
		}			
	}
	else if(g_GPSInfo.uMode == 4 || g_GPSInfo.uMode == 7)   /*固定解*/
	{
		if(g_RTKLedFlag == 0 && JudgeMilSecondDelayInPro(g_RTKLedDelay,2000) == true)
		{	
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13, GPIO_PIN_SET);			
			g_RTKLedDelay = uwTick;
			g_RTKLedFlag = 1;
		}	
		if(g_RTKLedFlag == 1 && JudgeMilSecondDelayInPro(g_RTKLedDelay,3000) == true)
		{	
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13, GPIO_PIN_RESET);
			g_RTKLedDelay = uwTick;
			g_RTKLedFlag = 0;				
		}	
	}
	
	
}	