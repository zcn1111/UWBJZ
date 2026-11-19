/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: BLE_BaseStation.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 蓝牙手机基站通信
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "RYDW_Def.h"

#include "BLE_Comm.h"
#include "BLE_BaseStation.h"


//#include "Drv_W25Qxx.h"

#include "string.h"


/********* 变量定义 **********/
uint8_t			g_BLESendBuf[BLE_BUFSIZE];
uint8_t			g_BLERecvBuf[BLE_BUFSIZE];
uint8_t			g_BLESendCardinfo[BLE_BUFSIZE];
uint8_t			g_BLERecvElec;
uint8_t			g_BLERecvdElecCnt;
uint8_t			g_BLERecvElecTotal;

uint8_t 		g_BLECodeType;
uint8_t			g_BLEUploadCard=0;
uint32_t		g_tmrBleUploadDelay;

/********* 蓝牙AT指令 **********/


/************************************** 蓝牙接收处理函数 ************************************************/
/******************************************************************
函数名称: BLEResult BLE_SendMoblieCommAck
功能描述: 蓝牙手机通信发送应答
参数输入: 
参数输出: 
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_SendMobileCommAck(uint8_t CodeType, uint8_t *DatBuf, uint16_t BufLen)
{
	BLEResult bleret;
	uint8_t chksum;
	uint16_t len;
	
	/* 处理包头 */
	memset(g_BLESendBuf, 0x00, BLE_BUFSIZE);
	g_BLESendBuf[0] = 0x5A;				//包头
	g_BLESendBuf[1] = 0xA5;
	g_BLESendBuf[2] = 0x01;				//类型码
	g_BLESendBuf[3] = CodeType;
	
	/* 处理数据 */
	switch (CodeType)
	{
		/* 0x02 = 上下道数据 */
		case 0x02:
			g_BLESendBuf[4] = DatBuf[0];
			memset(g_BLESendBuf + 5, 0x00, 10);
			chksum = CalcChecksum(g_BLESendBuf, 15);
			g_BLESendBuf[15] = chksum;
			len = 16;
			break;
		/* 0x03 = 来车数据 */
		case 0x03:
			memset(g_BLESendBuf + 4, 0x00, 11);
			chksum = CalcChecksum(g_BLESendBuf, 15);
			g_BLESendBuf[15] = chksum;
			len = 16;
			break;
		/* 0x04 = 施工时间数据 */
		case 0x04:
			memset(g_BLESendBuf + 4, 0x00, 11);
			chksum = CalcChecksum(g_BLESendBuf, 15);
			g_BLESendBuf[15] = chksum;
			len = 16;
			break;
		/* 0x01 = 电子围栏 */
		case 0x01:
			memcpy(g_BLESendBuf + 4, DatBuf, 3);
			memset(g_BLESendBuf + 7, 0x00, 8);
			chksum = CalcChecksum(g_BLESendBuf, 15);
			g_BLESendBuf[15] = chksum;
			len = 16;
			break;
		/* 0x06 = 接收定位数据 */
		case 0x06:
			break;
		default:
			break;
	}
	
	bleret = BLE_SendBuf(g_BLESendBuf, len);
	if (bleret == BLERET_OK)
	{
		return BLERET_OK;
	}
}

/******************************************************************
函数名称: BLEResult BLE_SendCardinfo
功能描述: 蓝牙发送定位卡数据
参数输入: 
参数输出: 
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_SendCardinfo(CardInfoTable_t CardTable)
{
	uint8_t i;
	uint8_t	chksum;
	uint16_t pos=0;
	uint16_t len=0;
	BLEResult bleret;
	
	/* 生成数据头 */
	memset(g_BLESendBuf, 0x00, BLE_BUFSIZE);
	g_BLESendBuf[0] = 0x5A;
	g_BLESendBuf[1] = 0xA5;
	g_BLESendBuf[2] = 0x01;						//上传定位数据
	g_BLESendBuf[3] = 0x06;						//定位数据
	
	/* 生成时间 */
	g_BLESendBuf[4] = g_SysTime.Year;
	g_BLESendBuf[5] = g_SysTime.Month;
	g_BLESendBuf[6] = g_SysTime.Day;
	g_BLESendBuf[7] = g_SysTime.Hour;
	g_BLESendBuf[8] = g_SysTime.Minute;
	g_BLESendBuf[9] = g_SysTime.Second;
	
	/* 数据长度 */
	ConvUint16ToArray((CardTable.CardNbr * 32), g_BLESendBuf + 10, M_LITTLE_ENDIAN);
	
	/* 卡数据 */
	for (i=0; i<CardTable.CardNbr; i++)
	{
		pos = i * 32 + 15;
		memcpy(g_BLESendBuf + (pos + 0), CardTable.Info[i].ID.array, 5);
		g_BLESendBuf[pos + 5] = (uint8_t)(CardTable.Info[i].ID.byte.UserID & 0xFF);						//用户ID按小端模式上传
		g_BLESendBuf[pos + 6] = (uint8_t)(CardTable.Info[i].ID.byte.UserID >> 8);
		ConvUint32ToArray(CardTable.Info[i].Latitude, g_BLESendBuf + (pos + 7), M_LITTLE_ENDIAN);
		ConvUint32ToArray(CardTable.Info[i].Longitude, g_BLESendBuf + (pos + 11), M_LITTLE_ENDIAN);
		g_BLESendBuf[(pos + 15)] = CardTable.Info[i].PosQuality;
		memcpy(g_BLESendBuf + (pos + 16), CardTable.Info[i].Altitude, 3);
		g_BLESendBuf[(pos + 19)] = CardTable.Info[i].Alram.all;
		g_BLESendBuf[(pos + 20)] = CardTable.Info[i].BatVoltage;
		ConvUint16ToArray(CardTable.Info[i].AlramType.all, g_BLESendBuf + (pos + 21), M_LITTLE_ENDIAN);
		g_BLESendBuf[(pos + 23)] = CardTable.Info[i].FenceOrder;
		g_BLESendBuf[(pos + 24)] = CardTable.Info[i].WordTimeOrder;
		g_BLESendBuf[(pos + 25)] = CardTable.Info[i].LedOrder;
		memcpy(g_BLESendBuf + (pos + 26), CardTable.Info[i].Reserver, 6);		//其他6个预留字节
	}
	len = CardTable.CardNbr * 32 + 15;
	chksum = CalcChecksum(g_BLESendBuf, len);
	g_BLESendBuf[len] = chksum;
	
	bleret = BLE_SendBuf(g_BLESendBuf, len+1);
	return bleret;
}

/******************************************************************
函数名称: BLEResult BLE_AnalysisMobileCommProtocol
功能描述: 蓝牙手机通信协议解析
参数输入: 
参数输出: 
返 回 值: 
其    他: 
******************************************************************/
BLEResult BLE_AnalysisMobileCommProtocol(uint8_t *CodeType, StationPara_t *Para, WorkTime_t *WorkTime, CardInfoTable_t CardTable)
{
	BLEResult bleret;
	uint16_t len, pos, ppos;
	uint16_t i;
	uint8_t chksum;
	uint8_t ctype=0;
	uint8_t recvok=0, ackok=0;
	uint8_t efnc_type=0, efnc_ord=0, efnc_tot=0;
	uint16_t e_pos;
	
	/* 接收蓝牙数据 */
	memset(g_BLERecvBuf, 0x00, BLE_BUFSIZE);
	bleret = BLE_RecvBuf(g_BLERecvBuf, &len);
	if (bleret != BLERET_OK)
	{
		*CodeType = 0x00;
		return BLERET_NODATA_ERR;
	}
	if (len < 16)
		return BLERET_LEN_ERR;
	
	/* 处理蓝牙数据 */
	pos = 0;
	while ((len - pos) >= 16)
	{
		/* 检查包头 */
		if ((g_BLERecvBuf[pos] == 0x5A) && (g_BLERecvBuf[pos+1] == 0xA5))
		{
			/* 获取类型码=人员定位数据 */
			if (g_BLERecvBuf[pos+2] == 0x00)
			{
				recvok = 0;
				/* 获取数据类型 */
				ctype = g_BLERecvBuf[pos+3];
				switch (ctype)
				{
					/* 0x02 = 上下道数据 */
					case 0x02:
						if (((len - pos) >= 16) && (len >= pos))
						{
							chksum = CalcChecksum(g_BLERecvBuf + pos, 15);
							if (chksum == g_BLERecvBuf[pos+15])
							{
								Para->PositionSta = g_BLERecvBuf[pos+4];
								pos += 16;
								recvok = 1;
								ackok = 0;
							}
							else
							{
								bleret = BLERET_CHK_ERR;
							}
						}
						break;
					/* 0x03 = 来车数据 */
					case 0x03:
						if (((len - pos) >= 16) && (len >= pos))
						{
							chksum = CalcChecksum(g_BLERecvBuf + pos, 15);
							if (chksum == g_BLERecvBuf[pos+15])
							{
								Para->TrainingSta = g_BLERecvBuf[pos+4];
								pos += 16;
								recvok = 1;
								ackok = 0;
							}
							else
							{
								bleret = BLERET_CHK_ERR;
							}
						}
						break;

					/* 0x05 = 开始发送定位卡数据 */
					case 0x05:
						if (((len - pos) >= 16) && (len > pos))
						{
							chksum = CalcChecksum(g_BLERecvBuf + pos, 15);
							if (chksum == g_BLERecvBuf[15])
							{
								pos += 16;
								recvok = 1;
								ackok = 0;
							}
							else
							{
								bleret = BLERET_CHK_ERR;
							}
						}
					/* 0x06 = 应答定位数据 */
					case 0x06:
						if (((len - pos) >= 16) && (len > pos))
						{
							chksum = CalcChecksum(g_BLERecvBuf + pos, 15);
							if (chksum == g_BLERecvBuf[15])
							{
								recvok = 1;
								ackok = 0;
							}
							else
							{
								bleret = BLERET_CHK_ERR;
							}
						}
					/* 其他 */
					default:
						break;
				}
				
				/* 接收到数据处理应答 */
				if ((recvok == 1) && (ackok == 0))
				{
					memset(g_BLEDataBuf, 0x00, BLE_BUFSIZE);
					if ((ctype == 0x02) || (ctype == 0x03))
					{
						Para->BoardcastType.Emergent = 1;
						memset(g_BLEDataBuf, 0x00, 10);
						g_BLEDataBuf[0] = g_BLERecvBuf[4];
						BLE_SendMobileCommAck(ctype, g_BLEDataBuf, 1);
					}
					if (ctype == 0x04)
					{
						memset(g_BLEDataBuf, 0x00, 10);
						BLE_SendMobileCommAck(ctype, g_BLEDataBuf, 1);
					}
					if (ctype == 0x01)
					{
						memset(g_BLEDataBuf, 0x00, 10);
						g_BLEDataBuf[0] = g_BLERecvBuf[4];
						g_BLEDataBuf[1] = g_BLERecvBuf[5];
						g_BLEDataBuf[2] = g_BLERecvBuf[6];
						BLE_SendMobileCommAck(ctype, g_BLEDataBuf, 3);
					}
					if (ctype == 0x05)
					{
						g_BLEUploadCard = 1;
						g_tmrBleUploadDelay = uwTick;
					}
					if (ctype == 0x06)
					{
					}
					ackok = 1;
//					break;		//跳出while循环
				}
				
			}
		}
		pos++;
	}
	return bleret;
}




/************************************** 蓝牙应答处理函数 ************************************************/



