/**
  **********************************************************************************
  * @file      ebyte_callback.h
  * @brief     EBYTE驱动库的收发完成回调函数 由客户实现自己的逻辑代码 
  * @details   详情请参见 https://www.ebyte.com/       
  * @author    JiangHeng     
  * @date      2021-05-13     
  * @version   1.0.0     
  **********************************************************************************
  * @copyright BSD License 
  *            成都亿佰特电子科技有限公司  
  *   ______   ____   __     __  _______   ______ 
  *  |  ____| |  _ \  \ \   / / |__   __| |  ____|
  *  | |__    | |_) |  \ \_/ /     | |    | |__   
  *  |  __|   |  _ <    \   /      | |    |  __|  
  *  | |____  | |_) |    | |       | |    | |____ 
  *  |______| |____/     |_|       |_|    |______| 
  *
  **********************************************************************************
  */
#include "ebyte_callback.h"
#include "ebyte_e22x.h"

/*= !!!配置目标硬件平台头文件 =======================================*/
#include "stm32l4xx.h"
/*= !!!配置目标硬件变量       =======================================*/
#include "LORA_Comm.h"
#include "BSP_debug.h"
#include "string.h"
/*==================================================================*/



/* !
 * @brief 发送完成回调接口 由客户实现自己的发送完成逻辑
 * 
 * @param state 上层回调提供的状态码 客户请根据示例注释找到对应区域
 *  
 * @note E22x 模块可提供的状态码
 *         IRQ_TX_DONE                             = 0x0001,
 *         IRQ_RX_DONE                             = 0x0002,
 *         IRQ_PREAMBLE_DETECTED                   = 0x0004,
 *         IRQ_SYNCWORD_VALID                      = 0x0008,
 *         IRQ_HEADER_VALID                        = 0x0010,
 *         IRQ_HEADER_ERROR                        = 0x0020,
 *         IRQ_CRC_ERROR                           = 0x0040,
 *         IRQ_CAD_DONE                            = 0x0080,
 *         IRQ_CAD_ACTIVITY_DETECTED               = 0x0100,
 *         IRQ_RX_TX_TIMEOUT                       = 0x0200, 
 */
void Ebyte_Port_TransmitCallback( uint16e_t state )
{       
    /* 发送: 正常完成 */
    if( state &= 0x0001 )
    {
        //To-do 实现自己的逻辑
		
		g_LoraPara.TxCpltFlag = 2;		//标记发送完成
		if (g_DbgEcho.EnableLoraEcho == 1) debug_info(g_LoraPara.pTxBuf, g_LoraPara.ucTxBufPos, "->LORA");
		else if (g_DbgEcho.EnableLoraEcho == 2) debug_ascinfo(g_LoraPara.pTxBuf, g_LoraPara.ucTxBufPos, "->LORA", 0);
    }
    /* 发送: 异常超时 */
    else if ( state &= 0x0200 )
    {
        //To-do 实现自己的逻辑
		g_LoraPara.TxCpltFlag = 3;		//标记发送超时
		debug_printf("Lora TX Timeout!\r\n");
    }
    /* 发送: 未知错误 */
    else
    {
        /* 发送状态标识不正确，请检查软硬件  
           常见问题 1:SPI通信不正确 2:模块供电不足 */
		g_LoraPara.TxCpltFlag = 0xFF;	//发送错误停止发送
		debug_printf("Lora TX Err!\r\n");
//        while(1);
    }
}

/* !
 * @brief 接收完成回调接口 由客户实现自己的发送完成逻辑
 * 
 * @param state 上层回调提供的状态码 客户请根据示例注释找到对应区域
 * 
 * @note E22x 模块可提供的状态码
 *         IRQ_TX_DONE                             = 0x0001,
 *         IRQ_RX_DONE                             = 0x0002,
 *         IRQ_PREAMBLE_DETECTED                   = 0x0004,
 *         IRQ_SYNCWORD_VALID                      = 0x0008,
 *         IRQ_HEADER_VALID                        = 0x0010,
 *         IRQ_HEADER_ERROR                        = 0x0020,
 *         IRQ_CRC_ERROR                           = 0x0040,
 *         IRQ_CAD_DONE                            = 0x0080,
 *         IRQ_CAD_ACTIVITY_DETECTED               = 0x0100,
 *         IRQ_RX_TX_TIMEOUT                       = 0x0200, 
 */
void Ebyte_Port_ReceiveCallback(  uint16e_t state, uint8e_t *buffer, uint8e_t length )
{
    uint8_t  j,  pcEchoLength;
    uint8_t *p;
	
    /* 接收: 正常 */
    if( state &= 0x0002 )
    {
		//To-do 实现自己的逻辑
		g_LoraPara.RxCpltFlag = 2;		//接收到数据
		HAL_GPIO_WritePin(LORA_LED_PORT, LORA_LED_PIN, GPIO_PIN_RESET);
		
		memset(g_LoraPara.pRxBuf, 0x00, LORA_RX_BUFSIZE);
		memcpy(g_LoraPara.pRxBuf, buffer, length);
		g_LoraPara.ucRxBufPos = length;
		
		if (g_DbgEcho.EnableLoraEcho == 1) debug_info(g_LoraPara.pRxBuf, g_LoraPara.ucRxBufPos, "LORA->");
		else if (g_DbgEcho.EnableLoraEcho == 1) debug_ascinfo(g_LoraPara.pRxBuf, g_LoraPara.ucRxBufPos, "LORA->", 0);
		HAL_GPIO_WritePin(LORA_LED_PORT, LORA_LED_PIN, GPIO_PIN_SET);
    }
    /* 接收: 异常超时 */
    else if ( state &= 0x0200 )
    {
        //To-do 实现自己的逻辑
		g_LoraPara.RxCpltFlag = 3;
		debug_printf("Lora RX Timeout!\r\n");
    }
    /* 接收: 未知错误 */
    else
    {
        /* 发送状态标识不正确，请检查软硬件  
           常见问题 1:SPI通信不正确 2:模块供电不足 */
		g_LoraPara.RxCpltFlag = 0xFF;	//接收错误停止接收
		debug_printf("Lora RX Err!\r\n");
//        while(1);
    }
}