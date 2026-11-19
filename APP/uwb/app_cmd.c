
#include "BSP.h"
#include "string.h"
#include "instance.h"
//#include "app_glovar.h"
#include "uwb_gps_transform.h"
#include "g_GlobalValDef.h"
#include "bsp_dw1000.h"
/******** 串口句柄变量 ********/

/******** 接收缓存变量 ********/
uint8_t g_DebugTmpBuf[256];
uint8_t g_DebugDatBuf[128];




/******* 静态命令表（编译期自动生成） *******/
static const DbgCmdItem_t s_CmdList[] = {
    #define X(name, func) {name, func},
    CMD_TABLE
    #undef X
};

/* 定义命令表 */
DbgCmdTable_t g_CmdTable = {
    .CmdPara = s_CmdList,
};

uint8_t CommMode;				//通信模式 0=正常;1=蓝牙透传
uint8_t RtkCommMode;			
uint16_t BleSize;
//uint8_t EchoSta=0;
uint8_t WorkMode=0;

DbgEcho_t g_DbgEcho;

char g_DebugInfoBuf[3076];
///********* 调试串口重定向 *********/
//int fputc(int ch, FILE *f)
//{
//	HAL_UART_Transmit(&hUSART2, (uint8_t *)&ch, 1, 100);
////	HAL_UART_Transmit(&hUART5, (uint8_t *)&ch, 1, 100);
//	return ch;
//}

//int fgetc(FILE *f)
//{
//	uint8_t ch = 0;
//	HAL_UART_Receive(&hUSART2, &ch, 1, 100);
////	HAL_UART_Receive(&hUART5, &ch, 1, 100);
//	return ch;
//}

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
	HAL_UART_Transmit(&hUSART2, (uint8_t *)&ch, 1, 100);
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
	HAL_UART_Receive(&hUSART2, &ch, 1, 100);
	return ch;
}
#endif

/******************************************************************
函数名称: static void ListRegisteredCmd
功能描述: 列出已注册的命令
参数输入: DbgCmdTable_t		CmdTable	命令表
参数输出: 
返 回 值: 无
其    他:
******************************************************************/
void ListRegisteredCmd(void)
{
	uint8_t i;
	
	debug_printf("Registered Cmd: \r\n");
	for (i=0; i<CMD_COUNT; i++)
	{
		debug_printf("%s  \r\n", g_CmdTable.CmdPara[i].CmdName);		//命令字
	}
}

/******************************************************************
函数名称: DbgResult_t ExecCmd
功能描述: 命令数据解析
参数输入: DbgCmdGroups_t		CmdTable	命令表
		  uint8_t			*InBuf		输入缓存
参数输出: 
返 回 值: 
其    他:
******************************************************************/
static DbgResult_t ExecCmd(DbgCmdTable_t *CmdTable, uint8_t *InBuf)
{
	uint16_t i, j;
	uint16_t slen;
	int16_t dbgret=DBGRET_OK;
	
	char	cmdname[20]={0};
	char	*argv[10]={NULL};	//初始化
	char	*token;
	uint8_t argc=0;
	char	*pstrchr=NULL;
	char 	*ptr=NULL;
	
		
	/* 处理接收字符 分离命令字和参数 */
	slen = strlen((char*)InBuf);
	if (slen == 0)
		return DBGRET_ERR_CMD_NONE;
	
	pstrchr = (char*)InBuf;
	i = 0;
	
	/* 按空格拆分字符串 最多10个token */
	ptr = NULL;
	token = strtok_r(pstrchr, " ", &ptr);
	while ((token != NULL) && (i < 10))
	{
		argv[i] = token;
		i++;
		token = strtok_r(NULL, " ", &ptr);
	}
	argc = i;
	if (argc == 0)
		return DBGRET_ERR_CMD_NONE;
	
	/* 提取命令字 保证\0终止 */
	slen = strlen(argv[0]);
	if (slen >= 19) slen = 19;
	strncpy(cmdname, argv[0], slen);
	cmdname[slen] = '\0';
	
	/* 查询命令表 执行命令 */
	dbgret = DBGRET_ERR_NOMATCH;
	for (i=0; i<CMD_COUNT; i++)
	{
		if (strcasecmp(CmdTable->CmdPara[i].CmdName, cmdname) == 0)		//找到对应命令
		{
			dbgret = CmdTable->CmdPara[i].cbCmdFunc(argc-1, argv);
			break;
		}
	}
	return dbgret;
}

/******************************************************************
函数名称: void DbgErrProcess
功能描述: 错误处理
参数输入: DbgResult_t	ErrCode		错误代码
参数输出: 
返 回 值: 
其    他:
******************************************************************/
static void DbgErrProcess(DbgResult_t ErrCode)
{
	switch (ErrCode)
	{
		/* 返回成功 */
		case DBGRET_OK:
			debug_printf("OK!\r\n");
			break;
		/* 命令错误 */
		case DBGRET_ERR_NOMATCH:
			debug_printf("Command NOT found.\r\n");
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
			debug_printf("Other Error.\r\n");
			break;
	}
}

/********************************* 外部函数 *******************************/
/******************************************************************
函数名称: void DebugRecvProcess(void)
功能描述: 命令接收处理
参数输入: char		*InStr		输入字符串
		  char		StChr		起始字符 (0x00=不判断起始符)
		  uint8_t	ForceEnding	强制判定结束符
参数输出: 
返 回 值: 
其    他:
******************************************************************/
DbgResult_t CmdRecvProcess(char *InStr, char StChr, uint8_t ForceEnding)
{
	uint16_t i, j;
	uint16_t size;
	uint16_t finflag=0;	//0=无;1=起始;2=结束
	char ch;
	
	DbgResult_t dbgret;
	
	/* 字符串校验 */
	size = strlen(InStr);
	if (size == 0)
		return DBGRET_ERR_NODATA;
	
	/* 字符串处理 */
	finflag = 0;
	j = 0;
//	memset(g_DebugDatBuf, 0x00, sizeof(g_DebugDatBuf));
//	if (StChr == 0x00)
//	{
//		finflag = 1;
//	}
//	for (i=0; i<size; i++)
//	{
//		/* 未开始 判断起始符 */
//		if (finflag == 0)
//		{
//			if (StChr != 0x00)
//			{
//				if (InStr[i] == StChr)
//					finflag = 1;
//				else
//					finflag = 0;
//			}
//		}
//		/* 处理中 判断结束符 */
//		else if (finflag == 1)
//		{
//			/* 数据未结束 判断结束标记 */
//			if (i <= (size - 2))	//
//			{
//				if ((InStr[i] == 0x0D) && (InStr[i+1] == 0x0A))
//				{
//					finflag = 2;
//					break;
//				}
//			}
//			/* 字符串已结束 */
//			else if (i == (size - 1))
//			{
//				if (ForceEnding == 0)	//不强制要求结束符
//				{
//					finflag = 2;
//				}
//			}
//			g_DebugDatBuf[j] = InStr[i];
//			j++;
//		}
//	}
//		
//	/* 命令处理 */
//	/* 命令结束处理命令表 */
//	dbgret = DBGRET_ERR_NOMATCH;
//	if (finflag == 2)
	{
		/* 内核命令 list 列出所有命令 */
		if (strcasecmp((char*)&InStr, "list") == 0)	//list 列出所有命令
		{
			dbgret = DBGRET_OK;
			ListRegisteredCmd();
		}
		/* 内核命令 reset 系统复位 */
		else if (strcasecmp((char*)&InStr, "reset") == 0)
		{
			dbgret = DBGRET_OK;
			HAL_NVIC_SystemReset();	//重启 
		}
		else
		{
			dbgret = ExecCmd(&g_CmdTable, InStr);
		}
	}
	DbgErrProcess(dbgret);
	
	return DBGRET_OK;
}





/********************************* 命令函数 *******************************/
/* 获取编译信息 */
DbgResult_t Cmd_GetCompilerInfo(uint8_t Argc, char *Argv[])
{
	debug_printf("Progrma File : %s \r\n", __FILE__);
	debug_printf("Compiled in %s %s", __DATE__, __TIME__);
	
	return DBGRET_OK;
}

/* 设置UWB基站信息 */
DbgResult_t Cmd_SetUwbAnchorsInfo(uint8_t Argc, char *Argv[])
{
	uint8_t anc_order=0;
	double e=0.0, n=0.0;
	uint8_t i;
	
	/* 检查参数个数 */
	if (instance_mode == ANCHOR)
	{
		debug_printf("Error: This cmd anchor useless.\r\n");
		return DBGRET_OK;
	}
	
	/* 无参数显示当前基站信息 */
	if (Argc == 0)
	{
		ShowAnchorCoordinates();
		return DBGRET_OK;
	}
	
	/* 单参数 */
	if (Argc == 1)
	{
		if (strcasecmp(Argv[1], "help") == 0)
		{
			debug_printf("Usage:\r\n");
			debug_printf("setcoord                 | Show coordinate info\r\n");
			debug_printf("setcoord ord lon lat     | Set coordinate\r\n");
			debug_printf("setcoord conv [enu/xyz]  | Convertion coord system\r\n");
		}
		return DBGRET_OK;
	}
	
	/* 2参数 */
	if (Argc == 2)
	{
		/* 坐标系转换 */
		if (strcasecmp(Argv[1], "conv") == 0)
		{
			/* GPS坐标系转换成 ENU坐标 (东-北-天) */
			if (strcasecmp(Argv[2], "enu") == 0)
			{
				/* 判断坐标可转换 */
				if ((anchor_cood[0].lat == 0.0) || (anchor_cood[0].lon == 0.0))
				{
					debug_printf("Error: anchor #0 NOT exist!\r\n");
					return DBGRET_ERR_ARG_ILLEGAL;
				}
				for (i=0; i<MAX_AHCHOR_NUMBER; i++)
				{
					if (i == 0)		//0号基站坐标 坐标原点
					{
						anchor_cood[i].x = 0.0;
						anchor_cood[i].y = 0.0;
					}
					else
					{
						gps_to_enu(anchor_cood[i].lat, anchor_cood[i].lon, anchor_cood[0].lat, anchor_cood[0].lon, &e, &n);
						anchor_cood[i].x = e;	//ENU坐标系东坐标 等效 横向x
						anchor_cood[i].y = n;	//ENU坐标系北坐标 等效 纵向y
					}
				}
				debug_printf("Convertion Complete!\r\n");
				return DBGRET_OK;
			}
		}
		return DBGRET_ERR_ARG_ILLEGAL;
	}
	
	else if (Argc < 3)
		return DBGRET_ERR_ARG_LESS;
	
	/* 处理参数 */
	anc_order = atol(Argv[1]);		//基站编号
	if (anc_order >= MAX_AHCHOR_NUMBER)
	{
		return DBGRET_ERR_ARG_OUTRANGE;
	}
	/* 保存基站原始经纬度 */
	anchor_cood[anc_order].lon = atof(Argv[2]);	//经度
	anchor_cood[anc_order].lat = atof(Argv[3]);	//纬度
	
	/* 坐标转换 */
	if (anc_order == 0)
	{
		anchor_cood[anc_order].x = 0.0;		//0号为原点
		anchor_cood[anc_order].y = 0.0;
	}
	else
	{
		if ((anchor_cood[0].lat == 0.0) || (anchor_cood[0].lon == 0.0))
		{
			debug_printf("Error: anchor #0 NOT exist!\r\n");
			return DBGRET_ERR_ARG_ILLEGAL;
		}
		gps_to_enu(anchor_cood[anc_order].lat, anchor_cood[anc_order].lon, anchor_cood[0].lat, anchor_cood[0].lon, &e, &n);
		anchor_cood[anc_order].x = e;	//ENU坐标系东坐标 等效 横向x
		anchor_cood[anc_order].y = n;	//ENU坐标系北坐标 等效 纵向y
	}
	
	return DBGRET_OK;
}

/* 保存UWB基站信息 */
DbgResult_t Cmd_SaveUwbAnchorsInfo(uint8_t Argc, char *Argv[])
{
	uint16_t wb;
	uint16_t len;
	
	len = sizeof(anchor_cood);
	wb = WriteDataToFlashWithCRC16((uint8_t*)&anchor_cood, len, FLASH_UWB_COORDINATES_START, FLASH_UWB_COORDINATES_LEN);	//写基站信息
	/* 写入成功 */
	if (wb == len)
	{
		debug_printf("Write coordinates successfully.\r\n");
	}
	else
	{
		debug_printf("Write failure.\r\n");
	}
	
	return DBGRET_OK;
}

/* 读取UWB基站信息 */
DbgResult_t Cmd_ReadUwbAnchorsInfo(uint8_t Argc, char *Argv[])
{
	uint16_t rb;
	uint8_t i;
	uint16_t len;
	
	len = sizeof(anchor_cood);
	rb = ReadDataFromFlashWithCRC16((uint8_t*)&anchor_cood, len, FLASH_UWB_COORDINATES_START, FLASH_UWB_COORDINATES_LEN);	//读基站信息
	/* 读取成功 */
	if (rb == len)
	{
		ShowAnchorCoordinates();
		return DBGRET_OK;
	}
	
	return DBGRET_ERR_ARG_ILLEGAL;
}



/* 配置设备参数 */
DbgResult_t Cmd_SetDevConfig(uint8_t Argc, char *Argv[])
{
	uint16_t cfg;
	uint16_t len;
	double dcfg=0;
	
	/* 检查参数个数 不带参数显示当前参数 */
	if (Argc == 0)
	{
		ShowDevConfig();	//显示参数
		return DBGRET_OK;
	}
	
	/* 处理单参数 */
	if (Argc == 1)
	{
		/* 保存当前参数 */
		if (strcasecmp(Argv[1], "save") == 0)
		{
			SaveDevConfig(g_DevConfig, sizeof(g_DevConfig));		//保存参数数据
			return DBGRET_OK;
		}
		else if (strcasecmp(Argv[1], "help") == 0)
		{
			debug_printf("Usage:\r\n");
			debug_printf("setconfig                   | Show device config\r\n");
			debug_printf("setconfig save              | Save config\r\n");
			debug_printf("setconfig mode [tag/anchor] | Set device as tag or anchor\r\n");
			debug_printf("setconfig id [num]          | Set device ID\r\n");
			debug_printf("setconfig antdly [num]      | Set device ant delay\r\n");
			debug_printf("setconfig anch [num]        | Set anchor height\r\n");
			debug_printf("setconfig tagh [num]        | Set tag height\r\n");
			debug_printf("setconfig help              | Show this\r\n");
			return DBGRET_OK;
		}
	}
	
	/* 配置参数 暂不包含DW配置 */
	if (Argc >= 2)
	{
		/* 设备模式 */
		if (strcasecmp(Argv[1], "mode") == 0)
		{
			if (strcasecmp(Argv[2], "tag") == 0)
			{
				g_DevConfig.DeviceMode = TAG;
				instance_mode = g_DevConfig.DeviceMode;
				return DBGRET_OK;
			}
			else if (strcasecmp(Argv[2], "anchor") == 0)
			{
				g_DevConfig.DeviceMode = ANCHOR;
				instance_mode = g_DevConfig.DeviceMode;
				return DBGRET_OK;
			}
			else
			{
				return DBGRET_ERR_ARG_ILLEGAL;
			}
		}
		/* 设备编号 */
		else if (strcasecmp(Argv[1], "id") == 0)
		{
			cfg = atol(Argv[2]);
			if ((cfg >= 0) && (cfg <= 3))
			{
				g_DevConfig.DeviceID = cfg;
				dev_id = g_DevConfig.DeviceID;
				return DBGRET_OK;
			}
			else
			{
				return DBGRET_ERR_ARG_ILLEGAL;
			}
		}
		/* 天线延迟 */
		else if (strcasecmp(Argv[1], "antdly") == 0)
		{
			cfg = atol(Argv[2]);
			g_DevConfig.AntDelay = cfg;
			dwt_setrxantennadelay(g_DevConfig.AntDelay);	//设置天线延时
			dwt_settxantennadelay(g_DevConfig.AntDelay);
			return DBGRET_OK;
		}
		/* 基站高度 */
		else if (strcasecmp(Argv[1], "anch") == 0)
		{
			dcfg = atof(Argv[2]);
			if ((dcfg > 0) && (dcfg >= g_DevConfig.TagHeight))
			{
				g_DevConfig.AnchorHeight = dcfg;
				g_anchor_h = g_DevConfig.AnchorHeight;
				return DBGRET_OK;
			}
			else 
				return DBGRET_ERR_ARG_ILLEGAL;
		}
		/* 标签高度 */
		else if (strcasecmp(Argv[1], "tagh") == 0)
		{
			dcfg = atof(Argv[2]);
			if (dcfg > 0)
			{
				g_DevConfig.TagHeight = dcfg;
				g_tag_h = g_DevConfig.TagHeight;
				return DBGRET_OK;
			}
			else
				return DBGRET_ERR_ARG_ILLEGAL;
		}
//		SaveDevConfig(g_DevConfig, sizeof(g_DevConfig));		//保存参数数据
//		debug_printf("Device config has been changed.\r\n");
//		return DBGRET_OK;
	}
	
	return DBGRET_ERR_ARG_ILLEGAL;
}

/* 设置解算模式 */
DbgResult_t Cmd_SetSolutionMode(uint8_t Argc, char *Argv[])
{
	uint8_t mode=0;
	
	/* 检查参数个数 不含参数显示当前解算模式 */
	if (Argc == 0)
	{
		if (g_DevConfig.SolutionMode == 0)
		{
			debug_printf("Mode = Distance\r\n");
		}
		else if (g_DevConfig.SolutionMode == 1)
		{
			debug_printf("Mode = 1D\r\n");
		}
		else if (g_DevConfig.SolutionMode == 2)
		{
			debug_printf("Mode = 2D\r\n");
		}
		return DBGRET_OK;
	}
	
	/* 配置参数 */
	if (Argc >= 1)
	{
		mode = atol(Argv[1]);
		if ((mode >= 0) && (mode <= 2))
		{
			g_DevConfig.SolutionMode = mode;
			debug_printf("Set solution mode OK\r\n");
			return DBGRET_OK;
		}
	}
	
	return DBGRET_ERR_ARG_ILLEGAL;
}

/* 设置统计模式 */
DbgResult_t Cmd_SetStatMode(uint8_t Argc, char *Argv[])
{
	uint8_t mode=0;
	
	/* 检查参数个数 不含参数显示当前解算模式 */
	if (Argc == 0)
	{
		debug_printf("Mode = %d\r\n", g_DevConfig.StatisticsMode);
	}
	
	/* 配置参数 */
	if (Argc >= 1)
	{
		mode = atol(Argv[1]);
		if ((mode >= 0) && (mode <= 1))
		{
			g_DevConfig.StatisticsMode = mode;
			debug_printf("Set statistic mode OK!\r\n");
			return DBGRET_OK;
		}
	}
	
	return DBGRET_ERR_ARG_ILLEGAL;
}

/* 设置统计模式 */
DbgResult_t Cmd_SetShowMode(uint8_t Argc, char *Argv[])
{
	uint8_t mode=0;
	
	/* 检查参数个数 */
	if (Argc >= 1)
	{
		mode = atol(Argv[1]);
		if (mode != 0)
		{
			g_DevConfig.ShowSolution = 1;
			debug_printf("Show Solution ON!\r\n");
			return DBGRET_OK;
		}
		else
		{
			g_DevConfig.ShowSolution = 0;
			debug_printf("Show Solution OFF!\r\n");
			return DBGRET_OK;
		}
	}
	
	return DBGRET_ERR_ARG_ILLEGAL;
}

/* 设置用户数据 */
DbgResult_t Cmd_SetUserdata(uint8_t Argc, char *Argv[])
{
	uint8_t i;
	uint8_t user=0;
	char tmpstr[40];
	uint8_t len=0;
	
	/* 无参数 */
	if (Argc == 0)
	{
		for (i=0; i<10; i++)
		{
			if (user_data[i] != 0x00)
			{
				user = 1;
			}
		}
		if (user == 0)
		{
			debug_printf("NO Userdata!\r\n");
		}
		else
		{
			memset(tmpstr, 0x00, 32);
			len = 0;
			sprintf(tmpstr, "User:");
			len = strlen(tmpstr);
			for (i=0; i<10; i++)
			{
				sprintf(tmpstr + len, "%02X ", user_data[i]);
				len = strlen(tmpstr);
			}
			debug_printf("%s\r\n", tmpstr);
		}
		return DBGRET_OK;
	}
}
