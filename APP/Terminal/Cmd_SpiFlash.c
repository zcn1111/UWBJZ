/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_SpiFlash.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 SpiFlash命令


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
#include "Drv_W25Qxx_QSPI.h"



#include "Cmd_SpiFlash.h"

uint8_t g_SpiDataBuf[384];
/******************************************************************
函数名称: int16_t Cmd_ReadPageSpiFlash
功能描述: 读一页SpiFlash
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_ReadPageSpiFlash(uint8_t Argc, char *Argv[])
{
	uint8_t *pbuf;
	uint32_t staddr, len;
	
	if (Argc < 2)
	{
		return DBGRET_ERR_ARG_LESS;
	}
	
//	pbuf = pvPortMalloc(384);	//申请内存
	pbuf = &g_SpiDataBuf[0];
	memset(pbuf, 0x00, 384);
	/* 对16进制数处理 */
	if ((Argv[1][0] == '0') && (Argv[1][1] == 'x'))
	{
		staddr = HexStr2Int32(Argv[1]);
	}
	else
	{
		staddr = atol(Argv[1]);
	}
	len = atol(Argv[2]);
	if (len > 256) len = 256;
	FlashReadData(pbuf, staddr, len, false);
	debug_info(pbuf, len, "Flash->");
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_WritePageSpiFlash
功能描述: 写一页SpiFlash
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_WritePageSpiFlash(uint8_t Argc, char *Argv[])
{
	uint8_t *pbuf;
	uint32_t staddr, len;
	
	if (Argc < 3)
	{
		return DBGRET_ERR_ARG_LESS;
	}
	
//	pbuf = pvPortMalloc(384);
	pbuf = &g_SpiDataBuf[0];
	memset(pbuf, 0x00 ,384);
	/* 对16进制数处理 */
	if ((Argv[1][0] == '0') && (Argv[1][1] == 'x'))
	{
		staddr = HexStr2Int32(Argv[1]);
	}
	else
	{
		staddr = atol(Argv[1]);
	}
	len = atol(Argv[2]);
	if (len > 256) len = 256;
	memcpy(pbuf, Argv[3], len);
	FlashWriteData(pbuf, staddr, len);
	debug_info(pbuf, len, "->Flash");
	
	return DBGRET_OK;
}


