/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: BLE_Boardcast.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 蓝牙广播
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "DeviceDrvConf.h"

#include "BLE_Boardcast.h"
#include "BLE_Comm.h"

//#include "Drv_W25Qxx.h"

#include "string.h"


/********* 变量定义 **********/

uint8_t g_BLEBoardcastBuf[64];

TickTimerPara_t tmrBLEBoardcastTimer;	//蓝牙广播定时器

/********* 蓝牙AT指令 **********/
//const char *BleCmd_SetBaud		="AT+BAUD=115200\r\n";			//配置波特率115200
//const char *BleCmd_SetPswd		="AT+PASSWORD=12589490\r\n";	//配置密码
//const char *BleCmd_SetRole		="AT+ROLE=0\r\n";				//配置角色=从模式
//const char *BleCmd_SetName		="AT+NAME=CZ4_SXSH\r\n";		//配置蓝牙名称
//const char *BleCmd_SetMaxconn	="AT+MAXCONN=7\r\n";			//配置最大链接数=7

/******************************************************************
函数名称: BLEResult BLE_BoardCastProcess
功能描述: 蓝牙广播
参数输入: BLEDataPara_t	*BleData	蓝牙数据参数
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_BoardCastProcess(BLEDataPara_t *BleData)
{
	HAL_StatusTypeDef hret;

	BLEResult ret;
	
	/* 发送蓝牙广播数据 */
	memset(g_BLEBoardcastBuf, 0x00, sizeof(g_BLEBoardcastBuf));
	BLE_GenerateBoardcast(BleData, g_BLEBoardcastBuf);
	vTaskSuspendAll();
	hret = HAL_UART_Transmit(&hUSART1, g_BLEBoardcastBuf, 40, 1000);	//发送
	xTaskResumeAll();
	if (hret == HAL_OK)
	{
		ret = BERET_OK;
	}
	else
	{
		ret = BERET_SEND_FAIL;
	}
	
	return ret;
}


