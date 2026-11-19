/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: 人员定位
文件名称: LORA_Comm.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: LORA定位卡通信函数
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
//#include "ebyte_core.h"
#include "RYDW_Def.h"
#include "LORA_Comm.h"
#include "LORA_Card.h"


/********* 变量定义 **********/


/************************************** 定位卡 数据处理函数 Demo ************************************************/

/******************************************************************
函数名称: LoraResult LORA_Card_BoardcastFence
功能描述: Lora定位卡广播电子围栏数据
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_BoardcastFence(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo, EFence_t FenceInfo)
{
	uint16_t slen;
	uint16_t i, j, pos;
	uint8_t chksum;
	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_UPSTREAM;
	
	Para->pTxBuf[2] = 136;						//数据长度 数据头(8)+本卡数据(32)+其他数据-电子围栏(96)
	
	Header.PositionSta = 0x00;
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	Para->pTxBuf[3] = LORA_TYPE_EFENCE;
	
	/* 获取定位卡数据 一组32字节 */
	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 11);
	
	/* 电子围栏数据 */
	pos = 0;
	for (i=0; i<3; i++)
	{
		for (j=0; j<4; j++)
		{
			pos = 43 + i * 32 + j * 8;
			ConvUint32ToArray(FenceInfo.Area[i][j].Latitude, Para->pTxBuf + pos, M_LITTLE_ENDIAN);
			ConvUint32ToArray(FenceInfo.Area[i][j].Longitude, Para->pTxBuf + (pos + 4), M_LITTLE_ENDIAN); 
		}
	}
	
	chksum = CheckSum(139, Para->pTxBuf);
	Para->pTxBuf[139] = chksum;
	
	/* 发送数据 */
	Para->TxCpltFlag = 1;
	Para->ucTxBufPos = 140;
//	Ebyte_RF.Send(Para->pTxBuf, 140, 0);
//	Ebyte_RF.EnterReceiveMode(0);			//切换接收模式
	if (Para->TxCpltFlag != 2)
	{
		return LORARET_SEND_FAIL;
	}
	
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Card_BoardcastCardInfo
功能描述: Lora定位卡广播卡定位信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_BoardcastCardInfo(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo, CardData_t *OtherCardInfo[], uint8_t CardNum)
{
	uint16_t slen;
	uint16_t cplen;
	uint16_t pos;
	uint16_t i;
	
	uint8_t chksum;

	
	/* 生成数据头 */
	memset(Para->pTxBuf, 0x00, LORA_TX_BUFSIZE);
	Para->pTxBuf[0] = LORA_HEADER;
	Para->pTxBuf[1] = LORA_UPSTREAM;
	Para->pTxBuf[2] = 136;
	
	Header.PositionSta = 0x00;
	LORA_Header2Arr(Header, Para->pTxBuf + 3);
	Para->pTxBuf[3] = LORA_TYPE_DIFFINFO;
	
	/* 获取自身定位卡数据 一组32字节 */
	LORA_CardInfo2Arr(SelfCardInfo, Para->pTxBuf + 11);
	/* 携带其他定位卡数据 一组32字节 */
	/* 有其他定位卡数据 */
	if (CardNum != 0)
	{
		CardNum = (CardNum <= 3) ? CardNum : 3;
		for (i=0; i<CardNum; i++)
		{
			pos = 43 + i * 32;
			LORA_CardInfo2Arr(OtherCardInfo[i], Para->pTxBuf + pos);
		}
	}
	
	chksum = CheckSum(139, Para->pTxBuf);
	Para->pTxBuf[139] = chksum;
	
	/* 发送数据 */
	Para->TxCpltFlag = 1;
	Para->ucTxBufPos = 140;
//	Ebyte_RF.Send(Para->pTxBuf, 140, 0);
//	Ebyte_RF.EnterReceiveMode(0);			//切换接收模式
	if (Para->TxCpltFlag != 2)
	{
		return LORARET_SEND_FAIL;
	}
	
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseFence
功能描述: Lora定位卡接收基站围栏信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvBaseFence(LoraPara_t *Para, LoraHeader_t *Header, EFence_t *FenceInfo)
{
	uint16_t rlen;
	uint16_t pos, i, j;
	uint8_t chksum;
	
	/* 未接收到数据 */
	if (Para->RxCpltFlag != 2)
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 判断数据头 */
	rlen = 0;
	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
	{
		rlen = Para->pRxBuf[2] + 4;
		/* 检查校验和 */
		chksum = CheckSum(rlen-1, Para->pRxBuf);
		if (chksum == Para->pRxBuf[rlen-1])
		{
			/* 检查数据类型=围栏数据 */
			if (Para->pRxBuf[3] == LORA_TYPE_EFENCE)
			{
				/* 获取基站数据头 */
				LORA_Arr2Header(Para->pRxBuf + 3, Header);
				
				/* 获取电子围栏信息 */
				for (i=0; i<3; i++)
				{
					for (j=0; j<4; j++)
					{
						pos = 11 + i * 32 + j * 8;
						FenceInfo->Area[i][j].Latitude = ConvArrayToUint32(Para->pRxBuf, pos, M_LITTLE_ENDIAN);
						FenceInfo->Area[i][j].Longitude = ConvArrayToUint32(Para->pRxBuf, pos + 4, M_LITTLE_ENDIAN);
					}
				}
			}
			else
			{
				return LORARET_TYPE_ERR;
			}
		}
		else
		{
			return LORARET_CHK_ERR;
		}
	}
	else
	{
		return LORARET_KEYWORD_ERR;
	}
	
	return LORARET_OK;
}

/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseDiffInfo
功能描述: Lora定位卡接收基站差分信息
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 无
******************************************************************/
LoraResult LORA_Card_RecvBaseDiffInfo(LoraPara_t *Para, LoraHeader_t *Header, uint8_t *DiffInfo, uint16_t *DiffLen)
{
	uint16_t rlen;
	uint16_t dlen;
	uint8_t chksum;
	
	/* 未接收到数据 */
	if ((Para->RxCpltFlag != 1) && (Para->RxCpltFlag != 2))
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 判断数据头 */
	rlen = 0;
	if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
	{
		rlen = Para->pRxBuf[2] + 4;
		/* 检查校验和 */
		chksum = CheckSum(rlen-1, Para->pRxBuf);
		if (chksum == Para->pRxBuf[rlen-1])
		{
			/* 检查数据类型=差分数据 */
			if (Para->pRxBuf[3] == LORA_TYPE_DIFFINFO)
			{
				/* 获取基站数据头 */
				LORA_Arr2Header(Para->pRxBuf + 3, Header);
				
				/* 获取差分数据 */
				Para->TotalPktNum = Para->pRxBuf[11];
				Para->PktOrder = Para->pRxBuf[12];
				/* 判断是最后一包数据 */
				if (Para->TotalPktNum == (Para->TotalPktNum - 1))
				{
					memcpy(DiffInfo, Para->pRxBuf + 13, 241);
					*DiffLen = 241;
					return LORARET_OK;
				}
				else
				{
					memcpy(DiffInfo, Para->pRxBuf + 13, 241);
					*DiffLen = 241;
					return LORARET_INCOMPLETE_ERR;
				}
			}
			else
			{
				return LORARET_TYPE_ERR;
			}
		}
		else
		{
			return LORARET_CHK_ERR;
		}
	}
	else
	{
		return LORARET_KEYWORD_ERR;
	}
	
	return LORARET_OK;
}

uint8_t tmpLoraRecvBuf[2048];
/******************************************************************
函数名称: LoraResult LORA_Card_RecvBaseDiffByBlock
功能描述: Lora定位卡接收基站差分信息(阻塞)
参数输入: 
参数输出: 
返 回 值: LoraResult 返回值
其    他: 当接收到差分正确的差分定位数据时，进入阻塞状态等待下一包数据直到超时
		  当接收到差分数据错误时直接返回
******************************************************************/
LoraResult LORA_Card_RecvBaseDiffByBlock(LoraPara_t *Para, LoraHeader_t *Header, uint8_t *DiffInfo, uint16_t *DiffLen, uint32_t Timeout)
{
	uint8_t rlen;
	uint16_t pos;
	uint8_t chksum;
	uint32_t timer;
	
	uint32_t tdiff;	
	uint16_t i;

	uint16_t dlen;
	uint16_t datlen;
	uint16_t datpos;
	uint16_t dbufpos;
	
	/* 未接收到数据 */
	if ((Para->RxCpltFlag != 1) && (Para->RxCpltFlag != 2))
	{
		return LORARET_NODATA_ERR;
	}
	
	/* 拼包 */
	timer = uwTick;		//重置定时器
	dlen = 0;
	
	/* 接收定位数据 */
	while (1)
	{
		/* 接收到数据 */
		if (Para->RxCpltFlag == 2)
		{
			/* 判断数据头 */
			rlen = 0;
			if ((Para->pRxBuf[0] == LORA_HEADER) && (Para->pRxBuf[1] == LORA_DOWNSTREAM))
			{
				rlen = Para->pRxBuf[2] + 4;
				/* 检查校验和 */
				chksum = CheckSum(rlen-1, Para->pRxBuf);
				if (chksum == Para->pRxBuf[rlen-1])
				{
					/* 检查数据类型=差分数据 */
					if (Para->pRxBuf[3] == LORA_TYPE_DIFFINFO)
					{
						timer = uwTick;		//重置定时
						
						/* 获取基站数据头 */
						LORA_Arr2Header(Para->pRxBuf + 3, Header);
						
						/* 获取差分数据信息 */
						Para->TotalPktNum = Para->pRxBuf[11];
						Para->PktOrder = Para->pRxBuf[12];
						pos = Para->PktOrder * 241;
						memcpy(tmpLoraRecvBuf + pos, Para->pRxBuf + 13, 241);
						dlen += 241;
						
						/* 最后一包数据 接收完成 */
						if (Para->PktOrder == (Para->TotalPktNum - 1))
							break;
					}
					else
					{
						return LORARET_TYPE_ERR;
					}
				}
				else
				{
					return LORARET_CHK_ERR;
				}
			}
			else
			{
				return LORARET_KEYWORD_ERR;
			}
		}
		
		/* 判断超时 */
		tdiff = (uwTick >= timer) ? (uwTick - timer) : (0xFFFFFFFF - timer + uwTick);
		if (tdiff > Timeout)
		{
			Para->TotalPktNum = 0;
			Para->PktOrder = 0;
			memset(Para->pRxBuf, 0x00, LORA_RX_BUFSIZE);
			memset(tmpLoraRecvBuf, 0x00, 2048);
			Para->ucRxBufPos = 0;
			Para->RxCpltFlag = 0;
			return LORARET_TIMEOUT_ERR;
		}
		Para->RxCpltFlag = 0;
//		Ebyte_RF.StartPollTask();		//检查数据
	}
	
	/* 计算差分数据长度 */
	i = 0;
	datlen = 0;
	dbufpos = 0;
	do
	{
		if (tmpLoraRecvBuf[i] == 0xD3)
		{
			datlen = 1 + 2 + (uint16_t)tmpLoraRecvBuf[i+1] * 16 + (uint16_t)tmpLoraRecvBuf[i+2] + 3;	//计算数据长度
			if (datlen <= 1023)
			{
				memcpy(DiffInfo + dbufpos, tmpLoraRecvBuf + i, datlen);									//复制D3数据
				dbufpos += datlen;
				i += datlen;
			}
		}
		else
		{
			i++;
		}
	} while (i < dlen);
	*DiffLen = dbufpos;
	
	return LORARET_OK;
}


/***********************接收板*******************************/
uint8_t g_RecIdleFlag = 0;
uint8_t g_RecWarnFlag = 0;
uint8_t g_RecDataFlag = 0;




