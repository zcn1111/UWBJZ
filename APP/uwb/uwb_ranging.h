#ifndef __UWB_RANGING_H
#define __UWB_RANGING_H

#include "stm32l4xx.h"
#include "deca_device_api.h"
//#include <stdint.h>

/* ==================== Ranging Parameters ==================== */
#define RANGING_TIMEOUT_MS      100
#define RANGING_MAX_RETRY       3
#define ONE_SLOT_TIME			28
#define MAX_SLOT_NUMBER			10

#define ID_ANCHOR_FLAG		0x8000
#define TIMESTAMP_NULL		0xFFFFFFFF
//#define SPEED_OF_LIGHT          299792458.0		//在api中定义
//#define UWB_TICK_TO_S           (1.0/63897952.0)

#define UWBTXBUF_SIZE		128
#define UWBRXBUF_SIZE		128

#define UWB_MACHEAD_LEN		(9)
#define UWB_PAYLOAD_LEN		(37)
#define UWB_FCS_LEN			(2)
/* ==================== Ranging State ==================== */
/* 返回参数 */
typedef enum
{
	UWBRET_OK			= 0,	// 0=正常
	/* 错误代码 100开始 */
	UWBRET_SENDERR		= 100,	// 100=发送失败
	UWBRET_LENERR,				// 101=数据长度错误
	UWBRET_PANIDERR,			// 102=PAN网络地址错误
	UWBRET_DSTIDERR,			// 103=目标地址错误
	UWBRET_MACERR,				// 104=MAC错误
	UWBRET_TSLESS,				// 105=时间戳错误(不全)
	UWBRET_CONFIGERR,			// 106=配置错误
	UWBRET_SENDTIMEOUT	= 130,	// 130=发送超时
	UWBRET_OTHER		= 199	// 199=其他错误
} UWBResult_t;


/* UWB通信状态 */
typedef enum
{
	/* 运行状态 0开始 */
	UWB_IDLE 			= 0,	// 0=空闲
	UWB_SEND_POLL,				// 发送Poll帧
	UWB_SEND_RESP,				// 发送Resp帧
	UWB_SEND_FINAL,				// 发送Final帧
	UWB_WAITING_POLL,			// 等待Poll帧
	UWB_WAITING_RESP,			// 等待Resp帧
	UWB_WAITING_FINAL,			// 等待Final帧
	UWB_RECV_POLL,				// 接收Poll帧
	UWB_RECV_RESP,				// 接收Resp帧
	UWB_RECV_FINAL,				// 接收Final帧
	/* 结束状态 200开始 */
	UWB_CALC_DISTANCE	= 200,	// 200=接收完成计算距离
	/* 错误状态 300开始 */
	UWB_TX_TIMEOUT		= 300,	// 300=发送超时
	UWB_RX_TIMEOUT,				// 301=接收超时
	UWB_TX_ERROR,				// 302=发送错误
	UWB_RX_ERROR,				// 303=接收错误
} UwbCommState_e;

/* TWR测距状态 */
typedef enum
{
	UWB_TWR_NONE		= 0,	// 0=未测距
	UWB_TWR_OK			= 1,	// 1=测距成功
	UWB_TWR_ERR			= 2		// 2=测距错误
} UwbTwr_e;

/* 时间戳状态 */
typedef union
{
	struct
	{
		uint8_t PollSendValid:1;	//b0 Poll发送时间戳有效
		uint8_t PollRecvValid:1;	//b1 Poll接收时间戳有效
		uint8_t RespSendValid:1;	//b2 Resp发送时间戳有效
		uint8_t RespRecvValid:1;	//b3 Resp接收时间戳有效
		uint8_t :2;					//b4-b5 预留
		uint8_t RssiValid:1;		//b6 信号质量有效
		uint8_t DistanceValid:1;	//b7 测距有效
	} bit;
	uint8_t all;
} UwbTimestampSta_u;

/* 角色模式定义 */
#define MODE_ANCHOR_MASTER		0		//主基站模式
#define MODE_ANCHOR_SLAVE		1		//从基站模式
#define MODE_TAG				2		//标签模式

/* UWB参数 */
typedef struct
{
	uint8_t			InstanceMode;	//角色模式
	UwbCommState_e	CommState;		//通信状态
	uint16_t		PanID;			//网络ID
	uint16_t		DevID;			//设备ID
	uint16_t		DestID;			//目标ID
	uint8_t			SlotOrder;		//slot编号
	uint16_t		BatteryVol;		//电池电量
	uint8_t			FrameSeq;		//帧序号
	
	uint16_t		RecvSrcID;		//接收源ID
	uint16_t		RecvDstID;		//接收目标ID

	uint32_t		NextPeriod_ms;	//下一周期时间
	uint16_t		RxIrqTimeout_ms;//接收中断超时
	uint16_t		RecvTimeout_ms;	//接收超时时间
	
	/* 无线通信参数 */
	struct
	{
		uint8_t ChNum;				//信道号
		uint8_t PrfNum;				//Prf号
		uint8_t DataRate;			//通信速率
		uint8_t	OneSlotTime_ms;		//一个slot的时间，根据通信速率不同而不同，单位ms
		uint8_t RetreatCnt;			//退避次数
	} AirCommArg;
	
	/* 测距状态数据 */
	struct
	{
		UwbTwr_e			TwrStatus;		//测距状态
		UwbTimestampSta_u	TsStatus;		//时间戳状态
		/* 时间戳 */
		uint32_t			u32PollSendTs;	//时间戳 64位，实际使用低40位
		uint32_t			u32PollRecvTs;
		uint32_t			u32RespSendTs;
		uint32_t			u32RespRecvTs;
		double				dDistance_m;	//距离
		float				fRSSI;			//信号强度
	} TwrCommArg;
	
} UwbArgs_t;

/* 数据帧类型 */
#define POLL_FRAME		0x01
#define RESP_FRAME		0x02
#define FINAL_FRAME		0x03
#define BEACON_FRAME	0x04

/* ==================== Ranging Result ==================== */

/* ==================== Variable Declarations ==================== */
extern UwbArgs_t	g_UwbInstance;
extern uint8_t		g_UwbTxBuf[UWBTXBUF_SIZE];
extern uint8_t		g_UwbRxBuf[UWBRXBUF_SIZE];
extern uint16_t	g_UwbRxLen;
extern uint32_t	g_UwbRxIrqTimer;

/* ==================== Function Declarations ==================== */

UWBResult_t Ranging_Init(UwbArgs_t *UArg, uint8_t SetInstanceMode, uint16_t SetDevID, uint16_t SetPanID, dwt_config_t *ptrDwtConfig);
UWBResult_t Ranging_Process(UwbArgs_t *UArg);

/* 自定义数据设置/获取函数 */

#endif /* __UWB_RANGING_H */
