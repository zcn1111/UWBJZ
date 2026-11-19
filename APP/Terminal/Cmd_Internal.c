/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_Internal.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 内部命令


其他说明:
功能列表:  
历史记录:
1. 日期:
作者:
修改内容:
2. ..
***************************************************************************************/
/* BSP支持 */
#include "BSP.h"
#include "string.h"
#include "stdio.h"

#include "TerminalConf.h"

uint16_t Addnum;
/******************************************************************
函数名称: int16_t Cmd_GetCompilerInfo
功能描述: 获取编译信息
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
//int16_t Cmd_GetCompilerInfo(uint8_t Argc, char *Argv[])
//{
//	debug_printf("Progrma File : %s \r\n", __FILE__);
//	debug_printf("Compiled in %s %s", __DATE__, __TIME__);
//	
//	return 0;
//}

/******************************************************************
函数名称: int16_t Cmd_SetDateTime
功能描述: 设置日期时间
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_SetDateTime(uint8_t Argc, char *Argv[])
{
	HexDateTime_t	tmptime;
	uint16_t 		slen;
	uint16_t		i;
	
	char			strtmp[]={0};
	uint16_t		spiltpos1;
	uint16_t		spiltpos2;
	char			ch;
	uint16_t		val;
	uint8_t			timeflag;
	
	int16_t			iret;
	
	/* 不带参数显示当前时间 */
	if (Argc == 0)
	{
		debug_printf("Now Time: 20%02d/%02d/%02d %02d:%02d:%02d", g_SysTime.Year, g_SysTime.Month, g_SysTime.Day, g_SysTime.Hour, g_SysTime.Minute, g_SysTime.Second);
		return DBGRET_OK;
	}
	if (Argc == 1)
	{
		debug_printf("Argument Less.\r\n");
		return DBGRET_ERR_ARG_LESS;
	}
	
	/* 带参数修改当前时间 */
	if (Argc == 2)
	{
		/* 处理日期 */
		slen = strlen(Argv[1]);
		spiltpos1 = 0;
		spiltpos2 = 0;
		timeflag = 0;
		for (i=0; i<slen; i++)
		{
			ch = Argv[1][i];
			if (ch == '/')
			{
				if (spiltpos1 == 0)
				{
					spiltpos1 = i;
				}
				else if (spiltpos2 == 0)
				{
					spiltpos2 = i;
				}
			}
		}
		iret = 0;
		strncpy(strtmp, Argv[1] + (spiltpos1 - 2), 2);
		val = atol(strtmp);
		if (val > 99)
		{
			iret = -1;
		}
		else
		{
			tmptime.Year = val;
		}
		if (iret == 0)
		{
			strncpy(strtmp, Argv[1] + (spiltpos1 + 1), (spiltpos2 - spiltpos1 - 1));
			val = atol(strtmp);
			if (val > 12)
			{
				iret = -1;
			}
			else
			{
				tmptime.Month = val;
			}
		}
		if (iret == 0)
		{
			strncpy(strtmp, Argv[1] + (spiltpos2 + 1), 2);
			val = atol(strtmp);
			if (val > 31)
			{
				iret = -1;
			}
			else
			{
				tmptime.Day = val;
			}
		}
		
		/* 处理时间 */
		slen = strlen(Argv[2]);
		spiltpos1 = 0;
		spiltpos2 = 0;
		timeflag = 0;
		for (i=0; i<slen; i++)
		{
			ch = Argv[2][i];
			if (ch == ':')
			{
				if (spiltpos1 == 0)
				{
					spiltpos1 = i;
				}
				else if (spiltpos2 == 0)
				{
					spiltpos2 = i;
				}
			}
		}
		iret = 0;
		strncpy(strtmp, Argv[2] + (spiltpos1 - 2), 2);
		val = atol(strtmp);
		if (val > 24)
		{
			iret = -1;
		}
		else
		{
			tmptime.Hour = val;
		}
		if (iret == 0)
		{
			strncpy(strtmp, Argv[2] + (spiltpos1 + 1), (spiltpos2 - spiltpos1 - 1));
			val = atol(strtmp);
			if (val > 59)
			{
				iret = -1;
			}
			else
			{
				tmptime.Minute = val;
			}
		}
		if (iret == 0)
		{
			strncpy(strtmp, Argv[2] + (spiltpos2 + 1), 2);
			val = atol(strtmp);
			if (val > 59)
			{
				iret = -1;
			}
			else
			{
				tmptime.Second = val;
			}
		}
	}
	
	if (iret == 0)
	{
		g_SysTime.Year = tmptime.Year;
		g_SysTime.Month = tmptime.Month;
		g_SysTime.Day = tmptime.Day;
		g_SysTime.Hour = tmptime.Hour;
		g_SysTime.Minute = tmptime.Minute;
		g_SysTime.Second = tmptime.Second;
		debug_printf("DateTime update Successfully.\r\n");
		return DBGRET_OK;
	}
	if (iret != 0)
	{
		debug_printf("Argument illegal.\r\n");
		return DBGRET_ERR_ARG_ILLEGAL;
	}
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_SpiTest
功能描述: SPI测试
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_SpiTest(uint8_t Argc, char *Argv[])
{
//	uint8_t txrx=0;
//	uint8_t mode=0;
//	uint8_t i;
//	
//	if (Argc == 0)
//	{
//		return DBGRET_ERR_ARG_LESS;
//	}
//	
//	if (strcasecmp(Argv[1], "tx") == 0)
//	{
//		txrx = 1;
//	}
//	else if (strcasecmp(Argv[1], "rx") == 0)
//	{
//		txrx = 0;
//	}
//	else if (strcasecmp(Argv[1], "txrx") == 0)
//	{
//		txrx = 2;
//	}
//	
//	if (Argc == 2)
//	{
//		if (strcasecmp(Argv[2], "dma") == 0)
//		{
//			mode = 1;
//		}
//	}
//	
//	for (i=0; i<10; i++)
//	{
//		SpiTxBuf[i] = i + Addnum;
//		SpiRxBuf[i] = 0x00;
//	}
//	Addnum += 16;
//	
//	if (txrx == 0)
//	{
//		if (mode == 1)
//		{
//			memset(SpiRxBuf, 0x00, 10);
//			HAL_SPI_Receive_DMA(&hSPI4, SpiRxBuf, 10);
//			Delay1ms(100);
//			HAL_SPI_Abort(&hSPI4);
//		}
//		else
//		{
//			memset(SpiRxBuf, 0x00, 10);
//			HAL_SPI_Receive(&hSPI4, SpiRxBuf, 10, 1000);
//			for (i=0; i<10; i++)
//			{
//				debug_printf("%02X ", SpiRxBuf[i]);
//			}
//			debug_printf("\r\n");
//		}
//	}
//	else if (txrx == 1)
//	{
//		if (mode == 1)
//		{
//			HAL_SPI_Transmit_DMA(&hSPI4, SpiTxBuf, 10);
//			Delay1ms(100);
//			HAL_SPI_Abort(&hSPI4);
//		}
//		else
//		{
//			HAL_SPI_Transmit(&hSPI4, SpiTxBuf, 10, 1000);
//		}
//	}
//	else if (txrx == 2)
//	{
//		if (mode == 1)
//		{
//			memset(SpiRxBuf, 0x00, 10);
//			HAL_SPI_TransmitReceive_DMA(&hSPI4, SpiTxBuf, SpiRxBuf, 10);
//			Delay1ms(100);
//			HAL_SPI_Abort(&hSPI4);
//		}
//		else
//		{
//			memset(SpiRxBuf, 0x00, 10);
//			HAL_SPI_TransmitReceive(&hSPI4, SpiTxBuf, SpiRxBuf, 10, 1000);
//			for (i=0; i<10; i++)
//			{
//				debug_printf("%02X ", SpiRxBuf[i]);
//			}
//			debug_printf("\r\n");
//		}
//	}
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_SoftReset
功能描述: 程序重启
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_SoftReset(uint8_t Argc, char *Argv[])
{
	debug_printf("It will RESET\r\n");
	SoftReset();
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_Echo
功能描述: 数据回显控制
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_Echo(uint8_t Argc, char *Argv[])
{
	if (Argc == 2)
	{
		/* LORA回显控制 */
		if (strcasecmp(Argv[1], "lora") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableLoraEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableLoraEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableLoraEcho = 0;
			}
		}
		/* RTK回显控制 */
		else if (strcasecmp(Argv[1], "rtk") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableRtkEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableRtkEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableRtkEcho = 0;
			}
		}
		/* GPS回显控制 */
		else if (strcasecmp(Argv[1], "gps") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableGpsEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableGpsEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableGpsEcho = 0;
			}
		}
		/* DTU回显控制 */
		else if (strcasecmp(Argv[1], "dtu") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableDtuEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableDtuEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableDtuEcho = 0;
			}
		}
		/* 蓝牙回显控制 */
		else if (strcasecmp(Argv[1], "ble") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableBleEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableBleEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableBleEcho = 0;
			}
		}
		/* RTCM(差分定位)回显控制 */
		else if (strcasecmp(Argv[1], "rtcm") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableRTCMEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableRTCMEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableRTCMEcho = 0;
			}
		}
		/* IMU(陀螺仪)回显控制 */
		else if (strcasecmp(Argv[1], "imu") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableIMUEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableIMUEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableIMUEcho = 0;
			}
		}
		/* CORS回显控制 */
		else if (strcasecmp(Argv[1], "cors") == 0)
		{
			if ((strcasecmp(Argv[2], "on") == 0) || (strcasecmp(Argv[2], "bin") == 0))
			{
				g_DbgEcho.EnableCORSEcho = 1;
			}
			else if (strcasecmp(Argv[2], "asc") == 0)
			{
				g_DbgEcho.EnableCORSEcho = 2;
			}
			else if (strcasecmp(Argv[2], "off") == 0)
			{
				g_DbgEcho.EnableCORSEcho = 0;
			}
		}
	}
	
	return DBGRET_OK;
}

