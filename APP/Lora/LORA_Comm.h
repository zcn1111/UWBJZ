/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_COMM_H
#define __LORA_COMM_H

#include "stdint.h"
#include "BSP_timer.h"
//#include "stm32l4xx.h"
#include "RYDW_Def.h"

/******* 选择Lora版本 *******/
#define LORA_USE_E22x	0
#define LORA_USE_RA01S	1
#define LORA_USE_E28x	0
#define LORA_RADIO_CONFIG	1		//可配置Lora参数

/******* 缓存宏定义 *******/

#define LORA_TX_BUFSIZE		1024	//发送缓存大小
#define LORA_RX_BUFSIZE		1024	//接收缓存大小
#define LORA_PKT_SIZE		255		//数据包大小

/******* 宏定义 *******/
/* 数据包头 */
#define LORA_HEADER				0xB1
#define LORA_DOWNSTREAM			0x01
#define LORA_UPSTREAM			0x02

/* 数据类型 */
/* 基站广播和索要 */
#define LORA_TYPEB_DIFFINFO		0x00	//广播差分数据
#define LORA_TYPEB_EFENCE		0x01	//广播电子围栏
#define LORA_TYPEB_WORKTIME		0x02	//广播施工时间
#define LORA_TYPEB_LEDSETUP		0x04	//广播指示灯配置

#define LORA_TYPEP_ASKEFENCE	0x03	//点对点索要电子围栏

/* 定位卡上传 */
#define LORA_TYPEU_POSINFO		0x00	//上传定位数据
#define LORA_TYPEU_WORKTIME		0x02	//上传施工时间
#define LORA_TYPEU_REQP2P		0x03	//申请点对点传输
#define LORA_TYPEU_LEDSETUP		0x04	//上传指示灯设置

#define LORA_TYPEP_EFENCE		0x81	//点对点上传电子围栏

/******* 返回状态定义 *******/
typedef enum
{
	LORARET_OK				= 0,	/* 0 = 成功 */
	LORARET_MEM_FAIL		= 1,	/* 1 = 申请内存失败 */
	LORARET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	LORARET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	LORARET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	LORARET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	LORARET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	LORARET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	LORARET_TYPE_ERR		= 8,	/* 8 = 类型错误 */
	LORARET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	LORARET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
} LoraResult;

/******* LORA参数定义 *******/
typedef struct
{
	uint8_t *pTxBuf;		//发送缓存指针
	uint16_t ucTxBufPos;	//发送缓存当前位置
	uint8_t TxCpltFlag;		//发送完成标记 在发送完成回调中处理 0=未发送;1=发送中;2=已发送;3=发送超时错误;0xFF=其他错误

	uint8_t *pRxBuf;		//接收缓存指针
	uint16_t ucRxBufPos;	//接收缓存当前位置
	uint8_t RxCpltFlag;		//接收完成标记 在接收完成回调中处理 0=未接收;1=接收中;2=已接收;3=接收超时错误;0xFF=其他错误
	
	uint8_t	*pPktBuf;		//数据包缓存指针
	
	uint8_t	TotalPktNum;	//分包总包数
	uint8_t	PktOrder;		//包序号
	uint8_t RetryCnt;		//重试次数
} LoraPara_t;

/* LORA信号参数 */
typedef struct
{
	int16_t	Rssi;
	int8_t	SNR;
} LoraSignal_t;

/* LORA无线配置参数 */
#pragma pack(push, 1)	//按1字节对齐
typedef struct
{
	uint32_t	Frequency;		//中心频率(Hz) 410MHz~528MHz
	uint8_t		Bandwidth;		//带宽(Hz)	0=125KHz;1=250KHz;2=500KHz
	uint8_t		SpreadingFactor;//分频因子	7~12
	uint8_t		CodingRate;		//编码率		1= 4/5;[2= 4/6];3= 4/7;4= 4/8
	uint8_t		TxPower;		//发送功率	3~5dBm 不能高于5dBm	
	uint32_t	TxTimeout;		//发送超时	ms
	uint16_t	Crc16;
} LoraRadioPara_t;
#pragma pack(pop)		//对齐恢复

#endif /* __LORA_COMM_H */
