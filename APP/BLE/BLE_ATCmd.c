/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: BLE_ATCmd.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: AT命令处理
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "DeviceDrvConf.h"

#include "BLE_Boardcast.h"
#include "BLE_Comm.h"
#include "BLE_ATCmd.h"

//#include "Drv_W25Qxx.h"

#include "string.h"


/********* 变量定义 **********/
ATCmdGroups_t	g_ATCmdTable;

char *g_ATRespones[ATRESP_MAXNUM];
char *g_ATCmdArgv[ATPARA_MAXNUM];

//TickTimerPara_t tmrBLEBoardcastTimer;	//蓝牙广播定时器

/********* 蓝牙AT指令 **********/
//const char *BleCmd_SetBaud		="AT+BAUD=115200\r\n";			//配置波特率115200
//const char *BleCmd_SetPswd		="AT+PASSWORD=12589490\r\n";	//配置密码
//const char *BleCmd_SetRole		="AT+ROLE=0\r\n";				//配置角色=从模式
//const char *BleCmd_SetName		="AT+NAME=CZ4_SXSH\r\n";		//配置蓝牙名称
//const char *BleCmd_SetMaxconn	="AT+MAXCONN=7\r\n";			//配置最大链接数=7

//const char *test="AT+BAUD?\r\n";
/************************************** AT指令注册 ************************************************/

/******************************************************************
函数名称: ATResult AT_InitATMem
功能描述: 初始化AT内存
参数输入: char		*ptrMem[]		指针数组
		  uint8_t	MaxNum			最大数组个数
		  uint16_t	MemSize			单个数组申请内存大小
参数输出: 无
返 回 值: ATResult	AT返回值
其    他: 
******************************************************************/
ATResult AT_InitATMem(char *ptrMem[], uint8_t MaxNum, uint16_t MemSize)
{
	uint8_t i;
	
	for (i=0; i<MaxNum; i++)
	{
		ptrMem[i] = pvPortMalloc(MemSize);	//申请内存
		if (ptrMem[i] == NULL)
		{
			/* 申请内存失败 */
			return ATRET_MEM_FAIL;
		}
	}
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_UnInitATMem
功能描述: 反初始化AT内存
参数输入: char		*ptrMem[]		指针数组
		  uint8_t	MaxNum			最大数组个数
参数输出: 无
返 回 值: ATResult	AT返回值
其    他: 
******************************************************************/
ATResult AT_UnInitATMem(char *ptrMem[], uint8_t MaxNum)
{
	uint8_t i;
	
	for (i=0; i<MaxNum; i++)
	{
		vPortFree(ptrMem[i]);			//释放内存
	}
	
	return ATRET_OK;
}
/******************************************************************
函数名称: ATResult AT_InitATCmdPara
功能描述: 初始化AT指令参数
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_InitATCmdPara(ATCmd_t *Para)
{
	uint8_t i;
	
	memset(Para->CmdKeyword, 0x00, AT_KEYLEN_MAXNUM);
	Para->CmdType = ATCMD_TYP_NONE;
	Para->Argc = 0;
	for (i=0; i<ATPARA_MAXNUM; i++)
	{
		Para->Argv[i] = NULL;
	}
	Para->cbATFunc = NULL;
	
	return ATRET_OK;
}


/******************************************************************
函数名称: ATResult AT_SearchATCmd
功能描述: 搜索AT指令
参数输入: BLEDataPara_t	*BleData	蓝牙数据参数
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_SearchATCmd(char *KeyWord, ATCmdGroups_t *ATCmdGrp, uint8_t *CmdPos)
{
	uint8_t i;
	uint8_t pos=0;
	
	ATResult ret;
	
	ret = ATRET_FUNC_NOTMATCH;
	/* 查找关键字 */
	for (i=0; i<ATCMD_MAXNUM; i++)
	{
		if (strcasecmp(KeyWord, ATCmdGrp->ATCmdTable[i].CmdKeyword) == 0)
		{
			ret = ATRET_OK;
			pos = i;
			break;
		}
	}
	
	*CmdPos = pos;
	return ret;
}

/******************************************************************
函数名称: ATResult AT_SetupATCmdKeyword
功能描述: 配置AT指令关键字
参数输入: BLEDataPara_t	*BleData	蓝牙数据参数
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_SetupATCmd(char *KeyWord, uint8_t CmdType, ATCmd_t *Para)
{
	uint16_t keylen;
	
	/* 写入关键字 */
	keylen = strlen(KeyWord);
	if (keylen > AT_KEYLEN_MAXNUM) keylen = AT_KEYLEN_MAXNUM;
	memset(Para->CmdKeyword, 0x00, AT_KEYLEN_MAXNUM);
	strncpy(Para->CmdKeyword, KeyWord, keylen);
	
	/* 类型 */
	Para->CmdType = CmdType;
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_SetupATCmdPara
功能描述: 配置AT指令参数
参数输入: BLEDataPara_t	*BleData	蓝牙数据参数
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_SetupATCmdPara(uint8_t ParaArgc, char *ParaArgv[], ATCmd_t *Para)
{
	uint8_t i;
	uint16_t argvlen;
	
	Para->Argc = ParaArgc;
	for (i=0; i<ParaArgc; i++)
	{
		argvlen = strlen(ParaArgv[i]);
		if (argvlen > AT_ARGVLEN_MAXNUM) argvlen = AT_ARGVLEN_MAXNUM;
		strncpy(Para->Argv[i], ParaArgv[i], argvlen);
	}
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_RegisterATCmd
功能描述: 注册AT指令
参数输入: BLEDataPara_t	*BleData	蓝牙数据参数
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_RegisterATCmd(char *KeyWord, uint8_t CmdType, uint8_t ParaArgc, char *ParaArgv[], ATResult (*cbAtCmdFunc)(uint8_t Argc, char *Argv[]), ATCmdGroups_t *ATCmdGrp)
{
	uint8_t		i;
	uint16_t	keylen;
	uint8_t		cmdnbr;
	uint16_t	argvlen;
	ATCmd_t		at_cmd;
	
	ATResult	ret;

	
	/* 检查数据条数 */
	cmdnbr = ATCmdGrp->CmdNbr;
	if ((cmdnbr + 1) > ATCMD_MAXNUM)
	{
		return ATRET_FUNC_OVERMAX;
	}
	
	keylen = strlen(KeyWord);
	if (keylen > 20) keylen = 20;
	
	/* 检查重复命令字 */
	for (i=0; i<cmdnbr; i++)
	{
		if (strcasecmp(KeyWord, ATCmdGrp->ATCmdTable[i].CmdKeyword) == 0)
		{
			return ATRET_FUNC_REPEAT;
		}
	}
	
	/* 注册参数 */
	/* 命令回调函数原型为 int16_t (uint8_t, char*) */
	/* 参数 uint8_t 为传入参数个数 */
	/* 参数 char* 为传入参数列表 */
	AT_InitATCmdPara(&at_cmd);
	strncpy(at_cmd.CmdKeyword, KeyWord, keylen);		
	at_cmd.CmdType = CmdType;
	at_cmd.Argc = ParaArgc;
	ret = AT_InitATMem(at_cmd.Argv, at_cmd.Argc, AT_ARGVLEN_MAXNUM);
	if (ret == ATRET_MEM_FAIL)
	{
		return ATRET_MEM_FAIL;
	}
	for (i=0; i<ParaArgc; i++)
	{
		argvlen = strlen(ParaArgv[i]);
		if (argvlen > AT_ARGVLEN_MAXNUM) argvlen = AT_ARGVLEN_MAXNUM;
		strncpy(at_cmd.Argv[i], ParaArgv[i], argvlen);
	}
	at_cmd.cbATFunc = cbAtCmdFunc;
	
	/* 注册命令表 */
	ATCmdGrp->ATCmdTable[cmdnbr] = at_cmd;
	ATCmdGrp->CmdNbr = cmdnbr + 1;
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_InitATCmdGroup
功能描述: 初始化AT指令组
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_InitATCmdGroup(ATCmdGroups_t *ATCmdGrp)
{
	uint8_t i;
	
	ATCmdGrp->CmdNbr = 0;
	for (i=0; i<ATCMD_MAXNUM; i++)
	{
		AT_InitATCmdPara(&(ATCmdGrp->ATCmdTable[i]));
	}
	
	return ATRET_OK;
}


/************************************** AT指令处理函数 ************************************************/

/******************************************************************
函数名称: ATResult AT_KeywordPreProcess
功能描述: AT指令关键字预处理
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_KeywordPreProcess(char *Before, char *After)
{
	uint16_t slen;
	uint16_t stpos, edpos;
	uint16_t i, j;
	
	slen = strlen(Before);
	if (slen < 2)
		return ATRET_LEN_ERR;
	if (slen > AT_KEYLEN_MAXNUM)
	{
		slen = AT_KEYLEN_MAXNUM;		//超过最大关键字长度截断 默认20字节
	}
	
	/* AT命令 */
	if ((Before[0] == 'A') && (Before[1] == 'T') && (slen == 2))
	{
		After[0] = 'A';
		After[1] = 'T';
		After[2] = 0x00;
	}
	/* 命令关键字以'+'开始 查找加号 */
	else
	{
		stpos = 0xFFFF;
		edpos = slen;
		for (i=0; i<slen; i++)
		{
			/* 查找加号 '+' */
			if (Before[i] == '+')
			{
				stpos = i;
			}
			/* 如果关键字尾部含有'\r\n' 去掉并结束 */
			if ((Before[i] == 0x0D) && (Before[i+1] == 0x0A))
			{
				edpos = i - 1;
				break;
			}
		}
		/* 没有找到命令关键字起始标记'+' 加号 */
		if (stpos == 0xFFFF)
		{
			return ATRET_KEYWORD_ERR;
		}
		memcpy(After, Before + stpos, (edpos - stpos));
	}
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_GenerateCmd
功能描述: 生成AT指令
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_GenerateCmd(char *KeyWord, uint8_t CmdType, ATCmdGroups_t *ATCmdGrp, char *OutBuf, uint16_t *BufLen)
{
	uint8_t i;
	uint16_t keylen;
	uint8_t pos;
	uint8_t argc;
	char cmdword[24];
	char paraword[128];
	uint8_t ctype;
	
	uint16_t blen=0;
	ATResult atret;
	
	keylen = strlen(KeyWord);
	if (keylen == 0)
		return ATRET_KEYWORD_ERR;		//无关键字
	if (keylen > AT_KEYLEN_MAXNUM)
	{
		keylen = AT_KEYLEN_MAXNUM;		//超过最大关键字长度截断 默认20字节
	}
	
	/* 查找关键字 */
	atret = AT_SearchATCmd(KeyWord, ATCmdGrp, &pos);
	if (atret == ATRET_FUNC_NOTMATCH)
	{
		return ATRET_FUNC_NOTMATCH;	//未找到匹配
	}
	
	/* 关键字预处理 */
	memset(cmdword, 0x00, sizeof(cmdword));
	atret = AT_KeywordPreProcess(KeyWord, cmdword);
	if (atret == ATRET_KEYWORD_ERR)
	{
		return ATRET_KEYWORD_ERR;		//关键字错误
	}
	
	/* 判断类型添加参数 */
	memset(paraword, 0x00, sizeof(paraword));
	ctype = ATCmdGrp->ATCmdTable[pos].CmdType;
	/* 判断是否可以使用该类型发送 */
	if ((ctype & CmdType) != CmdType)
	{
		return ATRET_TYPE_ERR;			//类型错误
	}
	switch (CmdType)
	{
		/* 无类型 */
		case ATCMD_TYP_NONE:
			break;
		/* 命令可设置 设置标记'=' */
		case ATCMD_TYP_SET:
			paraword[0] = '=';											//设置参数 '='
			argc = ATCmdGrp->ATCmdTable[pos].Argc;
			for (i=0; i<argc; i++)
			{
				sprintf(paraword, "%s%s", paraword, ATCmdGrp->ATCmdTable[pos].Argv[i]);
				if (i != (argc - 1))									//不是最后一个参数
				{
					sprintf(paraword, "%s,", paraword);					//参数中间添加逗号','
				}
			}
			break;
		/* 命令可查询 索要标记'?' */
		case ATCMD_TYP_QUERY:
			paraword[0] = '?';											//索要参数 '?'
			break;
		default:
			break;
	}
	
	/* 拼接完整数据 */
	sprintf(OutBuf, "AT%s%s\r\n", cmdword, paraword);
	blen = 0;
	blen += strlen(cmdword);
	blen += strlen(paraword);
	blen += 4;
	
	*BufLen = blen;
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_SpiltRespones
功能描述: 拆分AT响应
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_SpiltRespones(char *InStr, uint16_t StrLen, char *Resp[], uint8_t *RespNum)
{
	uint16_t i;
	uint16_t hpos, tpos;
	uint8_t step;
	uint8_t cnt;
	
	ATResult ret;
	
	i = 0;
	hpos = 0;
	tpos = 0;
	step = 0;
	cnt = 0;
	ret = ATRET_INCOMPLETE_ERR;
	do
	{
		/* 查找数据头 */
		if (step == 0)
		{
			if ((InStr[i] == 0x0D) && (InStr[i+1] == 0x0A))
			{
				hpos = i + 2;
				step = 1;
				i += 2;
			}
		}
		/* 已找到数据头 查找数据尾 */
		else if (step == 1)
		{
			if ((InStr[i] == 0x0D) && (InStr[i+1] == 0x0A))
			{
				tpos = i - 1;
				step = 2;
				i += 2;
			}
		}
		/* 处理头和尾之间的数据 */
		if (step == 2)
		{
			/* 没有处理过响应 处理响应数据 */
			memcpy(Resp[cnt], InStr + hpos, (tpos - hpos + 1));		//取出中间数据
			ret = ATRET_OK;
			step = 0;
			hpos = 0;
			tpos = 0;
			cnt++;
		}
		else
		{
			i++;
		}
	} while (i < StrLen);
	
	*RespNum = cnt;
	return ret;
}

/******************************************************************
函数名称: ATResult AT_ExtractPara
功能描述: 提取参数
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_ExtractPara(char *InStr, uint16_t StrLen, ATCmdGroups_t *ATCmdGrp, uint8_t *CmdPos)
{
	char keyword[24];
	
	uint16_t i, k;
	uint16_t parapos;
	
	uint8_t	cmdpos;
	char	*pstrchr=NULL;
	char 	*ptr=NULL;
	
	ATResult atret;
	ATResult ret;
	
	memset(keyword, 0x00, sizeof(keyword));
	parapos = 0;
	
	/* 分离关键字和参数 */
	for (i=0; i<StrLen; i++)
	{
		/* 取出命令关键字 */
		if (InStr[i] == ':')						//找到分隔符
		{
			memcpy(keyword, InStr, i);
			parapos = i + 1;
			pstrchr = (char*)&InStr[parapos];		//取出参数数据指针
			break;
		}
	}
	
	/* 未找到命令关键字分隔符 返回错误 */
	if (parapos == 0)
	{
		return ATRET_KEYWORD_ERR;
	}
	
	/* 查找关键字 */
	cmdpos = 0;
	atret = AT_SearchATCmd(keyword, ATCmdGrp, &cmdpos);
	if (atret == ATRET_FUNC_NOTMATCH)
	{
		return ATRET_KEYWORD_ERR;
	}
	
	/* 拆分参数 */
	/* 按','拆分字符串 */
	ptr = NULL;
	k = 0;
	do
	{
		ATCmdGrp->ATCmdTable[cmdpos].Argv[k] = strtok_r(pstrchr, ",", &ptr);	//按逗号拆分
		if (ATCmdGrp->ATCmdTable[cmdpos].Argv[k] != NULL)
		{
			k++;
			pstrchr = NULL;
		}
	} while (ATCmdGrp->ATCmdTable[cmdpos].Argv[k] != NULL);
	ATCmdGrp->ATCmdTable[cmdpos].Argc = k;
	
	*CmdPos = cmdpos;
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_AnaysisCmd
功能描述: 解析AT指令
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_AnaysisCmd(char *InBuf, uint16_t BufLen, char *RespWord, ATCmdGroups_t *ATCmdGrp)
{
	uint16_t i;
	uint16_t parapos;
	uint16_t tlen;
	uint8_t mpos;
	
	uint8_t respnum;

//	ATCmdParaTable_t atpara;
	
	ATResult atret;
	ATResult cbret;
	ATResult ret;
	
	
	/* 提取指令响应 可能存在多条响应数据 */
	ret = ATRET_RESP_OTHER;
	for (i=0; i<ATRESP_MAXNUM; i++)
	{
		memset(g_ATRespones[i], 0x00, AT_RESPLEN_MAXNUM);
	}
	atret = AT_SpiltRespones(InBuf, BufLen, g_ATRespones, &respnum);
	if (atret == ATRET_OK)
	{	
		/* 提取响应数据 */
		for (i=0; i<respnum; i++)
		{
			tlen = strlen(g_ATRespones[i]);
			/* 处理应答 */
			if (strcasecmp(g_ATRespones[i], "ok") == 0)
			{
				strncpy(RespWord, g_ATRespones[i], tlen);
				ret = ATRET_RESP_OK;
			}
			else if (strcasecmp(g_ATRespones[i], "error") == 0)
			{
				strncpy(RespWord, g_ATRespones[i], tlen);
				ret = ATRET_RESP_ERROR;
			}
			/* 查找命令返回参数 执行回调 */
			else
			{
				atret = AT_ExtractPara(g_ATRespones[i], tlen, ATCmdGrp, &mpos);		//从AT指令表中提取对应指令位置
				if (atret == ATRET_OK)
				{
					/* 找到对应指令 */
					ret = ATRET_RESP_PARA;
					if (ATCmdGrp->ATCmdTable[mpos].cbATFunc != NULL)
					{
						cbret = ATCmdGrp->ATCmdTable[mpos].cbATFunc(ATCmdGrp->ATCmdTable[mpos].Argc, ATCmdGrp->ATCmdTable[mpos].Argv);	//执行回调
					}
				}
				else if (atret == ATRET_KEYWORD_ERR)
				{
					/* 未找到对应指令 */
					ret = ATRET_FUNC_NOTMATCH;
				}
			}
		}
	}
	
	return ret;
}

/******************************************************************
函数名称: ATResult AT_BLEBaudrate
功能描述: 处理蓝牙波特率
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_BLEBaudrate(uint8_t Argc, char *Argv[])
{
	uint32_t baud;
	
	if (Argc == 1)
	{
		baud = atol(Argv[0]);
		debug_printf("BLE Baudrate=%d\r\n", baud);
	}
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_BLEPassword
功能描述: 处理蓝牙密码
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_BLEPassword(uint8_t Argc, char *Argv[])
{
	if (Argc == 1)
	{
		if (strcasecmp(Argv[0], "12589490") == 0)
		{
			debug_printf("BLE Password OK\r\n");
		}
	}
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_BLERole
功能描述: 处理蓝牙角色
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_BLERole(uint8_t Argc, char *Argv[])
{
	uint8_t role;
	
	if (Argc == 1)
	{
		role = atol(Argv[0]);
		if (role == 0)
		{
			debug_printf("BLE Role OK\r\n");
		}
	}
	
	return ATRET_OK;
}

/******************************************************************
函数名称: ATResult AT_BLEMaxconn
功能描述: 处理蓝牙最大连接数
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
ATResult AT_BLEMaxconn(uint8_t Argc, char *Argv[])
{
	uint8_t maxconn;
	
	if (Argc == 1)
	{
		maxconn = atol(Argv[0]);
		if (maxconn == 7)
		{
			debug_printf("BLE MaxConn=%d\r\n", maxconn);
		}
	}
	
	return ATRET_OK;
}


