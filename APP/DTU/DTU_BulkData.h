#ifndef __DTU_BULKDATA_H
#define __DTU_BULKDATA_H

/* 引用头文件 */
#include "stm32l4xx.h"

//#include "g_GlobalValDef.h"
#include "BSP_timer.h"
#include "BSP_gpio.h"
#include "app_tool.h"
/******* 批量数据命令头定义 *******/
#define BULKCMD_CLI_REQUEST		0x0000		//客户端批量请求数据命令
#define BULKCMD_SVR_RETURN		0x8000		//服务器批量返回数据命令
#define BULKCMD_CLI_RECVD		0x0001		//客户端确认接收命令
#define BULKCMD_SVR_CONFIRM		0x8001		//服务器确认命令
#define BULKCMD_CLI_UPLOAD		0x0005		//客户端批量上传命令
//#define BULKCMD_SVR_RECVD		0x8005		//服务器确认接收命令
#define BULKCMD_SVR_RECVD		0x8002		//服务器确认接收命令
#define BULKCMD_SVR_HEAD        0x5AA5      //服务器应答基站定位/差分数据命令

//enum
//{
//	STA_PWR_ON 			= 1,		//上电
//	STA_CLOSE_ECHO 		= 2,		//关闭数据回显
//	STA_CHK_SIM_STA		= 3,		//查询信号强度 
//	STA_CHK_SELF_NO 	= 4,		//查询本机号
//	STA_CHK_SIM_CGREG 	= 5,    //查询sim卡PS业务状态
//	STA_CHK_NET_TYPE 	= 6,	   //查询运营商
//	STA_CHK_SIGNAL		= 7,	
//	STA_CFG_PDP			= 8,       //配置PDP场景、参数
//	STA_CHK_PDP 		= 9,	   	//查询PDP上下文
//	STA_ACTIVE_PDP 		= 10,      	//激活PDP上下文
//	STA_DEACTIVE_PDP 	= 11,      	//激活PDP上下文
//	STA_SET_SLEEP		= 12,      
//	STA_CREAT_DATATCP 	= 13,      	//创建TCP数据发送连接
//	STA_CREAT_UPDATETCP = 14,      	//创建TCP升级程序连接
//	STA_CREAT_UPLOADTCP = 15,      	//创建TCP数据转储连接
//	STA_CLOSE_TCP 		= 16,      	//断开SOCKET
//	STA_SEND_TO_SOCKET 	= 17,      	//发送数据到SOCKET缓存
//	STA_SENDEX_TO_SOCKET =18,      		//发送缓存数据到远程主机
//	STA_SET_MOBILE 		= 19,      
//	STA_PWR_OFF			= 20,      
//	STA_PWR_SLEEP		= 21,      
//	STA_PWR_WAKEUP		= 22,      
//	STA_NET_CHK_AT		= 23, 
//	STA_STATE_IDLE 		= 99,   
//	STA_REBOOT 			= 100   		//
//};
#define D67bit	0
#define D68bit	1

/******* 批量数据流程定义 *******/
typedef enum
{
	BULK_PROG_NONE=0,		//无
	BULK_PROG_REQUEST,		//索要请求
	BULK_PROG_RECEIVING,	//正在接收数据
	BULK_PROG_CONFIRMING,	//等待确认
	BULK_PROG_UPLOADING,	//正在上传数据
	BULK_PROG_TIMEOUT,		//传输超时
	BULK_PROG_IDLE=255		//空闲
} BulkProgress_e;

/******* 批量数据状态定义 *******/
#define	BULK_STA_OK				(0)		//
#define BULK_STA_CONNECTED		(1)		//已连接
#define BULK_STA_DOWNLOADED		(2)		//下载完成
#define BULK_STA_UPLOADED		(3)		//上传完成
#define BULK_STA_DL_CONFIRMED	(4)		//下载已确认
#define BULK_STA_UL_CONFIRMED	(5)		//上传已确认
#define BULK_ERR_TIMEOUT		(-1)	//超时
#define BULK_ERR_DTYPEERR		(-2)	//数据类型错误
#define BULK_ERR_CRCERR			(-3)	//CRC错误
#define BULK_ERR_SENDFAIL		(-10)	//发送失败
#define BULK_ERR_SENDNODATA		(-11)	//无数据
#define BULK_ERR_SENDNOMEM		(-12)	//内存不足


/******* 数据参数定义 *******/
typedef union
{
	struct
	{
		uint8_t	Order:1;		//b0:数据顺序	0=顺序;1=倒序
		uint8_t Compressed:1;	//b1:压缩		0=普通;1=压缩
		uint8_t :6;				//b2-b7:预留	
	} bit;
	uint8_t all;
} BulkDataPara_t;


/******* 批量数据参数定义 *******/
typedef struct
{
	BulkProgress_e	Step;			//当前流程
	
	char			cID[16];		//ID号 字符串格式 不足16字节补0
	char			cSN[16];		//SN号 字符串格式 不足16字节补0
	BulkDataPara_t	DataPara;		//记录数据参数
	uint16_t		uRecordCnt;		//记录数
	uint32_t		iMinRecordOrder;//最小记录号
	uint32_t		iMaxRecordOrder;//最大记录号
	uint32_t		iRawRecordSize;	//原始记录大小
} BulkPara_t;

/******* 数据结构体定义 *******/
/******* 数据头定义 *******/
#pragma pack(push, 1)	//按1字节对齐
typedef union
{
	struct
	{
		uint16_t		uCmd;			//命令字
		uint16_t		Reserve1;		//预留1
		char			cID[16];		//ID号 字符串格式 不足16字节补0
		char			cSN[16];		//SN号 字符串格式 不足16字节补0
		BulkDataPara_t	DataPara;		//数据数据参数
		uint16_t		uRecordCnt;		//记录数
		uint8_t			Reserve2;		//预留2
		uint32_t		iMinRecordOrder;//最小记录号
		uint32_t		iMaxRecordOrder;//最大记录号
		uint32_t		iRawRecordSize;	//原始记录大小
		uint32_t		Reserve3;		//预留3
	} byte;
	uint8_t	all[56];
} BulkHeader_t;
#pragma pack(pop)		//对齐恢复

/******* 发送数据记录头定义 *******/
#pragma pack(push, 1)	//按1字节对齐
typedef union
{
	struct
	{
		uint16_t		uLength;		//记录长度
		char			cSrcNum[16];	//源号码
		char			cDstNum[16];	//目标号码
		uint16_t		uType;			//类型
	} byte;
	uint8_t all[36];
} BulkSendRecordHeader_t;
#pragma pack(pop)		//对齐恢复

/******* 队列数据记录体定义 *******/
#pragma pack(push, 1)	//按1字节对齐
typedef struct
{
//	uint32_t	ID;				//记录ID
	uint16_t	uLength;		//记录长度
	uint8_t		RecordBody[128];//记录体
} BulkRecord_t;
#pragma pack(pop)		//对齐恢复

/******* 队列数据记录体定义 *******/
#pragma pack(push, 1)	//按1字节对齐
typedef struct
{
	uint8_t	    flag;		//是否应答服务器标记
	uint8_t		type;       //应答数据类型
	uint8_t     seri[4];
} RespServ_t;
#pragma pack(pop)		//对齐恢复

/******* 外部变量声明 *******/
extern RespServ_t       g_ReplyData;
extern uint8_t g_FileSendDataToDtu;
extern uint32_t g_tmrUploadBasePosDelay;	//基站定位上传定时器
extern uint32_t g_SendPunCoorDelay;         //重新发送打点数据延迟
extern uint8_t g_BasePosUploaded;		//基站定位上传标识
//extern uint32_t g_DTUSendDataDelay;
//extern uint8_t 	CelluarStep;
//extern bool 	g_SendD6Suc;
extern UPDATAPROGRAM g_UpdateCodePageTotal;
typedef struct
{
    uint8_t bootversion[4];
	uint8_t userversion[4]; 
    uint16_t Crc16;
}_VERSION_FLASH;
extern _VERSION_FLASH g_VersioninFlash;
extern uint8_t g_ShouldJumpToStartFlag;
extern uint8_t g_checkbuf[512];
extern uint8_t LastMajorVersion;                 //主版本号  
extern uint8_t LastMinorVersion;                 //次版本号 
extern uint32_t  LastTotalBagNum;
extern uint8_t g_RemoteUpdateGCFlag; 
extern uint8_t g_RemoteUpdateFirstFlag;/* 远程升级开始标志 */
extern uint16_t g_RemoteUpdatePackNo;
extern uint16_t g_RemoteUpdatePackTotal;
extern uint8_t g_RemoteOutTimer;         /*max=3*/
extern uint8_t g_RemoteDataOkTimer;      /*max=3*///g_RemoteUpdateNoDataTimer
extern uint8_t g_UpdateType;	//0-全量升级 1-增量升级
extern uint8_t g_SendDiffMark;             //基站是否发送差分数据标记
extern bool g_GetMyMobileOKFlag;
extern int8_t g_ResendTimes;
extern uint8_t g_ResendFlag;
extern uint32_t g_ResendDelay;
extern uint8_t g_IPBuffer[3][4];
extern uint16_t g_PortNo[3];
extern uint8_t 	g_DataIPBack[4];      //数据IP备份
extern uint16_t  g_DataPortBack;      //数据端口号备份
extern uint8_t	g_VersionHex[2];				//16进制版本号
extern uint8_t	g_VersionAscii[4];				//ASCII版本号
extern uint8_t	g_BulkRecvBuf[4096];
/******* 外部函数声明 *******/
int16_t DTU_BulkParaInit(BulkPara_t *Para);
//void DTU_BulkProgrss(BulkPara_t *Para);
void DTU_BulkProgrss(void);
//测试函数
//void InsertC3Record(xQueueHandle RecordQue);

#endif

