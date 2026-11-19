#ifndef __TYPE_H
#define __TYPE_H

#include "stdint.h"

/******* 数据结构体定义 *******/
typedef struct
{
	char	CmdName[20];								//命令字 最多20个字节 多余的会被截掉
	
	int16_t (*cbCmdFunc)(uint8_t Argc, char *Argv[]);	//回调函数 参数个数,参数表
	void	(*cbCmdHelpFunc)(void);						//函数帮助信息
} DbgCmdPara_t;

typedef struct
{
	uint8_t			CmdTotalNbr;				//命令总数
	DbgCmdPara_t	CmdPara[3];	//命令参数
} DbgCmdGroups_t;
void TypeProcess(void);
int16_t DebugRecvProcess(void);
#endif