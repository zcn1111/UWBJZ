/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TERMINAL_CONF_H
#define __TERMINAL_CONF_H

/* 引用头文件 */
#include "stm32l4xx_hal.h"
#define DTU_CMD
#define RYDW_CMD

/******* 头文件配置 *******/
#include "Cmd_Internal.h"
//#include "Cmd_Nand.h"
#include "Cmd_SpiFlash.h"
#ifdef FATFS_CMD
	#include "Cmd_Fatfs.h"
#endif
#ifdef FLASHFS_CMD
	#include "Cmd_FlashFS.h"
#endif
#ifdef RTOS_CMD
	#include "Cmd_Sys.h"
#endif

#ifdef NET_CMD
	#include "Cmd_Network.h"
#endif

#ifdef BLE_CMD
	#include "Cmd_Bluetooth.h"
#endif

#ifdef DTU_CMD
	#include "Cmd_DTU.h"
#endif

#ifdef RYDW_CMD
	#include "Cmd_RYDW.h"
#endif
/******* 数据结构体定义 *******/

/******* 外部变量声明 *******/

/******* 外部函数声明 *******/


#endif /* __TERMINAL_CONF_H */
