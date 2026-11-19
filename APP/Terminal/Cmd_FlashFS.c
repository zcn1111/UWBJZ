/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_Fatfs.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 Fatfs相关命令


其他说明:
功能列表:  
历史记录:
1. 日期:
作者:
修改内容:
2. ..
***************************************************************************************/
#include "BSP.h"

/* FlashFS支持库 */
#include "RTL.h"
#include "File_Config.h"
#include "FFS.h"

//#include "stdio.h"

#include "Cmd_FlashFS.h"


/******************************************************************
函数名称: int16_t Cmd_MountNand
功能描述: 挂载Nand
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_MountNand(uint8_t Argc, char *Argv[])
{
	uint8_t  ffret;
	uint8_t  retrycnt;
	uint32_t totalinkb;
	uint32_t freeinkb;
	char path[4];
	
	Media_INFO minfo;
	FAT_VI *mc0;
		
	retrycnt = 3;
	if (Argc == 0)
	{
		strncpy(path, "N0:", 3);
	}
	if (Argc == 1)
	{
		strncpy(path, Argv[1], 3);
	}
	do
	{
		/* 挂载NandFlash */
		ffret = finit(path);									//尝试挂载Nand
		
		if (ffret == 0)
		{
			/* 挂载成功 */
			freeinkb = FFS_GetFreeSpaceInKilobytes(path);		//获取可用容量
			funinit(path);										//卸载文件系统
			totalinkb = FFS_GetTotalSpaceInKilobytes(path);		//获取总容量
			debug_printf("%10u KiB total drive space. \r\n", totalinkb);
			debug_printf("%10u KiB available. \r\n", freeinkb);
			break;
		}
		else
		{
			/* 存储设备初始化失败 */
			if (totalinkb == 0)
			{
				debug_printf("Init media failure. \r\n");
			}
		}
		debug_printf("Init failure. Retry. \r\n");
		retrycnt--;
	} while (retrycnt > 0);
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_UnMountNand
功能描述: 卸载Nand
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_UnMountNand(uint8_t Argc, char *Argv[])
{	
	uint8_t ffret;
	char path[4];
	
	if (Argc == 0)
	{
		strncpy(path, "N0:", 4);
	}
	if (Argc == 1)
	{
		strncpy(path, Argv[1], 4);
	}
	ffret = funinit(path);
	if (ffret == 0)
	{
		debug_printf("Unmount OK.\r\n");
	}
	else
	{
		debug_printf("Unmount failure. \r\n");
	}
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_FormatNand
功能描述: 格式化Nand
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_FormatNand(uint8_t Argc, char *Argv[])
{
	uint8_t ffret;
	char path[4];
		
	/* 格式化 */
	if (Argc == 0)
	{
		strncpy(path, "N0:", 4);
	}
	if (Argc == 1)
	{
		strncpy(path, Argv[1], 4);
	}
	debug_printf("Formatting... \r\n");
	ffret = fformat(path);
	if (ffret == 0)
	{
		debug_printf("Format OK! \r\n");
	}
	else
	{
		debug_printf("Foramt fail.\r\n");
	}
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_GetFreeSpace
功能描述: 获取空闲磁盘空间
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_GetFreeSpace(uint8_t Argc, char *Argv[])
{
	uint32_t freeinkb;
	uint32_t totalinkb;
	char path[4];
	
	if (Argc == 0)
	{
		strncpy(path, "N0:", 4);
	}
	if (Argc == 1)
	{
		strncpy(path, Argv[1], 4);
	}
	freeinkb = FFS_GetFreeSpaceInKilobytes(path);
	totalinkb = FFS_GetTotalSpaceInKilobytes(path);
	debug_printf("%10u KiB total drive space. \r\n", totalinkb);
	debug_printf("%10u KiB available. \r\n", freeinkb);
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_DeleteFile
功能描述: 删除文件
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_DeleteFile(uint8_t Argc, char *Argv[])
{	
//	FRESULT fret;
//	uint16_t nfiles;
//	
//	
//	fret = f_getcwd(g_Path, 64);		//获取当前路径
//	if (Argc == 1)
//	{
//		MergePathAndFilename(g_Path, Argv[1], g_FullName);	//合并路径
//		fret = f_unlink(g_FullName);						//删除文件
//		if (fret == FR_OK)
//		{
//			debug_printf("File deleted. \r\n");
//		}
//		else if (fret == FR_DENIED)
//		{
//			debug_printf("Operater has been denied. \r\n");
//		}
//		else
//		{
//			debug_printf("File delete failed. ErrCode=%d\r\n", fret);
//		}
//	}
//	else if (Argc == 0)
//	{
//		debug_printf("Syntax error. need filename. \r\n");
//	}
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_ListDir
功能描述: 列当前路径下目录
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_ListDir(uint8_t Argc, char *Argv[])
{
	uint8_t ffret;
	FINFO finfo;
	uint32_t freeinbyte;
	uint16_t nfiles, ndirs;
	
	finfo.fileID = 0;
	nfiles = 0;
	ndirs = 0;
	ffret = finit("N0:");
	if (ffret != 0)
	{
		/* 处理挂载失败 */
		debug_printf("Mount failure.\r\n");
		ffret = funinit("N0:");
		return DBGRET_OK;
	}
	
	debug_printf("Current Path: / \r\n");
	freeinbyte = ffree("N0:");
	while (ffind("N0:*.*", &finfo) == 0)
	{
		/* 处理目录 */
		if (finfo.attrib & ATTR_DIRECTORY)
		{
			/* 判断是目录 */
			debug_printf("      <DIR>     %s \n", finfo.name);
			ndirs++;
		}
		else
		{
			/* 判断是文件 */
			debug_printf("%10u %s \n", (uint32_t)finfo.size, finfo.name);
			nfiles++;
		}
	}
	debug_printf("%d dirs, %d files. \n", ndirs, nfiles);
	
	if (finfo.fileID == 0)
	{
		debug_printf("no file.\r\n");
	}
	
	debug_printf("%10u KiB available. \r\n", freeinbyte / 1024);

	/* 卸载Nand */
	ffret = funinit("N0:");
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_WriteFile
功能描述: 写文件
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_WriteFile(uint8_t Argc, char *Argv[])
{
	char filename[64];
	FILE *fout;
	uint32_t bw;
	
	uint8_t ffret;
	
	/* 没有参数 返回错误 */
	if (Argc == 0)
	{
		debug_printf("Syntax error. Need filename. \r\n");
		return DBGRET_ERR_ARG_LESS;
	}
//	memset(filename, 0x00, sizeof(filename));
//	strncpy(filename, "N0:\\", 5);
//	strncpy(filename + 5, Argv[1], strlen(Argv[1]));
		
	ffret = finit("N0:");
	if (ffret != 0)
	{
		ffret = funinit("N0:");
		return DBGRET_OK;
	}
	fout = fopen("N0:\\t.txt", "w");
	if (fout != NULL)
	{
		bw = fwrite(Argv[2], sizeof(char), strlen(Argv[2]), fout);
		if (bw == strlen(Argv[2]))
		{
			debug_printf("Write %d byte ok.\r\n", strlen(Argv[2]));
		}
		else
		{
			debug_printf("Write failure.\r\n");
		}
		fclose(fout);
	}
	else
	{
		debug_printf("open file %s failure.\r\n", Argv[1]);
	}
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: void DisplayData
功能描述: 显示数据
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void DisplayData(uint8_t *Buf, uint8_t DispMode, uint16_t Datlen)
{
	uint16_t i;
	
	for (i=0; i<Datlen; i++)
	{
		/* ASC显示 */
		if (DispMode == 0)
		{
			if (((Buf[i] >= 0) && (Buf[i] <=31)) || Buf[i] == 127)
			{
				debug_printf(".");
			}
			else
			{
				debug_printf("%c", Buf[i]);
			}
		}
		/* Hex显示 */
		if (DispMode == 1)
		{
			debug_printf("%02X ", Buf[i]);
		}
		if (i % 15 == 0)
		{
			debug_printf("\r\n");
		}
	}
}

/******************************************************************
函数名称: int16_t Cmd_ReadFile
功能描述: 读文件
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_ReadFile(uint8_t Argc, char *Argv[])
{
//	FRESULT	fret;
//	UINT	slen=0;
//	UINT	rlen=0;
//	FSIZE_t	filesize=0;
//	DWORD	stbyte, edbyte, selbyte;
//	uint8_t	mode;
//	
//	uint16_t i;
//	uint8_t	*rbuf;
//	
//	/* 没有参数 返回错误 */
//	if (Argc == 0)
//	{
//		debug_printf("Syntax error. Need filename. \r\n");
//		return DBGRET_ERR_ARG_LESS;
//	}
//	else if (Argc < 2)
//	{
//		debug_printf("Argument less.\r\n");
//		return DBGRET_ERR_ARG_LESS;
//	}
//	
//	/* 获取文件名 */
//	fret = f_getcwd(g_Path, 64);	//获取当前路径
//	if (fret == FR_OK)
//	{
//		MergePathAndFilename(g_Path, Argv[1], g_FullName);
//		fret = f_open(&g_objFile, g_FullName, FA_READ);
//		if (fret == FR_NO_FILE)
//		{
//			debug_printf("File not exist.\r\n");
//			return DBGRET_ERR_ARG_ILLEGAL;
//		}
//		if (fret == FR_OK)
//		{
//			filesize = f_size(&g_objFile);
//		}
//	}
//	
//	/* 显示参数 */
//	if (Argc == 2)
//	{
//		if (strcasecmp(Argv[2], "-hex") == 0)
//		{
//			mode = 1;
//		}
//		else if (strcasecmp(Argv[2], "-asc") == 0)
//		{
//			mode = 0;
//		}
//		else
//		{
//			debug_printf("Argument illegal.\r\n");
//			return DBGRET_ERR_ARG_ILLEGAL;
//		}
//	}
//	
//	/* 获取起始和结束字节位置 */
//	/* 没有位置参数 全部文件 */
//	if (Argc == 2)
//	{
//		stbyte = 0;
//		edbyte = filesize;
//		selbyte = filesize;
//	}
//	/* 有起始位置 */
//	else if (Argc == 3)
//	{
//		stbyte = atol(Argv[3]);
//		if (stbyte >= filesize)
//		{
//			debug_printf("Start byte out of range.\r\n");
//			return DBGRET_ERR_ARG_OUTRANGE;
//		}
//		selbyte = edbyte - stbyte;
//	}
//	/* 有起始和终止位置 */
//	else if (Argc == 4)
//	{
//		stbyte = atol(Argv[3]);
//		edbyte = atol(Argv[4]);
//		if ((stbyte > filesize) || (stbyte > edbyte))
//		{
//			debug_printf("Start byte out of range.\r\n");
//			return DBGRET_ERR_ARG_OUTRANGE;
//		}
//		selbyte = edbyte - stbyte;
//	}
//	
//	/* 读文件 */
//	rbuf = pvPortMalloc(1024);
//	slen = selbyte;
//	fret = f_lseek(&g_objFile, stbyte);						//移动读写指针
//	do
//	{
//		/* 分段读取 */
//		if (slen > 1024)
//			slen = 1024;
//		memset(rbuf, 0x00, 1024);
//		fret = f_read(&g_objFile, rbuf, slen, &rlen);
//		DisplayData(rbuf, mode, rlen);
//		slen -= 1024;
//	} while (slen > 0);
//	fret = f_close(&g_objFile);
//	vPortFree(rbuf);
	
	return DBGRET_OK;
}

