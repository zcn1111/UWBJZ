/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: CORS_Comm.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: CORS定位模块
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "gfunc_String.h"
#include "g_GlobalValDef.h"
#include "CORS_Comm.h"
#include "DTU_BulkData.h"
#include "app_dtu.h"
/******* ML307A命令定义 *******/
#if (USE_ML307A)
const char CORSMCMD_SetNtripServerp[] = "AT*SERVER1=0,rtk.ntrip.qxwz.com,8002#\r\n";	//设置Ntrip服务器地址和端口号 TCP模式 rtk.ntrip.qxwz.com:8002
const char CORSMCMD_UserName[] = "AT*NTRIP=";
const char CORSMCMD_AUTO[] = ",AUTO#\r\n";
//const char CORSMCMD_SetUserName[] = "AT*NTRIP=qxwoyp0091639,e3c4a0a,AUTO#\r\n";			//设置Ntrip账号用户名、密码和挂载点


const char DataServerp[] = "AT*SERVER2=0,"; 
//const char CORSMCMD_SetDataServerp[] = "AT*SERVER2=0,221.204.213.61,22041#\r\n";
const char CORSMCMD_GetNtripServerp[] = "AT*SERVER1?\r\n";								//获取Ntrip服务器配置
const char CORSMCMD_GetDataServerp[] = "AT*SERVER2?\r\n";								//获取数据服务器配置
const char CORSMCMD_GetSignalQuality[] = "AT*CSQ?\r\n";								//获取信号质量
const char CORSMCMD_GetGSMStatus[] = "AT*GSTATE?\r\n";									//获取连接状态
const char CORSMCMD_Reset[] = "AT*RESTART#\r\n";										//复位模块
const char CORSMCMD_SetOffline[] = "AT*OFFLINE#\r\n";									//设置模块离线
const char CORSMCMD_ICCID[] = "AT*ICCID?\r\n";									//设置模块离线
const char CORSMCMD_SetOffNtripServerp[] = "AT*SERVER1=2,rtk.ntrip.qxwz.com,8002#\r\n";	//断开模块和千寻服务连接



///*********************************************************/
//#define ate0             "ATE0\x0d";                                       //关闭回显
//#define csq              "AT+CSQ\x0d"                                      //查询信号质量
//#define cnum             "AT+CNUM\x0d"                                     //读取本机号码
//#define cpbs             "AT+CPBS=\"ON\"\x0d"                                             //选择电话号码存储单元：本机号码区
//#define cpbw             "AT+CPBW=1,\""                                                   //向电话号码薄写电话号码

///*连服命令*/
//#define IPClose_Cors   "AT+MIPCLOSE=0\x0d";                          //关闭上一次连接
//#define IPCont_Cors    "AT+CGDCONT=0,\"IP\",\"CMNET\"\x0d";          //设置APN参数
//#define IPCgact_Cors   "AT+CGACT=1,0\x0d";                           //激活网络，PDP
//#define IPOpen_Cors    "AT+MIPOPEN=0,\"TCP\",\"39.107.207.235\",8002\x0d"    //非透传
//#define _Ipopen_Cors   "+MIPOPEN: 0,0";
//#define IPSta_Cors     "AT+MIPSTATE=0\x0d";   //查询指定连接的连接状态

//#define IPClose        "AT+MIPCLOSE=1\x0d";                          //关闭上一次连接
//#define IPCont         "AT+CGDCONT=1,\"IP\",\"CMNET\"\x0d";          //设置APN参数
//#define IPCgact        "AT+CGACT=1,1\x0d";                           //激活网络，PDP
//#define IPOpen         "AT+MIPOPEN=1,\"TCP\",\"221.204.213.61\",22041,60,1,0\x0d";    //透传
//#define _Ipopen        "+MIPOPEN: 1,0";
//#define IPSta          "AT+MIPSTATE=1\x0d";   //查询指定连接的连接状态

//const char* host = "39.107.207.235";                    // 服务器地址rtk.ntrip.qxwz.com
//const char* port = "8002";                              // 服务端口
//const char* mountpoint = "AUTO";                        // NTRIP挂载点
//const char* username = "qxtjtg006";                     // 鉴权用户名
//const char* password = "c563576";                       // 鉴权密码
//const char* UserAgent = "NTRIP M307Client/1.0";         // 鉴权密码
//const char* http = "200";
#endif


/******* 变量定义 *******/
uint8_t			g_CorsRecvBuf[CORS_RX_BUFSIZE];		//Cors缓存
//char 		iccidbuff[32]={0};
/************************************** Cors指令通信函数 ************************************************/
/******************************************************************
函数名称: CorsResult CORS_SendBuf
功能描述: 发送CORS命令
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_SendBuf(uint8_t *StrBuf, uint16_t StrLen)
{
	tmpsendDataNum = StrLen;
	HAL_UART_Transmit(&hUART4, StrBuf, StrLen, 1000);
	if (g_DbgEcho.EnableCORSEcho == 1) debug_info(StrBuf, StrLen, "->CORS");
	else if (g_DbgEcho.EnableCORSEcho == 2) debug_ascinfo(StrBuf, StrLen, "->CORS", 0);
	
	return CORSRET_OK;
}

/******************************************************************
函数名称: CorsResult CORS_RecvBuf
功能描述: 接收Cors数据
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_RecvBuf(uint8_t *OutBuf, uint16_t *StrLen)
{
	uint16_t rlen;
	
	CorsResult corsret;
	
	/* 接收CORS数据 */
	rlen = GetRingUsedLen(&g_Uart3RxRing);
//	if(rlen >2048)
//	{
//		rlen = 2048;
//	}	
	if (rlen > 0)
	{
		if (rlen > CORS_RX_BUFSIZE)
		{
			*StrLen = 0;
			corsret = CORSRET_NODATA_ERR;
		}
		else
		{
			RingBufPop(&g_Uart3RxRing, OutBuf, rlen);
			if (g_DbgEcho.EnableCORSEcho == 1) debug_info(OutBuf, rlen, "CORS->");
			else if (g_DbgEcho.EnableCORSEcho == 2) debug_ascinfo(OutBuf, rlen, "CORS->", 0);
			*StrLen = rlen;
			return CORSRET_OK;
			if (RtkCommMode == 1)
			{
				debug_printf("%s", OutBuf);
			}

		}
	}
		
	*StrLen = 0;
	return CORSRET_NODATA_ERR;
}

/******************************************************************
函数名称: CorsResult CORS_SetupCommand
功能描述: 向CORS模块发送配置
参数输入: char		*SetCmd		发送命令
		 char		*SuccRetStr	成功返回字符
		 uint8_t	RetryCnt	重试次数
		 uint16_t	Timeout		超时时间(ms)
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_SetupCommand(char *SetCmd, char *SuccRetStr, uint8_t RetryCnt, uint16_t Timeout)
{
	uint8_t rcnt;
	uint16_t slen;
	uint32_t tmrrecv;
	
	CorsResult corsret;
	
	rcnt = 0;
	
	for (rcnt=0; rcnt<=RetryCnt; rcnt++)
	{
		/* 发送命令 */
		tmrrecv = uwTick;
		slen = strlen(SetCmd);
		CORS_SendBuf((uint8_t*)SetCmd, slen);
		printf("[>>CORS] %s", SetCmd);
		
		/* 接收返回值 */
		memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
		RingBufClear(&g_Uart3RxRing);
		while ((uwTick - tmrrecv) < Timeout)
		{
			corsret = CORS_RecvBuf(g_CorsRecvBuf, &slen);
			if (corsret == CORSRET_OK)
			{
				printf("[CORS>>] %s", g_CorsRecvBuf);
				if (strstr((const char*)g_CorsRecvBuf, SuccRetStr) != NULL)
				{
					/* 匹配到结果字符串 */
					return CORSRET_OK;
				}
			}
		}
	}
	
	return CORSRET_TIMEOUT_ERR;
}

/******************************************************************
函数名称: CorsResult CORS_WaitingAnswer
功能描述: 等待指定应答
参数输入: char		*SuccRetStr	成功返回字符
		 uint32_t	TimeoutMs	超时时间(ms)
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_WaitingAnswer(char *SuccRetStr, uint32_t TimeoutMs)
{
	uint32_t tmrrecv;
	uint16_t rlen;
	
	CorsResult corsret;
	
	tmrrecv = uwTick;
	memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
	while (1)
	{
		corsret = CORS_RecvBuf(g_CorsRecvBuf, &rlen);
		if (corsret == CORSRET_OK)
		{
			debug_printf("%s", g_CorsRecvBuf);
			/* 找到对应字符串 */
			if (strstr((char*)g_CorsRecvBuf, SuccRetStr) != NULL)
			{
				return CORSRET_OK;
			}
		}
		
		/* 判断是否超时 超时重发 */
		if ((uwTick - tmrrecv) >= TimeoutMs)
		{
			return CORSRET_NODATA_ERR;
		}
	}
}




/************************************** CORS指令处理函数 ************************************************/

#if (USE_ML307A)
/******************************************************************
函数名称: CorsResult CORS_GetStatusCode_ML307A
功能描述: 获取状态码(ML307A)
参数输入: uint32_t	TimeoutMs	超时时间(ms)
参数输出: uint16_t	StatusCode	状态码
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_GetStatusCode_ML307A(uint16_t *StatusCode, uint32_t TimeoutMs)
{
	uint16_t rlen;
	uint32_t tmrrecv;
	uint16_t code;
	char* ptr;
	
	CorsResult corsret;
	
	tmrrecv = uwTick;
	code = 0xFFFF;
	memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
	while ((uwTick - tmrrecv) < TimeoutMs)
	{
		corsret = CORS_RecvBuf(g_CorsRecvBuf, &rlen);
		if (corsret == CORSRET_OK)
		{
			printf("[CORS>>] %s", g_CorsRecvBuf);
			ptr = strstr((char*)g_CorsRecvBuf, "$STATUS,");
			if (ptr != NULL)
			{
				code = atol(ptr + 8);
				*StatusCode = code;
				return CORSRET_OK;
			}
			else
			{
				memset(g_CorsRecvBuf, 0x00, CORS_RX_BUFSIZE);
				rlen = 0;
			}
		}
	}
	
	*StatusCode = code;
	return CORSRET_TIMEOUT_ERR;
}

/******************************************************************
函数名称: CorsResult CORS_ConnectNtrip_ML307A
功能描述: 模块连接Ntrip服务(ML307A)
参数输入: uint16_t	TimeoutSec	超时时间(s)
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_ConnectNtrip_ML307A(uint16_t *StatusCode, uint8_t TimeoutSec)
{
	uint32_t msec;
	uint16_t scode=0xFFFF;
	uint8_t step;
	
	CorsResult corsret;
	
	msec = (uint32_t)TimeoutSec * 1000;		

	/* 第一步 查找DTU联网成功信息 */
	corsret = CORS_WaitingAnswer("$STATUS,5001", msec);
	if (corsret != CORSRET_OK)
	{
		return CORSRET_CONNECT_FAIL;
	}
	
	/* 第二步 查找鉴权成功信息 */
	corsret = CORS_WaitingAnswer("$STATUS,6001", msec);
	if (corsret != CORSRET_OK)
	{
		return CORSRET_AUTH_FAIL;
	}
	
	return CORSRET_OK;
}

/******************************************************************
函数名称: CorsResult CORS_SetupNtrip_ML307A(char*user,char*pass)
功能描述: 配置模块Ntrip服务(ML307A)
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_SetupNtrip_ML307A(char*user,char*pass)
{
	CorsResult corsret;
	char CORSMCMD_SetDataServerp[100];
	/* 配置Ntrip服务器地址 */

	corsret = CORS_SetupCommand((char*)CORSMCMD_SetNtripServerp, "OK", 3, 2000);
	if (corsret == CORSRET_OK)
	{
		/* 配置账号用户名和密码 */
		snprintf(CORSMCMD_SetDataServerp, sizeof(CORSMCMD_SetDataServerp), "%s%s,%s%s", CORSMCMD_UserName,user,pass,CORSMCMD_AUTO);	
		corsret = CORS_SetupCommand((char*)CORSMCMD_SetDataServerp, "OK", 3, 2000);
		if (corsret == CORSRET_OK)
		{
			/* 复位 */
			corsret = CORS_SetupCommand((char*)CORSMCMD_Reset, "OK", 3, 1000);
			if (corsret == CORSRET_OK)
			{
				return CORSRET_OK;
			}
		}
	}
	
	return CORSRET_TIMEOUT_ERR;
}
#ifndef ntohl
#define ntohl(x) ((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | (((x) << 24) & 0xFF000000))
#endif

#ifndef ntohs
#define ntohs(x) ((((x) >> 8) & 0xFF) | (((x) << 8) & 0xFF00))
#endif


void convert_ipaddr(uint8_t Nun, char *ipStr, char *portStr) 
{// 将IPAddr_t转换为IP字符串和端口字符串    
    uint8_t hostPort[2];
//	uint32_t hostIP = ntohl(ipAddr->byte.uIPAddr);      // 转换IP地址：网络字节序 -> 主机字节序 -> 点分十进制
//  uint8_t *ipBytes = (uint8_t*)&hostIP;
    sprintf(ipStr, "%d.%d.%d.%d,", g_ServerInfo.g_ServerIPAddr[Nun][0], g_ServerInfo.g_ServerIPAddr[Nun][1], g_ServerInfo.g_ServerIPAddr[Nun][2], g_ServerInfo.g_ServerIPAddr[Nun][3]);   
    hostPort[0] = g_ServerInfo.g_ServerPortNo[Nun]/100;      // 转换端口：网络字节序 -> 主机字节序 -> 字符串
    hostPort[1] = g_ServerInfo.g_ServerPortNo[Nun]%100;      // 转换端口：网络字节序 -> 主机字节序 -> 字符串
	sprintf(portStr, "%d%d#\r\n", hostPort[0],hostPort[1]);
}

//CorsResult CORS_ReadICCIDML307A(void)
//{
//	CorsResult corsret;	
//	char *p;
//	corsret = CORS_SetupCommand((char*)CORSMCMD_ICCID, "+ICCID:", 3, 2000);
//	if (corsret == CORSRET_OK)
//	{
//		p = strstr((char*)g_CorsRecvBuf, "+ICCID:");
//		if (p) 
//		{
//			memset(iccidbuff, 0, sizeof(iccidbuff));
//			sscanf(p + strlen("+MCCID:"), "%s", iccidbuff);
//		}
//		
//		return CORSRET_OK;
//	}	
//	return CORSRET_TIMEOUT_ERR;
//}

/******************************************************************
函数名称: CorsResult CORS_SetupDataSvr_ML307A
功能描述: 配置模块数据服务器(ML307A)
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_SetupDataSvr_ML307A(void)
{
	CorsResult corsret;
	char date_ip_str[30];
    char date_port_str[20];
	char CORSMCMD_SetDataServerp[50]; 
	/* 配置数据服务器 */ 
	if(g_RemoteUpdateGCFlag > 0)
	{//升级程序
		convert_ipaddr(0x00, date_ip_str, date_port_str);
	}
	else
	{
		convert_ipaddr(0x02, date_ip_str, date_port_str);
	}
	snprintf(CORSMCMD_SetDataServerp, sizeof(CORSMCMD_SetDataServerp), "%s%s%s", DataServerp,date_ip_str,date_port_str);	
	corsret = CORS_SetupCommand(CORSMCMD_SetDataServerp, "OK", 3, 2000);
	if (corsret == CORSRET_OK)
	{
		return CORSRET_OK;
	}	
	return CORSRET_TIMEOUT_ERR;
}


/******************************************************************
函数名称: CorsResult CORS_Disconnect_ML307A
功能描述: 模块断开连接(ML307A)
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_Disconnect_ML307A(void)
{
	CorsResult corsret;
	
	corsret = CORS_SetupCommand((char*)CORSMCMD_SetOffline, "OK", 3, 2000);
	if (corsret == CORSRET_OK)
	{
		return CORSRET_OK;
	}
	
	return CORSRET_TIMEOUT_ERR;
}

/******************************************************************
函数名称: CorsResult CORS_DisconnectNtrip_ML307A
功能描述: 模块断开连接(ML307A)
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_DisconnectNtrip_ML307A(void)
{
	CorsResult corsret;
	
	corsret = CORS_SetupCommand((char*)CORSMCMD_SetOffNtripServerp, "OK", 3, 2000);
	if (corsret == CORSRET_OK)
	{
		return CORSRET_OK;
	}
	
	return CORSRET_TIMEOUT_ERR;
}
/******************************************************************
函数名称: CorsResult CORS_GetNetSignalQuality_ML307A
功能描述: 获取信号强度(ML307A)
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_GetNetSignalQuality_ML307A(uint8_t *Rssi)
{
	uint16_t slen;
	uint32_t recvtimer;
	uint32_t timeout=2000;		//超时时间2秒
	uint8_t recvrssi;
	char *ptr;
	
	CorsResult corsret;
	
	slen = strlen(CORSMCMD_GetSignalQuality);
	CORS_SendBuf((uint8_t*)CORSMCMD_GetSignalQuality, slen);
	recvtimer = uwTick;
	slen = 0;
	while ((uwTick - recvtimer) <= timeout)
	{
		corsret = CORS_RecvBuf(g_CorsRecvBuf, &slen);
		if (corsret == CORSRET_OK)
		{
			ptr = strstr((const char*)g_CorsRecvBuf, "+CSQ:");
			if (ptr != NULL)
			{
				recvrssi = atol(ptr + 5);
				*Rssi = recvrssi;
				return CORSRET_OK;
			}
		}
	}
	
	return CORSRET_TIMEOUT_ERR;
}

/******************************************************************
函数名称: CorsResult CORS_GetConnectionStatus_ML307A
功能描述: 获取连接状态(ML307A)
参数输入: 
参数输出: 
返 回 值: CorsResult 返回值
其    他: 无
******************************************************************/
CorsResult CORS_GetConnectionStatus_ML307A(ConnStatus_t *Sta)
{
	uint16_t slen;
	uint32_t recvtimer;
	uint32_t timeout=2000;		//超时时间2秒
	uint8_t spiltnum;
	uint8_t i;
	uint8_t k;
	ConnStatus_t csta;
	char *ptr;
	
	CorsResult corsret;
	
	slen = strlen(CORSMCMD_GetGSMStatus);
	CORS_SendBuf((uint8_t*)CORSMCMD_GetGSMStatus, slen);
	debug_printf("[>>CORS] %s", CORSMCMD_GetGSMStatus);
	recvtimer = uwTick;
	slen = 0;
	while ((uwTick - recvtimer) <= timeout)
	{
		corsret = CORS_RecvBuf(g_CorsRecvBuf, &slen);
		if (corsret == CORSRET_OK)
		{
			debug_printf("[CORS>>] %s", g_CorsRecvBuf);
			ptr = strstr((const char*)g_CorsRecvBuf, "+GSTATE:");
			if (ptr != NULL)
			{
				/* 返回数据 拆分状态 */
				slen = strlen(ptr);
				spiltnum = CalcSpiltChrNum(ptr, slen, ',');
				for (i=0; i<=spiltnum; i++)
				{
					/* 解析结果 */
					if (ptr[8 + i*2] == '0') k = 0;
					else if (ptr[8 + i*2] == '1') k = 1;
					else if (ptr[8 + i*2] == '2') k = 2;
					else k = 3;
					
					if (i == 0) Sta->SvrSta1 = k;
					else if (i == 1) Sta->SvrSta2 = k;
					else if (i == 2) Sta->SvrSta3 = k;
					else if (i == 3) Sta->SvrSta4 = k;
				}
				return CORSRET_OK;
			}
		}
	}
	
	return CORSRET_TIMEOUT_ERR;
}


#endif /* USE_ML307A */


