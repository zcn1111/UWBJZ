/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: DTU_Comm.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: DTU通信命令函数
其他说明:
功能列表:
******************************************************************/

#include "DTU_Comm.h"

#include "Update.h"
#include "g_func.h"
#include "g_GlobalValDef.h"
#include "string.h"

#include "BSP.h"


/********* 变量定义 **********/
uint16_t g_DtuDataLen;

LEDState_t		g_DTUSendLED;
LEDState_t		g_DTURecvLED;
LEDState_t		g_DTUGprsOKLED;
//ExtFuncGroups_t	g_DtuFuncGroupArr[EXTFUNC_GRP_MAX];	//外部函数
//ExtFuncGroups_t g_GenFuncGroupArr[GENFUNC_GRP_MAX];	//通用外部函数


/********* 缓存变量定义 **********/
//uint8_t g_DataDomainBuf[520];	//数据域缓存变量

//uint8_t g_DtuDataBuf[520];		//数据缓存变量

//uint8_t	g_DtuSendBuf[600];		//发送缓存
//uint8_t	g_DtuRecvBuf[600];		//接收缓存


/********* IP端口号 **********/
//MobileNum_t		g_ObjPhonenum;

/************************************** DTU通信函数 ************************************************/

/******************************************************************
函数名称: int16_t DTU_SendRequestCmd
功能描述: 发送DTU请求命令
参数输入: DevSelfSta_t		SelfSta		本机状态
		  uint8_t			TypeCode	类型码
		  uint16_t			DomainLen	数据长度
参数输出: 
返 回 值: int16_t 发送成功=0;错误<0
其    他: 无
******************************************************************/
int16_t DTU_SendRequestCmd(DevSelfSta_t SelfSta, uint8_t TypeCode, uint16_t DomainLen)
{
//	uint8_t chksum;
//	uint16_t sendlen;
//	
//	HAL_StatusTypeDef halsta;
//	int16_t ret;
//	
//	/* 检查类型码正确性 */
//	if (((TypeCode & 0x60) != 0x60) && ((TypeCode & 0x70) != 0x70))
//	{
//		return DTU_COMM_ERR_TYPCODE;
//	}
//	
//	/* 生成DTU数据包 */
//	chksum = 0;
//	memset(g_DtuSendBuf, 0x00, sizeof(g_DtuSendBuf));
//	g_DtuSendBuf[0] = 0xAA;
//	g_DtuSendBuf[1] = 0xBB;
//	g_DtuSendBuf[2] = TypeCode;
//	g_DtuSendBuf[3] = SelfSta.all;
//	ConvUint16ToArray(DomainLen, g_DtuSendBuf + 4, M_LITTLE_ENDIAN);	//计算长度
//	/* 处理数据域 */
//	if (DomainLen == 0)
//	{
//		chksum = CalcChecksum(g_DtuSendBuf + 2, 4);	//计算校验和
//		sendlen = 7;
//		g_DtuSendBuf[sendlen - 1] = chksum;
//	}
//	else
//	{
//		memcpy(g_DtuSendBuf + 6, g_DataDomainBuf, DomainLen);
//		chksum = CalcChecksum(g_DtuSendBuf + 2, DomainLen + 4);	//计算校验和
//		sendlen = 7 + DomainLen;
//		g_DtuSendBuf[sendlen - 1] = chksum;
//	}
//	
//	/* 发送DTU数据 */
//	/* 发送时关闭接收超时定时器 */
//	if (g_Timeout1ms != 0)
//	{
//		HAL_TIM_Base_Stop_IT(&hTIM16);
//	}
////	halsta = HAL_UART_Transmit(&hUART4, g_DtuSendBuf, sendlen, 1000);
//	halsta = HAL_UART_Transmit(&hUSART3, g_DtuSendBuf, sendlen, 1000);		//使用DTU高速串口发送
//	if (halsta != HAL_OK)
//	{
//		ret = DTU_COMM_ERR_SENDFAIL;
//	}
//	else
//	{
//		ret = DTU_COMM_OK;
//	}
//	HAL_Delay(sendlen * 5);
//	g_DTUSendLED.DispType = LEDT_OFF;	//发送结束后熄灭发送指示灯
//	/* 发送后开启接收超时定时器 */
//	if (g_Timeout1ms != 0)
//	{
//		HAL_TIM_Base_Start_IT(&hTIM16);
//	}
//	debug_info(g_DtuSendBuf, sendlen, "->DTU");
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_RecvReturnCmd
功能描述: 接收DTU返回命令
参数输入: 
参数输出: DtuSelfSta_t		*SelfSta	DTU状态
		  uint8_t			*TypeCode	类型码
		  uint16_t			DomainLen	数据长度
返 回 值: int16_t 接收成功=0; <0错误类型
其    他: 无
******************************************************************/
int16_t DTU_RecvReturnCmd(DtuSelfSta_t *SelfSta, uint8_t *TypeCode, uint16_t *DomainLen)
{
//	uint16_t i;
//	uint16_t size;
//	
//	uint8_t type;
//	DtuSelfSta_t sta;
//	uint16_t dlen;
//	uint16_t datsize;
//	uint8_t chksum;
//	
//	int16_t ret;
//		
//	size = GetRingUsedLen(&g_Uart3RxRing);
//	/* 检查数据长度 */
//	if ((size > 0) && (size < 6))
//		return DTU_COMM_ERR_LEN;
//	else if (size == 0)
//		return DTU_COMM_ERR_NODATA;
//	
//	/* 提取数据 */
//	ret = DTU_COMM_ERR_CHKSUM;
//	for (i=0; i<size; i++)
//	{
//		memset(g_DtuRecvBuf, 0x00, sizeof(g_DtuRecvBuf));
//		RingBufPop(&g_Uart3RxRing, g_DtuRecvBuf, 1);
//		if (g_DtuRecvBuf[0] == 0xAA)
//		{
//			RingBufPop(&g_Uart3RxRing, g_DtuRecvBuf + 1, 1);
//			if (g_DtuRecvBuf[1] == 0xBB)
//			{
//				RingBufPop(&g_Uart3RxRing, g_DtuRecvBuf + 2, 4);
//				type = g_DtuRecvBuf[2];										//获取类型码
//				sta.all = g_DtuRecvBuf[3];									//获取本机状态
//				dlen = ConvArrayToUint16(g_DtuRecvBuf, 4, M_LITTLE_ENDIAN);	//获取数据长度
//				datsize = GetRingUsedLen(&g_Uart3RxRing);					//获取缓存数据长度
//				if (datsize > dlen)
//				{
//					RingBufPop(&g_Uart3RxRing, g_DtuRecvBuf + 6, dlen + 1);			//获取剩余数据
//					chksum = CalcChecksum(g_DtuRecvBuf + 2, (4 + dlen));
//					if (chksum == g_DtuRecvBuf[6 + dlen])
//					{
//						ret = DTU_COMM_OK;
//						*SelfSta = sta;						//获取参数
//						*TypeCode = type;
//						*DomainLen = dlen;
//						if (dlen != 0)		//有数据域的情况
//						{
//							memcpy(g_DataDomainBuf, g_DtuRecvBuf + 6, dlen);		//获取数据域
//						}
//						debug_info(g_DtuRecvBuf, dlen + 7, "DTU->");
////						debug_ascinfo(g_DtuRecvBuf, dlen + 7, "DTU->", 0);
//						break;
//					}
//					else
//					{
////						debug_info(g_DtuRecvBuf, dlen + 7, "E.DTU->");
//						ret = DTU_COMM_ERR_CHKSUM;
//					}
//				}
//			}
//		}
//	}
//	
//	return ret;
}

/************************************** DTU指令发送函数 ************************************************/

/******************************************************************
函数名称: void DTU_AddSendModeInPhoneNum
功能描述: 手机号最后半字节添加发送模式
参数输入: MobileNum_t		*PhNum	本机号
		  DataSendMode_t	SendMode发送模式
参数输出: DtuPhoneNum_t		*PhNum	本机号
返 回 值: 无
其    他: 无
******************************************************************/
void DTU_AddSendModeInPhoneNum(MobileNum_t *PhNum, DataSendMode_t SendMode)
{
	PhNum->MobNum[5] &= 0xF0;
	PhNum->MobNum[5] |= (SendMode.all) & 0x0F;
}

/******************************************************************
函数名称: int16_t DTU_QueryDTUStatus
功能描述: 查询DTU状态
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_QueryDTUStatus(void)
{
	int16_t ret;
	DevSelfSta_t sta;
	
	sta.all = 0x00;
	ret = DTU_SendRequestCmd(sta, TCODE_DEV_QUERYDTUSTATE, 0);
	
	return ret;
}

/******************************************************************
函数名称: int16_t DTU_ShowDtuInfo
功能描述: 显示DTU状态信息
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_ShowDtuInfo(DtuSelfSta_t *Sta)
{
	char str[128]={0};
	
	char dtusta[10]={0};
	char gsmsta[10]={0};
	char gprssta[10]={0};
	char havedata[10]={0};
	char sendmode[10]={0};
	char sendok[10]={0};
	
	if (Sta->bit.DtuState == 1)
	{
		strncpy(dtusta, "STA:BUSY", 8);
	}
	else
	{
		strncpy(dtusta, "STA:IDLE", 8);
	}
	
	if (Sta->bit.GSMState == 1)
	{
		strncpy(gsmsta, "GSM:ON", 6);
	}
	else
	{
		strncpy(gsmsta, "GSM:OFF", 7);
	}
	
	if (Sta->bit.GPRSState == 1)
	{
		strncpy(gprssta, "GPRS:OK", 7);
	}
	else
	{
		strncpy(gprssta, "GPRS:OFF", 8);
		g_DTUGprsOKLED.DispType = LEDT_OFF;
	}
	
	if (Sta->bit.HaveData == 1)
	{
		strncpy(havedata, "DATA=Y", 6);
	}
	else
	{
		strncpy(havedata, "DATA=N", 6);
	}
	
	if (Sta->bit.SendMode == 1)
	{
		strncpy(sendmode, "MODE:GPRS", 9);
	}
	else
	{
		strncpy(sendmode, "MODE:SMS", 8);
	}
	
	if (Sta->bit.SendOK == 1)
	{
		strncpy(sendok, "SEND:OK", 7);
	}
	else
	{
		strncpy(sendok, "SEND:FAIL", 9);
	}
	
	sprintf(str, "DTU INFO: %s %s %s %s %s %s", dtusta, gsmsta, gprssta, havedata, sendmode, sendok);
	debug_printf("%s\r\n", str);
	
	return DTU_COMM_OK;
}

/******************************************************************
函数名称: int16_t DTU_ReturnState
功能描述: 返回本机状态
参数输入: DevSelfSta_t	DevSta		本机状态
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_ReturnState(DevSelfSta_t DevSta)
{
//	int16_t ret;
//	
//	ret = DTU_SendRequestCmd(DevSta, TCODE_DEV_RETURNSTATE, 0);
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_AckRecvSuccess
功能描述: 接收成功应答
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AckRecvSuccess(void)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_ACKSUCCESS, 0);
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_AskSignalStrength
功能描述: 索要信号强度
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AskSignalStrength(void)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	g_DataDomainBuf[0] = 0x01;	//索要信号强度
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_ASKSIGNAL, 8);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_AskDateTime
功能描述: 索要时间日期
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AskDateTime(void)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	g_DataDomainBuf[0] = 0x02;	//索要时间日期
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_ASKSIGNAL, 8);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_AskPhoneNum
功能描述: 索要本机号
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AskPhoneNum(void)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	g_DataDomainBuf[0] = 0x03;	//索要本机号
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_ASKPHONENUM, 8);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_WritePhoneNumToDTU
功能描述: 写本机号
参数输入: MobileNum_t	PhoneNum	本机号
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_WritePhoneNumToDTU(MobileNum_t PhoneNum)
{
//	int16_t ret;
//	MobileNum_t sendphone;
//	DataSendMode_t sendmode;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	sendphone = PhoneNum;
//	sendmode.all = 0x0F;
//	DTU_AddSendModeInPhoneNum(&sendphone, sendmode);	//处理最后一个字节低4位	
//	memcpy(g_DataDomainBuf, sendphone.MobNum, 6);		//复制手机号
//	
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_WRITEPHONENUM, 8);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_ChangeIP
功能描述: 指定设备IP
参数输入: IPAddr_t	IPAddr		IP地址
		  uint8_t	IPMode		IP类型
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_ChangeIP(IPAddr_t IPAddr, uint8_t IPMode)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	memcpy(g_DataDomainBuf, IPAddr.all, 4);					//复制IP地址
//	g_DataDomainBuf[4] = IPAddr.byte.uPort / 100;			//复制端口号
//	g_DataDomainBuf[5] = IPAddr.byte.uPort % 100;
//	g_DataDomainBuf[6] = IPMode;
//	
//	memcpy(g_DtuPara.ChangeIPAddr.all, IPAddr.all, 6);

//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_CHANGEIP, 8);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_RequestUpdatePackage
功能描述: 索要升级包
参数输入: uint8_t	*UpdateBuf	缓存数据
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_RequestUpdatePackage(uint8_t *UpdateBuf)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	memcpy(g_DataDomainBuf, UpdateBuf, 16);
//	DTU_SendRequestCmd(sta, TCODE_DEV_SENDDATA, 16);
//	
//	return DTU_COMM_OK;
}

/******************************************************************
函数名称: int16_t DTU_AckDtuUpdate
功能描述: 应答DTU升级
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AckDtuUpdate(void)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	g_DataDomainBuf[0] = 0x01;
//	
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_ACKDTUUPDATE, 8);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_TransparentTransMode
功能描述: 切换透传模式
参数输入: uint8_t	TransMode	传输模式 1=开启透传;2=关闭透传
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_TranparentTransMode(uint8_t TransMode)
{
//	int16_t ret;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	/* 数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	g_DataDomainBuf[0] = TransMode;
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_TRANSPARENT, 8);
//	
//	return ret;
}

/************************************** DTU D6数据收发函数 ************************************************/

/******************************************************************
函数名称: int16_t DTU_SendDataByD6
功能描述: 生成D6数据包
参数输入: uint8_t			*DatBuf		发送数据内容
		  uint16_t			DatLen		发送数据长度
		  MobileNum_t		ObjPhNum	目标号
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_SendDataByD6(uint8_t *DatBuf, uint16_t DatLen, MobileNum_t ObjPhNum)
{
//	int16_t ret;
//	MobileNum_t obj;
//	DevSelfSta_t sta;
//	uint8_t asc_send=0;		//ASC码发送  [0=Hex格式发送];1=ASC码格式发送
//	uint8_t obj_send=1;		//目标号发送 0=目标号填FF;[1=按照传入目标号发送]
//	
//	uint16_t i;
//	
//	sta.all = 0x00;
//	
//	/* CF和CFFF数据需要目标号 ACSII码方式 其他目标号为FF */
//	if (DatBuf[0] == 0xCF)
//	{
//		asc_send = 1;
//		obj_send = 1;
//	}
//	
//	/* 处理目标号 */
//	if (obj_send == 1)
//	{
//		obj = ObjPhNum;
//		if (obj.SendMode.bit.SMScode == 0)				//处理默认情况发送方式
//		{
//			obj.SendMode.bit.SMScode = 2;				//8bit发送方式
//		}
//		DTU_AddSendModeInPhoneNum(&obj, obj.SendMode);	//处理目标号
//	}
//	else
//	{
//		memset(obj.MobNum, 0xFF, 6);
//	}
//	
//	/* 处理数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	g_DataDomainBuf[0] = 0xD6;
//	memcpy(g_DataDomainBuf + 1, obj.MobNum, 6);
//	
//	/* 处理ASC码发送 */
//	if (asc_send == 1)
//	{
//		for (i=0; i<DatLen; i++)
//		{
//			Hex2AscInString(DatBuf[i], g_DataDomainBuf + (i * 2) + 7);		//转换为ASCII码
//		}
//		DatLen = DatLen * 2;
//	}
//	else
//	{
//		memcpy(g_DataDomainBuf + 7, DatBuf, DatLen);
//	}
//	
//	/* 发送数据 */
//	g_DTUSendLED.DispType = LEDT_BLINK_F;	//闪烁发送指示灯
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_SENDDATA, DatLen + 7);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_ProcessByD6
功能描述: 接收D6数据包
参数输入: uint16_t			DomainLen	数据域数据长度
参数输出: uint8_t			*DatBuf		接收数据内容
		  uint16_t			*DatLen		接收数据长度
		  MobileNum_t		*ObjNum		目标号
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_ProcessByD6(uint16_t DomainLen, uint8_t *DatBuf, uint16_t *DatLen, MobileNum_t *ObjNum)
{
//	uint16_t i;
//	uint16_t tlen;
//	uint8_t asc_change=0;
//	
//	/* D6数据头校验 */
//	if (g_DataDomainBuf[0] != 0xD6)
//		return DTU_COMM_ERR_NOTD6;
//	
//	/* 处理目标号 */
//	memcpy(ObjNum->MobNum, g_DataDomainBuf + 1, 6);
//	
//	/* 处理接收数据 */
//	/* 判断是否需要转换ASCII */
//	asc_change = 0;
//	if ((g_DataDomainBuf[7] == 'C') && (g_DataDomainBuf[8] == 'F'))
//		asc_change = 1;
//	else if ((g_DataDomainBuf[7] != 0xA1) && (g_DataDomainBuf[8] != 0xA1))
//		asc_change = 1;
//	
//	/* 接收数据ASCII码形式 */
//	if (asc_change == 1)
//	{
//		tlen = (DomainLen - 7) / 2;
//	}
//	else
//	{
//		tlen = DomainLen - 7;
//	}
//	for (i=0; i<tlen; i++)
//	{
//		/* 需要转换ASC码 */
//		if (asc_change == 1)
//		{		
//			DatBuf[i] = Asc2HexInString(g_DataDomainBuf + (i * 2) + 7);
//		}
//		else
//		{
//			DatBuf[i] = g_DataDomainBuf[i + 7];
//		}
//	}
//	*DatLen = tlen;
//	
//	return DTU_COMM_OK;
}

/************************************** DTU 通用数据收发函数 ************************************************/

/******************************************************************
函数名称: int16_t DTU_SendDataByInCommon
功能描述: 生成通用格式数据包
参数输入: uint8_t			*DatBuf		发送数据内容
		  uint16_t			DatLen		发送数据长度
		  MobileNum_t		ObjPhNum	目标号
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_SendDataByInCommon(uint8_t *DatBuf, uint16_t DatLen, MobileNum_t ObjPhNum, char *strHeader)
{
//	int16_t ret;
//	MobileNum_t obj;
//	DevSelfSta_t sta;
//	
//	uint8_t slen;
//	uint8_t header1;
//	uint8_t header2;
//	uint16_t offset;
//	char head[2];
//	
//	sta.all = 0x00;		//初始化本机状态
//	
//	/* 处理目标号 */
//	obj = ObjPhNum;
//	obj.MobNum[5] = obj.MobNum[5] | 0x0F;
//	
//	/* 处理数据域 */
//	memset(g_DataDomainBuf, 0x00, sizeof(g_DataDomainBuf));
//	
//	slen = strlen(strHeader);
//	/* 根据数据头字节数处理 */
//	if (slen == 2)
//	{
//		/* 1字节数据头 */
//		memcpy(head, strHeader, 2);
//		header1 = Asc2HexInString((uint8_t*)head);	//转换成16进制
//		g_DataDomainBuf[0] = header1;
//		offset = 1;
//	}
//	else if (slen == 4)
//	{
//		/* 2字节数据头 */
//		memcpy(head, strHeader, 2);
//		header1 = Asc2HexInString((uint8_t*)head);	//转换第一个字节
//		memcpy(head, strHeader + 2, 2);
//		header2 = Asc2HexInString((uint8_t*)head);	//转换第二个字节
//		g_DataDomainBuf[0] = header1;
//		g_DataDomainBuf[1] = header2;
//		offset = 2;
//	}
//	else
//	{
//		return DTU_COMM_ERR_NOHEADER;
//	}
//	memcpy(g_DataDomainBuf + offset, obj.MobNum, 6);
//	
//	/* 处理数据 */
//	memcpy(g_DataDomainBuf + (offset + 6), DatBuf + (offset + 6), DatLen - (offset + 6));
//	
//	/* 处理校验和 */
//	g_DataDomainBuf[DatLen - 1] = CalcChecksum(g_DataDomainBuf, DatLen - 1);
//	
//	/* 发送数据 */
//	g_DTUSendLED.DispType = LEDT_BLINK_F;	//闪烁发送指示灯
//	ret = DTU_SendRequestCmd(sta, TCODE_DEV_SENDDATA, DatLen);
//	
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_RecvDataInCommon
功能描述: 接收通用格式数据包
参数输入: uint16_t			DomainLen	数据域数据长度
参数输出: uint8_t			*DatBuf		接收数据内容
		  uint16_t			*DatLen		接收数据长度
		  MobileNum_t		*ObjNum		目标号
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_RecvDataInCommon(uint16_t DomainLen, uint8_t *DatBuf, uint16_t *DatLen, uint8_t *Header)
{
//	uint16_t i;
//	uint16_t tlen;
//	uint8_t asc_change=0;
//	
//	/* 处理数据头 */
//	*Header = g_DataDomainBuf[0];
//	
//	/* 处理接收数据 */
//	memcpy(DatBuf, g_DataDomainBuf, DomainLen);
//	*DatLen = DomainLen;
//	
//	return DTU_COMM_OK;
}

///******************************************************************
//函数名称: bool DTU_IsExecutable
//功能描述: 判断是否可执行
//参数输入: MobileNum_t	ObjNum	目标号
//参数输出: 
//返 回 值: uint8_t 可执行 0=目标号不正确;1=目标号正确
//其    他: 无
//******************************************************************/
//bool DTU_IsExecutable(MobileNum_t ObjNum)
//{
//	uint8_t ret=false;
//	
//	/* 检查是否为总控或研发总控 */
//	if (memcmp(ObjNum.MobNum, TotalControlPhoneNum, 6) == 0)
//	{
//		ret = true;
//	}
//	else if (memcmp(ObjNum.MobNum, YFTotalControlPhoneNum, 6) == 0)
//	{
//		ret = true;
//	}
//	/* 检查地面号 */
//	else if ((CheckNumIsBCD(g_MobileInFlash.DMnum.MobNum, 5) == 1) && (memcmp(ObjNum.MobNum, g_MobileInFlash.DMnum.MobNum, 6) == 0))
//	{
//		ret = true;
//	}
//	else if ((CheckNumIsBCD(g_MobileInFlash.DMnum.MobNum, 5) == 0) && (memcmp(ObjNum.MobNum, g_MobileInFlash.WHnum.MobNum, 6) == 0))
//	{
//		ret = true;
//	}
//	/* 检查维护号 */
//	else if ((CheckNumIsBCD(g_MobileInFlash.WHnum.MobNum, 5) == 1) && (memcmp(ObjNum.MobNum, g_MobileInFlash.WHnum.MobNum, 6) == 0))
//	{
//		ret = true;
//	}
//	else if ((CheckNumIsBCD(g_MobileInFlash.WHnum.MobNum, 5) == 0) && (memcmp(ObjNum.MobNum, g_MobileInFlash.DMnum.MobNum, 6) == 0))
//	{
//		ret = true;
//	}
//	
//	return ret;
//}


/******************************************************************
函数名称: void DTU_SetupExternalFunc
功能描述: DTU配置外部执行函数
参数输入: uint8_t	*strHead	识别头字符串
		  uint16_t	DatLen		数据长度
参数输出: DtuPara_t	*Para		DTU参数
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
void DTU_SetupExternalFunc(uint8_t *strHead, uint16_t DatLen, DtuResult (*cbFunc)(void), DtuExtFunc_t *ExtPara)
{
	uint8_t hexhead[20];
	uint16_t i;
	uint16_t slen;
	
	slen = (strlen((char*)strHead) - 1) / 2;
	slen = (slen > 20) ? 20 : slen;
	for (i=0; i<slen; i++)
	{
		hexhead[i] = Asc2HexInString(strHead + (i * 2));
		ExtPara->RegHead[i] = hexhead[i];
	}
	ExtPara->RegLen = slen;
	ExtPara->DatLen = DatLen;
	ExtPara->cbFunc = cbFunc;
}

/******************************************************************
函数名称: void DTU_RegisterExtFuncToGroup
功能描述: 注册DTU外部执行函数到函数组
参数输入: DtuExtFunc_t		Para		外部函数参数
		  uint8_t			Priority	执行优先级
参数输出: ExtFuncGroups_t	*ptrFunc	外部函数组变量指针
返 回 值: 无
其    他: 无
******************************************************************/
void DTU_RegisterExtFuncToGroup(DtuExtFunc_t Para, uint8_t Priority, ExtFuncGroups_t *ptrFunc)
{
	ptrFunc->ExtFuncPara = Para;
	ptrFunc->ExecFlag.NeedExec = 0;
	ptrFunc->ExecFlag.Priority = Priority & 0x0F;	//配置优先级
}

/******************************************************************
函数名称: void DTU_UnRegisterExtFunc
功能描述: 反注册DTU外部执行函数
参数输入: ExtFuncGroups_t	*ptrFunc	外部函数组变量指针
参数输出: DtuPara_t			*Para		DTU参数
返 回 值: 无
其    他: 无
******************************************************************/
void DTU_UnRegisterExtFunc(ExtFuncGroups_t *ptrFunc)
{
	uint8_t i;
	
	ptrFunc->ExecFlag.NeedExec = 0;
	ptrFunc->ExecFlag.Reserve = 0;
	ptrFunc->ExecFlag.Priority = 15;
	ptrFunc->ExtFuncPara.cbFunc = NULL;
	ptrFunc->ExtFuncPara.DatLen = 0;
	for (i=0; i<10; i++)
		ptrFunc->ExtFuncPara.RegHead[i] = 0x00;
}

/******************************************************************
函数名称: int16_t DTU_AnlysisHeadProcessInCommon
功能描述: DTU解析通用数据头流程
参数输入: DtuPara_t			*Para		DTU参数
参数输出: DtuPara_t			*Para		DTU参数
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AnlysisHeadProcessInCommon(ExtFuncGroups_t *ptrFunc, uint8_t *DatBuf, uint16_t DatLen, uint16_t Max)
{
	uint16_t i;
	uint16_t head;
	uint16_t cbfunc_ret;
	uint8_t pri_min;
	uint8_t needexec;
	
	int8_t pri_cnt;
	
	int16_t ret;
	
	pri_min = 15;
	needexec = 0;
	/* 接收数据处理 */
	for (i=0; i<Max; i++)
	{
		/* 检查识别头是否匹配 */
		if (memcmp(ptrFunc[i].ExtFuncPara.RegHead, DatBuf, ptrFunc[i].ExtFuncPara.RegLen) == 0)
		{
			/* 检查数据长度一致 */
			if (ptrFunc[i].ExtFuncPara.DatLen == DatLen)
			{
				ptrFunc[i].ExecFlag.NeedExec = 1;		//执行标记
				needexec = 1;
			}
		}
	}
	
	/* 执行外部函数处理 */
	do
	{
		pri_cnt = -1;
		pri_min = 15;
		needexec = 0;
		/* 查找需要执行的函数 */
		for (i=0; i<Max; i++)
		{
			if (ptrFunc[i].ExecFlag.NeedExec == 1)
			{
				needexec = 1;
				/* 查找需要执行函数里优先级最高(值最小)的 */
				if (ptrFunc[i].ExecFlag.Priority < pri_min)
				{
					pri_cnt = i;
					pri_min = ptrFunc[i].ExecFlag.Priority;
				}
			}
		}
		if (needexec)
		{
			cbfunc_ret = ptrFunc[pri_cnt].ExtFuncPara.cbFunc();		//执行回调
			if (cbfunc_ret == 0)
			{
				ret = DTU_COMM_OK;
				ptrFunc[pri_cnt].ExecFlag.NeedExec = 0;		//执行结束清掉执行标记
			}
			else
			{
				ret = DTU_COMM_ERR_CALLBACKFAIL;			//未执行成功
			}
		}
	} while (needexec);
	
	return ret;
}


/******************************************************************
函数名称: int16_t DTU_AnlysisHeadProcessByD6
功能描述: DTU解析D6数据头流程
参数输入: DtuPara_t			*Para		DTU参数
参数输出: DtuPara_t			*Para		DTU参数
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AnlysisHeadProcessByD6(ExtFuncGroups_t *ptrFunc, uint8_t *DatBuf, uint16_t DatLen)
{
	uint16_t i;
	uint16_t head;
	uint16_t cbfunc_ret;
	uint8_t pri_min;
	uint8_t needexec;
	
	int8_t pri_cnt;
	
	int16_t ret;
	
	pri_min = 15;
	needexec = 0;
	/* 接收数据处理 */
	for (i=0; i<EXTFUNC_GRP_MAX; i++)
	{
		/* 检查识别头是否匹配 */
		if (memcmp(ptrFunc[i].ExtFuncPara.RegHead, DatBuf, ptrFunc[i].ExtFuncPara.RegLen) == 0)
		{
			/* 检查数据长度一致 */
			if (ptrFunc[i].ExtFuncPara.DatLen == DatLen)
			{
				ptrFunc[i].ExecFlag.NeedExec = 1;		//执行标记
				needexec = 1;
			}
		}
	}
	
	/* 执行外部函数处理 */
	do
	{
		pri_cnt = -1;
		pri_min = 15;
		needexec = 0;
		/* 查找需要执行的函数 */
		for (i=0; i<EXTFUNC_GRP_MAX; i++)
		{
			if (ptrFunc[i].ExecFlag.NeedExec == 1)
			{
				needexec = 1;
				/* 查找需要执行函数里优先级最高(值最小)的 */
				if (ptrFunc[i].ExecFlag.Priority < pri_min)
				{
					pri_cnt = i;
					pri_min = ptrFunc[i].ExecFlag.Priority;
				}
			}
		}
		if (needexec)
		{
			cbfunc_ret = ptrFunc[pri_cnt].ExtFuncPara.cbFunc();		//执行回调
			if (cbfunc_ret == 0)
			{
				ret = DTU_COMM_OK;
				ptrFunc[pri_cnt].ExecFlag.NeedExec = 0;		//执行结束清掉执行标记
			}
			else
			{
				ret = DTU_COMM_ERR_CALLBACKFAIL;			//未执行成功
			}
		}
	} while (needexec);
	
	return ret;
}

char			strtime[14];
char			strip[30];
char			strphone[12];
HexDateTime_t	hextime;

/******************************************************************
函数名称: int16_t DTU_AnlysisTypecodeProgress
功能描述: 解析类型码流程
参数输入: DtuPara_t			*Para		DTU参数
参数输出: DtuPara_t			*Para		DTU参数
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_AnlysisTypecodeProgress(DtuPara_t *Para)
{
//	int16_t cmdret;
//	int16_t recvret;
//	int16_t sendret;
//	
//	DtuSelfSta_t	dtusta;
//	DevSelfSta_t	devsta;
//	uint8_t			typecode;
//	uint16_t		domainlen;
//	uint8_t			adjtime;
//	
//	uint16_t 		i;
//	uint16_t		val;
//	
//	uint16_t port;
//	
//	int16_t ret;
//	
//	uint8_t header;
//	
//	/* 初始化变量 */
//	dtusta.all = 0x00;
//	devsta.all = 0x00;
//	typecode = 0x00;
//	
//	do
//	{
//		cmdret = DTU_RecvReturnCmd(&dtusta, &typecode, &domainlen);
//		
//		/* 接收到数据 */
//		if (cmdret == DTU_COMM_OK)
//		{
//			Para->SelfSta = dtusta;		//获取DTU状态
//			/* GPRS=OK 且 GSM=OK 状态确认连接 */
//			if ((Para->SelfSta.bit.GPRSState == 1) && (Para->SelfSta.bit.GSMState == 1))
//			{
//				Para->DtuNetFlag.bit.Connected = 1;
//				g_DTUGprsOKLED.DispType = LEDT_ON;
//			}
//			else
//			{
//				Para->DtuNetFlag.bit.Connected = 0;
//				g_DTUGprsOKLED.DispType = LEDT_OFF;
//			}
////			DTU_ShowDtuInfo(&dtusta);
//			ret = DTU_COMM_OK;
//			/* 根据类型码处理 */
//			switch(typecode)
//			{
//				/* DTU索要设备状态 0x80 */
//				case TCODE_DTU_QUERYDEVSTATE:
//					devsta.all = 0x00;
//					sendret = DTU_ReturnState(devsta);		//返回状态
//					break;
//				/* DTU返回状态 0x81 */
//				case TCODE_DTU_RETURNSTATE:
////					DTU_ShowDtuInfo(&dtusta);
//					break;
//				/* DTU下发数据 0x82 */
//				case TCODE_DTU_SENDDATA:
//					/* 通用数据处理 根据数据头分别处理 */
//					recvret = DTU_RecvDataInCommon(domainlen, g_DtuDataBuf, &g_DtuDataLen, &header);
//					switch (header)
//					{
//						/* 通用数据 */
//						case 0xCA:
//							g_DTURecvLED.Toogle = 1;	//翻转接收指示灯
//							DTU_AckRecvSuccess();		//返回应答
////							DTU_AnlysisHeadProcessInCommon(g_GenFuncGroupArr, g_DtuDataBuf, g_DtuDataLen, GENFUNC_GRP_MAX);	//通用数据处理
//							break;
//						/* 升级数据 */
//						case 0xEE:
//						case 0xDC:
//							/* 远程升级数据处理 */
//							g_DTURecvLED.Toogle = 1;		//翻转接收指示灯
//							if (header == 0xEE)
//							{
//								DTU_AckRecvSuccess();		//升级数据只有EE返回应答
//							}
//							recvret = Update_RecvData(&g_UpdatePara, g_DtuDataBuf);
//							ret = recvret;
//							return ret;
//							break;
//						/* D6数据 */
//						case 0xD6:
//							memset(g_DtuDataBuf, 0x00, sizeof(g_DtuDataBuf));
//							recvret = DTU_ProcessByD6(domainlen, g_DtuDataBuf, &g_DtuDataLen, &g_ObjPhonenum);				//D6数据处理
//							if (recvret == DTU_COMM_OK)
//							{
//								g_DTURecvLED.Toogle = 1;			//翻转接收指示灯
//								sendret = DTU_AckRecvSuccess();		//给服务器返回应答
//								/* 检查手机号是否可执行 */
//								if (DTU_IsExecutable(g_ObjPhonenum) == true)
//								{
////									DTU_AnlysisHeadProcessByD6(g_DtuFuncGroupArr, g_DtuDataBuf, g_DtuDataLen);	//根据数据头处理外部函数
//								}
//							}
//							break;
//						default:
//							sendret = DTU_AckRecvSuccess();	//返回应答
//					}
//					break;
//				/* 返回时间日期 0x85 */
//				case TCODE_DTU_RETDATETIME:
//					if (CheckNumIsBCD(g_DataDomainBuf, 6) == 1)
//					{
//						Para->DtuDateTime.Year = g_DataDomainBuf[0];	//年
//						Para->DtuDateTime.Month = g_DataDomainBuf[1];	//月
//						Para->DtuDateTime.Day = g_DataDomainBuf[2];		//日
//						Para->DtuDateTime.Hour = g_DataDomainBuf[3];	//时
//						Para->DtuDateTime.Minute = g_DataDomainBuf[4];	//分
//						Para->DtuDateTime.Second = g_DataDomainBuf[5];	//秒
//						adjtime = AdjustSysTime(BcdTime2HexTime(Para->DtuDateTime), &g_SysTime);	//校时
//						if ((adjtime == ADJTIME_SUCCESS) || (adjtime == ADJTIME_HADTIME))
//							Para->DtuNetFlag.bit.ObtainedTime = 1;	//DTU已获取时间
//						hextime = BcdTime2HexTime(Para->DtuDateTime);
//						sprintf(strtime, "20%02d/%02d/%02d %02d:%02d:%02d", hextime.Year, hextime.Month, hextime.Day, hextime.Hour, hextime.Minute, hextime.Second);
//						debug_printf("DTU GetTime: %s\r\n", strtime);
//					}
//					break;
//				/* 返回信号强度 0x86 */
//				case TCODE_DTU_RETSIGNAL:
//					Para->SignalVal = g_DataDomainBuf[0];		//返回信号强度
////					if ((Para->SignalVal > 0) && (Para->SignalVal != 99))
////					{
////						Para->DtuNetFlag.bit.Connected = 1;		//DTU已连接网络
////					}
//					Para->DtuVersion = g_DataDomainBuf[1];		//获取DTU版本号
//					Para->GsmType = g_DataDomainBuf[2];			//获取连接类型
//					debug_printf("DTU Version: %02X, Singal: %02X\r\n", Para->DtuVersion, Para->SignalVal);
//					if (Para->GsmType == 0x01) debug_printf("2G Connected.\r\n");
//					if (Para->GsmType == 0x02) debug_printf("3G Connected.\r\n");
//					if (Para->GsmType == 0x03) debug_printf("4G Connected.\r\n");
//					break;
//				/* 返回本机号和IP 0x87 */
//				case TCODE_DTU_RETPHONEANDIP:
//					/* 检查本机号正确性 */
//					if ((CheckNumIsBCD(g_DataDomainBuf, 5) == 1) && (CheckStringIsSame(g_DataDomainBuf, 5, 0x00) == 0))
//					{
//						memcpy(Para->ConPhoneNum.MobNum, g_DataDomainBuf, 6);	//返回本机号
//						memcpy(g_MobileInFlash.BJnum.MobNum, Para->ConPhoneNum.MobNum, 6);
////						WriteIPAndMobileToFlash();								//写IP和本机号
//						Para->DtuNetFlag.bit.ObtainedPhoneNum = 1;				//已获取本机号
//						for (i=0; i<6; i++)
//						{
//							if (i != 5)
//							{
//								strphone[i*2] = ((Para->ConPhoneNum.MobNum[i] >> 4) & 0x0F) + '0';
//								strphone[i*2 + 1] = (Para->ConPhoneNum.MobNum[i] & 0x0F) + '0';
//							}
//							else
//							{
//								strphone[i*2] = ((Para->ConPhoneNum.MobNum[i] >> 4) & 0x0F) + '0';
//							}
//						}
//						debug_printf("DTU GetPhoneNum: %s\r\n", strphone);
//					}
//					/* 检查IP地址 */
//					if ((CheckStringIsSame(g_DataDomainBuf + 6, 4, 0x00) == 0) || (CheckStringIsSame(g_DataDomainBuf + 6, 4, 0xFF) == 0))
//					{
//						memcpy(Para->ConIPAddr.all, g_DataDomainBuf + 6, 4);			//返回IP地址
//						Para->ConIPAddr.byte.uPort = (g_DataDomainBuf[10] * 100) + (g_DataDomainBuf[11]);
//					}
//					else
//					{
//						/* 接收IP全0 或 全F */
//						InitDefaultIPGroup(&g_ServerInfo);
//						Para->ConIPAddr = g_ServerInfo.DataIP;
//					}
//					sprintf(strip, "%d.%d.%d.%d:%d", Para->ConIPAddr.all[0], Para->ConIPAddr.all[1], Para->ConIPAddr.all[2], Para->ConIPAddr.all[3], Para->ConIPAddr.byte.uPort);
//					debug_printf("DTU GetIP: %s\r\n", strip);
//					
//					Para->DtuNetFlag.bit.ObtainedIP = 1;	//已获取IP
//					break;
//				/* 返回DTU升级状态 0x88 */
//				case TCODE_DTU_SENDDTUUPDATE:
//					Para->UpdateFlag = g_DataDomainBuf[0];
//					break;
//				/* 透传模式 0x89 */
//				case TCODE_DTU_ACKTRANSP:
//					if (g_DataDomainBuf[0] == 0x01)
//					{
//						debug_printf("DTU: Entry transport mode\r\n");
//						Para->DtuNetFlag.bit.Transport = 1;
//					}
//					else if (g_DataDomainBuf[0] == 0x02)
//					{
//						debug_printf("DTU: Exit transport mode\r\n");
//						Para->DtuNetFlag.bit.Transport = 0;
//					}
//					break;
//				default:
//					break;
//			}
//		}
//		else
//		{
//			if (Para->DtuNetFlag.bit.Connected == 1)
//				ret = DTU_COMM_CONNECTED;
//			else
//				ret = cmdret;
//		}
//	} while (cmdret == DTU_COMM_OK);
//	
//	return ret;
}


