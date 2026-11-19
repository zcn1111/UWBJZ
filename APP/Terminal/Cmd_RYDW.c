/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:人员定位
文件名称:Cmd_RYDW.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 人员定位命令


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

#include "RYDW_Def.h"
#include "RYDW_StationCard.h"
#include "LORA_Comm.h"
#include "LORA_Base.h"
#include "Cmd_RYDW.h"
#include "app_cmd.h"

/******************************************************************
函数名称: int16_t Cmd_SetConstructionTime
功能描述: 设置施工时间
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_SetConstructionTime(uint8_t Argc, char *Argv[])
{
}



/******************************************************************
函数名称: int16_t Cmd_SetMode
功能描述: 设置模式
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_SetMode(uint8_t Argc, char *Argv[])
{
	uint8_t val;
	
	/* 无参数 显示当前模式 */
	if (Argc == 0)
	{
		if (WorkMode == 0)
		{
			debug_printf("Mode = Normal \r\n");
		}
		else if (WorkMode == 1)
		{
			debug_printf("Mode = LoraTest \r\n");
		}
		return DBGRET_OK;
	}
	
	/* 有参数 配置当前工作模式 */
	if (Argc == 1)
	{
		val = atol(Argv[1]);
		/* 设置正常模式 */
		if (val == 0)
		{
			WorkMode = 0;
			debug_printf("Set NORMAL Mode!\r\n")
		}
		/* 设置Lora测试模式 */
		else if (val == 1)
		{
			WorkMode = 1;
			debug_printf("Set LORATEST Mode!\r\n");
		}
		/* 参数错误 */
		else
		{
			return DBGRET_ERR_ARG_OUTRANGE;
		}
		return DBGRET_OK;
	}
	
	return DBGRET_ERR_ARG_LESS;
}


