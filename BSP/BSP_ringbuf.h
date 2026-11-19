#ifndef __BSP_RINGBUF_H
#define __BSP_RINGBUF_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"


/******* 宏定义 *******/
/* 入队列模式 */
#define RING_PUSHMODE_STOP		0	//队列满后停止入队列
#define RING_PUSHMODE_CYCLE		1	//队列满后丢弃旧数据

/******* 数据结构体定义 *******/
/* 串口环形缓存位置指针 */
typedef struct
{
	uint8_t		*Buf;	//缓存变量指针
	uint16_t	Head;	//头位置
	uint16_t	Tail;	//尾位置
	uint16_t	Size;	//队列长度
} RingBufPos_t;


/******* 外部变量声明 *******/
/* 环形接收缓存 */
extern RingBufPos_t	g_Uart1RxRing;		//UART1接收参数
extern RingBufPos_t	g_Uart2RxRing;		//UART2环形缓存
extern RingBufPos_t	g_Uart3RxRing;		//UART3接收参数
extern RingBufPos_t	g_Uart4RxRing;		//UART4接收参数
extern RingBufPos_t	g_Uart5RxRing;		//UART5接收参数
extern RingBufPos_t	g_LPUart1RxRing;	//LPUART1环形缓存参数


extern uint8_t			g_Uart1RingBuf[UART1_RX_BUFSIZE];	//UART1接收缓存
extern uint8_t			g_Uart2RingBuf[UART2_RX_BUFSIZE];	//UART2接受缓存
extern uint8_t			g_Uart3RingBuf[UART3_RX_BUFSIZE];	//UART3接收缓存
extern uint8_t			g_Uart4RingBuf[UART4_RX_BUFSIZE];	//UART4接收缓存
extern uint8_t			g_Uart5RingBuf[UART5_RX_BUFSIZE];	//UART5接收缓存
extern uint8_t			g_LPUart1RingBuf[LPUART1_RX_BUFSIZE];	//LPUART1接收缓存


/******* 外部函数声明 *******/
/* 初始化 */
void InitRingBuf(RingBufPos_t *Ring, uint16_t Size, uint8_t *Buf);

/* 计算缓存使用长度 */
uint16_t GetRingUsedLen(RingBufPos_t *Ring);
uint16_t GetRingRemainLen(RingBufPos_t *Ring);

/* 环形缓存读写 */
uint16_t RingBufPush(uint8_t *InBuf, uint16_t Size, RingBufPos_t *Ring, uint8_t Mode);
uint16_t RingBufPop(RingBufPos_t *Ring, uint8_t *OutBuf, uint16_t Size);
uint16_t RingBufSeek(RingBufPos_t *Ring, uint8_t *OutBuf, uint16_t Size);
void RingBufClear(RingBufPos_t *Ring);

#endif

