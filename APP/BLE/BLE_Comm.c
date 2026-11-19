/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: BLE_Comm.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 蓝牙通信
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"

#include "BLE_Comm.h"
#include "BLE_ATCmd.h"

#include "RYDW_Def.h"
#include "RYDW_StationCard.h"
//#include "Drv_W25Qxx.h"

#include "string.h"


/********* 变量定义 **********/

//BLEDataPara_t	g_BLEDataPara;
uint8_t			*BleDataBuf;

uint8_t			g_BLEDataBuf[BLE_BUFSIZE];

/********* 蓝牙AT指令 **********/
const char *BleCmd_SetSvrUUID	="AT+BLESPPSERVICE=f000\r\n";	//配置服务UUID=0xF000
const char *BleCmd_SetTxUUID	="AT+BLECHARACTERTXD=f002\r\n";	//配置发送UUID=0xF002
const char *BleCmd_SetRxUUID	="AT+BLECHARACTERRXD=f001\r\n";	//配置接收UUID=0xF001

const char *BleCmd_SetBaud		="AT+BAUD=115200\r\n";			//配置波特率115200
const char *BleCmd_SetAuth		="AT+AUTH=0\r\n";				//配置鉴权=无
const char *BleCmd_SetPswd		="AT+PASSWORD=000000\r\n";		//配置密码
const char *BleCmd_SetRole		="AT+ROLE=0\r\n";				//配置角色=主模式
const char *BleCmd_SetName		="AT+NAME=";	//配置蓝牙名称

const char *BleCmd_QrySvrUUID	="AT+BLESPPSERVICE?\r\n";		//查询数据UUID
const char *BleCmd_QryTxUUID	="AT+BLECHARACTERTXD?\r\n";		//查询发送UUID
const char *BleCmd_QryRxUUID	="AT+BLECHARACTERRXD?\r\n";		//查询接收UUID

const char *BleCmd_QryBaud		="AT+BAUD?\r\n";				//查询波特率
const char *BleCmd_QryPswd		="AT+PASSWORD?\r\n";			//查询密码
const char *BleCmd_QryRole		="AT+ROLE?\r\n";				//查询角色
const char *BleCmd_QryName		="AT+NAME?\r\n";				//查询名称
const char *BleCmd_QryMaxconn	="AT+MAXCONN?\r\n";				//查询最大连接数

const char *BleCmd_ClearAllAddr="AT+CLEARADDR\r\n";			//清除所有已记忆地址

/*连接车载蓝牙--鉴权*/




/************************************** 蓝牙收发函数 ************************************************/

/******************************************************************
函数名称: BLEResult BLE_SendBuf
功能描述: 发送蓝牙数据
参数输入: uint8_t	*InBuf	输入缓存
		  uint16_t	Len		数据长度
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_SendBuf(uint8_t *InBuf, uint16_t Len)
{
	uint16_t i;
	HAL_StatusTypeDef hret;
	
	BLEResult ret;
	
	/* 发送数据 */
	hret = HAL_UART_Transmit(&hLPUART1, InBuf, Len, 1000);
	if (hret == HAL_OK)
	{
		ret = BLERET_OK;
		if (g_DbgEcho.EnableBleEcho == 1) debug_info(InBuf, Len, "->BLE");
		else if (g_DbgEcho.EnableBleEcho == 2) debug_ascinfo(InBuf, Len, "->BLE", 0);
	}
	else
	{
		ret = BLERET_SEND_FAIL;
	}

	return ret;
}

/******************************************************************
函数名称: BLEResult BLE_RecvBuf
功能描述: 接收蓝牙数据
参数输入: 
参数输出: uint8_T	*RxBuf	蓝牙接收缓存
		  uint16_t	*DatLen	蓝牙接收数据长度
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_RecvBuf(uint8_t *RxBuf, uint16_t *DatLen)
{
	uint16_t i;
	uint16_t size=0;
	uint8_t	sum;
	
	BLEResult ret;
	
	/* 接收环形Buf数据 */
	size = GetRingUsedLen(&g_LPUart1RxRing);
	if (size == 0)
	{
		*DatLen = 0;
		return BLERET_NODATA_ERR;
	}

	RingBufPop(&g_LPUart1RxRing, RxBuf, size);
	if (g_DbgEcho.EnableBleEcho == 1) debug_info(RxBuf, size, "BLE->");
	else if (g_DbgEcho.EnableBleEcho == 2) debug_ascinfo(RxBuf, size, "BLE->", 0);
	*DatLen = size;
	if (CommMode == 1)
	{
		printf("%s", RxBuf);	//给串口转发蓝牙数据
	}
	else if (CommMode == 2)
	{
		printf("%s", RxBuf);
	}

	return BLERET_OK;
}



/******************************************************************
函数名称: BLEResult BLE_SetupCommand
功能描述: 配置蓝牙命令
参数输入: char		*SetupCmd	发送命令
		 char		*SuccRetStr	成功返回字符
		 uint8_t	RetryCnt	重试次数
		 uint16_t	Timeout		超时时间(ms)
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
BLEResult BLE_SetupCommand(char *SetupCmd, char *SuccRetStr, uint8_t RetryCnt, uint16_t Timeout)
{
	uint8_t rcnt;
	uint16_t slen;
	uint16_t recvlen;
	uint32_t tmrrecv;
	
	BLEResult bleret;
	
	rcnt = 0;
	for (rcnt=0; rcnt<=RetryCnt; rcnt++)
	{
		/* 发送命令 */
		tmrrecv = uwTick;
		slen = strlen(SetupCmd);
		BLE_SendBuf((uint8_t*)SetupCmd, slen);
		printf("[>>BLE] %s", SetupCmd);
		memset(g_BLEDataBuf, 0x00, BLE_BUFSIZE);
		
		/* 返回接收值 */
		memset(g_BLEDataBuf, 0x00, BLE_BUFSIZE);
		RingBufClear(&g_LPUart1RxRing);
		while ((uwTick - tmrrecv) < Timeout)
		{
			bleret = BLE_RecvBuf(g_BLEDataBuf, &recvlen);
			if (bleret == BLERET_OK)
			{
				printf("[BLE>>] %s", g_BLEDataBuf);
				if (strstr((const char*)g_BLEDataBuf, SuccRetStr) != NULL)
				{
					/* 匹配到结果字符串 */
					return BLERET_OK;
				}
			}
		}
	}
		
	return BLERET_TIMEOUT_ERR;
}

/******************************************************************
函数名称: BLEResult BLE_EntrySetupMode
功能描述: 进入配置模式
参数输入: 
参数输出: 
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_EntrySetupMode(void)
{
	/* 退出休眠状态 */
	HAL_GPIO_WritePin(BT_HOLDWAKE_PORT, BT_HOLDWAKE_PIN, GPIO_PIN_SET);
	
	/* 配置串口参数 */
	HAL_GPIO_WritePin(BT_MODESW_PORT, BT_MODESW_PIN, GPIO_PIN_SET);								//开启蓝牙配置模式
	BSP_UartSetupPara(&hLPUART1, 9600, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);	//配置串口参数
	BSP_UartBindInstance(&hLPUART1, LPUART1);
	
	return BLERET_OK;
}

/******************************************************************
函数名称: BLEResult BLE_ExitSetupMode
功能描述: 退出配置模式
参数输入: 
参数输出: 
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_ExitSetupMode(void)
{
	/* 退出蓝牙配置模式 */
	HAL_GPIO_WritePin(BT_MODESW_PORT, BT_MODESW_PIN, GPIO_PIN_RESET);
	BSP_UartSetupPara(&hLPUART1, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);	//配置串口参数
	BSP_UartBindInstance(&hLPUART1, LPUART1);
	
	return BLERET_OK;
}

/************************************** 蓝牙通信函数 ************************************************/

/******************************************************************
函数名称: BLEResult BLE_InitModulePara
功能描述: 初始化蓝牙模块参数
参数输入: 
参数输出: 
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_InitModule(void)
{
	BLEResult bleret;
	uint8_t cmpipbuf[2][EEP_UNIT_SIZE];												
	memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
	memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));	
	/* 进入配置模式 */
	BLE_EntrySetupMode();
	
	/* 配置UUID */
	/* 查询配置Service UUID */
	bleret = BLE_SetupCommand((char*)BleCmd_QrySvrUUID, "+BLESPPSERVICE:f000", 3, 2000);
	if (bleret != BLERET_OK)
	{
		bleret = BLE_SetupCommand((char*)BleCmd_SetSvrUUID, "OK", 3, 2000);
		if (bleret != BLERET_OK)
		{
			return BLERET_TIMEOUT_ERR;
		}
	}
	/* 查询配置发送UUID */
	bleret = BLE_SetupCommand((char*)BleCmd_QryTxUUID, "+BLECHARACTERTXD:f002", 3, 2000);
	if (bleret != BLERET_OK)
	{
		bleret = BLE_SetupCommand((char*)BleCmd_SetTxUUID, "OK", 3, 2000);
		if (bleret != BLERET_OK)
		{
			return BLERET_TIMEOUT_ERR;
		}
	}
	/* 查询配置接收UUID */
	bleret = BLE_SetupCommand((char*)BleCmd_QryRxUUID, "+BLECHARACTERRXD:f001", 3, 2000);
	if (bleret != BLERET_OK)
	{
		bleret = BLE_SetupCommand((char*)BleCmd_SetRxUUID, "OK", 3, 2000);
		if (bleret != BLERET_OK)
		{
			return BLERET_TIMEOUT_ERR;
		}
	}
	
	/* 配置模块 */
	/* 查询配置设备名称 */
	bleret = BLE_SetupCommand((char*)BleCmd_QryName, "+NAME:", 3, 2000);
	if (bleret == BLERET_OK)
	{

		if(memcmp(EquipmentNum,cmpipbuf[0],EEP_UNIT_SIZE) == 0 || memcmp(EquipmentNum,cmpipbuf[1],EEP_UNIT_SIZE) == 0)
		{//若没有设备编号,使用ID号
			uint8_t IDnum[14] ;
			uint8_t i;
			for(i=0;i<7;i++)
			{
				Bcd2Asc(g_IDInFlash.IDnum[i],IDnum+2*i);
			}
			if (strstr((const char*)g_BLEDataBuf, (char *)IDnum) == NULL)
			{			
				char BleCmd_SetNameChar[64];
				snprintf(BleCmd_SetNameChar, sizeof(BleCmd_SetNameChar), "%s%s\r\n", BleCmd_SetName,IDnum);	
				bleret = BLE_SetupCommand(BleCmd_SetNameChar, "OK", 3, 2000);		
				if (bleret != BLERET_OK)
				{
					return BLERET_TIMEOUT_ERR;
				}
			}				
		}		
		else
		{//写入设备编号
			if (strstr((const char*)g_BLEDataBuf, (char *)EquipmentNum) == NULL)
			{			
				char BleCmd_SetNameChar[64];
				snprintf(BleCmd_SetNameChar, sizeof(BleCmd_SetNameChar), "%s%s\r\n", BleCmd_SetName,EquipmentNum);	
				bleret = BLE_SetupCommand(BleCmd_SetNameChar, "OK", 3, 2000);		
				if (bleret != BLERET_OK)
				{
					return BLERET_TIMEOUT_ERR;
				}
			}				
		}
	}
	
	/* 查询配置波特率 */
	bleret = BLE_SetupCommand((char*)BleCmd_QryBaud, "+BAUD:115200", 3, 2000);
	if (bleret != BLERET_OK)
	{
		bleret = BLE_SetupCommand((char*)BleCmd_SetBaud, "OK", 3, 2000);
		if (bleret != BLERET_OK)
		{
			return BLERET_TIMEOUT_ERR;
		}
	}
	
	/* 查询设置设备类型 */
	bleret = BLE_SetupCommand((char*)BleCmd_QryRole, "+ROLE:0", 3, 2000);
	if (bleret != BLERET_OK)
	{
		bleret = BLE_SetupCommand((char*)BleCmd_QryRole, "OK", 3, 2000);
	}
	
	
	/* 退出蓝牙配置模式 */
	BLE_ExitSetupMode();
	
	return BLERET_OK;
}

#if 0
/******************************************************************
函数名称: BLEResult BLE_InitModulePara
功能描述: 初始化蓝牙模块参数
参数输入: BLEDataPara_t	*BleData	蓝牙数据
		  uint8_t		Header		数据头
参数输出: uint8_t		*OutBuf		数据输出
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_InitModulePara(void)
{
	uint8_t *sendbuf;
	uint16_t sendlen;
	uint8_t *recvbuf;
	uint16_t recvlen;
	
	BLEResult bleret;

	uint8_t step;
	uint8_t retrycnt;
	char resp[16];
	
	step = 0;
	retrycnt = 3;
	sendbuf = pvPortMalloc(128);
	recvbuf = pvPortMalloc(128);
	if ((sendbuf == NULL) || (recvbuf == NULL))
	{
		return BERET_MEM_FAIL;
	}
	
	HAL_GPIO_WritePin(BLE_PWR_GPIO_Port, BLE_PWR_Pin, GPIO_PIN_RESET);	//开启蓝牙电源
	
	/* 初始化蓝牙串口配置 命令模式=9600bps */
	BSP_UartSetupPara(&hUSART1, 9600, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);	//配置串口句柄
	BSP_UartBindInstance(&hUSART1, USART1);														//绑定串口
	HAL_GPIO_WritePin(BLE_CMD_GPIO_Port, BLE_CMD_Pin, GPIO_PIN_SET);							//设置蓝牙为命令模式
	vTaskDelay(100);
	while ((retrycnt > 0) && (step < 6))
	{
		memset(sendbuf, 0x00, 128);
		memset(recvbuf, 0x00, 128);
		recvlen = 0;
		
		/* 配置蓝牙波特率 115200 */
		if (step == 0)
		{
			sendlen = strlen(BleCmd_QryBaud);
			BLE_SendBuf((uint8_t*)BleCmd_QryBaud, sendlen);
			vTaskDelay(200);
			bleret = BLE_RecvBufDirectly(recvbuf, &recvlen);
			if ((bleret == BERET_OK) && (strstr((char*)recvbuf, "+BAUD:115200") != NULL))	//判断是否返回正常
			{
				/* 返回正常 */
				debug_printf("BLE BAUD OK\r\n");
				retrycnt = 3;
				step = 1;
			}
			else
			{
				/* 没有正常返回 */
				sendlen = strlen(BleCmd_SetBaud);
				BLE_SendBuf((uint8_t*)BleCmd_SetBaud, sendlen);		//配置参数
				retrycnt--;
				vTaskDelay(1000);
			}
		}
		/* 配置蓝牙密码 */
		else if (step == 1)
		{
			sendlen = strlen(BleCmd_QryPswd);
			BLE_SendBuf((uint8_t*)BleCmd_QryPswd, sendlen);
			vTaskDelay(200);
			bleret = BLE_RecvBufDirectly(recvbuf, &recvlen);
			if ((bleret == BERET_OK) && (strstr((char*)recvbuf, "+PASSWORD:12589490") != NULL))	//判断是否返回正常
			{
				/* 返回正常 */
				debug_printf("BLE PASSWORD OK\r\n");
				retrycnt = 3;
				step = 2;
			}
			else
			{
				/* 没有正常返回 */
				sendlen = strlen(BleCmd_SetPswd);
				BLE_SendBuf((uint8_t*)BleCmd_SetPswd, sendlen);		//配置参数
				retrycnt--;
				vTaskDelay(1000);
			}
		}
		/* 配置蓝牙类型 */
		else if (step == 2)
		{
			sendlen = strlen(BleCmd_QryRole);
			BLE_SendBuf((uint8_t*)BleCmd_QryRole, sendlen);
			vTaskDelay(200);
			bleret = BLE_RecvBufDirectly(recvbuf, &recvlen);
			if ((bleret == BERET_OK) && (strstr((char*)recvbuf, "+ROLE:0") != NULL))	//判断是否返回正常
			{
				/* 返回正常 */
				debug_printf("BLE ROLE OK\r\n");
				retrycnt = 3;
				step = 3;
			}
			else
			{
				/* 没有正常返回 */
				sendlen = strlen(BleCmd_SetRole);
				BLE_SendBuf((uint8_t*)BleCmd_SetRole, sendlen);		//配置参数
				retrycnt--;
				vTaskDelay(1000);
			}
		}
		/* 配置蓝牙名称 CZ4_SXSH */
		else if (step == 3)
		{
			sendlen = strlen(BleCmd_QryName);
			BLE_SendBuf((uint8_t*)BleCmd_QryName, sendlen);
			vTaskDelay(200);
			bleret = BLE_RecvBufDirectly(recvbuf, &recvlen);
			if ((bleret == BERET_OK) && (strstr((char*)recvbuf, "+NAME:CZ4_SXSH") != NULL))	//判断是否返回正常
			{
				/* 返回正常 */
				debug_printf("BLE NAME OK\r\n");
				retrycnt = 3;
				step = 4;
			}
			else
			{
				/* 没有正常返回 */
				sendlen = strlen(BleCmd_SetName);
				BLE_SendBuf((uint8_t*)BleCmd_SetName, sendlen);		//配置参数
				retrycnt--;
				vTaskDelay(1000);
			}
		}
		/* 配置最大连接数 7 */
		else if (step == 4)
		{
			sendlen = strlen(BleCmd_QryMaxconn);
			BLE_SendBuf((uint8_t*)BleCmd_QryMaxconn, sendlen);
			vTaskDelay(200);
			bleret = BLE_RecvBufDirectly(recvbuf, &recvlen);
			if ((bleret == BERET_OK) && (strstr((char*)recvbuf, "+MAXCONN:7") != NULL))	//判断是否返回正常
			{
				/* 返回正常 */
				debug_printf("BLE MAXCONN OK\r\n");
				retrycnt = 3;
				step = 5;
			}
			else
			{
				/* 没有正常返回 */
				sendlen = strlen(BleCmd_SetMaxconn);
				BLE_SendBuf((uint8_t*)BleCmd_SetMaxconn, sendlen);		//配置参数
				retrycnt--;
				vTaskDelay(1000);
			}
		}
		/* 清除已记忆的蓝牙地址 */
		else if (step == 5)
		{
			sendlen = strlen(BleCmd_ClearAllAddr);
			BLE_SendBuf((uint8_t*)BleCmd_ClearAllAddr, sendlen);
			vTaskDelay(200);
			bleret = BLE_RecvBufDirectly(recvbuf, &recvlen);
			if ((bleret == BERET_OK) && (strstr((char*)recvbuf, "OK") != NULL))	//判断是否返回正常
			{
				/* 返回正常 */
				debug_printf("ALL ADDR CLEAR OK!\r\n");
				retrycnt = 3;
				step = 6;
			}
			else
			{
				/* 没有正常返回 */
				sendlen = strlen(BleCmd_ClearAllAddr);
				BLE_SendBuf((uint8_t*)BleCmd_ClearAllAddr, sendlen);		//配置参数
				retrycnt--;
				vTaskDelay(1000);
			}
		}
	}
	
	vPortFree(sendbuf);
	vPortFree(recvbuf);
	
	/* 初始化蓝牙串口配置 正常模式=115200bps */
	BSP_UartSetupPara(&hUSART1, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);	//配置串口句柄
	BSP_UartBindInstance(&hUSART1, USART1);														//绑定串口
	vTaskDelay(100);
	if (step >= 5)
	{
		debug_printf("BLE Setup OK!\r\n");
	}
	else
	{
		if (retrycnt == 0)
		{
			debug_printf("BLE Setup Failure\r\n");
			return BERET_CHK_ERR;
		}
	}
	
	
	return BERET_OK;
}

#endif




