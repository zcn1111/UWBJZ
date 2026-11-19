#include "main.h"
#include "BSP.h"
#include "string.h"
#include "Cmd_Internal.h"
#include "app_cmd.h"
#include "BSP_debug.h"
/******** 串口句柄变量 ********/

/******** 接收缓存变量 ********/
//uint8_t g_DebugDatBuf[128];

//DbgCmdGroups_t	g_CmdTable;		//调试命令表

//uint8_t CommMode;				//通信模式 0=正常;1=蓝牙通信模式-透传；2=蓝牙配置状态模式
//uint8_t RtkCommMode;			
//uint16_t BleSize;
//uint8_t EchoSta=0;
//uint8_t WorkMode=0;

//DbgEcho_t g_DbgEcho;

//char g_DebugInfoBuf[3076];
/********* 调试串口重定向 *********/
int fputc(int ch, FILE *f)
{
	HAL_UART_Transmit(&hUSART3, (uint8_t *)&ch, 1, 100);
//	HAL_UART_Transmit(&hUART5, (uint8_t *)&ch, 1, 100);
	return ch;
}

int fgetc(FILE *f)
{
	uint8_t ch = 0;
	HAL_UART_Receive(&hUSART3, &ch, 1, 100);
//	HAL_UART_Receive(&hUART5, &ch, 1, 100);
	return ch;
}

#if 0
/*
*********************************************************************************************************
*	函 数 名: sendchar
*	功能说明: 用于文件Retarget.c文件的重定向
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int sendchar(int ch)
{
	/* 写一个字节到USART1 */
	HAL_UART_Transmit(&hUSART3, (uint8_t *)&ch, 1, 100);
	return ch;
}

/*
*********************************************************************************************************
*	函 数 名: getkey
*	功能说明: 用于文件Retarget.c文件的重定向
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
int getkey (void)
{
	uint8_t ch = 0;
	HAL_UART_Receive(&hUSART3, &ch, 1, 100);
	return ch;
}
#endif

/******************************************************************
函数名称: void debug_info
功能描述: 打印调试信息
参数输入: uint8_t	*Info	数据
		  uint16_t	Len		数据长度
		  char		*Lable	打印标签
参数输出: 无
返 回 值: 无
其    他:
******************************************************************/
void debug_info(uint8_t *Info, uint16_t Len, char *Lable)
{
	char *str;
	char deflable[10]={0};
	
	/* 接收数据显示 */
	strncpy(deflable, "Info", 4);
	if ((Len <= 1024) && (Len != 0))
	{
		memset(g_DebugInfoBuf, 0x00, 3076);
		Arr2AscStr(Info, Len, ' ', g_DebugInfoBuf);		//数组转换成可显示数据
		if (Lable == NULL)
			Lable = deflable;
		debug_printf("%s(%d): \r\n", Lable, Len);
		debug_printf("%s\r\n", g_DebugInfoBuf);
	}
	else
	{
		if (Len > 1024)
		{
			debug_printf("%s(%d): \r\n", Lable, Len);
			debug_printf("Debug Info: Len = %d Out of range!\r\n", Len);
		}
		else if (Len == 0)
		{
			debug_printf("%s(%d): \r\n", Lable, Len);
			debug_printf("Debug Info: No data!\r\n");
		}
	}
}

/******************************************************************
函数名称: void debug_ascinfo
功能描述: 打印ASCII调试信息
参数输入: uint8_t	*Info	数据
		  uint16_t	Len		数据长度
		  char		*Lable	打印标签
参数输出: 无
返 回 值: 无
其    他:
******************************************************************/
void debug_ascinfo(uint8_t *Info, uint16_t Len, char *Lable, char UnReadChar)
{
	char deflable[10]={0};
	uint16_t i;
	
	/* 接收数据显示 */
	strncpy(deflable, "Info", 4);
	if (UnReadChar == 0x00)
	{
		UnReadChar = '.';
	}
	if ((Len <= 1024) && (Len != 0))
	{
		memset(g_DebugInfoBuf, 0x00, 3076);
		for (i=0; i<Len; i++)
		{
			if ((Info[i] < 0x20) || (Info[i] > 0x7F))
			{
				g_DebugInfoBuf[i] = UnReadChar;
			}
			else
			{
				g_DebugInfoBuf[i] = Info[i];
			}
		}
		if (Lable == NULL)
			Lable = deflable;
		debug_printf("%s(%d): \r\n", Lable, Len);
		debug_printf("%s\r\n", g_DebugInfoBuf);
	}
	else
	{
		debug_printf("%s(%d): \r\n", Lable, Len);
		if (Len > 1024)
		{
			debug_printf("Debug Info: Len = %d Out of range!\r\n", Len);
		}
		else if (Len == 0)
		{
			debug_printf("Debug Info: No data!\r\n");
		}
	}
}


///******************************************************************
//函数名称: void InitDebugCmdPara
//功能描述: 初始化调试命令参数
//参数输入: DbgCmdPara_t	*Para	调试命令参数
//参数输出: 无
//返 回 值: 无
//其    他:
//******************************************************************/
//void InitDebugCmdPara(DbgCmdPara_t *Para)
//{
//	memset(Para->CmdName, 0x00, 20);	//命令字初始化
//	Para->cbCmdFunc = NULL;				//回调函数初始化
//	Para->cbCmdHelpFunc = NULL;			//帮助函数初始化
//}

///******************************************************************
//函数名称: void DebugRecvProcess(void)
//功能描述: 调试数据接收函数
//参数输入: 无
//参数输出: uint8_t *OutBuf
//返 回 值: 
//其    他:
//******************************************************************/
//int16_t DebugRecvProcess(void)
//{
//	uint16_t i, j;
//	uint16_t size;
//	uint16_t finflag;
//	char ch;
//	
//	size = GetRingUsedLen(&g_Uart2RxRing);
//	if (size == 0)
//		return DBGRET_ERR_NODATA;
//	finflag = 0;
//	j = 0;
//	memset(g_DebugDatBuf, 0x00, sizeof(g_DebugDatBuf));
//	for (i=0; i<size; i++)
//	{
//		RingBufPop(&g_Uart2RxRing, (uint8_t*)&ch, 1);
//		switch (finflag)
//		{
//			case 0:
//				if (ch == 0x0D)
//				{
//					finflag = 1;
//				}
//				else
//				{
//					g_DebugDatBuf[j] = ch;
//					finflag = 0;
//					j++;
//				}
//				break;
//			case 1:
//				if (ch == 0x0A)
//				{
//					finflag = 2;
//					if (i == (size - 1))
//					{
////						if (CommMode == 2)
////						{
////							g_DebugDatBuf[j] = 0x0D;
////							g_DebugDatBuf[j+1] = 0x0A;
////							BleSize = j+2;
////						}
//					}
//				}
//				else
//				{
//					g_DebugDatBuf[j] = ch;
//					finflag = 0;
//					j++;
//				}
//				break;
//			case 2:
//				finflag = 3;
//				break;
//			default:
//				break;
//		}
//		if (finflag == 3)
//		{
////			if (CommMode == 1)
////			{
////				g_DebugDatBuf[j+1] = 0x0D;
////				g_DebugDatBuf[j+2] = 0x0A;
////				BleSize = j+2;
////			}

//			break;
//		}
//	}
//	
//	if (CommMode == 1)
//	{
//		BleSize = j;
//	}
//	else if ((CommMode == 2) || (RtkCommMode == 1))
//	{
//		g_DebugDatBuf[j] = 0x0D;
//		g_DebugDatBuf[j + 1] = 0x0A;
//		BleSize = j + 2;
//	}
//	return DBGRET_OK;
//}

/******************************************************************
函数名称: void DbgErrProcess
功能描述: 错误处理
参数输入: int16_t	ErrCode		错误代码
参数输出: 
返 回 值: 
其    他:
******************************************************************/
void DbgErrProcess(int16_t ErrCode)
{
	switch (ErrCode)
	{
		/* 返回成功 */
		case DBGRET_OK:
			break;
		/* 非法参数 */
		case DBGRET_ERR_ARG_ILLEGAL:
			debug_printf("Argument illegal.\r\n");
			break;
		/* 缺少参数 */
		case DBGRET_ERR_ARG_LESS:
			debug_printf("Argument less.\r\n");
			break;
		/* 参数越界 */
		case DBGRET_ERR_ARG_OUTRANGE:
			debug_printf("Argument out of range.\r\n");
			break;
		default:
			break;
	}
}




/******************************************************************
函数名称: void RegisterDbgTerminalCmd
功能描述: 注册调试终端命令函数
参数输入: char			*CmdName	命令字
		  uint8_t		ParaNbr		参数个数
		  int16_t		(*cbCmdFunc)回调函数
		  void			(*cbHelp)	帮助回调函数
参数输出: DbgCmdGroups_t *CmdTable	命令表
返 回 值: int16_t	0=正常;<0=错误
其    他: 命令字不区分大小写 参数区分大小写
******************************************************************/
int16_t RegisterDbgTerminalCmd(char *CmdName, int16_t (*cbCmdFunc)(uint8_t Argc, char *Argv[]), void (*cbHelp)(void), DbgCmdGroups_t *CmdTable)
{
//	uint8_t i;
//	uint8_t tablenum;
//	uint8_t slen;
//	DbgCmdPara_t cmdpara;
//	
//	/* 检查当前命令条数 */
//	tablenum = CmdTable->CmdTotalNbr;
//	if ((tablenum + 1) > DBG_CMDTABLE_MAX)
//	{
//		return DBGRET_ERR_CMD_OVERMAX;
//	}
//	/* 检查命令字合法 命令字中不能包含空格 */
//	if (CmdName == NULL)
//		return DBGRET_ERR_CMD_NONE;
//	slen = strlen(CmdName);
//	if (CheckStringContaniedChar((uint8_t*)CmdName, slen, ' ') == 1)
//		return DBGRET_ERR_CMD_ILLEGAL;

//	InitDebugCmdPara(&cmdpara);
//	
//	/* 命令字 不区分大小写 最多20个字节 超过将被截断 */
//	slen = (strlen(CmdName) > 20) ? 20 : strlen(CmdName);
//	strncpy(cmdpara.CmdName, CmdName, slen);
//	
//	/* 回调函数 */
//	/* 命令回调函数原型为 int16_t (uint8-t, char*) */
//	/* 参数 uint8_t 为传入参数个数 */
//	/* 参数 char* 为传入参数列表 */
//	cmdpara.cbCmdFunc = cbCmdFunc;
//	cmdpara.cbCmdHelpFunc = cbHelp;	
//	
//	/* 注册命令表 */
//	CmdTable->CmdPara[tablenum] = cmdpara;
//	CmdTable->CmdTotalNbr = tablenum + 1;
//	
//	return DBGRET_OK;
}

/******************************************************************
函数名称: void UnRegisterDbgTerminalCmd
功能描述: 反注册调试终端命令函数
参数输入: char			*CmdName	查找命令字
参数输出: DbgCmdGroups_t *CmdTable	命令表
返 回 值: 无
其    他:
******************************************************************/
void UnRegisterDbgTerminalCmd(char *CmdName, DbgCmdGroups_t *CmdTable)
{
//	uint8_t i=0, j=0;
//	uint8_t t_nbr;
//	
//	t_nbr = CmdTable->CmdTotalNbr;
//	
//	/* 查找命令字为NULL 反注册所有命令 */
//	if (CmdName == NULL)
//	{
//		for (i=0; i<DBG_CMDTABLE_MAX; i++)
//		{
//			InitDebugCmdPara(&(CmdTable->CmdPara[i]));
//		}
//		CmdTable->CmdTotalNbr = 0;
//		return;
//	}
//	
//	/* 查找指定命令字 */
//	while ((t_nbr > 0) && (i < t_nbr))
//	{
//		/* 检查命令字为空 */
//		if (CheckStringIsSame((uint8_t*)(CmdTable->CmdPara[i].CmdName), 20, 0x00) == 1)
//		{
//			continue;	//跳过当前检查下一个
//		}
//		
//		/* 检查是否与命令字匹配 */
//		if (strcasecmp(CmdName, CmdTable->CmdPara[i].CmdName) == 0)
//		{
//			/* 命令字匹配 后面命令的前移 */
//			for (j=i; j<(t_nbr - 1); j++)
//			{
//				CmdTable->CmdPara[j] = CmdTable->CmdPara[j + 1];
//			}
//			InitDebugCmdPara(&(CmdTable->CmdPara[t_nbr-1]));
//			CmdTable->CmdTotalNbr = t_nbr - 1;
//			break;
//		}
//		i++;
//	}
}

/******************************************************************
函数名称: void ListRegisteredCmd
功能描述: 列出已注册的命令
参数输入: DbgCmdGroups_t		CmdTable	命令表
参数输出: 
返 回 值: 无
其    他:
******************************************************************/
//void ListRegisteredCmd(DbgCmdGroups_t CmdTable)
//{
//	uint8_t i, j;
//	uint8_t p_nbr;
//	uint8_t t_nbr;
//	
//	t_nbr = CmdTable.CmdTotalNbr;
//	debug_printf("Registered Cmd: \r\n");
//	for (i=0; i<t_nbr; i++)
//	{
//		debug_printf("%s  \r\n", CmdTable.CmdPara[i].CmdName);		//命令字
//	}
//}
/******************************************************************
函数名称: void InitTerminalCmdGroup
功能描述: 初始化调试终端命令
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void InitTerminalCmdGroup(void)
{	
	/* 清除命令表 */
//	UnRegisterDbgTerminalCmd(NULL, &g_CmdTable);
//	
//	/* 注册命令与回调函数 */		
//	RegisterDbgTerminalCmd("info", (int16_t(*)(uint8_t, char*[]))Cmd_GetCompilerInfo, NULL, &g_CmdTable);
//	RegisterDbgTerminalCmd("datetime", Cmd_SetDateTime, NULL, &g_CmdTable);
//	RegisterDbgTerminalCmd("reset", Cmd_SoftReset, NULL, &g_CmdTable);
//	RegisterDbgTerminalCmd("spitest", Cmd_SpiTest, NULL, &g_CmdTable);
//	
////	RegisterDbgTerminalCmd("blectrl", Cmd_BLEControl, NULL, &g_CmdTable);
////	RegisterDbgTerminalCmd("blesend", Cmd_BLESend, NULL, &g_CmdTable);
//	
//	RegisterDbgTerminalCmd("setip", Cmd_SetIP, NULL, &g_CmdTable);
//	RegisterDbgTerminalCmd("echo", Cmd_Echo, NULL, &g_CmdTable);
//	
//	RegisterDbgTerminalCmd("flashread", Cmd_ReadPageSpiFlash, NULL, &g_CmdTable);
//	RegisterDbgTerminalCmd("flashwrite", Cmd_WritePageSpiFlash, NULL, &g_CmdTable);
//	
//	RegisterDbgTerminalCmd("setconstrtime", Cmd_SetConstructionTime, NULL, &g_CmdTable);
//	RegisterDbgTerminalCmd("setworkmode", Cmd_SetMode, NULL, &g_CmdTable);
}



