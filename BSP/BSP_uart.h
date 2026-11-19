#ifndef __BSP_UART_H
#define __BSP_UART_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"


/******* 串口数据定义 *******/
#define UART1_RX_BUFSIZE	3072	//RTK接收缓存	4*1024=4KB
#define UART2_RX_BUFSIZE	1024
#define UART3_RX_BUFSIZE	125	//CORS接收缓存	3*1024=3KB
#define UART4_TX_BUFSIZE    2048
#define UART4_RX_BUFSIZE	3072	//DTU接收缓存	2*1024=2KB
#define UART5_RX_BUFSIZE	1500	//				2*1024
#define LPUART1_RX_BUFSIZE	512	//蓝牙接收缓存	1024B
#define UART5_TX_BUF_BUFSIZE     1470

#define Tool485_RX_EN()	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_0,GPIO_PIN_RESET);
#define Tool485_TX_EN()	HAL_GPIO_WritePin(GPIOE,GPIO_PIN_0,GPIO_PIN_SET);

/******* 数据结构体定义 *******/
typedef struct
{
	uint8_t		Lock;
	uint16_t		ucRxChar;
	uint16_t	usRxPos;
	uint16_t	usRxIdleTimeout;
	uint32_t	ulRxTimer;
	uint32_t	*plTickTimer;
	
	uint8_t		*pRxBuf;
} UartPara_t;



/******* 外部变量声明 *******/
/* 串口句柄 */
extern UART_HandleTypeDef hUSART1;
extern UART_HandleTypeDef hUSART2;
extern UART_HandleTypeDef hUSART3;
extern UART_HandleTypeDef hUART4;
extern UART_HandleTypeDef hUART5;
extern UART_HandleTypeDef hLPUART1;


/* 接收缓存 */
extern uint8_t		g_Uart1RxBuf[UART1_RX_BUFSIZE];	//UART1接收缓存 RTK
extern uint8_t		g_Uart2RxBuf[UART2_RX_BUFSIZE];	//UART2接收缓存 USB2Comm
extern uint8_t		g_Uart3RxBuf[UART3_RX_BUFSIZE];	//UART3接收缓存
extern uint8_t		g_Uart4TxBuf[UART4_TX_BUFSIZE];		//UART4接收缓存
extern uint8_t		g_Uart4RxBuf[UART4_RX_BUFSIZE];	//UART4接收缓存 DTU trans
extern uint8_t		g_Uart5RxBuf[UART5_RX_BUFSIZE];	//UART5接收缓存 RTKGPS
extern uint8_t		g_LPUart1RxBuf[LPUART1_RX_BUFSIZE];	//LPUART1接收缓存
extern uint16_t     g_TxBuf5[UART5_TX_BUF_BUFSIZE];		/* 发送缓冲区 */


/* 串口参数 */
extern UartPara_t	g_pUART3;	//串口2参数
extern UartPara_t	g_pUART2;	//串口2参数
extern UartPara_t	g_pUART4;	//串口4参数
extern UartPara_t	g_pUART5;	//串口4参数
extern UartPara_t	g_pLPUART1;	//低功耗串口1参数
extern uint32_t     g_Uart4TxTimer;
extern uint8_t      g_Uart5DirFlag;						//485方向标记 1-接收 2-发送
extern uint32_t     g_Uart5TxTimer;
void BSP_UartSetupPara(UART_HandleTypeDef *UartHandle, uint32_t BaudRate, uint32_t WordLength, uint32_t Parity, uint32_t StopBits);
void BSP_UartBindInstance(UART_HandleTypeDef *UartHandle, USART_TypeDef *Inst);

void BSP_UartParaInit(UartPara_t *Para, uint32_t *TickTimer, uint32_t Timeout, uint8_t *RxBuf);
void BSP_UartRecvByte(UartPara_t *Para);
uint8_t BSP_UartRecvTimeout_Interrupt(UartPara_t *Para);
void UartSend(UART_HandleTypeDef *huart, uint8_t *_ucaBuf, uint16_t _usLen);
#endif

