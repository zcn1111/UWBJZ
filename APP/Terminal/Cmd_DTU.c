/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_DTU.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 DTU命令


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
#include "DTU_Comm.h"

#include "Cmd_DTU.h"


/******************************************************************
函数名称: int16_t Cmd_SetIP
功能描述: 设置DTUIP
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_SetIP(uint8_t Argc, char *Argv[])
{
	IPAddr_t newip;
	uint16_t i;
	uint16_t slen;
	uint8_t dot, spare;
	
	if (Argc == 1)
	{
		slen = strlen(Argv[1]);
		dot = 0;
		spare = 0;
		for (i=0; i<slen; i++)
		{
			if (Argv[1][i] == '.')
				dot++;
			if (Argv[1][i] == ':')
				spare++;
		}
		if ((dot == 3) && (spare == 1))
		{
			ConvIPStrToArray(Argv[1], newip.all);
			DTU_ChangeIP(newip, 3);					//修改数据IP
			debug_printf("ChangeIP ->%s", Argv[1]);
			return DBGRET_OK;
		}
		else
		{
			debug_printf("Argument Error.\r\n");
			return DBGRET_ERR_ARG_ILLEGAL;			//参数错误
		}
		
	}
	
	debug_printf("Argument Less.\r\n");
	return DBGRET_ERR_ARG_LESS;
}


