#ifndef __BSP_DEBUG_H
#define __BSP_DEBUG_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"



/******* 数据定义 *******/
#define DBG_CMDTABLE_MAX	30		//调试命令表最大数量

/* 返回值 */
//#define DBGRET_OK				0		//正常
//#define DBGRET_ERR_CMD_OVERMAX	-1		//超过最大数量
//#define DBGRET_ERR_CMD_NONE		-2		//无命令字
//#define DBGRET_ERR_CMD_ILLEGAL	-3		//命令字非法
//#define DBGRET_ERR_ARG_ILLEGAL	-30		//参数非法
//#define DBGRET_ERR_ARG_LESS		-31		//缺少参数
//#define DBGRET_ERR_ARG_OUTRANGE	-32		//参数越界
//#define DBGRET_ERR_NODATA		-100	//无数据
//#define DBGRET_ERR_NOMATCH		-101	//未找到匹配命令
//#define DBGRET_ERR_OTHER		-200	//其他错误

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
	DbgCmdPara_t	CmdPara[DBG_CMDTABLE_MAX];	//命令参数
} DbgCmdGroups_t;

/* 数据回显结构体 */
/* 数据回显状态 0=不启用;1=显示二进制格式;2=显示ASC格式;3=预留 */
//typedef struct
//{
//	uint16_t	EnableLoraEcho:2;	//b0-b1:Lora数据回显
//	uint16_t	EnableRtkEcho:2;	//b2-b3:RTK数据回显
//	uint16_t	EnableGpsEcho:2;	//b4-b5:GPS数据回显
//	uint16_t	EnableDtuEcho:2;	//b6-b7:DTU数据回显
//	uint16_t	EnableBleEcho:2;	//b8-b9:蓝牙数据回显
//	uint16_t	EnableRTCMEcho:2;	//b10-b11:RTCM数据回显
//	uint16_t	EnableIMUEcho:2;	//b12-b13:IMU数据回显(陀螺仪)
//	uint16_t	EnableCORSEcho:2;	//b14-b15:CORS数据回显
//} DbgEcho_t;

/******* 外部变量声明 *******/

/* 接收缓存 */
//extern uint8_t g_DebugDatBuf[128];

/* 命令表 */
//extern DbgCmdGroups_t	g_CmdTable;		//调试命令表

//extern uint8_t			CommMode;		//通信模式 0=正常;1=蓝牙透传3
//extern uint8_t			RtkCommMode;			
//extern uint8_t			WorkMode;		//工作模式 0=正常;1=测试(发送固定差分数据)

//extern uint16_t		BleSize;
//extern uint8_t			EchoSta;
//extern DbgEcho_t		g_DbgEcho;		//数据回显
//extern char g_DebugInfoBuf[3076];

/******* 外部函数声明 *******/
void FatfsTest(void);
void debug_info(uint8_t *Info, uint16_t Len, char *Lable);
void debug_ascinfo(uint8_t *Info, uint16_t Len, char *Lable, char UnReadChar);

//uint8_t CheckParaInList(char ParaCh, char *List);

/* 内置命令 */
//void ListRegisteredCmd(DbgCmdGroups_t CmdTable);

/* 调试命令处理与执行 */
//int16_t DebugRecvProcess(void);
//int16_t ExecDebugCmdFromCmdTable(DbgCmdGroups_t CmdTable, uint8_t *InBuf);

/* 初始化 */
void InitTerminalCmdGroup(void);

/* 注册与反注册命令 */
int16_t RegisterDbgTerminalCmd(char *CmdName, int16_t (*cbCmdFunc)(uint8_t Argc, char *Argv[]), void (*cbHelp)(void), DbgCmdGroups_t *CmdTable);
void UnRegisterDbgTerminalCmd(char *CmdName, DbgCmdGroups_t *CmdTable);

#endif

