/**
  ******************************************************************************
  * @file    uwb_ranging.c
  * @brief   UWB ranging protocol implementation (SS-TWR, Tag side)
  ******************************************************************************
  */

#include "bsp.h"
#include "uwb_ranging.h"
#include "uwb_tdma.h"
#include "bsp_dw1000.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_types.h"

/* ==================== Internal Variables ==================== */

/* 中断回调状态（参考项目方式） */
#define TX_WAIT  0
#define TX_OK    1
#define RX_WAIT  0
#define RX_OK    1
#define RX_TIMEOUT 2
#define RX_ERROR   3

UwbArgs_t g_UwbInstance;
uint8_t g_UwbTxBuf[UWBTXBUF_SIZE];
uint8_t g_UwbRxBuf[UWBRXBUF_SIZE];
uint16_t g_UwbRxLen;
uint32_t g_UwbRxIrqTimer;
volatile uint8_t tx_irq=TX_WAIT;
volatile uint8_t rx_irq=RX_WAIT;

#define TXIRQ_RESET()	tx_irq = TX_WAIT
#define TXIRQ_OK()		tx_irq = TX_OK
#define RXIRQ_RESET()	rx_irq = RX_WAIT
#define RXIRQ_OK()		rx_irq = RX_OK


#define UWB_TICKS_PER_US  63898  /* 1us in DW1000 device time units */


/* ==================== IRQ Callback ==================== */
/* TX发送成功回调 */
static void range_tx_conf_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;
    TXIRQ_OK();
}

/* RX接收成功回调 */
static void range_rx_ok_cb(const dwt_cb_data_t *cb_data)
{
    if (cb_data->datalength <= (UWB_MACHEAD_LEN + UWB_PAYLOAD_LEN + UWB_FCS_LEN)) 
	{
		memset(g_UwbRxBuf, 0x00, UWBRXBUF_SIZE);
		g_UwbRxLen = 0;
        dwt_readrxdata(g_UwbRxBuf, cb_data->datalength, 0);
        g_UwbRxLen = cb_data->datalength;
        RXIRQ_OK();
    }
}

/* RX接收超时回调 */
static void range_rx_to_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;
    rx_irq = RX_TIMEOUT;
}

/* RX接收错误回调 */
static void range_rx_err_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;
    rx_irq = RX_ERROR;
}

/******************************************************************
函数名称: UWBResult_t Ranging_SendFrame
功能描述: 发送数据帧
参数输入: UwbArgs_t	*UArg	UWB参数
		  uint32_t	Timeout	超时时间(ms)
		  uint8_t	Delay	延迟发送 0=立即发送;1=延迟
		  uint64_t	DelaySt	延迟时间DW1000时间戳 
参数输出: 无
返 回 值: UWBResult_t
其    他:
******************************************************************/
static UWBResult_t Ranging_SendFrame(UwbArgs_t *UArg, uint32_t Timeout, uint8_t Delay, uint64_t DelaySt)
{
    uint64_t dlytime_u64=0;
	uint64_t tx_timestamp=0;
	uint32_t txdelay_u32=0;
	uint32_t tt=0;
	int dwtret;
	
	/* 有效性检查 */
	if ((Delay == 1) && (DelaySt > 0))
	{
		dlytime_u64 = DelaySt;
	}
	memset(g_UwbTxBuf, 0x00, UWBTXBUF_SIZE);
	
	/* 组织MAC头 控制字 */
	g_UwbTxBuf[0] = 0x41;
	g_UwbTxBuf[1] = 0x88;
	g_UwbTxBuf[2] = UArg->FrameSeq;
	g_UwbTxBuf[3] = UArg->PanID & 0xFF;		//网络ID
	g_UwbTxBuf[4] = UArg->PanID >> 8;
	g_UwbTxBuf[5] = UArg->DestID & 0xFF;	//目标ID
	g_UwbTxBuf[6] = UArg->DestID >> 8;
	g_UwbTxBuf[7] = UArg->DevID & 0xFF;		//源ID
	g_UwbTxBuf[8] = UArg->DevID >> 8;
	
    /* 数据写入缓冲区 */
	if (UArg->CommState == UWB_SEND_POLL)
	{
		g_UwbTxBuf[9] = POLL_FRAME;
		UArg->TwrCommArg.TsStatus.all = 0;
	}
	else if (UArg->CommState == UWB_SEND_RESP)
	{
		g_UwbTxBuf[9] = RESP_FRAME;
	}
	else if (UArg->CommState == UWB_SEND_FINAL)
	{
		g_UwbTxBuf[9] = FINAL_FRAME;
	}
	g_UwbTxBuf[10] = UArg->TwrCommArg.TsStatus.all;
	g_UwbTxBuf[11] = UArg->SlotOrder;
	ConvUint32ToArray((uint32_t)UArg->TwrCommArg.u32PollSendTs, g_UwbTxBuf + 12, M_LITTLE_ENDIAN);
	ConvUint32ToArray((uint32_t)UArg->TwrCommArg.u32PollRecvTs, g_UwbTxBuf + 16, M_LITTLE_ENDIAN);
	ConvUint32ToArray((uint32_t)UArg->TwrCommArg.u32RespSendTs, g_UwbTxBuf + 20, M_LITTLE_ENDIAN);
	ConvUint32ToArray((uint32_t)UArg->TwrCommArg.u32RespRecvTs, g_UwbTxBuf + 24, M_LITTLE_ENDIAN);
	memcpy(g_UwbTxBuf + 28, (uint8_t*)&UArg->TwrCommArg.dDistance_m, 4);
	ConvUint16ToArray((uint16_t)(UArg->TwrCommArg.fRSSI * 100), g_UwbTxBuf + 32, M_LITTLE_ENDIAN);

//    memcpy(g_UwbTxBuf + 9, Dat, DatLen);
    /* 写入寄存器和控制 */
	dwt_writetxdata(UWB_MACHEAD_LEN + UWB_PAYLOAD_LEN + UWB_FCS_LEN, g_UwbTxBuf, 0);
	dwt_writetxfctrl(UWB_MACHEAD_LEN + UWB_PAYLOAD_LEN + UWB_FCS_LEN, 0, 1);
    if (dlytime_u64 > 0)
    {
        txdelay_u32 = (uint32_t)(DelaySt >> 8);
        dwt_setdelayedtrxtime(txdelay_u32);         	//设置延迟发送时间
		dwtret = dwt_starttx(DWT_START_TX_DELAYED);		//延迟发送
		tt = uwTick;
		TXIRQ_RESET();
    }
	else
	{
		dwtret = dwt_starttx(DWT_START_TX_IMMEDIATE);	//立即发送
		tt = uwTick;
		TXIRQ_RESET();
	}
	if (dwtret == DWT_ERROR)
		return UWBRET_SENDERR;

	/* 等待发送成功或超时 */
	while (JudgeTick(tt, Timeout) == false)
	{
		if (tx_irq == TX_OK)
		{
			tx_timestamp = get_tx_timestamp_u64();
			/* 发送Poll帧 */
			if (UArg->CommState == UWB_SEND_POLL)
			{
				UArg->TwrCommArg.u32PollSendTs = (uint32_t)tx_timestamp;
				UArg->TwrCommArg.TsStatus.bit.PollSendValid = 1;
			}
			/* 发送Resp帧 */
			else if (UArg->CommState == UWB_SEND_RESP)
			{
				UArg->TwrCommArg.u32RespSendTs = (uint32_t)tx_timestamp;
				UArg->TwrCommArg.TsStatus.bit.RespSendValid = 1;
			}
			/* 发送Final帧 */
			else if (UArg->CommState == UWB_SEND_FINAL)
			{
				
			}
			return UWBRET_OK;
		}
	}
	
	return UWBRET_SENDTIMEOUT;
    
}

/******************************************************************
函数名称: UWBResult_t Ranging_RecvFrame
功能描述: 接收数据帧
参数输入: UwbArgs_t	*UArg	UWB参数
参数输出: 无
返 回 值: UWBResult_t
其    他:
******************************************************************/
static UWBResult_t Ranging_RecvFrame(UwbArgs_t *UArg, uint8_t *Buf, uint16_t BufLen)
{
	uint16_t dstid=0;
	uint16_t srcid=0;
	uint16_t panid=0;
	uint8_t recvok=0;
	uint32_t rx_timestamp=0;
	dwt_rxdiag_t rxdiag;
	
	/* 处理MAC头 */
	if ((Buf[0] == 0x41) && (Buf[1] == 0x88))
	{
		panid = ConvArrayToUint16(Buf, 3, M_LITTLE_ENDIAN);
		dstid = ConvArrayToUint16(Buf, 5, M_LITTLE_ENDIAN);
		srcid = ConvArrayToUint16(Buf, 7, M_LITTLE_ENDIAN);
		/* 给自己的数据 */
		/* 网络ID相同 且 目前地址是自己或广播 */
		if (panid != UArg->PanID)
		{
			return UWBRET_PANIDERR;
		}
		if ((dstid != UArg->DevID) && (dstid != 0xFFFF))
		{
			return UWBRET_DSTIDERR;
		}
		UArg->RecvDstID = dstid;
		UArg->RecvSrcID = srcid;
	}
	else
	{
		return UWBRET_MACERR;
	}
	
	/* 接收Poll帧 */
	if ((UArg->CommState == UWB_RECV_POLL) && (Buf[9] == POLL_FRAME))
	{
		rx_timestamp = get_rx_timestamp_u64();	//获取时间戳
		UArg->TwrCommArg.u32PollRecvTs = (uint32_t)rx_timestamp;
		UArg->TwrCommArg.TsStatus.bit.PollRecvValid = 1;
	}
	/* 接收Resp帧 */
	else if ((UArg->CommState == UWB_RECV_RESP) && (Buf[9] == RESP_FRAME))
	{
		rx_timestamp = get_rx_timestamp_u64();	//获取时间戳
		UArg->TwrCommArg.u32RespRecvTs = (uint32_t)rx_timestamp;
		UArg->TwrCommArg.TsStatus.bit.RespRecvValid = 1;
	}
	/* 接收Final帧 */
	else if ((UArg->CommState == UWB_RECV_FINAL) && (Buf[9] == FINAL_FRAME))
	{
		/* 当前SS-TWR模式不处理Final帧 */
		return UWBRET_OTHER;
	}
	else
	{
		/* 帧类型与当前通信状态不匹配 */
		return UWBRET_MACERR;
	}
	
	dwt_readdiagnostics(&rxdiag);				//获取监测数据
	UArg->TwrCommArg.fRSSI = rxdiag.rxPower;	//获取信号强度
	return UWBRET_OK;
	
}

/******************************************************************
函数名称: UWBResult_t Ranging_CalcDistance
功能描述: 计算距离
参数输入: UwbArgs_t	*UArg	UWB参数
参数输出: 无
返 回 值: UWBResult_t
其    他:
******************************************************************/
static UWBResult_t Ranging_CalcDistance(UwbArgs_t *UArg)
{
	uint64_t t_round=0, t_reply=0;
	uint64_t tof_ticks=0;
	double dd=0.0;
	
	UArg->TwrCommArg.dDistance_m = -1;
	UArg->TwrCommArg.TwrStatus = UWB_TWR_ERR;
	
	/* 标签在接收到4个时间戳时测距 */
	if (UArg->InstanceMode == MODE_TAG)
	{
		/* 接收到4个有效时间戳 */
		if (((UArg->TwrCommArg.TsStatus.all & 0x0F) == 0x0F) &&
			(UArg->TwrCommArg.u32PollSendTs != TIMESTAMP_NULL) && 
			(UArg->TwrCommArg.u32PollRecvTs != TIMESTAMP_NULL) &&
			(UArg->TwrCommArg.u32RespSendTs != TIMESTAMP_NULL) &&
			(UArg->TwrCommArg.u32RespRecvTs != TIMESTAMP_NULL))
		{
			t_round = UArg->TwrCommArg.u32RespRecvTs - UArg->TwrCommArg.u32PollSendTs;
			t_reply = UArg->TwrCommArg.u32RespSendTs - UArg->TwrCommArg.u32PollRecvTs;
			tof_ticks = (t_round - t_reply) / 2;
			dd = (double)tof_ticks * DWT_TIME_UNITS * SPEED_OF_LIGHT;
			if ((dd > 0) && (dd < 1000))
			{
				UArg->TwrCommArg.dDistance_m = dd;
				UArg->TwrCommArg.TwrStatus = UWB_TWR_OK;
			}
		}
		else
		{
			UArg->TwrCommArg.dDistance_m = 0.0;
			UArg->TwrCommArg.TwrStatus = UWB_TWR_ERR;
		}
	}
	/* 基站不测距 */
	else if ((UArg->InstanceMode == MODE_ANCHOR_MASTER) || (UArg->InstanceMode == MODE_ANCHOR_SLAVE))
	{
		UArg->TwrCommArg.dDistance_m = 0;
		UArg->TwrCommArg.TwrStatus = UWB_TWR_NONE;
	}
	
	return UWBRET_OK;
}

/******************************************************************
函数名称: void Ranging_ChangeState
功能描述: 切换状态
参数输入: UwbArgs_t	*UArg	UWB参数
参数输出: 无
返 回 值: UWBResult_t
其    他:
******************************************************************/
static void Ranging_ChangeState(UwbArgs_t *UArg, UwbCommState_e NewState)
{
	UArg->CommState = NewState;
}

/* ==================== Implementation ==================== */
/******************************************************************
函数名称: UWBResult_t Ranging_Init
功能描述: 初始化
参数输入: UwbArgs_t	*UArg	UWB参数
参数输出: 无
返 回 值: UWBResult_t
其    他:
******************************************************************/
UWBResult_t Ranging_Init(UwbArgs_t *UArg, uint8_t SetInstanceMode, uint16_t SetDevID, uint16_t SetPanID, dwt_config_t *ptrDwtConfig)
{
	/* 配置参数 */
	if (ptrDwtConfig == NULL)
	{
		return UWBRET_CONFIGERR;
	}
	else
	{
		UArg->AirCommArg.ChNum = ptrDwtConfig->chan;
		UArg->AirCommArg.DataRate = ptrDwtConfig->dataRate;
		UArg->AirCommArg.PrfNum = ptrDwtConfig->prf;
	}
	
	/* 初始化编号 */
	UArg->InstanceMode = SetInstanceMode;
	if ((SetInstanceMode != MODE_TAG) && (SetInstanceMode != MODE_ANCHOR_MASTER) && (SetInstanceMode != MODE_ANCHOR_SLAVE))
		UArg->InstanceMode = MODE_TAG;
	UArg->CommState = UWB_IDLE;
	UArg->PanID = SetPanID;
	UArg->DevID = SetDevID;
	UArg->DestID = 0;
	UArg->SlotOrder = 0;
	UArg->FrameSeq = 0;
	
	UArg->RecvDstID = 0;
	UArg->RecvSrcID = 0;
	
	UArg->AirCommArg.OneSlotTime_ms = ONE_SLOT_TIME;
	UArg->NextPeriod_ms = uwTick + ONE_SLOT_TIME + MAX_SLOT_NUMBER;
	UArg->RxIrqTimeout_ms = 1000;
	UArg->RecvTimeout_ms = 500;
	g_UwbRxIrqTimer = uwTick;
	
	/* 初始化测距状态 */
	UArg->TwrCommArg.TwrStatus = UWB_TWR_NONE;	//测距状态无
	UArg->TwrCommArg.TsStatus.all = 0;
	UArg->TwrCommArg.u32PollSendTs = TIMESTAMP_NULL;
	UArg->TwrCommArg.u32PollRecvTs = TIMESTAMP_NULL;
	UArg->TwrCommArg.u32RespSendTs = TIMESTAMP_NULL;
	UArg->TwrCommArg.u32RespRecvTs = TIMESTAMP_NULL;
	UArg->TwrCommArg.dDistance_m = 0.0;
	UArg->TwrCommArg.fRSSI = 0.0;
	
    /* 注册DW1000中断回调（参考项目方式） */
    dwt_setcallbacks(&range_tx_conf_cb, &range_rx_ok_cb, &range_rx_to_cb, &range_rx_err_cb);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
	return UWBRET_OK;
}

/******************************************************************
函数名称: UWBResult_t Ranging_Process
功能描述: 检测状态机
参数输入: UwbArgs_t	*UArg	UWB参数
参数输出: 无
返 回 值: UWBResult_t
其    他:
******************************************************************/
UWBResult_t Ranging_Process(UwbArgs_t *UArg)
{
	uint32_t slottime=0;
	UWBResult_t uwbret;
	
	/* ==================== 标签状态机 ==================== */
	if (UArg->InstanceMode == MODE_TAG)
	{
		switch (UArg->CommState)
		{
			/* 状态 空闲，等待下一周期 */
			case UWB_IDLE:
				if (uwTick >= UArg->NextPeriod_ms)
				{
					UArg->TwrCommArg.TwrStatus = UWB_TWR_NONE;
					Ranging_ChangeState(UArg, UWB_SEND_POLL);
				}
				break;

			/* 状态 发送POLL帧 */
			case UWB_SEND_POLL:
				UArg->DestID = ID_ANCHOR_FLAG | 0x0000;
				UArg->FrameSeq++;
				uwbret = Ranging_SendFrame(UArg, 10, 0, 0);			//立即发送
				if (uwbret == UWBRET_OK)
				{
					dwt_setrxtimeout(UArg->RecvTimeout_ms * UWB_TICKS_PER_US);	//设置接收超时
					dwt_rxenable(DWT_START_RX_IMMEDIATE);				//使能接收等待RESP
					RXIRQ_RESET();
					Ranging_ChangeState(UArg, UWB_WAITING_RESP);
				}
				else
				{
					dwt_forcetrxoff();									//硬件恢复
					slottime = UArg->AirCommArg.OneSlotTime_ms * MAX_SLOT_NUMBER;
					UArg->NextPeriod_ms = uwTick + slottime;
					Ranging_ChangeState(UArg, (uwbret == UWBRET_SENDERR) ? UWB_TX_ERROR : UWB_TX_TIMEOUT);
				}
				break;

			/* 状态 等待RESP帧（由中断回调驱动） */
			case UWB_WAITING_RESP:
				if (rx_irq == RX_OK)
				{
					Ranging_ChangeState(UArg, UWB_RECV_RESP);
				}
				else if (rx_irq == RX_ERROR || rx_irq == RX_TIMEOUT)
				{
					dwt_forcetrxoff();									//硬件恢复
					slottime = UArg->AirCommArg.OneSlotTime_ms * MAX_SLOT_NUMBER;
					UArg->NextPeriod_ms = uwTick + slottime;
					Ranging_ChangeState(UArg, (rx_irq == RX_ERROR) ? UWB_RX_ERROR : UWB_RX_TIMEOUT);
				}
				RXIRQ_RESET();
				break;

			/* 状态 接收并处理RESP帧 */
			case UWB_RECV_RESP:
				uwbret = Ranging_RecvFrame(UArg, g_UwbRxBuf, g_UwbRxLen);
				if (uwbret == UWBRET_OK)
				{
					Ranging_CalcDistance(UArg);						//SS-TWR计算距离
				}
				else
				{
					UArg->TwrCommArg.dDistance_m = -1;
					UArg->TwrCommArg.TwrStatus = UWB_TWR_ERR;
				}
				/* 无论成功或失败，设置下一周期并回到空闲 */
				slottime = UArg->AirCommArg.OneSlotTime_ms * MAX_SLOT_NUMBER;
				UArg->NextPeriod_ms = uwTick + slottime;
				dwt_forcetrxoff();
				dwt_rxenable(DWT_START_RX_IMMEDIATE);
				Ranging_ChangeState(UArg, UWB_IDLE);
				break;

			/* 错误恢复状态：硬件复位后回到空闲 */
			case UWB_TX_ERROR:
			case UWB_TX_TIMEOUT:
			case UWB_RX_ERROR:
			case UWB_RX_TIMEOUT:
				dwt_forcetrxoff();
				dwt_rxenable(DWT_START_RX_IMMEDIATE);
				Ranging_ChangeState(UArg, UWB_IDLE);
				break;

			default:
				dwt_forcetrxoff();
				dwt_rxenable(DWT_START_RX_IMMEDIATE);
				Ranging_ChangeState(UArg, UWB_IDLE);
				break;
		}
	}
	/* ==================== 基站状态机 ==================== */
	else if (UArg->InstanceMode == MODE_ANCHOR_MASTER || UArg->InstanceMode == MODE_ANCHOR_SLAVE)
	{
		switch (UArg->CommState)
		{
			/* 状态 空闲，等待接收POLL */
			case UWB_IDLE:
				if (rx_irq == RX_OK)
				{
					Ranging_ChangeState(UArg, UWB_RECV_POLL);
					g_UwbRxIrqTimer = uwTick;
				}
				else if (rx_irq == RX_ERROR || rx_irq == RX_TIMEOUT)
				{
					RXIRQ_RESET();
					dwt_forcetrxoff();
					dwt_setrxtimeout(0);							//清除超时，一直接收
					dwt_rxenable(DWT_START_RX_IMMEDIATE);
				}
				else
				{
					/* 定期重新使能RX，防止DW1000意外退出接收模式 */
					if (JudgeTick(g_UwbRxIrqTimer, UArg->RxIrqTimeout_ms) == true)
					{
						g_UwbRxIrqTimer = uwTick;
						dwt_setrxtimeout(0);
						dwt_rxenable(DWT_START_RX_IMMEDIATE);
					}
				}
				break;

			/* 状态 接收并处理POLL帧 */
			case UWB_RECV_POLL:
				uwbret = Ranging_RecvFrame(UArg, g_UwbRxBuf, g_UwbRxLen);
				RXIRQ_RESET();
				if (uwbret == UWBRET_OK)
				{
					/* 计算RESP延迟发送时间：POLL接收时间 + 固定延迟 */
					UArg->TwrCommArg.u32RespSendTs = UArg->TwrCommArg.u32PollRecvTs + UWB_MS_TO_TICKS(5);
					UArg->DestID = UArg->RecvSrcID;					//回复给发送POLL的标签
					Ranging_ChangeState(UArg, UWB_SEND_RESP);
				}
				else
				{
					dwt_forcetrxoff();
					dwt_rxenable(DWT_START_RX_IMMEDIATE);
					Ranging_ChangeState(UArg, UWB_IDLE);
				}
				break;

			/* 状态 发送RESP帧 */
			case UWB_SEND_RESP:
				uwbret = Ranging_SendFrame(UArg, 10, 1, UArg->TwrCommArg.u32RespSendTs);	//延迟发送
				dwt_forcetrxoff();
				dwt_rxenable(DWT_START_RX_IMMEDIATE);					//重新使能接收，等待下一个POLL
				if (uwbret == UWBRET_OK)
				{
					UArg->TwrCommArg.TwrStatus = UWB_TWR_NONE;		//基站不计算距离
				}
				Ranging_ChangeState(UArg, UWB_IDLE);
				break;

			/* 错误恢复状态 */
			case UWB_TX_ERROR:
			case UWB_TX_TIMEOUT:
			case UWB_RX_ERROR:
			case UWB_RX_TIMEOUT:
				dwt_forcetrxoff();
				dwt_rxenable(DWT_START_RX_IMMEDIATE);
				Ranging_ChangeState(UArg, UWB_IDLE);
				break;

			default:
				dwt_forcetrxoff();
				dwt_rxenable(DWT_START_RX_IMMEDIATE);
				Ranging_ChangeState(UArg, UWB_IDLE);
				break;
		}
	}
	
	return UWBRET_OK;
}


