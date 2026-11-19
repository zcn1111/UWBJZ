/**
  ******************************************************************************
  * @file    uwb_anchor.c
  * @brief   UWB Anchor (Base Station) Implementation
  * @note    Anchor functions:
  *          1. Broadcast Beacon for time synchronization
  *          2. Receive POLL from tags
  *          3. Send RESPONSE with timestamps
  *          4. Receive FINAL from tags
  *          5. Calculate distances
  ******************************************************************************
  */

#include "bsp.h"
#include "uwb_anchor.h"
#include "uwb_tdma.h"
#include "bsp_dw1000.h"
#include "deca_device_api.h"
#include "deca_regs.h"

/* ==================== Global Variables ==================== */
Anchor_Context_t g_anchorCtx;

/* 基站中断回调状态 */
static volatile uint8_t anc_rx_status = 0;  // 0=等待, 1=收到数据
static uint8_t anc_rx_buffer[64];
static uint16_t anc_rx_len = 0;

static void anc_tx_conf_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;
}

static void anc_rx_ok_cb(const dwt_cb_data_t *cb_data)
{
    if (cb_data->datalength <= 64) {
        dwt_readrxdata(anc_rx_buffer, cb_data->datalength, 0);
        anc_rx_len = cb_data->datalength;
        anc_rx_status = 1;
    }
}

static void anc_rx_to_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;
}

static void anc_rx_err_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;
}

/* ==================== Implementation ==================== */

int Anchor_Init(uint16_t anchorId, uint16_t panId, uint16_t totalSlots)
{
    memset((uint8_t *)&g_anchorCtx, 0, sizeof(Anchor_Context_t));

    g_anchorCtx.anchorId = anchorId;
    g_anchorCtx.panId = panId;
    g_anchorCtx.totalSlots = totalSlots;
    g_anchorCtx.state = ANCHOR_STATE_IDLE;
    g_anchorCtx.beaconSeqNum = 0;
    g_anchorCtx.lastBeaconTime = 0;

    /* 注册DW1000中断回调 */
    dwt_setcallbacks(&anc_tx_conf_cb, &anc_rx_ok_cb, &anc_rx_to_cb, &anc_rx_err_cb);

    /* Enable DW1000 receive */
    Anchor_EnableRx();

    return 0;
}

void Anchor_Process(void)
{
    uint8_t frameType;
    uint32_t currentTime;

    currentTime = uwTick;

    /* 每5秒打印一次DW1000状态和事件计数器 */
    {
        static uint32_t lastDiagTime = 0;
        static uint8_t diagOnce = 0;
        if (diagOnce == 0 || JudgeMilSecondDelayInPro(lastDiagTime, 2000) == true) {
            lastDiagTime = currentTime;
            diagOnce = 1;
            uint32_t status = dwt_read32bitoffsetreg(SYS_STATUS_ID, 0);
            dwt_deviceentcnts_t counters;
            dwt_readeventcounters(&counters);
            debug_printf("[ANCHOR] Status=0x%08X TXF=%u CRCG=%u CRCB=%u SFDTO=%u RTO=%u\r\n",
                status, counters.TXF, counters.CRCG, counters.CRCB, counters.SFDTO, counters.RTO);
        }
    }

    /* 确保DW1000处于RX模式 */
    {
        static uint32_t lastRxEnable = 0;
        if ((currentTime - lastRxEnable) >= 10000) {  /* 每100ms重新使能RX */
            lastRxEnable = currentTime;
            Anchor_EnableRx();
        }
    }

    /* Check if it's time to send beacon */
    if ((currentTime - g_anchorCtx.lastBeaconTime) >= (ANCHOR_BEACON_INTERVAL * 100)) {
        Anchor_SendBeacon();
        g_anchorCtx.lastBeaconTime = currentTime;
        g_anchorCtx.beaconCount++;
    }

    /* Check for received data - 通过中断回调 */
    if (anc_rx_status == 1) {
        anc_rx_status = 0;
        frameType = anc_rx_buffer[0];
        debug_printf("[ANCHOR] Frame type=%d len=%d\r\n", frameType, anc_rx_len);

        switch (frameType) {
            case FRAME_TYPE_POLL:
                Anchor_ProcessPoll(anc_rx_buffer, anc_rx_len);
                break;

            case FRAME_TYPE_FINAL:
                Anchor_ProcessFinal(anc_rx_buffer, anc_rx_len);
                break;

            default:
                break;
        }
    }
}

int Anchor_SendBeacon(void)
{
    BeaconFrame_t beacon;
    uint32_t timeout;

    /* Build beacon frame */
    beacon.frameType = FRAME_TYPE_BEACON;
    beacon.seqNum = g_anchorCtx.beaconSeqNum++;
    beacon.panId = g_anchorCtx.panId;
    beacon.coordinatorId = g_anchorCtx.anchorId;
    beacon.totalSlots = g_anchorCtx.totalSlots;
    beacon.cycleNum = g_anchorCtx.beaconSeqNum;
    beacon.txTimestamp = TDMA_GetUWBTime();
    beacon.slotStartTime = beacon.txTimestamp + UWB_MS_TO_TICKS(5);
    memset(beacon.reserved, 0, sizeof(beacon.reserved));

    /* 使用官方驱动函数发送 */
    dwt_forcetrxoff();  /* 先关闭收发器 */
    dwt_writetxdata(sizeof(BeaconFrame_t), (uint8_t *)&beacon, 0);
    dwt_writetxfctrl(sizeof(BeaconFrame_t) + 2, 0, 0);

    if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) {
        return -1;
    }

    /* 等待发送完成 */
    timeout = 0;
    while (!(dwt_read32bitoffsetreg(SYS_STATUS_ID, 0) & SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) {
            return -1;
        }
    }

    /* 清除发送完成标志 */
    dwt_write32bitoffsetreg(SYS_STATUS_ID, 0, SYS_STATUS_TXFRS);

    /* 重新使能接收 */
    dwt_rxenable(0);

    return 0;
}

int Anchor_ProcessPoll(const uint8_t *data, uint16_t len)
{
    PollFrame_t *poll;
    uint64_t pollRxTime;

    if (len < sizeof(PollFrame_t)) return -1;

    poll = (PollFrame_t *)data;

    /* Validate frame */
    if (poll->frameType != FRAME_TYPE_POLL) return -1;
    if (poll->panId != g_anchorCtx.panId) return -1;

    /* Get POLL receive timestamp */
    pollRxTime = Anchor_GetRxTimestamp();
    debug_printf("[ANCHOR] POLL from Tag=%d\r\n", poll->tagId);

    int respRet = Anchor_SendResponse(poll->tagId, pollRxTime);
    debug_printf("[ANCHOR] RESP ret=%d\r\n", respRet);

    return 0;
}

int Anchor_SendResponse(uint16_t tagId, uint64_t pollRxTime)
{
    ResponseFrame_t resp;
    uint32_t timeout;
    uint64_t respTxTime;

    /* 预计算RESPONSE发送时间戳: pollRxTime + 固定延迟(5ms) */
    respTxTime = pollRxTime + UWB_MS_TO_TICKS(5);

    /* Build RESPONSE frame */
    resp.frameType = FRAME_TYPE_RESPONSE;
    resp.seqNum = 0;
    resp.panId = g_anchorCtx.panId;
    resp.tagId = tagId;
    resp.anchorId = g_anchorCtx.anchorId;
    resp.rxTimestamp = pollRxTime;
    resp.txTimestamp = respTxTime;
    memset(resp.reserved, 0, sizeof(resp.reserved));

    /* 使用官方驱动发送(延迟发送) */
    dwt_forcetrxoff();
    dwt_writetxdata(sizeof(ResponseFrame_t), (uint8_t *)&resp, 0);
    dwt_writetxfctrl(sizeof(ResponseFrame_t) + 2, 0, 0);
    dwt_setdelayedtrxtime((uint32)(respTxTime & 0xFFFFFFFF));

    int txRet = dwt_starttx(DWT_START_TX_DELAYED);
    if (txRet != DWT_SUCCESS) {
        /* 延迟发送失败，改用立即发送 */
        dwt_forcetrxoff();
        dwt_writetxdata(sizeof(ResponseFrame_t), (uint8_t *)&resp, 0);
        dwt_writetxfctrl(sizeof(ResponseFrame_t) + 2, 0, 0);
        if (dwt_starttx(DWT_START_TX_IMMEDIATE) != DWT_SUCCESS) return -1;
    }

    /* 等待发送完成 */
    timeout = 0;
    while (!(dwt_read32bitoffsetreg(SYS_STATUS_ID, 0) & SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) return -1;
    }

    /* 获取实际发送时间戳 */
    respTxTime = Anchor_GetTxTimestamp();

    /* 清除发送完成标志 */
    dwt_write32bitoffsetreg(SYS_STATUS_ID, 0, SYS_STATUS_TXFRS);

    /* Update tag info */
    Anchor_TagInfo_t *tag = Anchor_FindTag(tagId);
    if (tag == NULL && g_anchorCtx.activeTagCount < ANCHOR_MAX_TAGS) {
        tag = &g_anchorCtx.tags[g_anchorCtx.activeTagCount];
        tag->tagId = tagId;
        g_anchorCtx.activeTagCount++;
    }

    if (tag != NULL) {
        tag->pollRxTime = pollRxTime;
        tag->respTxTime = respTxTime;
        tag->lastUpdateTime = TIM2->CNT;
    }

    /* Re-enable RX */
    Anchor_EnableRx();

    return 0;
}

int Anchor_ProcessFinal(const uint8_t *data, uint16_t len)
{
    FinalFrame_t *final;
    uint64_t finalRxTime;

    if (len < sizeof(FinalFrame_t)) return -1;

    final = (FinalFrame_t *)data;

    /* Validate frame */
    if (final->frameType != FRAME_TYPE_FINAL) return -1;
    if (final->panId != g_anchorCtx.panId) return -1;

    /* Get FINAL receive timestamp */
    finalRxTime = Anchor_GetRxTimestamp();

    /* Find tag and calculate distance */
    Anchor_TagInfo_t *tag = Anchor_FindTag(final->tagId);
    if (tag != NULL) {
        tag->finalRxTime = finalRxTime;

        /* Calculate distance using timestamps */
        /* Tround1 = tag side: t4 - t1 (RESPONSE_RX - POLL_TX) */
        /* Treply1 = anchor side: respTxTime - pollRxTime */
        /* Tround2 = tag side: finalTxTime - respRxTime */
        /* Treply2 = anchor side: finalRxTime - respTxTime */

        uint64_t t1 = final->pollTxTime;
        uint64_t t2 = tag->pollRxTime;
        uint64_t t3 = tag->respTxTime;
        uint64_t t4 = final->respRxTime;
        uint64_t t5 = final->finalTxTime;
        uint64_t t6 = finalRxTime;

        /* Convert to seconds */
        double dt1 = (double)t1 * UWB_TICK_TO_S;
        double dt2 = (double)t2 * UWB_TICK_TO_S;
        double dt3 = (double)t3 * UWB_TICK_TO_S;
        double dt4 = (double)t4 * UWB_TICK_TO_S;
        double dt5 = (double)t5 * UWB_TICK_TO_S;
        double dt6 = (double)t6 * UWB_TICK_TO_S;

        /* DS-TWR formula */
        double Tround1 = dt4 - dt1;
        double Treply1 = dt3 - dt2;
        double Tround2 = dt6 - dt5;
        double Treply2 = dt5 - dt4;

        /* Calculate ToF */
        double tof = (Tround1 * Tround2 - Treply1 * Treply2) /
                     (Tround1 + Tround2 + Treply1 + Treply2);

        /* Calculate distance */
        double distance = tof * SPEED_OF_LIGHT;

        /* Sanity check */
        if (distance > 0 && distance < 1000) {
            tag->distance = distance;
            tag->valid = 1;
            g_anchorCtx.rangingCount++;
            debug_printf("[ANCHOR] Tag=%d Distance: %.2f m\r\n", final->tagId, distance);
        }
    }

    return 0;
}

Anchor_TagInfo_t* Anchor_FindTag(uint16_t tagId)
{
    uint16_t i;

    for (i = 0; i < g_anchorCtx.activeTagCount; i++) {
        if (g_anchorCtx.tags[i].tagId == tagId) {
            return &g_anchorCtx.tags[i];
        }
    }

    return NULL;
}

double Anchor_GetTagDistance(uint16_t tagId)
{
    Anchor_TagInfo_t *tag = Anchor_FindTag(tagId);

    if (tag != NULL && tag->valid) {
        return tag->distance;
    }

    return -1.0;
}

uint16_t Anchor_GetActiveTagCount(void)
{
    return g_anchorCtx.activeTagCount;
}

Anchor_State_t Anchor_GetState(void)
{
    return g_anchorCtx.state;
}

void Anchor_EnableRx(void)
{
    dwt_rxenable(0);
}

int Anchor_ReadRxData(uint8_t *buf, uint16_t *len)
{
    uint32 status;

    /* 检查RX FCS Good标志 (帧接收完成) */
    status = dwt_read32bitoffsetreg(SYS_STATUS_ID, 0);
    if (!(status & SYS_STATUS_RXFCG)) return -1;

    /* 读取帧长度 */
    *len = dwt_read32bitoffsetreg(RX_FINFO_ID, 0) & RX_FINFO_RXFL_MASK_1023;
    if (*len > 64 || *len == 0) return -1;

    /* 读取数据 */
    dwt_readrxdata(buf, *len, 0);

    /* 清除RX完成标志并重新使能RX */
    dwt_write32bitoffsetreg(SYS_STATUS_ID, 0, SYS_STATUS_RXFCG);
    dwt_rxenable(0);

    return 0;
}

uint64_t Anchor_GetTxTimestamp(void)
{
    uint8_t buf[5];
    uint64_t timestamp = 0;

    dwt_readtxtimestamp(buf);

    timestamp = (uint64_t)buf[0] |
                ((uint64_t)buf[1] << 8) |
                ((uint64_t)buf[2] << 16) |
                ((uint64_t)buf[3] << 24) |
                ((uint64_t)buf[4] << 32);

    return timestamp;
}

uint64_t Anchor_GetRxTimestamp(void)
{
    uint8_t buf[5];
    uint64_t timestamp = 0;

    dwt_readrxtimestamp(buf);

    timestamp = (uint64_t)buf[0] |
                ((uint64_t)buf[1] << 8) |
                ((uint64_t)buf[2] << 16) |
                ((uint64_t)buf[3] << 24) |
                ((uint64_t)buf[4] << 32);

    return timestamp;
}
