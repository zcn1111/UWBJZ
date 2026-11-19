#include "main.h"
#include "BSP.h"
#include "String.h"

/******** 环形缓存定义 ********/
RingBufPos_t	g_Uart1RxRing;		//UART1环形缓存参数
RingBufPos_t	g_Uart2RxRing;		//UART2环形缓存参数
RingBufPos_t	g_Uart3RxRing;		//UART3环形缓存参数
RingBufPos_t	g_Uart4RxRing;		//UART4环形缓存参数
RingBufPos_t	g_Uart5RxRing;		//UART5环形缓存参数
RingBufPos_t	g_LPUart1RxRing;	//LPUART1环形缓存参数

uint8_t			g_Uart1RingBuf[UART1_RX_BUFSIZE];		//UART1接收缓存
uint8_t			g_Uart2RingBuf[UART2_RX_BUFSIZE];		//UART2接受缓存
uint8_t			g_Uart3RingBuf[UART3_RX_BUFSIZE];		//UART3接收缓存
uint8_t			g_Uart4RingBuf[UART4_RX_BUFSIZE];		//UART4接收缓存
uint8_t			g_Uart5RingBuf[UART5_RX_BUFSIZE];		//UART5接收缓存
uint8_t			g_LPUart1RingBuf[LPUART1_RX_BUFSIZE];	//LPUART1接收缓存
/******************************************************************
函数名称: void InitRingBuf
功能描述: 环形队列初始化
参数输入: uint8_t		*Buf	队列缓存
		  uint16_t		Size	队列大小
参数输出: RingBufPos_t	*Ring	环形队列位置
返 回 值: 
其    他: 无
******************************************************************/
void InitRingBuf(RingBufPos_t *Ring, uint16_t Size, uint8_t *Buf)
{
	uint16_t i;
	
	Ring->Head = 0;
	Ring->Tail = 0;
	Ring->Size = Size;
	if (Buf == NULL)
	{
		Ring->Buf = malloc(Size);		//申请内存
	}
	else
	{
		Ring->Buf = Buf;				//配置内存
	}
	memset(Ring->Buf, 0x00, Size);
}

/******************************************************************
函数名称: uint16_t GetRingUsedLen
功能描述: 获取当前环形缓存已用大小
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint16_t 已用大小
其    他: 无
******************************************************************/
uint16_t GetRingUsedLen(RingBufPos_t *Ring)
{
	uint16_t len;
	
	if (Ring->Head >= Ring->Tail)
	{
		len = Ring->Head - Ring->Tail;
	}
	else
	{
		len = Ring->Size + Ring->Head - Ring->Tail + 1;
	}
//	if(len >2048)
//	{
//		while(1);
//	}
	return len;
}

/******************************************************************
函数名称: uint16_t GetRingRemainLen
功能描述: 获取当前环形缓存剩余大小
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint16_t 剩余大小
其    他: 无
******************************************************************/
uint16_t GetRingRemainLen(RingBufPos_t *Ring)
{
	uint16_t len;
	
	len = Ring->Size - GetRingUsedLen(Ring);
	return len;
}

/******************************************************************
函数名称: uint8_t IsRingFull
功能描述: 环形缓存是否已满
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint8_t 0=未满;1=已满
其    他: 无
******************************************************************/
uint8_t IsRingFull(RingBufPos_t *Ring)
{
	if (Ring->Head == (Ring->Size - 1))
	{
		if (Ring->Tail == 0)
			return 1;
		else
			return 0;
	}
	else if (Ring->Tail == (Ring->Head + 1))
		return 1;
	else
		return 0;
}

/******************************************************************
函数名称: uint8_t IsRingEmpty
功能描述: 环形缓存是否空
参数输入: RingBufPos_t	*Ring	环形队列位置
参数输出: 
返 回 值: uint8_t 0=非空;1=空
其    他: 无
******************************************************************/
uint8_t IsRingEmpty(RingBufPos_t *Ring)
{
	if (Ring->Head == Ring->Tail)
		return 1;
	else
		return 0;
}

/******************************************************************
函数名称: uint16_t RingBufPush
功能描述: 入队列
参数输入: uint8_t		*InBuf		输入缓存
		  uint16_t		Size		缓存大小
		  uint8_t		PushMode	队列模式 0=队列满后停止;1=队列满后放弃旧数据
参数输出: RingBufPos_t	*Ring		环形缓存变量
返 回 值: uint16_t 入队数据大小 0=无数据
其    他: 无
******************************************************************/
uint16_t RingBufPush(uint8_t *InBuf, uint16_t Size, RingBufPos_t *Ring, uint8_t PushMode)
{
	uint16_t remainlen=0;
	uint16_t i;
	
	uint8_t full=0;
	uint8_t willfull=0;
		
	/* 判断队列已满 */
	if (IsRingFull(Ring) == 1)
		full = 1;
	
	/* 判断入队长度 如果不足本次数据入队 置队列即将满 */
	remainlen = GetRingRemainLen(Ring);
	if (Size > remainlen)
		willfull = 1;
	
	/* 数据入队 */
	if (PushMode == RING_PUSHMODE_STOP)			//队列满后停止模式
	{
		if ((full) || (willfull)) return 0;		//队列满或将满后停止入队
	}
	for (i=0; i<Size; i++)
	{
		Ring->Buf[Ring->Head] = InBuf[i];
		Ring->Head++;
		if (IsRingFull(Ring) == 1)
		{
			if (Ring->Head > (Ring->Size - 1))
			{
				Ring->Head = 0;
				Ring->Tail = Ring->Size - 1;
			}
			else
			{
				Ring->Tail = Ring->Head + 1;
			}
		}
		else if (Ring->Head > (Ring->Size - 1)) Ring->Head = 0;
	}
	
	return Size;
}

/******************************************************************
函数名称: uint16_t RingBufPop
功能描述: 出队列
参数输入: RingBufPos_t	*Ring	环形缓存变量
参数输出: uint8_t		*OutBuf	输出缓存
		  uint16_t		Size	出队列长度
返 回 值: uint16_t 出队数据大小 0=无数据
其    他: 无
******************************************************************/
uint16_t RingBufPop(RingBufPos_t *Ring, uint8_t *OutBuf, uint16_t Size)
{
	uint16_t len=0;
	uint16_t i;
	
	/* 判断队列空 */
	if (IsRingEmpty(Ring) == 1)
		return 0;
	
	for (i=0; i<Size; i++)
	{
		OutBuf[i] = Ring->Buf[Ring->Tail];
		Ring->Tail++;
		if (IsRingEmpty(Ring) == 1)
			break;
		if (Ring->Tail > (Ring->Size - 1)) Ring->Tail = 0;
	}
	
	len = (i <= Size) ? i : Size;
	return len;
}

/******************************************************************
函数名称: uint16_t RingBufSeek
功能描述: 获取数据但不出队列
参数输入: RingBufPos_t	*Ring	环形缓存变量
参数输出: uint8_t		*OutBuf	输出缓存
		  uint16_t		Size	获取长度
返 回 值: uint16_t 获取数据大小 0=无数据
其    他: 无
******************************************************************/
uint16_t RingBufSeek(RingBufPos_t *Ring, uint8_t *OutBuf, uint16_t Size)
{
	uint16_t len=0;
	uint16_t i;
	uint16_t head;
	uint16_t tail;
	
	/* 判断队列空 */
	if (IsRingEmpty(Ring) == 1)
		return 0;
	
	head = Ring->Head;
	tail = Ring->Tail;
	for (i=0; i<Size; i++)
	{
		OutBuf[i] = Ring->Buf[tail];
		tail++;
		if (tail == head)		//判断是否队列已经结束
			break;
		if (tail > (Ring->Size - 1)) tail = 0;
	}
	
	len = (i <= Size) ? i : Size;
	return len;
}

/******************************************************************
函数名称: void RingBufClear
功能描述: 清除队列数据
参数输入: RingBufPos_t	*Ring	环形缓存变量
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void RingBufClear(RingBufPos_t *Ring)
{
	memset(Ring->Buf, 0x00, Ring->Size);	//清缓存
	Ring->Head = 0;
	Ring->Tail = 0;
}

