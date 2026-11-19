#include "main.h"
#include "BSP_can.h"

/******** CAN句柄变量 ********/
FDCAN_HandleTypeDef hFDCAN1;

#if 0
/******************************************************************
函数名称: void Can1Init(void)
功能描述: CAN1初始化
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void Can1Init(void)
{
	FDCAN_FilterTypeDef filter;	//过滤器配置
	
	/* 初始化CAN总线 */
	hFDCAN1.Instance = FDCAN1;
	hFDCAN1.Init.Prescaler = 6;					//CAN波特率		=PCLK1 / ((SJW + BS1 + BS2) * Prescaler) = 42MHz / ((1+7+6) * 6) = 0.5Mhz
	hFDCAN1.Init.Mode = CAN_MODE_NORMAL;		//CAN模式		=普通模式
	hFDCAN1.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hFDCAN1.Init.TimeSeg1 = CAN_BS1_7TQ;
	hFDCAN1.Init.TimeSeg2 = CAN_BS2_6TQ;
	hFDCAN1.Init.TimeTriggeredMode = DISABLE;	//时间触发模式	=禁用
	hFDCAN1.Init.AutoBusOff = DISABLE;			//自动关闭总线	=禁用
	hFDCAN1.Init.AutoWakeUp = DISABLE;			//自动唤醒		=禁用
	hFDCAN1.Init.AutoRetransmission = DISABLE;	//自动重发		=禁用
	hFDCAN1.Init.ReceiveFifoLocked = DISABLE;	//接收FIFO锁		=关闭
	hFDCAN1.Init.TransmitFifoPriority = DISABLE;//优先发送FIFO	=关闭
	if (HAL_FDCAN_Init(&hFDCAN1) != HAL_OK)
	{
		Error_Handler();
	}

	/* 初始化过滤器 */
	filter.FilterActivation = ENABLE;				//过滤器		=启用
	filter.FilterMode = CAN_FILTERMODE_IDMASK;		//过滤模式		=掩码模式
	filter.FilterScale = CAN_FILTERSCALE_32BIT;		//过滤位宽		=32位
	filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;	//过滤通道选择	=FIFO0
	filter.FilterBank = 0;
	filter.FilterIdHigh = 0x0000;					//过滤ID		=0
	filter.FilterIdLow = 0x0000;
	filter.FilterMaskIdHigh = 0x0000;				//过滤掩码		=0
	filter.FilterMaskIdLow = 0x0000;

	HAL_CAN_ConfigFilter(&hCAN1, &filter);			//配置过滤器


}
#endif

