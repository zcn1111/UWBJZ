/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_Nand.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 Nand命令


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
//#include "Drv_FTL.h"

#include "Cmd_Nand.h"

uint8_t ECCBuf[512];

/******************************************************************
函数名称: int16_t Cmd_EraseNand
功能描述: 擦除Nand
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_EraseNand(uint8_t Argc, char *Argv[])
{
    uint8_t		status;
    uint16_t	i=0;
	uint16_t	blk_st, blk_ed, blk_d;
	uint16_t	tblk;
	
	debug_printf("Erasing NandFlash. It will take some time...\r\n");
	tblk = nand_dev.block_totalnum;
	/* 无参数 */
	if (Argc == 0)
	{
		debug_printf("It will erasing entire chip. Total block number=%d\r\n", tblk);
		blk_st = 0x0000;
		blk_ed = tblk;
		blk_d = blk_ed - blk_st;
	}
	/* 带起始位置 */
	else if (Argc == 1)
	{
		if (blk_st > tblk)
		{
			debug_printf("Out of Range.\r\n");
			return DBGRET_ERR_ARG_OUTRANGE;
		}
		blk_st = atol(Argv[1]);
		debug_printf("It will erasing from %04X to end.\r\n", blk_st);
		blk_ed = tblk;
		blk_d = blk_ed - blk_st;
	}
	/* 带起始和终止位置 */
	else if (Argc == 2)
	{
		if (Argv[1] > Argv[2])
		{
			debug_printf("Argument Error.\r\n");
			return DBGRET_ERR_ARG_ILLEGAL;
		}
		blk_st = atol(Argv[1]);
		blk_ed = atol(Argv[2]);
		debug_printf("It will erasing form %04X to %04X.\r\n", blk_st, blk_ed);
	}
	
    for(i=blk_st; i<blk_ed; i++) //循环擦除块
    {
        status = NAND_EraseBlock(i);
        if (status)
		{			
			debug_printf("Erase %d block fail!! ErrCode=%d\r\n", i, status);	//擦除失败
		}
		else
		{
			if ((i % 64) == 0)
			{
				debug_printf("Progress.. %3.1f%%\r\n", ((float)i/(float)blk_ed) * 100);
			}
		}
    }
	debug_printf("\r\n Erase finished.\r\n");
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_LowFormatNand
功能描述: 低级格式化Nand
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 使用FTL层格式化，重建FTL表
******************************************************************/
int16_t Cmd_LowFormatNand(uint8_t Argc, char *Argv[])
{
//	uint8_t ret;
//	uint8_t mode;
//	
//	mode = 0;
//	if (Argc == 1)
//	{
//		mode = 0;
//		if (strcasecmp(Argv[1], "-full") == 0)
//		{
//			debug_printf("FTL format in full mode. It will take some times.\r\n");
//			mode = 1;
//		}
//	}
//	debug_printf("FTL_formatting... \r\n");
//	
//	ret = FTL_Format(mode);
//	if (ret == 1)
//	{
//		debug_printf("Low format failed. Good block not enough.\r\n");
//		return DBGRET_ERR_OTHER;
//	}
//	else if (ret == 2)
//	{
//		debug_printf("Rebuild FTL failed.\r\n");
//		return DBGRET_ERR_OTHER;
//	}
//	
//	debug_printf("Format OK!\r\n");
//	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_CalcECC
功能描述: 计算ECC
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_CalcECC(uint8_t Argc, char *Argv[])
{
	uint32_t	pagenum;
	uint16_t	colnum;
	uint32_t	ecc;
	
	uint16_t 	i;
	
	if (Argc < 2)
	{
		debug_printf("Argument less.\r\n");
	}
	pagenum = atol(Argv[1]);
	colnum = atol(Argv[2]);
	ecc = NAND_CalcECCByHardware(pagenum, colnum, ECCBuf, 512);
	debug_printf("ECC=0x%08X\r\n", ecc);
	if (Argc == 3)
	{
		if (strcasecmp(Argv[3], "-show") == 0)
		{
			for (i=0; i<512; i++)
			{
				debug_printf("%02X ", ECCBuf[i]);
				if ((i % 16) == 15)
				{
					debug_printf("\r\n");
				}
			}
		}
	}
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_NandBlockInfo
功能描述: Nand块信息
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_NandBlockInfo(uint8_t Argc, char *Argv[])
{
//	FTL_DispBlockInfo();
//	
//	return DBGRET_OK;
}


