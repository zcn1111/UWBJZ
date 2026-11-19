/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: DTU_Process.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 处理DTU通用流程
其他说明:
功能列表:
******************************************************************/

#include "DTU_Process.h"
#include "DTU_Comm.h"
#include "DTU_Update.h"

#include "BSP.h"
#include "g_func.h"
#include "g_GlobalValDef.h"
#include "app_dtu.h"
#include "Update.h"


/********* 变量定义 **********/
DtuPara_t		g_DtuPara;	//DTU参数

TickTimerPara_t	g_AskSignalDelay;
TickTimerPara_t g_AskDateTimeDelay;
TickTimerPara_t	g_AskPhoneNumDelay;
TickTimerPara_t g_DtuResetDelay;
TickTimerPara_t g_DtuPowerDownDelay;
TickTimerPara_t g_DtuPowerUpDelay;

TickTimerPara_t	g_TransConnectDelay;		//透传模式连接延迟

TickTimerPara_t	g_UpdateRetryDelay;			//升级重试延迟

MobileNum_t		g_MyMobileNum;


/******************************************************************
函数名称: void DTU_GenericExtFuncInit
功能描述: DTU通用车载外部函数初始化
参数输入: DtuPara_t			*Para		DTU参数
参数输出: DtuPara_t			*Para		DTU参数
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
void DTU_GenericExtFuncInit(void)
{
	uint8_t i;
	
	DtuExtFunc_t trainfunc;
	
	/* 初始化外部函数组 */
	for (i=0; i<GENFUNC_GRP_MAX; i++)
	{
//		DTU_UnRegisterExtFunc(&g_GenFuncGroupArr[i]);
	}
	
	/* 注册DTU外部函数 */
	
//	/* EE 升级命令接收 */
//	DTU_SetupExternalFunc((uint8_t*)"EEEEEEEEEEEEEEEE", 16, DTU_RecvUpdateCmd, &trainfunc);
//	DTU_RegisterExtFuncToGroup(trainfunc, 6, &g_GenFuncGroupArr[1]);
//	
//	/* DC 升级数据接收 */
//	DTU_SetupExternalFunc((uint8_t*)"DCDCDCDCDCDCDC", 512, DTU_RecvUpdateData, &trainfunc);
//	DTU_RegisterExtFuncToGroup(trainfunc, 6, &g_GenFuncGroupArr[2]);
}

/******************************************************************
函数名称: void DTU_TimerInit
功能描述: DTU定时器初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void DTU_TimerInit(void)
{
	InitTickTimerPara(&g_AskSignalDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_ASKSIGNAL_NORMAL, TTM_MODE_RESTART, 0, NULL, NULL, &g_AskSignalDelay);
	InitTickTimerPara(&g_AskDateTimeDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_ASKTIME, TTM_MODE_RESTART, 0, NULL, NULL, &g_AskDateTimeDelay);
	InitTickTimerPara(&g_AskPhoneNumDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_ASKPHONENUM, TTM_MODE_RESTART, 0, NULL, NULL, &g_AskPhoneNumDelay);
	InitTickTimerPara(&g_DtuResetDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_RESETPOWER_DISCONN, TTM_MODE_RESTART, 0, NULL, NULL, &g_DtuResetDelay);
	
	InitTickTimerPara(&g_DtuPowerUpDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_DTU_POWERUP, TTM_MODE_SINGLE, 0, NULL, NULL, &g_DtuPowerUpDelay);
	InitTickTimerPara(&g_DtuPowerDownDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_DTU_POWERDOWN, TTM_MODE_SINGLE, 0, NULL, NULL, &g_DtuPowerDownDelay);
	InitTickTimerPara(&g_UpdateRetryDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_UPDATE_RETRY, TTM_MODE_RESTART, 3, NULL, NULL, &g_UpdateRetryDelay);
	InitTickTimerPara(&g_TransConnectDelay, &g_Tick1s);
	SetupTickTimerPara(DELAY_TRANS_DISCONN, TTM_MODE_SINGLE, 0, NULL, NULL, &g_TransConnectDelay);
}

/******************************************************************
函数名称: void DTU_PowerOff
功能描述: DTU关断电源
参数输入: uint16_t	DelayMs	延迟毫秒	
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void DTU_PowerOff(uint16_t DelayMs)
{
	/* 关断前延迟 */
	if (DelayMs != 0)
	{
		HAL_Delay(DelayMs);
	}
	HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);
	g_InitAskParFlag = 0;
}

/******************************************************************
函数名称: void DTU_PowerOn
功能描述: DTU打开电源
参数输入: uint16_t	DelayMs	延迟毫秒	
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void DTU_PowerOn(uint16_t DelayMs)
{
	/* 打开前延迟 */
	if (DelayMs != 0)
	{
		HAL_Delay(DelayMs);
	}
	HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);
}

/******************************************************************
函数名称: int16_t DTU_PowerCtrlProgress
功能描述: DTU电源控制流程
参数输入: DtuPara_t	*Para	DTU设备参数
参数输出: DtuPara_t	*Para	DTU设备参数
返 回 值: int16_t 返回值
其    他: 无
******************************************************************/
int16_t DTU_PowerCtrlProgress(DtuPara_t *Para)
{
	int16_t ret;
	
	ret = DTU_COMM_OK;
	
	/* 正常状态DTU */
	if (Para->DtuNetFlag.bit.PoweredReset == 0)
	{
		if (Para->DtuNetFlag.bit.PoweredSta != 0)		//切换到上电状态
		{
			HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);
			Para->DtuNetFlag.bit.PoweredSta = 3;
		}
		else if (Para->DtuNetFlag.bit.PoweredSta == 0)	//切换到断电状态
		{
			HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);
			ret = DTU_PWR_OFF;
			g_InitAskParFlag = 0;
		}
	}
	
	/* 重启状态DTU */
	if (Para->DtuNetFlag.bit.PoweredReset == 1)
	{
		/* 等待断电 */
		if (Para->DtuNetFlag.bit.PoweredSta == 1)
		{
			StartTickTimer(&g_DtuPowerDownDelay);
			if (g_DtuPowerDownDelay.TimerFlag.TimeOutFlag == 1)
			{
				g_DtuPowerDownDelay.TimerFlag.TimeOutFlag = 0;
				HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);
				g_InitAskParFlag = 0;
				Para->DtuNetFlag.bit.PoweredSta = 0;										//切换到已断电状态
				StopTickTimer(&g_DtuPowerDownDelay);
			}
			ret = DTU_PWR_RESETTING;
		}
		/* 已断电 */
		if (Para->DtuNetFlag.bit.PoweredSta == 0)
		{
			Para->DtuNetFlag.bit.PoweredSta = 2;											//切换到等待上电状态
			Para->DtuNetFlag.bit.Connected = 0;
			Para->DtuNetFlag.bit.ObtainedIP = 0;
			Para->DtuNetFlag.bit.ObtainedPhoneNum = 0;
			Para->DtuNetFlag.bit.ObtainedTime = 0;
			ret = DTU_PWR_OFF;
		}
		/* 等待上电 */
		if (Para->DtuNetFlag.bit.PoweredSta == 2)
		{
			StartTickTimer(&g_DtuPowerUpDelay);
			if (g_DtuPowerUpDelay.TimerFlag.TimeOutFlag == 1)
			{
				HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);
				g_DtuPowerUpDelay.TimerFlag.TimeOutFlag = 0;
				Para->DtuNetFlag.bit.PoweredSta = 3;										//切换到已上电状态
				StopTickTimer(&g_DtuPowerUpDelay);
			}
			ret = DTU_PWR_RESETTING;
		}
		/* 已上电 */
		if (Para->DtuNetFlag.bit.PoweredSta == 3)
		{
			Para->DtuNetFlag.bit.PoweredReset = 0;											//清除DTU重启标记
			ret = DTU_COMM_OK;
		}
	}
	
	return ret;
}

/******************************************************************
函数名称: int16_t DTU_ConnectionProgress
功能描述: DTU连接流程
参数输入: DtuPara_t	*Para	DTU设备参数
参数输出: DtuPara_t	*Para	DTU设备参数
返 回 值: int16_t 返回值
其    他: 无
******************************************************************/
int16_t DTU_ConnectionProgress(DtuPara_t *Para)
{
//	uint16_t slen;
//	int16_t	recvret;
//	int16_t ret;
//	
//	MobileNum_t pnum;
//	
//	/* 接收数据 */
//	recvret = DTU_AnlysisTypecodeProgress(Para);
//	
//	ret = DTU_COMM_OK;
//	
//	/* 当前正在切换IP */
//	if (Para->DtuNetFlag.bit.ChangedIPSta == 1)
//	{
//		/* 未连接 */
//		if (Para->DtuNetFlag.bit.Connected == 0)
//		{
////			Para->DtuNetFlag.bit.Connected = 1;
//			g_DTUGprsOKLED.DispType = LEDT_OFF;		//切换LED关灯
//			
//			
//			StartTickTimer(&g_AskSignalDelay);
//			StartTickTimer(&g_AskPhoneNumDelay);
//			/* 处理索要信号强度 */
//			if (g_AskSignalDelay.TimerFlag.TimeOutFlag == 1)
//			{
//				g_AskSignalDelay.TimerFlag.TimeOutFlag = 0;
//				if (g_AskSignalDelay.TimerFlag.ReStartMaxFlag == 1)
//				{
//					debug_printf("Change IP failed.\r\n");
//					Para->DtuNetFlag.bit.ChangedIPSta = 3;	//切换IP失败
//					StopTickTimer(&g_AskSignalDelay);		//停止索要信号定时器
//					StopTickTimer(&g_AskPhoneNumDelay);		//停止索要手机号定时器
//				}
//				else
//				{
//					DTU_AskSignalStrength();
//					debug_printf("AskSignal.\r\n");
//				}
//			}
//			/* 处理索要本机号 */
//			if (g_AskPhoneNumDelay.TimerFlag.TimeOutFlag == 1)
//			{
//				g_AskPhoneNumDelay.TimerFlag.TimeOutFlag = 0;
//				DTU_AskPhoneNum();
//				debug_printf("AskPhoneNum\r\n");
//			}
//		}
//		/* 已连接 索要IP */
//		if (Para->DtuNetFlag.bit.Connected == 1)
//		{
//			/* 已获取IP */
//			if (Para->DtuNetFlag.bit.ObtainedIP == 1)
//			{
//				g_DTUGprsOKLED.DispType = LEDT_ON;
//				/* 获取到IP 判断IP是否正确 */
//				if (memcmp(Para->ConIPAddr.all, Para->ChangeIPAddr.all, 8) == 0)
//				{
//					g_DTUGprsOKLED.DispType = LEDT_ON;
//					Para->DtuNetFlag.bit.ChangedIPSta = 2;	//已切换IP
//					memset(Para->ChangeIPAddr.all, 0x00, 8);				//重置切换IP
//					debug_printf("Change OK\r\n");
//				}
//				else
//				{
//					Para->DtuNetFlag.bit.ChangedIPSta = 3;	//切换失败
//				}
//			}
//			/* 未获取IP */
//			if (Para->DtuNetFlag.bit.ObtainedIP == 0)
//			{
//				StartTickTimer(&g_AskPhoneNumDelay);
//				if (g_AskPhoneNumDelay.TimerFlag.TimeOutFlag == 1)
//				{
//					Para->DtuNetFlag.bit.ObtainedIP = 0;
//					DTU_AskPhoneNum();
//					g_AskPhoneNumDelay.TimerFlag.TimeOutFlag = 0;
//				}
//			}
//		}
//	}
//	
//	/* 切换IP失败 */
//	if (Para->DtuNetFlag.bit.ChangedIPSta == 3)
//	{
//		/* 当前IP是默认数据IP */
//		if (memcmp(Para->ConIPAddr.all, g_ServerInfo.DataIP.all, 8) == 0)
//		{
//			Para->DtuNetFlag.bit.ChangedIPSta = 0;
//			Para->DtuNetFlag.bit.Connected = 0;
//		}
//		/* 当前IP不是默认数据IP 切换成默认数据IP */
//		else
//		{
//			DTU_ChangeIP(g_ServerInfo.DataIP, 0);		//切换到默认数据IP
//			memcpy(Para->ChangeIPAddr.all, g_ServerInfo.DataIP.all, 8);
//			g_DTUGprsOKLED.DispType = LEDT_OFF;
//			Para->DtuNetFlag.bit.ChangedIPSta = 1;		//开始切换
//			Para->DtuNetFlag.bit.Connected = 0;
//			Para->DtuNetFlag.bit.ObtainedIP = 0;
//		}
//	}
//		
//	/* 当前正常连接模式 */
//	if (Para->DtuNetFlag.bit.ChangedIPSta == 0)
//	{
//		/* 当前未连接 */
//		if (Para->DtuNetFlag.bit.Connected == 0)
//		{
//			g_DTUGprsOKLED.DispType = LEDT_OFF;
//			/* 不在重启状态 且 已上电 */
//			if ((Para->DtuNetFlag.bit.PoweredReset == 0) && (Para->DtuNetFlag.bit.PoweredSta == 3))
//			{
//				/* 配置定时器 */
//				StartTickTimer(&g_AskSignalDelay);
//				StartTickTimer(&g_DtuResetDelay);
//				StopTickTimer(&g_AskDateTimeDelay);
//				StopTickTimer(&g_AskPhoneNumDelay);
//				/* 重启DTU定时器时间到 */
//				if (g_DtuResetDelay.TimerFlag.TimeOutFlag == 1)
//				{
//					Para->DtuNetFlag.bit.PoweredSta = 1;
//					Para->DtuNetFlag.bit.PoweredReset = 1;			//切换到重启DTU状态
//					g_DtuResetDelay.TimerFlag.TimeOutFlag = 0;
//					StopTickTimer(&g_AskSignalDelay);
//					StopTickTimer(&g_AskPhoneNumDelay);
//					StopTickTimer(&g_AskDateTimeDelay);
//				}
//				/* 索要信号强度定时器时间到 */
//				if (g_AskSignalDelay.TimerFlag.TimeOutFlag == 1)
//				{
//					DTU_AskSignalStrength();		//索要信号强度
//					g_AskSignalDelay.TimerFlag.TimeOutFlag = 0;
//				}
//			}
//		}
//		
//		/* 当前已连接 */
//		if (Para->DtuNetFlag.bit.Connected == 1)
//		{
//			ret = DTU_COMM_CONNECTED;
//			/* 已校时 且 有本机号 */
//			if ((Para->DtuNetFlag.bit.ObtainedTime == 1) && (Para->DtuNetFlag.bit.ObtainedPhoneNum == 1))
//			{
//				g_DTUGprsOKLED.DispType = LEDT_ON;
//				StopTickTimer(&g_AskPhoneNumDelay);
//				StopTickTimer(&g_AskDateTimeDelay);
//				if (g_AskSignalDelay.TimerFlag.TimeOutFlag == 1)
//				{
//					SetupTickTimerPara(DELAY_ASKSIGNAL_NORMAL, TTM_MODE_RESTART, 0, NULL, NULL, &g_AskSignalDelay);
////					DTU_AskSignalStrength();
//					g_AskSignalDelay.TimerFlag.TimeOutFlag = 0;
//				}
//				ret = DTU_COMM_GOTTENCONNINFO;
//			}
//			/* 未校时 */
//			if (Para->DtuNetFlag.bit.ObtainedTime == 0)
//			{
//				StartTickTimer(&g_AskDateTimeDelay);
//				if (g_AskDateTimeDelay.TimerFlag.TimeOutFlag == 1)
//				{
//					DTU_AskDateTime();		//索要时间
//					g_AskDateTimeDelay.TimerFlag.TimeOutFlag = 0;
//				}
//			}
//			/* 无本机号 */
//			if (Para->DtuNetFlag.bit.ObtainedPhoneNum == 0)
//			{
//				StartTickTimer(&g_AskPhoneNumDelay);
//				if (g_AskPhoneNumDelay.TimerFlag.TimeOutFlag == 1)
//				{
//					DTU_AskPhoneNum();		//索要本机号
//					g_AskPhoneNumDelay.TimerFlag.TimeOutFlag = 0;
//					if (g_AskPhoneNumDelay.TimerFlag.ReStartMaxFlag == 1)
//					{
//						ConvStrToPhonenum("18406503524", pnum.MobNum);
////						DTU_WritePhoneNumToDTU(pnum);
//						Para->DtuNetFlag.bit.ObtainedPhoneNum = 1;
//						StopTickTimer(&g_AskPhoneNumDelay);
//					}
//				}
//			}
//		}
//	}
//	
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_TransConnectionProgress
功能描述: DTU透传连接流程
参数输入: DtuPara_t	*Para	DTU设备参数
参数输出: DtuPara_t	*Para	DTU设备参数
返 回 值: int16_t 返回值
其    他: 无
******************************************************************/
int16_t DTU_TransConnectionProgress(DtuPara_t *Para)
{
	int16_t	recvret;
	int16_t ret;
	
	/* 接收数据 */
	recvret = DTU_AnlysisTypecodeProgress(Para);
	
	/* 进入透传模式 开始连接 */
	if (Para->TransMode == 1)
	{
		/* 已获取应答 */
		if (Para->DtuNetFlag.bit.Transport == 1)
		{
			StopTickTimer(&g_TransConnectDelay);
//			Para->TransMode = 1;
			ret = DTU_COMM_TRANS_CONN;
		}
		/* 当前未应答 */
		else if (Para->DtuNetFlag.bit.Transport == 0)
		{
			/* 配置透传连接定时器 */
			StartTickTimer(&g_TransConnectDelay);
			if (g_TransConnectDelay.TimerFlag.TimeOutFlag == 1)
			{
				Para->DtuNetFlag.bit.Transport = 0;			//关闭透传
				ret = DTU_COMM_ERR_TRANSTFAIL;				//切换透传失败
			}
		}
	}
	
	/* 退出透传模式 */
	if (Para->TransMode == 2)
	{
		/* 已获取DTU透传状态	*/
		if (Para->DtuNetFlag.bit.Transport == 0)
		{
			StopTickTimer(&g_TransConnectDelay);
//			Para->TransMode = 2;
			ret = DTU_COMM_TRANS_EXIT;
		}
		/* 当前未应答 无时间 */
		else if (Para->DtuNetFlag.bit.Transport == 1)
		{
			/* 配置透传连接定时器 */
			StartTickTimer(&g_TransConnectDelay);
			if (g_TransConnectDelay.TimerFlag.TimeOutFlag == 1)
			{
				Para->DtuNetFlag.bit.Transport = 0;			//关闭透传
				ret = DTU_COMM_ERR_TRANSTFAIL;				//切换透传失败
			}
		}
	}
	
	return ret;
}

IPAddr_t 	g_BatchIP;
IPAddr_t	g_tmpIP;
/******************************************************************
函数名称: int16_t DTU_CommProgress
功能描述: DTU通信流程
参数输入: DtuPara_t	*Para	DTU设备参数
参数输出: DtuPara_t	*Para	DTU设备参数
返 回 值: int16_t 返回值
其    他: 无
******************************************************************/
int16_t DTU_CommProgress(DtuPara_t *Para)
{
	int16_t recvret;
	int16_t ret;
	uint8_t adjtime;
	
	int16_t pwrret;
	
		
	/* 当前状态=未连接 */
	if (Para->DtuProgress == DTU_PROG_DISCONNECT)
	{
		/* DTU已断电 给DTU上电 */
		if (Para->DtuNetFlag.bit.PoweredSta == 0)
		{
			Para->DtuNetFlag.bit.PoweredSta = 1;
			recvret = DTU_PowerCtrlProgress(Para);
			ret = recvret;
			Para->DtuProgress = DTU_PROG_DISCONNECT;	//切换到->未连接
			debug_msg("DTU -> Disconnect\r\n");
		}
		/* DTU已上电 */
		else if (Para->DtuNetFlag.bit.PoweredSta == 3)
		{
			StartTickTimer(&g_DtuResetDelay);
			Para->DtuProgress = DTU_PROG_CONNECTING;	//切换到->等待连接
			debug_msg("DTU -> Connecting\r\n");
			ret = DTU_COMM_OK;
		}
		/* 复位中 */
		else if (Para->DtuNetFlag.bit.PoweredReset == 1)
		{
			recvret = DTU_PowerCtrlProgress(Para);
		}
	}
	/* 当前状态=等待连接 */
	else if (Para->DtuProgress == DTU_PROG_CONNECTING)
	{
		recvret = DTU_ConnectionProgress(Para);
		
		/* 已联网 */
		if (Para->DtuNetFlag.bit.Connected == 1)
		{
			/* 已获取时间 本机号和IP */
			if ((Para->DtuNetFlag.bit.ObtainedTime == 1) &&	\
				(Para->DtuNetFlag.bit.ObtainedIP == 1) && \
				(Para->DtuNetFlag.bit.ObtainedPhoneNum == 1))
			{
				Para->DtuProgress = DTU_PROG_NORMAL;	//切换到->正常
				StopTickTimer(&g_DtuResetDelay);
				debug_msg("DTU -> Normal\r\n");
			}
			/* 只获取到本机号和IP */
			else if ((Para->DtuNetFlag.bit.ObtainedIP == 1) && \
					  (Para->DtuNetFlag.bit.ObtainedPhoneNum == 1))
			{
				if ((g_SysTime.Year != 0) && (g_SysTime.Month != 0))
				{
					/* 从其他渠道获取到时间 */
					Para->DtuNetFlag.bit.ObtainedTime = 1;
					Para->DtuProgress = DTU_PROG_NORMAL;	//切换到->正常
					StopTickTimer(&g_DtuResetDelay);
					debug_msg("DTU -> Normal\r\n");
				}
				else
				{
					Para->DtuProgress = DTU_PROG_CONNECTING;
				}
			}
			DTU_AnlysisTypecodeProgress(Para);
		}
		/* 未联网 */
		if (Para->DtuNetFlag.bit.Connected == 0)
		{
			if (g_DtuResetDelay.TimerFlag.TimeOutFlag == 1)
			{
				g_DtuResetDelay.TimerFlag.TimeOutFlag = 0;
				Para->DtuNetFlag.bit.PoweredReset = 1;			//切换到复位DTU
				Para->DtuNetFlag.bit.PoweredSta = 1;
				debug_msg("DTU -> Disconnect\r\n");
				Para->DtuProgress = DTU_PROG_DISCONNECT;		//切换到 -> 未连接
			}
		}
		/* 已联网 批量数据端口 */
		if (Para->ConIPAddr.byte.uPort == 2028)
		{
			/* 切换透传 */
			DTU_TranparentTransMode(1);							//发送透传命令
			Para->DtuNetFlag.bit.Transport = 0;
			Para->DtuProgress = DTU_PROG_TRASPORT_ENTRY;		//切换到->开始透传
			debug_msg("DTU -> Entry Transport\r\n");
		}
		ret = recvret;
	}
	/* 当前状态=正常 */
	else if (Para->DtuProgress == DTU_PROG_NORMAL)
	{
//		DTU_ConnectionProgress(Para);
		recvret = DTU_AnlysisTypecodeProgress(Para);
		
		/* */
		if (Para->SelfSta.bit.GPRSState == 1)
		{
		}
		
		/* 切换透传模式 */
		if (Para->TransMode == 0)
		{
			if ((Para->ConIPAddr.byte.uPort == 2020) && (Para->DtuNetFlag.bit.ObtainedIP == 1))		//已获得数据IP
			{
				debug_printf("ChangeIP to 111.53.192.68:2028\r\n");
				ConvIPStrToArray("111.53.192.68:2028", g_BatchIP.all);
//				memcpy(Para->ChangeIPAddr.all, g_BatchIP.all, 8);
				DTU_ChangeIP(g_BatchIP, 0);
				Para->DtuNetFlag.bit.ObtainedIP = 0;
				Para->DtuNetFlag.bit.ChangedIPSta = 1;	//开始切换IP
				SetupTickTimerPara(6, 0, 0, NULL, NULL, &g_AskSignalDelay);
				StartTickTimer(&g_AskSignalDelay);
				StartTickTimer(&g_AskPhoneNumDelay);
			}
			if ((Para->ConIPAddr.byte.uPort == 2028) && (Para->DtuNetFlag.bit.ObtainedIP == 1))		//切换到批量IP
			{
				debug_printf("Change IP Successful.\r\n");
				StopTickTimer(&g_AskSignalDelay);
				StopTickTimer(&g_AskPhoneNumDelay);
				Para->TransMode = 1;
			}
		}
	}
//		/* 切换透传测试 */
//		if (Para->TransMode == 0)
//		{
//			if ((Para->ConIPAddr.byte.uPort == PORTNUM_DATA) && (Para->DtuNetFlag.bit.ObtainedIP == 1))		//已获得数据IP
//			{
//				vTaskDelay(3000);
//				debug_msg("ChangeIP to 111.53.192.10:2030\r\n");
//				ConvIPStrToArray("111.53.192.10:2030", g_BatchIP.all);
//				DTU_ChangeIP(g_BatchIP, 0);
//				Para->DtuNetFlag.bit.ObtainedIP = 0;
//				Para->DtuNetFlag.bit.ChangedIPSta = 1;	//开始切换IP
//				SetupTickTimerPara_1s(DELAY_ASKSIGNAL_DISCONN, 0, NULL, &g_AskSignalDelay);
//				SetupTickTimerPara_1s(DELAY_ASKPHONENUM, RETRYMAX_CHANGEIP, NULL, &g_AskPhoneNumDelay);
//			}
//			if ((Para->ConIPAddr.byte.uPort == PORTNUM_BATCH) && (Para->DtuNetFlag.bit.ObtainedIP == 1))	//已获得批量IP
//			{
//				debug_msg("Change IP Successful.\r\n");
//				SetupTickTimerSta_1s(TTM_MODE_RESTART, TTM_STOP, &g_AskSignalDelay);
//				SetupTickTimerSta_1s(TTM_MODE_RESTART, TTM_STOP, &g_AskPhoneNumDelay);		//停止定时器
//				Para->TransMode = 1;
//			}
//		}
		
//		/* DTU重启 */
//		if (Para->DtuNetFlag.bit.PoweredReset == 1)
//		{
//			pwrret = DTU_PowerCtrlProgress(Para);
//			/* 重启流程完成 */
//			if (pwrret == DTU_COMM_OK)
//			{
//				SoftReset();			//重启程序
//			}
//		}
//		/* 接收开始升级命令 */
//		if (recvret == UPD_STA_RECVCMD)
//		{
//			DTU_ChangeIP(g_ServerInfo.UpdateIP, 1);
//			memcpy(Para->ChangeIPAddr.all, g_ServerInfo.UpdateIP.all, 8);
//			Para->DtuNetFlag.bit.ObtainedIP = 0;
//			Para->DtuNetFlag.bit.Connected = 0;
//			Para->DtuNetFlag.bit.ChangedIPSta = 1;	//开始切换IP
//			SetupTickTimerPara(DELAY_ASKSIGNAL_DISCONN, TTM_MODE_RESTART, RETRYMAX_CHANGEIP, NULL, NULL, &g_AskSignalDelay);
//			ResetTickTimer(&g_AskSignalDelay);
//			SetupTickTimerPara(DELAY_ASKPHONENUM, TTM_MODE_RESTART, RETRYMAX_CHANGEIP, NULL, NULL, &g_AskPhoneNumDelay);
//			ResetTickTimer(&g_AskPhoneNumDelay);
//			Para->DtuProgress = DTU_PROG_UPDATE_ENTRY;
//			debug_msg("DTU -> Entry Update\r\n");
//		}
//		/* 进入透传模式 */
//		if (Para->TransMode == 1)
//		{
//			/* 切换透传 */
//			DTU_TranparentTransMode(1);							//发送透传命令
//			Para->DtuNetFlag.bit.Transport = 0;
//			Para->DtuProgress = DTU_PROG_TRASPORT_ENTRY;		//切换到->开始透传
//			debug_msg("DTU -> Entry Transport\r\n");
//		}
//		ret = recvret;
//	}
//	/* 当前状态=开始升级 */
//	else if (Para->DtuProgress == DTU_PROG_UPDATE_ENTRY)
//	{
//		recvret = DTU_ConnectionProgress(Para);
//		
//		/* 已连接 */
//		if (recvret == DTU_COMM_GOTTENCONNINFO)
//		{
//			/* 连接到升级IP */
//			if (memcmp(Para->ConIPAddr.all, g_ServerInfo.UpdateIP.all, 8) == 0)
//			{
//				Para->DtuProgress = DTU_PROG_UPDATE_REQ;
//				
//				debug_msg("DTU -> Update Request\r\n");
//			}
//		}
//	}
//	/* 当前状态=请求升级包 */
//	else if (Para->DtuProgress == DTU_PROG_UPDATE_REQ)
//	{
//		
//		DTU_SendUpdateRequest();
//		StartTickTimer(&g_UpdateRetryDelay);
//		debug_printf("Request %d/%d \r\n", g_UpdatePara.RequestPktOrder, g_UpdatePara.TotalPktNbr);
//		Para->DtuProgress = DTU_PROG_UPDATE_RECV;			//切换到接收升级数据
//		debug_msg("DTU -> Update Recv\r\n");
//	}
//	/* 当前状态=接收升级包 */
//	else if (Para->DtuProgress == DTU_PROG_UPDATE_RECV)
//	{
//		recvret = DTU_AnlysisTypecodeProgress(Para);
//		
//		if (recvret == UPD_STA_RECVDAT)
//		{
//			g_UpdatePara.RetryCnt = 0;
//			StopTickTimer(&g_UpdateRetryDelay);
//			debug_printf("Recv %d/%d \r\n", g_UpdatePara.RecvdPktOrder, g_UpdatePara.TotalPktNbr);
//			Para->DtuProgress = DTU_PROG_UPDATE_REQ;		//切换到 ->索要升级
//			debug_msg("DTU -> Update Request\r\n");
//		}
//		else if (recvret == UPD_STA_FINISH)
//		{
//			g_UpdatePara.RetryCnt = 0;
//			StopTickTimer(&g_UpdateRetryDelay);
//			DTU_SendUpdateFinish();
//			HAL_Delay(100);
//			WriteNormalFileSizeToFlash();
//			HAL_Delay(100);
//			Para->DtuProgress = DTU_PROG_UPDATE_EXIT;		//切换到 -> 退出升级
//			debug_msg("DTU -> Update Exit\r\n");
//		}
//		else
//		{
//			/* 未接收成功 */
//			if (g_UpdateRetryDelay.TimerFlag.TimeOutFlag == 1)
//			{
//				g_UpdateRetryDelay.TimerFlag.TimeOutFlag = 0;
//				g_UpdatePara.RetryCnt++;
//				debug_printf("Retry %d/%d\r\n", g_UpdatePara.RetryCnt, NOTGETPACK_MAXNUM);
//				if (g_UpdatePara.RetryCnt > NOTGETPACK_MAXNUM)
//				{
//					DTU_SendUpdateFinish();
//					Para->DtuProgress = DTU_PROG_UPDATE_EXIT;	//切换到 -> 退出升级
//					debug_msg("DTU -> Update Exit\r\n");
//				}
//				else
//				{
//					Para->DtuProgress = DTU_PROG_UPDATE_REQ;	//切换到 -> 请求升级包
//					debug_msg("DTU -> Update Request\r\n");
//				}
//			}
//		}
//	}
//	/* 当前状态=退出升级 */
//	else if (Para->DtuProgress == DTU_PROG_UPDATE_EXIT)
//	{
//		DTU_ChangeIP(g_ServerInfo.DataIP, 0);
//		memcpy(Para->ChangeIPAddr.all, g_ServerInfo.DataIP.all, 8);
//		Para->DtuNetFlag.bit.ObtainedIP = 0;
//		Para->DtuNetFlag.bit.ChangedIPSta = 1;	//开始切换IP
//		Para->DtuNetFlag.bit.PoweredReset = 1;	//重启DTU
//		SetupTickTimerPara(DELAY_ASKSIGNAL_DISCONN, TTM_MODE_RESTART, RETRYMAX_CHANGEIP, NULL, NULL, &g_AskSignalDelay);
//		ResetTickTimer(&g_AskSignalDelay);
//		SetupTickTimerPara(DELAY_ASKPHONENUM, TTM_MODE_RESTART, RETRYMAX_CHANGEIP, NULL, NULL, &g_AskPhoneNumDelay);
//		ResetTickTimer(&g_AskPhoneNumDelay);
//		Update_InitUpdatePara(&g_UpdatePara);
//		Para->DtuProgress = DTU_PROG_CONNECTING;			//切换到 -> 正在连接
//		debug_msg("DTU -> Connecting\r\n");
//	}
//	/* 当前状态=开始透传 */
//	else if (Para->DtuProgress == DTU_PROG_TRASPORT_ENTRY)
//	{
//		recvret = DTU_TransConnectionProgress(Para);
//		
//		/* 返回透传 */
//		if (recvret == DTU_COMM_TRANS_CONN)
//		{
//			Para->DtuProgress = DTU_PROG_TRASPORTION;			//切换到->透传模式
//			debug_msg("DTU -> Transportion\r\n");
//		}
//		/* 透传切换失败 应答超时 */
//		else if (recvret == DTU_COMM_ERR_TRANSTFAIL)
//		{
//			Para->DtuProgress = DTU_PROG_TRASPORT_EXIT;			//切换到->退出透传
//			DTU_TranparentTransMode(2);
//			Para->DtuNetFlag.bit.Transport = 1;
//			debug_msg("DTU -> Exit Transport\r\n");
//		}
//	}
//	/* 当前状态=透传模式 */
//	else if (Para->DtuProgress == DTU_PROG_TRASPORTION)
//	{

//		
//		/* 退出透传模式 */
//		if (Para->TransMode == 2)
//		{
//			DTU_TranparentTransMode(2);						//发送退出透传
//			Para->DtuNetFlag.bit.Transport = 1;
//			Para->DtuProgress = DTU_PROG_TRASPORT_EXIT;		//切换到->退出透传
//			debug_msg("DTU -> Exit Transport\r\n");
//		}
//	}
//	/* 当前状态=退出透传 */
//	else if (Para->DtuProgress == DTU_PROG_TRASPORT_EXIT)
//	{
//		recvret = DTU_TransConnectionProgress(Para);
//		
//		/* 返回透传退出 */
//		if (recvret == DTU_COMM_TRANS_EXIT)
//		{
//			Para->DtuProgress = DTU_PROG_NORMAL;			//切换到->正常
//			Para->TransMode = 0;
//			debug_msg("DTU -> Normal\r\n");
//		}
//		/* 透传切换失败 应答超时 */
//		else if (recvret == DTU_COMM_ERR_TRANSTFAIL)
//		{
//			Para->DtuProgress = DTU_PROG_NORMAL;			//切换到->正常
//			Para->TransMode = 0;
//		}
//	}
//	
//	return ret;
}

