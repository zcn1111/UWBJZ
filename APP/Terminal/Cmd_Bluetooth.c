/***************************************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称:四型车载
文件名称:Cmd_Sys.c
作    者: gx
版    本: V1.0
完成日期:
功能描述: 终端控制台 蓝牙命令


其他说明:
功能列表:  
历史记录:
1. 日期:
作者:
修改内容:
2. ..
***************************************************************************************/
/* BSP支持 */
#include "BSP.h"
#include "stdio.h"

#include "Cmd_Bluetooth.h"


/******************************************************************
函数名称: int16_t Cmd_BLEControl
功能描述: 蓝牙控制
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_BLEControl(uint8_t Argc, char *Argv[])
{
	if (Argc == 1)
	{
		/* 开启蓝牙 */
		if (strcasecmp(Argv[1], "on") == 0)
		{
			HAL_GPIO_WritePin(BLE_PWR_GPIO_Port, BLE_PWR_Pin, GPIO_PIN_RESET);
			debug_printf("BLE Power ON\r\n");
			CommMode = 1;
		}
		/* 关闭蓝牙 */
		else if (strcasecmp(Argv[1], "off") == 0)
		{
			HAL_GPIO_WritePin(BLE_PWR_GPIO_Port, BLE_PWR_Pin, GPIO_PIN_SET);
			debug_printf("BLE Power OFF\r\n");
			CommMode = 0;
		}
		/* 开启指令模式 */
		else if (strcasecmp(Argv[1], "cmdon") == 0)
		{
//			HAL_UART_DeInit(&hUSART1);
			HAL_GPIO_WritePin(BLE_CMD_GPIO_Port, BLE_CMD_Pin, GPIO_PIN_SET);
			BSP_UartSetupPara(&hUSART1, 9600, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);	//配置串口
			BSP_UartBindInstance(&hUSART1, USART1);
			debug_printf("BLE Enter CMD mode\r\n");
			if (CommMode == 0)
			{
				HAL_GPIO_WritePin(BLE_PWR_GPIO_Port, BLE_PWR_Pin, GPIO_PIN_RESET);
				debug_printf("BLE Power ON\r\n");
				CommMode = 1;
			}
		}
		/* 关闭指令模式 */
		else if (strcasecmp(Argv[1], "cmdoff") == 0)
		{
			HAL_GPIO_WritePin(BLE_CMD_GPIO_Port, BLE_CMD_Pin, GPIO_PIN_RESET);
			BSP_UartSetupPara(&hUSART1, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);	//配置串口
			BSP_UartBindInstance(&hUSART1, USART1);
			debug_printf("BLE Exit CMD mode\r\n");
		}
	}
	return DBGRET_OK;
}

/******************************************************************
函数名称: int16_t Cmd_BLESend
功能描述: 蓝牙控制
参数输入: uint8_t	Argc	参数个数
		  char		*Argv[]	参数表
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
int16_t Cmd_BLESend(uint8_t Argc, char *Argv[])
{
	uint8_t mode;	//数据发送模式 1=ASCII模式;2=Hex模式
	uint8_t *sendbuf;
	uint16_t sendlen;
	uint16_t i, j;
	uint8_t tmp[2];
	
	mode = 0;
	if (Argc == 1)
	{
		debug_printf("Argument illegal\r\n");
		return DBGRET_ERR_ARG_ILLEGAL;
	}
	
	if (Argc == 2)
	{
		if (strcasecmp(Argv[1], "hex") == 0)
		{
			mode = 2;
		}
		else if (strcasecmp(Argv[1], "asc") == 0)
		{
			mode = 1;
		}
		
		if (strcasecmp(Argv[2], "") == 0)
		{
			debug_printf("Argument Less. Cannot send empty.\r\n");
			return DBGRET_ERR_ARG_LESS;
		}
		sendbuf = pvPortMalloc(512);		//申请内存
		/* ASCII模式发送 */
		if (mode == 1)
		{
			memset(sendbuf, 0x00, 512);
			sendlen = strlen(Argv[2]);
			memcpy(sendbuf, Argv[2], sendlen);
		}
		/* HEX格式发送 */
		else if (mode == 2)
		{
			memset(sendbuf, 0x00, 512);
			sendlen = strlen(Argv[2]);
			j = 0;
			for (i=0; i<sendlen; i++)
			{
				tmp[0] = Argv[2][i];
				tmp[1] = Argv[2][i+1];
				sendbuf[j] = Asc2HexInString(tmp);
				i++;
				j++;
				if (j >= 512) break;
			}
			sendlen = j;
		}
		HAL_UART_Transmit(&hUSART1, sendbuf, sendlen, 1000);
		vPortFree(sendbuf);
	}
	
	return DBGRET_OK;
}

