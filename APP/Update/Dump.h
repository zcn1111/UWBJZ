#ifndef __DUMP_H__
#define __DUMP_H__

#include <stdbool.h>
#include "stm32l4xx_hal.h"
//#include "tool.h"

#include "BSP_timer.h"
//#include "DTU_Process.h"


/********** 宏定义 **********/
#define DUMP_SEND_BUFSIZE	4096

/******* 数据结构体定义 *******/

/* 函数返回值状态 */
typedef enum
{
	DPRET_OK				= 0,	/* 0 = 成功 */
	DPRET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	DPRET_SEND_FAIL			= 2,	/* 2 = 发送失败 */
	DPRET_RECV_FAIL			= 3,	/* 3 = 接收失败 */
	DPRET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	DPRET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	DPRET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	DPRET_SEND_PAGE_FIN		= 200,	/* 200 = 当前页已结束 */
	DPRET_SEND_PACKAGE_FIN	= 201,	/* 201 = 当前包已结束 一页数据会根据发送大小拆分为一个或多个包进行发送 */
	DPRET_TOOL_FILENAME		= 300,	/* 300 = 索要文件名 开始转储 */
	DPRET_TOOL_START		= 301,	/* 301 = 开始转储 */
	DPRET_TOOL_NEWPAGE		= 302,	/* 302 = 新页 */
	DPRET_TOOL_FINISH		= 303,	/* 303 = 转储结束 */
} DPResult;

/* 转储状态 */
typedef enum
{
	DUMP_PROG_NONE		=0,		/* 0 = 没有转储 */
	DUMP_PROG_DUMPING	=1,		/* 1 = 正在转储 */
	DUMP_PROG_FINISH	=2,		/* 2 = 转储结束 */
} DumpProg_e;

/* 转储参数 */
typedef struct
{
	uint8_t		DumpType;			//转储类型
	
	uint32_t	PageSize;			//页大小
	uint32_t	TotalPageNbr;		//总页数
	uint32_t	RequestPageOrd;		//索要页编号
	uint32_t	SendPageSize;		//发送页大小
	uint32_t	SendPageOrd;		//已发送的页编号
	uint32_t	SendPageOffset;		//已发送的页偏移
	uint8_t		ChkSum;				//校验和
	
	uint8_t 	FileName[64];		//文件名
	uint32_t	FileSizeInByte;		//文件大小(字节)
	uint32_t	FileOffsetInByte;	//文件偏移(字节)
	
	DumpProg_e	DumpProgress;		//转储状态
	uint8_t		RetryCnt;			//重试次数
	uint8_t		RetryWaiting;		//重试等待时间(秒)
} DumpPara_t;

/******* 外部变量声明 *******/
extern DumpPara_t	g_DumpPara;
extern uint8_t 	*pDumpBuf;	//转储缓存


/******* 外部函数声明 *******/
/* 初始化 */
DPResult Dump_InitDumpPara(DumpPara_t *DmpPara);
DPResult Dump_SetupPageSize(DumpPara_t *DmpPara, uint32_t PageSize, uint32_t SendPageSize, uint8_t *pBuf);

/* 接收数据 */
DPResult Dump_GenLocateDumpFilename(DumpPara_t *DmpPara, uint16_t TrainCode, HexDateTime_t NowTime, uint8_t MemSize);
DPResult Dump_GenDumpData(DumpPara_t *DmpPara, uint8_t *Buf);

DPResult Dump_RecvDataFromTool(DumpPara_t *DmpPara, uint8_t *InBuf);

#endif /* __DUMP_H */
