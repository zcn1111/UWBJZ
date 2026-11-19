/**
  ******************************************************************************
  * @file    Templates/Src/stm32l4xx_it.c 
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_it.h"
#include "stm32l4xx_hal.h"

#include "BSP.h"
#include "DTU_BulkData.h"
#include "app_dtu.h"
/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	uint8_t timeout;
	uint32_t difflp1;
	
	HAL_IncTick();
	
	/* SX126x定时器处理 */
//	SX126xTimerProcess();

	/* 判断串口2超时 */
	timeout = BSP_UartRecvTimeout_Interrupt(&g_pUART2);
	if (timeout)
	{
		RingBufPush(g_pUART2.pRxBuf, g_pUART2.usRxPos, &g_Uart2RxRing, RING_PUSHMODE_CYCLE);
		g_pUART2.usRxPos = 0;
		memset(g_pUART2.pRxBuf, 0x00, UART2_RX_BUFSIZE);
	}
		
//	/* 判断串口4超时 */
//	timeout = BSP_UartRecvTimeout_Interrupt(&g_pUART4);
//	if (timeout)
//	{
//		RingBufPush(g_pUART4.pRxBuf, g_pUART4.usRxPos, &g_Uart4RxRing, RING_PUSHMODE_CYCLE);
//		g_pUART4.usRxPos = 0;
//		memset(g_pUART4.pRxBuf, 0x00, UART4_RX_BUFSIZE);
//	}
		
//	/* 判断串口5超时 */
//	timeout = BSP_UartRecvTimeout_Interrupt(&g_pUART5);
//	if (timeout)
//	{
//		RingBufPush(g_pUART5.pRxBuf, g_pUART5.usRxPos, &g_Uart5RxRing, RING_PUSHMODE_CYCLE);
//		g_pUART5.usRxPos = 0;
//		memset(g_pUART5.pRxBuf, 0x00, UART5_RX_BUFSIZE);
//	}
	
	/* 判断串口LP1超时 */
	timeout = BSP_UartRecvTimeout_Interrupt(&g_pLPUART1);
	if (timeout)
	{
		RingBufPush(g_pLPUART1.pRxBuf, g_pLPUART1.usRxPos, &g_LPUart1RxRing, RING_PUSHMODE_CYCLE);
		g_pLPUART1.usRxPos = 0;
		memset(g_pLPUART1.pRxBuf, 0x00, LPUART1_RX_BUFSIZE);
	}
}

/******************************************************************************/
/*                 STM32L4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l4xxxx.s).                                             */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/****************************************  串口中断服务函数 ***********************************************************/
void USART1_IRQHandler(void)
{
	uint32_t rxlen;
	
	/* 处理空闲中断 */
	if(__HAL_UART_GET_FLAG(&hUSART1, UART_FLAG_IDLE))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&hUSART1);		//清中断
		HAL_UART_DMAStop(&hUSART1);					//停止DMA
		rxlen = UART1_RX_BUFSIZE - __HAL_DMA_GET_COUNTER(&hDMA_Usart1Rx);	//计算接收字节数
		
		if (rxlen > 0)
		{
			RingBufPush(g_Uart1RxBuf, rxlen, &g_Uart1RxRing, RING_PUSHMODE_CYCLE);	//数据保存到环形Buf
		}
	}

	HAL_UART_IRQHandler(&hUSART1);
	memset(g_Uart1RxBuf, 0x00, UART1_RX_BUFSIZE);
	HAL_UART_Receive_DMA(&hUSART1, g_Uart1RxBuf, UART1_RX_BUFSIZE);			//启动接收 数据通过DMA保存在g_Uart1RxBuf中 最多保存UART1_RX_BUFSIZE
}

void USART2_IRQHandler(void)
{
	uint32_t rxlen;
	
	/* 处理中断 */
	HAL_UART_IRQHandler(&hUSART2);
	HAL_UART_Receive_IT(&hUSART2, (uint8_t*)&g_pUART2.ucRxChar, 1);
}

void USART3_IRQHandler(void)
{
	uint32_t rxlen;
	
	/* 处理空闲中断 */
	if(__HAL_UART_GET_FLAG(&hUSART3, UART_FLAG_IDLE))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&hUSART3);		//清中断
		HAL_UART_DMAStop(&hUSART3);					//停止DMA
		rxlen = UART3_RX_BUFSIZE - __HAL_DMA_GET_COUNTER(&hDMA_Usart3Rx);	//计算接收字节数
		
		if (rxlen > 0)
		{
			RingBufPush(g_Uart3RxBuf, rxlen, &g_Uart3RxRing, RING_PUSHMODE_CYCLE);	//数据保存到环形Buf
		}
	}

	HAL_UART_IRQHandler(&hUSART3);
	memset(g_Uart3RxBuf, 0x00, UART3_RX_BUFSIZE);
	HAL_UART_Receive_DMA(&hUSART3, g_Uart3RxBuf, UART3_RX_BUFSIZE);			//启动接收 数据通过DMA保存在g_Uart3RxBuf中 最多保存UART3_RX_BUFSIZE
	
}

void UART4_IRQHandler(void)
{
	uint32_t rxlen;

	/* 处理空闲中断 */
	if(__HAL_UART_GET_FLAG(&hUART4, UART_FLAG_IDLE))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&hUART4);		//清中断
		HAL_UART_DMAStop(&hUART4);					//停止DMA
		rxlen = UART4_RX_BUFSIZE - __HAL_DMA_GET_COUNTER(&hDMA_Uart4Rx);	//计算接收字节数
		
		if (rxlen > 0)
		{
			g_Uart4TxTimer = uwTick;	
			RingBufPush(g_Uart4RxBuf, rxlen, &g_Uart4RxRing, RING_PUSHMODE_CYCLE);	//数据保存到环形Buf
		}
	}

	HAL_UART_IRQHandler(&hUART4);
	memset(g_Uart4RxBuf, 0x00, UART4_RX_BUFSIZE);
	HAL_UART_Receive_DMA(&hUART4, g_Uart4RxBuf, UART4_RX_BUFSIZE);			//启动接收 数据通过DMA保存在g_Uart3RxBuf中 最多保存UART3_RX_BUFSIZE	
//	/* 处理中断 */
//	HAL_UART_IRQHandler(&hUART4);
//	HAL_UART_Receive_IT(&hUART4, (uint8_t*)&g_pUART4.ucRxChar, 1);
}

void UART5_IRQHandler(void)
{
	uint32_t rxlen;

	/* 处理空闲中断 */
	if(__HAL_UART_GET_FLAG(&hUART5, UART_FLAG_IDLE))
	{ 
		__HAL_UART_CLEAR_IDLEFLAG(&hUART5);		//清中断
		HAL_UART_DMAStop(&hUART5);					//停止DMA
		rxlen = UART5_RX_BUFSIZE - __HAL_DMA_GET_COUNTER(&hDMA_Uart5Rx);	//计算接收字节数
		
		if (rxlen > 0)
		{
			RingBufPush(g_Uart5RxBuf, rxlen, &g_Uart5RxRing, RING_PUSHMODE_CYCLE);	//数据保存到环形Buf
		}
	}

	HAL_UART_IRQHandler(&hUART5);
	memset(g_Uart5RxBuf, 0x00, UART5_RX_BUFSIZE);
	HAL_UART_Receive_DMA(&hUART5, g_Uart5RxBuf, UART5_RX_BUFSIZE);			//启动接收 数据通过DMA保存在g_Uart3RxBuf中 最多保存UART3_RX_BUFSIZE
		
//	/* 处理中断 */
//	HAL_UART_IRQHandler(&hUART5);
//	HAL_UART_Receive_IT(&hUART5, (uint8_t*)&g_pUART5.ucRxChar, 1);
}

void LPUART1_IRQHandler(void)
{
	HAL_UART_IRQHandler(&hLPUART1);
	HAL_UART_Receive_IT(&hLPUART1, (uint8_t*)&g_pLPUART1.ucRxChar, 1);
}


/****************************************  定时器中断服务函数 ***********************************************************/

/* 定时器TIM7 中断 */
void TIM7_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&hTIM7);
}

/* 定时器TIM6 中断 */
void TIM6_DAC_IRQHandler(void)
{
	
	HAL_TIM_IRQHandler(&hTIM6);
	
}

/****************************************  外部中断服务函数 ***********************************************************/
void EXTI9_5_IRQHandler(void)
{
	/* 处理Lora中断 */
	HAL_GPIO_EXTI_IRQHandler(LORA_DIO1_PIN);
	HAL_GPIO_EXTI_IRQHandler(KEY3_OK_PIN);
	if (HAL_GPIO_ReadPin(LORA_DIO1_PORT, LORA_DIO1_PIN) == SET)
	{
		
	}
}


/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
