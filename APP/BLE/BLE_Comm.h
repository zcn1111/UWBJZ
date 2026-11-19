#ifndef __BLE_COMM_H
#define __BLE_COMM_H

/* 引用头文件 */
#include "stm32l4xx.h"

#include "BSP.h"
#include "RYDW_StationCard.h"
#include "g_GlobalValDef.h"

/******* 宏定义 *******/

#define BLE_BUFSIZE		1024

/* 类型码 */
#define BLE_DOWNSTREAM		0x00	//手机下发数据
#define BLE_UPSTREAM		0x01	//基站应答数据
/* 数据类型 */
#define BLE_TYPE_EFENCE		0x01	//电子围栏数据
#define BLE_TYPE_WORKING	0x02	//上下道数据
#define BLE_TYPE_TRAINING	0x03	//来车数据
#define BLE_TYPE_CONSTRTIME	0x04	//施工时间

/******* 状态定义 *******/
typedef enum
{
	BLERET_OK				= 0,	/* 0 = 成功 */
	BLERET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	BLERET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	BLERET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	BLERET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	BLERET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	BLERET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	BLERET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
	BLERET_NOTMATCH_ERR		= 11,	/* 11 = 不匹配 */
} BLEResult;

/******* 数据结构体定义 *******/
/* 蓝牙通信参数 */
#pragma pack(push, 1)	//按1字节对齐
//typedef struct
//{
//} BLEDataPara_t;
#pragma pack(pop)		//对齐恢复

/* 新增蓝牙协议 */


/******* 外部变量声明 *******/
//extern BLEDataPara_t	g_BLEDataPara;


/******* 外部函数声明 *******/
BLEResult BLE_InitModule(void);

BLEResult BLE_InitModulePara(void);

BLEResult BLE_SendBuf(uint8_t *InBuf, uint16_t Len);
BLEResult BLE_RecvBuf(uint8_t *RxBuf, uint16_t *DatLen);

BLEResult BLE_RecvBufBySum(uint8_t *OutBuf, uint16_t *DatLen);
BLEResult BLE_RecvBufDirectly(uint8_t *OutBuf, uint16_t *DatLen);


#endif /* __BLE_COMM_H */

