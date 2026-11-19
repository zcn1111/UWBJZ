#ifndef __DTU_COMM_H
#define __DTU_COMM_H

/* 引用头文件 */
#include "stm32l4xx.h"

//#include "g_GlobalValDef.h"
#include "bsp.h"
//#include "BSP_timer.h"
//#include "BSP_gpio.h"

/******* 设备通信类型码 *******/
/* 数据定义 */
#define EXTFUNC_GRP_MAX		20		//外部函数组最大个数 默认20
#define GENFUNC_GRP_MAX		30		//通用外部函数最大 默认30

/* 设备数据类型 */
#define TCODE_DEV_QUERYDTUSTATE	0x60
#define TCODE_DEV_RETURNSTATE	0x61
#define TCODE_DEV_SENDDATA		0x62
#define TCODE_DEV_ACKSUCCESS	0x63
#define TCODE_DEV_ASKDATETIME	0x65
#define TCODE_DEV_ASKSIGNAL		0x65
#define TCODE_DEV_ASKPHONENUM	0x65
#define TCODE_DEV_CHANGEIP		0x66
#define TCODE_DEV_WRITEPHONENUM	0x67
#define TCODE_DEV_ACKDTUUPDATE	0x68
#define TCODE_DEV_TRANSPARENT	0x69
/* DTU数据类型 */
#define TCODE_DTU_QUERYDEVSTATE	0x80
#define TCODE_DTU_RETURNSTATE	0X81
#define TCODE_DTU_SENDDATA		0x82
#define TCODE_DTU_RETDATETIME	0x85
#define TCODE_DTU_RETSIGNAL		0x86
#define TCODE_DTU_RETPHONEANDIP	0x87
//#define TCODE_DTU_RETPHONENUM	0x87
//#define TCODE_DTU_RETIPADDRESS	0x87
#define TCODE_DTU_SENDDTUUPDATE	0x88
#define TCODE_DTU_ACKTRANSP		0x89

/******* 状态定义 *******/
/******* 返回状态定义 *******/
typedef enum
{
	DTURET_OK				= 0,	/* 0 = 成功 */
	DTURET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	DTURET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	DTURET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	DTURET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	DTURET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	DTURET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	DTURET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	DTURET_TYPE_ERR			= 8,	/* 8 = 类型错误 */
	DTURET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	DTURET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
	DTURET_HEADER_NOHEADER	= 200,	/* 200 = 数据头 错误数据头 */
	DTURET_HEADER_NOTD6		= 201,	/* 201 = 数据头 不是D6头 */
	DTURET_HEADER_NOTCA		= 202,	/* 202 = 数据头 不是CA头 */
	DTURET_HEADER_NOTEE		= 203,	/* 203 = 数据头 不是EE头 */
	DTURET_CALLBACK_FAIL	= 300,	/* 300 = 函数回调 失败 */
	DTURET_PWR_OFF			= 400,	/* 400 = 电源 已关闭 */
	DTURET_PWR_RESETTING	= 401,	/* 401 = 电源 正在重启 */
} DtuResult;

#define DTU_COMM_OK					0
#define DTU_COMM_ERR_LEN			(-1)	//长度错误
#define DTU_COMM_ERR_CHKSUM			(-2)	//校验和错误
#define DTU_COMM_ERR_TYPCODE		(-3)	//类型码错误
#define DTU_COMM_ERR_NODATA			(-20)	//未收到数据
#define DTU_COMM_ERR_NOTD6			(-21)	//不是D6头
#define DTU_COMM_ERR_NOTCA			(-22)	//不是CA头
#define DTU_COMM_ERR_NOTEE			(-23)	//不是升级
#define DTU_COMM_ERR_SENDFAIL		(-40)	//发送失败
#define DTU_COMM_ERR_NOHEADER		(-41)	//数据头错误
#define DTU_COMM_ERR_CALLBACKFAIL	(-60)	//回调失败
#define DTU_COMM_ERR_TRANSTFAIL		(-70)	//切换透传失败
#define DTU_COMM_ERR_NOCTLINFO		(-80)	//未找到控制信息
#define DTU_COMM_ERR_ENDCTLINFO		(-81)	//控制信息已结束
#define DTU_PWR_OFF					(-100)	//DTU断电
#define DTU_PWR_RESETTING			(-101)	//DTU正在重启

#define DTU_COMM_CONNECTED			1		//DTU已连接
#define DTU_COMM_GOTTENCONNINFO		2		//DTU已获取手机号和时间
#define DTU_COMM_TRANS_CONN			3		//DTU已连接(透传模式)
#define DTU_COMM_TRANS_GOTTENTIME	4		//DTU已获取时间(透传模式)
#define DTU_COMM_TRANS_EXIT			5		//DTU已退出(透传模式)
#define DTU_COMM_UPDATE_ENTRY		6		//DTU升级数据

/******* IP端口号类型 *******/
#define PORTNUM_DATA	2020	//数据端口
#define PORTNUM_UPDATE	2021	//升级端口
#define PORTNUM_DUMP	2029	//转储端口
#define PORTNUM_BATCH	2030	//批量端口

/******* 数据结构体定义 *******/
/* DTU连接流程 */
typedef enum
{
	DTU_PROG_DISCONNECT=0,		// 0=未连接
	DTU_PROG_CONNECTING,		// 1=正在连接
	DTU_PROG_NORMAL,			// 2=正常
	DTU_PROG_UPDATE_ENTRY,		// 3=开始升级
	DTU_PROG_UPDATE_REQ,		// 4=请求升级数据包
	DTU_PROG_UPDATE_RECV,		// 5=接收升级数据包
	DTU_PROG_UPDATE_EXIT,		// 6=退出升级
	DTU_PROG_DUMP_ENTRY,		// 7=开始转储
	DTU_PROG_DUMP_UPLOAD,		// 8=上传转储数据包
	DTU_PROG_DUMP_EXIT,			// 9=退出转储
	DTU_PROG_TRASPORT_ENTRY,	//10=开始透传
	DTU_PROG_TRASPORTION,		//11=透传模式
	DTU_PROG_TRASPORT_EXIT		//12=退出透传
} DtuProgress_e;

/* DTU本机状态 */
typedef union
{
	struct
	{
		uint8_t DtuState:1;	//DTU状态 0=空闲;1=忙
		uint8_t GSMState:1;	//GSM状态 0=未开机;1=开机
		uint8_t GPRSState:1;//GPRS状态 0=未连接;1=连接
		uint8_t HaveData:1;	//有数据发送 0=无数据;1=有数据
		uint8_t SendMode:1;	//数据发送方式 0=GPRS;1=短信
		uint8_t SendOK:1;	//发送成功 0=失败;1=成功
		uint8_t Reserve1:1;	//预留1
		uint8_t Reserve2:1;	//预留2
	}bit;
	uint8_t all;
} DtuSelfSta_t;

/* 设备本机状态 */
typedef union
{
	struct
	{
		uint8_t DevState:1;	//设备状态 0=空闲;1=忙
		uint8_t Reserve:7;	//预留
	}bit;
	uint8_t all;
} DevSelfSta_t;


/* DTU本机号 */
/* 数据发送方式 */
typedef union
{
	struct
	{
		uint8_t	SendMode:1;	//b0:发送方式 0=GPRS或短信方式;1=强制短信方式
		uint8_t	SMScode:2;	//b1-b2:短信编码方式 0=文本;1=7-bit编码;2=8-bit编码;3=UCS2编码
		uint8_t	DataLevel:1;//b3:数据级别 0=低级别数据(发送3次失败后放弃);1=超级别数据(保证成功率)
		uint8_t	:4;			//b4-b7:不用
	} bit;
	uint8_t all;
} DataSendMode_t;

/* 手机号结构体定义 */
typedef struct
{
	uint8_t			MobNum[6];	//手机号按BCD码顺序表示
	DataSendMode_t	SendMode;	//数据发送模式
} MobileNum_t;

/* IP和端口号 */
typedef union
{
	struct
	{
		uint32_t uIPAddr;	//IP地址 4个字节
		uint16_t uPort;		//端口号 2个字节
	} byte;
	uint8_t all[8];
} IPAddr_t;



/* DTU连接标记 */
typedef union
{
	struct
	{
		uint16_t	Connected:1;		//b0:已连接
		uint16_t	ObtainedTime:1;		//b1:已获取时间
		uint16_t	ObtainedPhoneNum:1;	//b2:已获取本机号
		uint16_t	ObtainedIP:1;		//b3:已获取IP
		uint16_t	ChangedIPSta:2;		//b4-b5:切换IP状态	0=不切换;1=正在切换;2=已切换;3=切换失败
		uint16_t	PoweredSta:2;		//b6-b7:电源状态		0=已断电;1=等待重启断电;2=等待重启上电;3=已上电
		uint16_t	PoweredReset:1;		//b8:已重启标记		0=正常;1=重启
		uint16_t	Transport:1;		//b9:透传模式		0=正常;1=透传模式
		uint16_t	Reserve:6;			//b10-b15:预留
	} bit;
	uint16_t all;
} DtuNetFlag_t;

///* DTU外部函数参数 */
//typedef struct
//{
//	uint8_t		Header[20];
//	uint8_t		HeaderLen;
//	uint16_t	DatLen;
//	
//}

/********* DTU外部函数变量定义 **********/
/* DTU外部函数参数 */
typedef struct
{
	uint8_t		RegHead[20];		//识别数据头 最长20个字节
	uint8_t		RegLen;				//识别头长度
	uint16_t	DatLen;				//数据长度

	DtuResult	(*cbFunc)(void);	//执行回调函数 返回值:0=正常返回;其他=错误
} DtuExtFunc_t;

/* 外部函数执行标记 */
typedef struct
{
	uint8_t NeedExec:1;		//b0:需要执行命令 0=不执行;1=执行
	uint8_t Reserve:3;		//b1-b3:预留
	uint8_t Priority:4;		//b4-b7:优先级 0x0-0xF=优先级从高到低
} ExecFlag_t;

/* 外部函数组 */
typedef struct
{
	/* 函数执行标记 */
	struct
	{
		uint8_t NeedExec:1;		//b0:需要执行命令 0=不执行;1=执行
		uint8_t Reserve:3;		//b1-b3:预留
		uint8_t Priority:4;		//b4-b7:优先级 0x0-0xF=优先级从高到低
	} ExecFlag;
	DtuExtFunc_t	ExtFuncPara;	//外部函数参数
} ExtFuncGroups_t;

/* DTU设备参数 */
typedef struct
{
	DtuSelfSta_t	SelfSta;		//DTU本机状态
	
	uint8_t			SignalVal;		//信号强度
	uint8_t			GsmType;		//连接类型
	uint8_t			DtuVersion;		//DTU版本号
	BcdDateTime_t	DtuDateTime;	//DTU时间日期
	IPAddr_t		ConIPAddr;		//当前IP地址
	IPAddr_t		ChangeIPAddr;	//切换IP地址
	MobileNum_t		ConPhoneNum;	//当前本机号
	
	uint8_t			UpdateFlag;		//升级标记 1=开始升级;2=结束升级
	uint8_t			TransMode;		//透传模式 1=透传;2=关闭透传
	DtuNetFlag_t	DtuNetFlag;		//DTU当前网络标记
	
	DtuProgress_e	DtuProgress;	//DTU状态流程
	
} DtuPara_t;

/******* 外部变量声明 *******/
//extern uint8_t g_DtuDataBuf[520];			//数据缓存变量

extern LEDState_t		g_DTUSendLED;
extern LEDState_t		g_DTURecvLED;
extern LEDState_t		g_DTUGprsOKLED;


//extern ExtFuncGroups_t	g_DtuFuncGroupArr[EXTFUNC_GRP_MAX];	//外部函数
//extern ExtFuncGroups_t g_GenFuncGroupArr[GENFUNC_GRP_MAX];	//通用外部函数


/******* 外部函数声明 *******/
int16_t DTU_SendRequestCmd(DevSelfSta_t SelfSta, uint8_t TypeCode, uint16_t DomainLen);
int16_t DTU_RecvReturnCmd(DtuSelfSta_t *SelfSta, uint8_t *TypeCode, uint16_t *DomainLen);

/* 生成DTU发送命令 */
int16_t DTU_QueryDTUStatus(void);
int16_t DTU_ReturnState(DevSelfSta_t DevSta);
int16_t DTU_AckRecvSuccess(void);
int16_t DTU_AskSignalStrength(void);
int16_t DTU_AskDateTime(void);
int16_t DTU_AskPhoneNum(void);
int16_t DTU_WritePhoneNumToDTU(MobileNum_t PhoneNum);
int16_t DTU_ChangeIP(IPAddr_t IPAddr, uint8_t IPMode);
int16_t DTU_AckDtuUpdate(void);
int16_t DTU_TranparentTransMode(uint8_t TransMode);
int16_t DTU_RequestUpdatePackage(uint8_t *UpdateBuf);


/* 接收DTU处理 */
int16_t DTU_AnlysisTypecodeProgress(DtuPara_t *Para);

/* D6数据处理 */
int16_t DTU_SendDataByD6(uint8_t *DatBuf, uint16_t DatLen, MobileNum_t ObjPhNum);
int16_t AnlysisHeadProcessByD6(ExtFuncGroups_t *ptrFunc, uint8_t *DatBuf, uint16_t DatLen);
int16_t DTU_ProcessByD6(uint16_t DomainLen, uint8_t *DatBuf, uint16_t *DatLen, MobileNum_t *ObjNum);


/* 通用数据处理 */
int16_t DTU_SendDataByInCommon(uint8_t *DatBuf, uint16_t DatLen, MobileNum_t ObjPhNum, char *strHeader);
int16_t DTU_RecvDataInCommon(uint16_t DomainLen, uint8_t *DatBuf, uint16_t *DatLen, uint8_t *Header);


/* 外部函数处理 */
void DTU_SetupExternalFunc(uint8_t *strHead, uint16_t DatLen, DtuResult (*cbFunc)(void), DtuExtFunc_t *ExtPara);
void DTU_RegisterExtFuncToGroup(DtuExtFunc_t Para, uint8_t Priority, ExtFuncGroups_t *ptrFunc);
void DTU_UnRegisterExtFunc(ExtFuncGroups_t *ptrFunc);



#endif

