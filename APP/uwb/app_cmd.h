#ifndef __APP_CMD_H
#define __APP_CMD_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"
//#include "BSP_debug.h"


/******* 数据定义 *******/
#define DBG_CMDTABLE_MAX	30		//调试命令表最大数量
/* 返回值 */
typedef enum
{
	DBGRET_OK				= 0,		//0 = 正常
	/* 命令错误代码 从100开始 */
	DBGRET_ERR_CMD_OVERMAX	= 100,		//100 = 超过最大数量
	DBGRET_ERR_CMD_NONE,				//无命令字
	DBGRET_ERR_CMD_ILLEGAL,				//命令字非法
	DBGRET_ERR_ARG_ILLEGAL,				//参数非法
	DBGRET_ERR_ARG_LESS,				//缺少参数
	DBGRET_ERR_ARG_OUTRANGE,			//参数越界
	/* 返回值错误代码 从200开始 */
	DBGRET_ERR_NODATA		= 200,		//无数据
	DBGRET_ERR_NOMATCH,					//未找到匹配命令
	DBGRET_ERR_OTHER,					//其他错误
} DbgResult_t;

DbgResult_t Cmd_GetCompilerInfo(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_SetUwbAnchorsInfo(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_SaveUwbAnchorsInfo(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_ReadUwbAnchorsInfo(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_SetDevConfig(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_SetSolutionMode(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_SetShowMode(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_UwbDiag(uint8_t Argc, char *Argv[]);
DbgResult_t Cmd_SetUserdata(uint8_t Argc, char *Argv[]);
/******* 命令表 X-Macro *******/
/* 加删命令只改这里，其他地方自动生效 */
#define CMD_TABLE                        	\
    X("info",		Cmd_GetCompilerInfo)	\
	X("setcoord",	Cmd_SetUwbAnchorsInfo)	\
	X("savecoord",	Cmd_SaveUwbAnchorsInfo) \
	X("readcoord",	Cmd_ReadUwbAnchorsInfo) \
	X("setconfig",	Cmd_SetDevConfig)		\
	X("setmode",	Cmd_SetSolutionMode)	\
	X("setshow",	Cmd_SetShowMode)		\
	X("uwbdiag",	Cmd_UwbDiag)		\
	X("userdata",	Cmd_SetUserdata)

/* 自动生成枚举，CMD_COUNT = 命令总数 */
typedef enum {
    #define X(name, func) CMD_ENUM_##func,
    CMD_TABLE
    #undef X
    CMD_COUNT
} DbgCmdIndex_t;

/******* 数据结构体定义 *******/
typedef struct
{
    char		CmdName[20];                              // 命令字
    DbgResult_t	(*cbCmdFunc)(uint8_t Argc, char *Argv[]); // 回调函数
} DbgCmdItem_t;

typedef struct
{
    const DbgCmdItem_t *CmdPara;    // 指向命令表
} DbgCmdTable_t;

/* 数据回显结构体 */
/* 数据回显状态 0=不启用;1=显示二进制格式;2=显示ASC格式;3=预留 */
typedef struct
{
	uint16_t	EnableLoraEcho:2;	//b0-b1:Lora数据回显
	uint16_t	EnableRtkEcho:2;	//b2-b3:RTK数据回显
	uint16_t	EnableGpsEcho:2;	//b4-b5:GPS数据回显
	uint16_t	EnableDtuEcho:2;	//b6-b7:DTU数据回显
	uint16_t	EnableBleEcho:2;	//b8-b9:蓝牙数据回显
	uint16_t	EnableRTCMEcho:2;	//b10-b11:RTCM数据回显
	uint16_t	EnableIMUEcho:2;	//b12-b13:IMU数据回显(陀螺仪)
	uint16_t	EnableCORSEcho:2;	//b14-b15:CORS数据回显
} DbgEcho_t;

/******* 外部变量声明 *******/

/* 接收缓存 */
extern uint8_t g_DebugTmpBuf[256];
extern uint8_t g_DebugDatBuf[128];

//


/* 命令表 */
extern DbgCmdTable_t	g_CmdTable;		//调试命令表

extern uint8_t			CommMode;		//通信模式 0=正常;1=蓝牙透传3
extern uint8_t			RtkCommMode;			
extern uint8_t			WorkMode;		//工作模式 0=正常;1=测试(发送固定差分数据)
extern char g_DebugInfoBuf[3076];
extern uint16_t		BleSize;
//extern uint8_t			EchoSta;
extern DbgEcho_t		g_DbgEcho;		//数据回显


/******* 外部函数声明 *******/


/* 调试命令处理与执行 */
DbgResult_t CmdRecvProcess(char *InStr, char StChr, uint8_t ForceEnding);
void ListRegisteredCmd(void);

/******** 命令表定义声明 ********/

#endif

