#ifndef __BSP_H
#define __BSP_H

/* STM32HAL库支持 */
#include "stm32l4xx_hal.h"

/* 自定义函数库支持 */
#include "g_func.h"

/* BSP支持 */
#include "BSP_uart.h"
#include "BSP_gpio.h"
#include "BSP_led.h"
#include "BSP_ringbuf.h"
#include "BSP_timer.h"
#include "BSP_DMA.h"
#include "BSP_spi.h"
#include "BSP_qspi.h"
//#include "BSP_can.h"
//#include "BSP_fmc_fsmc.h"
#include "BSP_i2c.h"
//#include "BSP_debug.h"
#include "BSP_softi2c.h"
//#include "BSP_MPU.h"
#include "BSP_watchdog.h"
#include "BSP_adc.h"
#include "BSP_button.h"
#include "BSP_Store.h"
#include "BSP_Voice.h"
#include "BSP_EEProm.h"
/* c标准库支持 */
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "stdbool.h"
#include "app_cmd.h"

/* 配置功能定义 */
#define PRT_DEBUG			//打印串口调试信息 UART8
//#define RTOS_SUPPORT		//支持RTOS 

#ifdef PRT_DEBUG
	#ifdef RTOS_SUPPORT
		#define debug_printf(format, args...)	do { \
														taskENTER_CRITICAL(); \
														printf(format, ##args); \
														taskEXIT_CRITICAL(); } while (0)
		#define debug_msg(format, args...)		do { \
														taskENTER_CRITICAL();	\
														printf("[%s] : "format, __FUNCTION__, ##args);	\
														taskEXIT_CRITICAL(); } while (0)
	#else
		#define debug_printf(format, args...)	printf(format, ##args);
		#define debug_msg(format, args...)		printf("[%s] : "format, __FUNCTION__, ##args);
	#endif
#else
	#define debug_printf(format, args...)	;
	#define debug_msg(format, args...)		;
#endif
												
/********* 外部变量声明 **********/

/********* 外部函数声明 *********/
/* 系统时钟配置 */
void SystemPower_Config(void);
void SystemClock_Config(void);
void PeripheralsClock_Config(void);

/* 板级初始化 */												
void BSPInit(void);
#endif

