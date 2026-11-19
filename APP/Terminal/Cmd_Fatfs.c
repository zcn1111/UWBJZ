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
//#include "RawDataStore.h"
//#include "main.h"
#include "BSP.h"
//#include "ftl.h"

/* fatfs支持库 */
#include "fatfs.h"
#include "nand_diskio.h"

#include "stdio.h"

#include "Cmd_Fatfs.h"


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
	FRESULT fret;
	DWORD fre_clust, fre_sect, tot_sect;
	
	uint8_t getfree=0;
	
	/* 挂载NandFlash */
	fret = f_mount(&NandDiskFatfs, NandDiskPath, 1);	//尝试挂载Nand
	
	/* 挂载成功 */
	if (fret == FR_OK)
	{
		debug_printf("Mount OK! \r\n");
		getfree = 1;
	}
	
	/* 没有文件系统 尝试格式化FAT32后重新挂载 */
	if (fret == FR_NO_FILESYSTEM)
	{
		debug_printf("No FileSystem! \r\n");
		
		if (Argc == 0)
		{
			debug_printf("Add -format argument for formatting disk.\r\n");
		}
		
		/* 格式化参数 -format */
		if ((Argv[1] != NULL) && (strcasecmp(Argv[1], "-format") == 0))
		{
			debug_msg("Format FAT32... \r\n");
			fret = FATFS_FormatDisk(NandDiskPath, FM_FAT32, 512);			//格式化
			if (fret != FR_OK)
			{
				debug_printf("Format Fail ErrCode=%d \r\n", fret);			//格式化错误
			}
			else
			{
				debug_printf("Format OK! \r\n");							//格式化成功
				fret = f_mount(&NandDiskFatfs, NandDiskPath, 1);			//格式化后再挂载
				if (fret == FR_OK)
				{
					getfree = 1;
				}
			}
		}
	}
	else
	{
		if (fret != FR_OK)
		{
			debug_printf("Mount Fail ErrCode=%d \r\n", fret);
		}
	}
	
	/* 获取剩余空间 */
	if (getfree == 1)
	{
		fret = f_getfree(NandDiskPath, &fre_clust, &p_objFatFs);
		if (fret == FR_OK)
		{
			tot_sect = (p_objFatFs->n_fatent - 2) * p_objFatFs->csize;
			fre_sect = fre_clust * p_objFatFs->csize;
			debug_printf("%10lu KiB total drive space. \r\n", tot_sect / 2);
			debug_printf("%10lu KiB available. \r\n", fre_sect / 2);
		}
		return DBGRET_OK;
	}
	
	return fret;
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
	FRESULT fret;

	fret = f_mount(NULL, NandDiskPath, 1);	//卸载
	if (fret == FR_OK)
	{
		debug_printf("Unmount OK! \r\n");
	}
	else
	{
		debug_printf("Unmount Fail! ErrCode=%d \r\n", fret);
	}
	
	return fret;
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
	FRESULT fret;
	BYTE	opt;	//文件系统格式
	DWORD	au;		//分配单元大小(簇)
	
	char*	ffmt;
	
	ffmt = pvPortMalloc(16);
	/* 不指定 使用默认参数 FAT32格式 簇大小=512Byte*/
	if (Argc == 0)
	{
		opt = FM_FAT32;
		strncpy(ffmt, "FAT32", 5);
		au = 512;
	}
	
	/* 指定文件系统格式和簇大小 默认FAT32簇大小=512 */
	if ((Argc == 1) || (Argc == 2))
	{
		opt = FM_FAT32;
		strncpy(ffmt, "FAT32", 5);
		au = 512;
		
		/* 文件系统格式 */
		if (strcasecmp(Argv[1], "-FAT") == 0)
		{
			opt = FM_FAT;
			strncpy(ffmt, "FAT", 3);
		}
		else if (strcasecmp(Argv[1], "-FAT32") == 0)
		{
			opt = FM_FAT32;
			strncpy(ffmt, "FAT32", 5);
		}
		else if (strcasecmp(Argv[1], "-ExFAT") == 0)
		{
			opt = FM_EXFAT;
			strncpy(ffmt, "ExFAT", 5);
		}
		
		/* 配置簇大小 */
		if (Argc == 2)
		{
			if (strcasecmp(Argv[2], "-512") == 0)
			{
				au = 512;
			}
			else if (strcasecmp(Argv[2], "-1024") == 0)
			{
				au = 1024;
			}
			else if (strcasecmp(Argv[2], "-2048") == 0)
			{
				au = 2048;
			}
		}
	}
	
	/* 格式化 */
	debug_printf("Formatting %s in %d ... \r\n", ffmt, (int)au);
	vPortFree(ffmt);
	fret = FATFS_FormatDisk(NandDiskPath, opt, au);
	if (fret == FR_OK)
	{
		debug_printf("Format OK! \r\n");
	}
	else
	{
		debug_printf("Format Fail ErrCode=%d", fret);
	}
	
	return fret;
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
	DWORD	nclst, tot_sect, fre_sect;
	FRESULT	fret;
	TCHAR	*path;
	
	if (Argc == 1)
	{
		path = Argv[1];
	}
	else
	{
		path = NandDiskPath;
	}
	fret = f_getfree(path, &nclst, &p_objFatFs);
	if (fret == FR_OK)
	{
		tot_sect = (p_objFatFs->n_fatent - 2) * p_objFatFs->csize;
		fre_sect = nclst * p_objFatFs->csize;
		debug_printf("%10lu KiB total drive space. \r\n", tot_sect / 2);
		debug_printf("%10lu KiB available. \r\n", fre_sect / 2);
	}
	
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
	FRESULT fret;
	uint16_t nfiles;
	
	
	fret = f_getcwd(g_Path, 64);		//获取当前路径
	if (Argc == 1)
	{
		MergePathAndFilename(g_Path, Argv[1], g_FullName);	//合并路径
		fret = f_unlink(g_FullName);						//删除文件
		if (fret == FR_OK)
		{
			debug_printf("File deleted. \r\n");
		}
		else if (fret == FR_DENIED)
		{
			debug_printf("Operater has been denied. \r\n");
		}
		else
		{
			debug_printf("File delete failed. ErrCode=%d\r\n", fret);
		}
	}
	else if (Argc == 0)
	{
		debug_printf("Syntax error. need filename. \r\n");
	}
	
	return fret;
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
	FRESULT	fret;
	
	uint16_t nfiles, ndirs;
	
	fret = f_getcwd(g_Path, 64);			//获取当前路径
	if (fret == FR_OK)
	{
		debug_printf("Get Current path OK! \r\n");
	}
	else
	{
		debug_printf("Get current path Fail! ErrCode=%d \r\n", fret);
		return fret;
	}
	fret = f_opendir(&g_objDir, g_Path);		//打开目录
	debug_printf("Current Path: %s \r\n", g_Path);
	if (fret == FR_OK)
	{
		nfiles = 0;
		ndirs = 0;
		
		/* 遍历目录 */
		while (1)
		{
			fret = f_readdir(&g_objDir, &g_objFileInfo);						//读取目录
			/* 判断是否已到目录结尾 */
			if ((fret != FR_OK) || (g_objFileInfo.fname[0] == 0x00))
				break;
			
			/* 判断当前是目录或文件 */
			if (g_objFileInfo.fattrib & AM_DIR)
			{
				debug_printf("      <DIR>     %s \n", g_objFileInfo.fname);
				ndirs++;
			}
			else
			{
				debug_printf("%10lu %s \n", g_objFileInfo.fsize, g_objFileInfo.fname);
				nfiles++;
			}
		}
		f_closedir(&g_objDir);
		debug_printf("%d dirs, %d files. \n", ndirs, nfiles);
	}
	else
	{
		debug_printf("Failed to open \r\n");
	}
	
	return fret;
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
	FRESULT	fret;
	UINT	slen=0;
	UINT	wlen=0;
	
	/* 没有参数 返回错误 */
	if (Argc == 0)
	{
		debug_printf("Syntax error. Need filename. \r\n");
		return DBGRET_ERR_ARG_LESS;
	}
	fret = f_getcwd(g_Path, 64);	//获取当前路径
	if (fret == FR_OK)
	{
		MergePathAndFilename(g_Path, Argv[1], g_FullName);
		fret = f_open(&g_objFile, g_FullName, FA_CREATE_ALWAYS | FA_WRITE);
		if (Argc == 2)
		{
			slen = strlen(Argv[2]);
			fret = f_write(&g_objFile, Argv[2], slen, &wlen);
			if (fret == FR_OK)
			{
				debug_printf("Write %d bytes.\r\n", wlen);
				f_close(&g_objFile);
			}
		}
		else
		{
			debug_printf("Write 0 bytes.\r\n");
			f_close(&g_objFile);
		}
	}
	
	return fret;
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
	FRESULT	fret;
	UINT	slen=0;
	UINT	rlen=0;
	FSIZE_t	filesize=0;
	DWORD	stbyte, edbyte, selbyte;
	uint8_t	mode;
	
	uint16_t i;
	uint8_t	*rbuf;
	
	/* 没有参数 返回错误 */
	if (Argc == 0)
	{
		debug_printf("Syntax error. Need filename. \r\n");
		return DBGRET_ERR_ARG_LESS;
	}
	else if (Argc < 2)
	{
		debug_printf("Argument less.\r\n");
		return DBGRET_ERR_ARG_LESS;
	}
	
	/* 获取文件名 */
	fret = f_getcwd(g_Path, 64);	//获取当前路径
	if (fret == FR_OK)
	{
		MergePathAndFilename(g_Path, Argv[1], g_FullName);
		fret = f_open(&g_objFile, g_FullName, FA_READ);
		if (fret == FR_NO_FILE)
		{
			debug_printf("File not exist.\r\n");
			return DBGRET_ERR_ARG_ILLEGAL;
		}
		if (fret == FR_OK)
		{
			filesize = f_size(&g_objFile);
		}
	}
	
	/* 显示参数 */
	if (Argc == 2)
	{
		if (strcasecmp(Argv[2], "-hex") == 0)
		{
			mode = 1;
		}
		else if (strcasecmp(Argv[2], "-asc") == 0)
		{
			mode = 0;
		}
		else
		{
			debug_printf("Argument illegal.\r\n");
			return DBGRET_ERR_ARG_ILLEGAL;
		}
	}
	
	/* 获取起始和结束字节位置 */
	/* 没有位置参数 全部文件 */
	if (Argc == 2)
	{
		stbyte = 0;
		edbyte = filesize;
		selbyte = filesize;
	}
	/* 有起始位置 */
	else if (Argc == 3)
	{
		stbyte = atol(Argv[3]);
		if (stbyte >= filesize)
		{
			debug_printf("Start byte out of range.\r\n");
			return DBGRET_ERR_ARG_OUTRANGE;
		}
		selbyte = edbyte - stbyte;
	}
	/* 有起始和终止位置 */
	else if (Argc == 4)
	{
		stbyte = atol(Argv[3]);
		edbyte = atol(Argv[4]);
		if ((stbyte > filesize) || (stbyte > edbyte))
		{
			debug_printf("Start byte out of range.\r\n");
			return DBGRET_ERR_ARG_OUTRANGE;
		}
		selbyte = edbyte - stbyte;
	}
	
	/* 读文件 */
	rbuf = pvPortMalloc(1024);
	slen = selbyte;
	fret = f_lseek(&g_objFile, stbyte);						//移动读写指针
	do
	{
		/* 分段读取 */
		if (slen > 1024)
			slen = 1024;
		memset(rbuf, 0x00, 1024);
		fret = f_read(&g_objFile, rbuf, slen, &rlen);
		DisplayData(rbuf, mode, rlen);
		slen -= 1024;
	} while (slen > 0);
	fret = f_close(&g_objFile);
	vPortFree(rbuf);
	
	return fret;
}

