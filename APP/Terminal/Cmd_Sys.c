/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_Sys.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 系统命令


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
#include "Update.h"

/* RTOS头文件 */
#include "FreeRTOS.h"
#include "task.h"

#include "Cmd_Sys.h"


/******************************************************************
函数名称: int16_t Cmd_GetFreeHeap
功能描述: 获取剩余堆栈空间
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_GetFreeHeap(uint8_t Argc, char *Argv[])
{
	uint32_t usize=0;
	uint32_t usize_min=0x00;
	
	char *strfrm;
	
	strfrm = pvPortMalloc(8);
	memset(strfrm, 0x00, 8);
	usize = xPortGetFreeHeapSize();					//当前空余堆大小
	usize_min = xPortGetMinimumEverFreeHeapSize();	//最小空余堆大小
	strncpy(strfrm, "B", 1);
	if (Argc == 1)
	{
		if (strcasecmp(Argv[1], "-h") == 0)
		{
			/* usize >= 1MB */
			if (usize >= 1048576)
			{
				usize = usize / 1048576;
				usize_min = usize_min / 1048576;
				strncpy(strfrm, "MiB", 3);
			}
			/* usize >= 1KB */
			else if (usize >= 1024)
			{
				usize = usize / 1024;
				usize_min = usize_min / 1024;
				strncpy(strfrm, "KiB", 3);
			}
		}
	}
	else
	{
		debug_printf("        Current FreeHeap=%d %s \r\n", usize, strfrm);
		debug_printf("History Minimum FreeHeap=%d %s \r\n", usize_min, strfrm);
	}
	vPortFree(strfrm);
	
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_GetTaskStateList
功能描述: 获取任务状态列表
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_GetTaskStateList(uint8_t Argc, char *Argv[])
{
	char *wbuf;
	
	wbuf = pvPortMalloc(1024);		//申请内存
	vTaskList(wbuf);				//获取任务列表信息
	debug_printf("TaskName\t\t\tSta\tPri\tMem\tOrder\r\n");
	debug_printf("-----------------------------------------------\r\n");
	debug_printf("%s", wbuf);
	vPortFree(wbuf);				//释放内存
	
	return DBGRET_OK;
}


