/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_Network.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 网络命令


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
#include "stdio.h"
#include "Drv_Nand.h"

/* RTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"

#include "NET_Upload.h"
#include "Cmd_Network.h"
#include "g_GlobalValDef.h"


/******************************************************************
函数名称: int16_t Cmd_SendDataByNetwork
功能描述: 通过网络发送数据
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_SendDataByNetwork(uint8_t Argc, char *Argv[])
{
	uint32_t datasize;
	uint32_t blksize;
	
	if (Argc == 0)
	{
		g_SendDataSize = 128;	//默认发送128KB
		g_SendBlkSize = 1024;	//默认数据块 1024字节
		
	}
	else if (Argc < 2)
	{
		debug_printf("Argument less.\r\n");
		return DBGRET_ERR_ARG_LESS;
	}
	else
	{
		g_SendDataSize = atol(Argv[1]);
		g_SendBlkSize = atol(Argv[2]);
		if ((g_SendDataSize <= 0) || (g_SendDataSize > 512))
		{
			debug_printf("Argument out of range. Use default.\r\n");
			g_SendDataSize = 128;
		}
		if ((g_SendBlkSize != 512) && (g_SendBlkSize != 1024) && (g_SendBlkSize != 2048) && (g_SendBlkSize != 4096))
		{
			debug_printf("Argument out of range. Use default.\r\n");
			g_SendBlkSize = 1024;
		}
	}
	debug_printf("Send size=%d, block=%d \r\n", g_SendDataSize, g_SendBlkSize);
	g_HttpSendFlag = 1;
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_GetCH395Version
功能描述: 获取CH395版本号
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_GetCH395Version(uint8_t Argc, char *Argv[])
{
	uint8_t ver=0xFF;
	
	ver = CH395_GetVersion();
	debug_printf("CH395 Version: %x\r\n", ver);
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_GetCH395EthInfo
功能描述: 获取CH395以太网信息
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_GetCH395EthInfo(uint8_t Argc, char *Argv[])
{
	CH395_DispEthPara();
}

/******************************************************************
函数名称: int16_t Cmd_CH395Init
功能描述: 初始化CH395
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_CH395Init(uint8_t Argc, char *Argv[])
{
	uint8_t sta;
	uint8_t dhcpsta;
	
	IPAddr_t iptmp;
	uint8_t sock[6];
	uint8_t socksta[2];
	CH395_SocketIntStatus_t sockintsta;
	
	uint16_t wait;
	
	
	debug_printf("Init Successfully.\r\n");
	
	
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_CH395Connect
功能描述: 打开Socket并连接
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_CH395Connect(uint8_t Argc, char *Argv[])
{
	uint8_t sta;
	
	uint8_t sock[6];
	uint8_t socksta[2];
	CH395_SocketIntStatus_t sockintsta;
	
	IPAddr_t iptmp;
	
	uint16_t wait;
	
	CH395_SendCmd(CH395_CMD_GET_SOCK_STA_SN, NULL, 0, socksta, 2, 0);
	if (socksta[0] == CH395_SOCK_STA_OPEN)
	{
		debug_printf("Socket Open.\r\n");
		if (socksta[1] == TCP_ESTABLISHED)
		{
			debug_printf("TCP Established.\r\n");
			CH395_DispSocketPara(0);
		}
	}
	return DBGRET_OK;
}

