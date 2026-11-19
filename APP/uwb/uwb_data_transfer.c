/**
  ******************************************************************************
  * @file    uwb_data_transfer.c
  * @brief   UWB数据传输模块 - 实现分包发送和接收功能
  * @note    支持RTK差分数据等大数据量传输
  ******************************************************************************
  */

#include "bsp.h"
#include "uwb_tdma.h"
#include "uwb_data_transfer.h"
#include "bsp_dw1000.h"
#include <string.h>

/* ==================== Global Variables ==================== */
DataTransferCtx_t g_dataTransferCtx;

/* ==================== Helper Functions ==================== */

/**
 * @brief  CRC16-CCITT校验计算
 */
uint16_t DataTransfer_CalcCRC16(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    uint16_t i, j;

    for (i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    return crc;
}

/* ==================== Mode Control Functions ==================== */

/**
 * @brief  设置UWB工作模式
 */
void UWB_SetMode(UWB_WorkMode_t mode)
{
    g_modeCtrl.currentMode = mode;
    g_modeCtrl.switchTime = TIM2->CNT;

    if (mode == UWB_MODE_COMM) {
        g_modeCtrl.commModeTimeout = DATA_TRANSFER_TIMEOUT_MS * 100; // 转换为tick
    }

    debug_printf("[UWB] Mode switch to: %s\r\n",
                 (mode == UWB_MODE_RANGING) ? "RANGING" : "COMM");
}

/**
 * @brief  获取当前UWB工作模式
 */
UWB_WorkMode_t UWB_GetMode(void)
{
    return g_modeCtrl.currentMode;
}

/**
 * @brief  是否为通信模式
 */
uint8_t UWB_IsCommMode(void)
{
    return (g_modeCtrl.currentMode == UWB_MODE_COMM);
}

/**
 * @brief  是否为测距模式
 */
uint8_t UWB_IsRangingMode(void)
{
    return (g_modeCtrl.currentMode == UWB_MODE_RANGING);
}

/* ==================== Frame Send Functions ==================== */

/**
 * @brief  发送数据传输开始帧
 */
int UWB_SendDataStart(uint16_t dataId, uint8_t totalPackets, uint16_t totalLen)
{
    DataStartFrame_t frame;
    uint8_t txfctrl[5];
    uint32_t timeout;

    frame.frameType = FRAME_TYPE_DATA_START;
    frame.reserved = 0;
    frame.dataId = dataId;
    frame.totalPackets = totalPackets;
    frame.totalDataLen = totalLen;
    memset(frame.reserved2, 0, sizeof(frame.reserved2));

    DW1000_SPI_Write(DW1000_REG_TX_BUFFER, (uint8_t *)&frame, sizeof(DataStartFrame_t));
    DW1000_ClearSysStatus(0xFFFFFFFF);

    txfctrl[0] = sizeof(DataStartFrame_t) + 2;
    txfctrl[1] = 0x00;
    txfctrl[2] = 0x00;
    txfctrl[3] = 0x00;
    txfctrl[4] = 0x00;
    DW1000_SPI_Write(DW1000_REG_TX_FCTRL, txfctrl, 5);

    DW1000_SPI_WriteByte(0x0D, 0x02);  /* TXSTRT */

    timeout = 0;
    while (!(DW1000_ReadSysStatus() & DW1000_SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) return -1;
    }

    return 0;
}

/**
 * @brief  发送数据包帧
 */
int UWB_SendDataPacket(uint16_t dataId, uint8_t packetIndex, uint8_t totalPackets,
                       const uint8_t *data, uint8_t dataLen)
{
    DataPacketFrame_t frame;
    uint8_t txfctrl[5];
    uint32_t timeout;

    if (dataLen > DATA_PACKET_MAX_LEN) return -1;

    frame.frameType = FRAME_TYPE_DATA_PACKET;
    frame.packetIndex = packetIndex;
    frame.totalPackets = totalPackets;
    frame.dataLen = dataLen;
    frame.dataId = dataId;
    memcpy(frame.data, data, dataLen);
    if (dataLen < DATA_PACKET_MAX_LEN) {
        memset(&frame.data[dataLen], 0, DATA_PACKET_MAX_LEN - dataLen);
    }
    frame.crc16 = DataTransfer_CalcCRC16(data, dataLen);

    DW1000_SPI_Write(DW1000_REG_TX_BUFFER, (uint8_t *)&frame, sizeof(DataPacketFrame_t));
    DW1000_ClearSysStatus(0xFFFFFFFF);

    txfctrl[0] = sizeof(DataPacketFrame_t) + 2;
    txfctrl[1] = 0x00;
    txfctrl[2] = 0x00;
    txfctrl[3] = 0x00;
    txfctrl[4] = 0x00;
    DW1000_SPI_Write(DW1000_REG_TX_FCTRL, txfctrl, 5);

    DW1000_SPI_WriteByte(0x0D, 0x02);  /* TXSTRT */

    timeout = 0;
    while (!(DW1000_ReadSysStatus() & DW1000_SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) return -1;
    }

    return 0;
}

/**
 * @brief  发送数据确认帧
 */
int UWB_SendDataAck(uint8_t ackPacketIndex, uint8_t status, uint8_t nextExpected)
{
    DataAckFrame_t frame;
    uint8_t txfctrl[5];
    uint32_t timeout;

    frame.frameType = FRAME_TYPE_DATA_ACK;
    frame.ackPacketIndex = ackPacketIndex;
    frame.status = status;
    frame.nextExpectedIndex = nextExpected;

    DW1000_SPI_Write(DW1000_REG_TX_BUFFER, (uint8_t *)&frame, sizeof(DataAckFrame_t));
    DW1000_ClearSysStatus(0xFFFFFFFF);

    txfctrl[0] = sizeof(DataAckFrame_t) + 2;
    txfctrl[1] = 0x00;
    txfctrl[2] = 0x00;
    txfctrl[3] = 0x00;
    txfctrl[4] = 0x00;
    DW1000_SPI_Write(DW1000_REG_TX_FCTRL, txfctrl, 5);

    DW1000_SPI_WriteByte(0x0D, 0x02);  /* TXSTRT */

    timeout = 0;
    while (!(DW1000_ReadSysStatus() & DW1000_SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) return -1;
    }

    return 0;
}

/**
 * @brief  发送数据传输结束帧
 */
int UWB_SendDataEnd(uint16_t dataId, uint16_t totalCrc)
{
    DataEndFrame_t frame;
    uint8_t txfctrl[5];
    uint32_t timeout;

    frame.frameType = FRAME_TYPE_DATA_END;
    frame.reserved = 0;
    frame.dataId = dataId;
    frame.totalCrc16 = totalCrc;
    memset(frame.reserved2, 0, sizeof(frame.reserved2));

    DW1000_SPI_Write(DW1000_REG_TX_BUFFER, (uint8_t *)&frame, sizeof(DataEndFrame_t));
    DW1000_ClearSysStatus(0xFFFFFFFF);

    txfctrl[0] = sizeof(DataEndFrame_t) + 2;
    txfctrl[1] = 0x00;
    txfctrl[2] = 0x00;
    txfctrl[3] = 0x00;
    txfctrl[4] = 0x00;
    DW1000_SPI_Write(DW1000_REG_TX_FCTRL, txfctrl, 5);

    DW1000_SPI_WriteByte(0x0D, 0x02);  /* TXSTRT */

    timeout = 0;
    while (!(DW1000_ReadSysStatus() & DW1000_SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) return -1;
    }

    return 0;
}

/**
 * @brief  发送模式切换帧
 */
int UWB_SendModeSwitch(UWB_WorkMode_t targetMode)
{
    ModeSwitchFrame_t frame;
    uint8_t txfctrl[5];
    uint32_t timeout;

    frame.frameType = FRAME_TYPE_MODE_SWITCH;
    frame.targetMode = (uint8_t)targetMode;
    memset(frame.reserved, 0, sizeof(frame.reserved));

    DW1000_SPI_Write(DW1000_REG_TX_BUFFER, (uint8_t *)&frame, sizeof(ModeSwitchFrame_t));
    DW1000_ClearSysStatus(0xFFFFFFFF);

    txfctrl[0] = sizeof(ModeSwitchFrame_t) + 2;
    txfctrl[1] = 0x00;
    txfctrl[2] = 0x00;
    txfctrl[3] = 0x00;
    txfctrl[4] = 0x00;
    DW1000_SPI_Write(DW1000_REG_TX_FCTRL, txfctrl, 5);

    DW1000_SPI_WriteByte(0x0D, 0x02);  /* TXSTRT */

    timeout = 0;
    while (!(DW1000_ReadSysStatus() & DW1000_SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) return -1;
    }

    return 0;
}

/* ==================== Transfer Control Functions ==================== */

/**
 * @brief  数据传输模块初始化
 */
void DataTransfer_Init(void)
{
    memset(&g_dataTransferCtx, 0, sizeof(DataTransferCtx_t));
    g_dataTransferCtx.state = TRANSFER_IDLE;
    g_dataTransferCtx.rxExpectedPacket = 0;
}

/**
 * @brief  重置传输上下文
 */
void DataTransfer_Reset(void)
{
    DataTransfer_Init();
}

/**
 * @brief  开始发送数据
 */
int DataTransfer_StartSend(uint16_t dataId, const uint8_t *data, uint16_t len)
{
    if (len > DATA_TRANSFER_MAX_SIZE) return -1;
    if (g_dataTransferCtx.state != TRANSFER_IDLE) return -1;

    g_dataTransferCtx.txData = data;
    g_dataTransferCtx.txDataLen = len;
    g_dataTransferCtx.dataId = dataId;
    g_dataTransferCtx.txTotalPackets = (len + DATA_PACKET_MAX_LEN - 1) / DATA_PACKET_MAX_LEN;
    g_dataTransferCtx.txCurrentPacket = 0;
    g_dataTransferCtx.txAckReceived = 0;
    g_dataTransferCtx.txRetryCount = 0;
    g_dataTransferCtx.state = TRANSFER_TX_SENDING;
    g_dataTransferCtx.lastActivityTime = TIM2->CNT;

    return 0;
}

/**
 * @brief  发送处理函数
 */
TransferState_t DataTransfer_ProcessSend(void)
{
    uint16_t offset;
    uint8_t chunkLen;

    if (g_dataTransferCtx.state != TRANSFER_TX_SENDING) {
        return g_dataTransferCtx.state;
    }

    /* 超时检查 */
    if (DataTransfer_CheckTimeout()) {
        g_dataTransferCtx.state = TRANSFER_TIMEOUT;
        return TRANSFER_TIMEOUT;
    }

    /* 发送数据开始帧 */
    if (g_dataTransferCtx.txCurrentPacket == 0 && !g_dataTransferCtx.txAckReceived) {
        UWB_SendDataStart(g_dataTransferCtx.dataId,
                          g_dataTransferCtx.txTotalPackets,
                          g_dataTransferCtx.txDataLen);
        g_dataTransferCtx.lastActivityTime = TIM2->CNT;
        return g_dataTransferCtx.state;
    }

    /* 发送数据包 */
    if (g_dataTransferCtx.txAckReceived || g_dataTransferCtx.txCurrentPacket == 0) {
        offset = g_dataTransferCtx.txCurrentPacket * DATA_PACKET_MAX_LEN;
        chunkLen = g_dataTransferCtx.txDataLen - offset;
        if (chunkLen > DATA_PACKET_MAX_LEN) chunkLen = DATA_PACKET_MAX_LEN;

        UWB_SendDataPacket(g_dataTransferCtx.dataId,
                           g_dataTransferCtx.txCurrentPacket,
                           g_dataTransferCtx.txTotalPackets,
                           &g_dataTransferCtx.txData[offset],
                           chunkLen);

        g_dataTransferCtx.txAckReceived = 0;
        g_dataTransferCtx.lastActivityTime = TIM2->CNT;

        /* 检查是否发送完成 */
        if (g_dataTransferCtx.txCurrentPacket >= g_dataTransferCtx.txTotalPackets - 1) {
            /* 发送结束帧 */
            uint16_t totalCrc = DataTransfer_CalcCRC16(g_dataTransferCtx.txData,
                                                       g_dataTransferCtx.txDataLen);
            UWB_SendDataEnd(g_dataTransferCtx.dataId, totalCrc);
            g_dataTransferCtx.state = TRANSFER_TX_COMPLETE;
        }
    }

    return g_dataTransferCtx.state;
}

/**
 * @brief  处理接收到的数据帧
 */
int DataTransfer_ProcessReceive(const uint8_t *data, uint16_t len)
{
    if (len < 1) return -1;

    uint8_t frameType = data[0];

    switch (frameType) {
        case FRAME_TYPE_DATA_START:
            if (len >= sizeof(DataStartFrame_t)) {
                DataStartFrame_t *start = (DataStartFrame_t *)data;
                g_dataTransferCtx.dataId = start->dataId;
                g_dataTransferCtx.rxTotalPackets = start->totalPackets;
                g_dataTransferCtx.rxDataLen = 0;
                g_dataTransferCtx.rxReceivedPackets = 0;
                g_dataTransferCtx.rxExpectedPacket = 0;
                g_dataTransferCtx.rxComplete = 0;
                g_dataTransferCtx.state = TRANSFER_RX_RECEIVING;
                g_dataTransferCtx.lastActivityTime = TIM2->CNT;

                debug_printf("[UWB] DataStart: id=%u packets=%u len=%u\r\n",
                             start->dataId, start->totalPackets, start->totalDataLen);

                /* 发送ACK */
                UWB_SendDataAck(0, 0, 0);
            }
            break;

        case FRAME_TYPE_DATA_PACKET:
            if (len >= sizeof(DataPacketFrame_t)) {
                DataPacketFrame_t *packet = (DataPacketFrame_t *)data;
                DataTransfer_HandleDataPacket(packet);
            }
            break;

        case FRAME_TYPE_DATA_ACK:
            if (len >= sizeof(DataAckFrame_t)) {
                DataAckFrame_t *ack = (DataAckFrame_t *)data;
                DataTransfer_HandleAck(ack);
            }
            break;

        case FRAME_TYPE_DATA_END:
            if (len >= sizeof(DataEndFrame_t)) {
                DataEndFrame_t *end = (DataEndFrame_t *)data;
                if (end->dataId == g_dataTransferCtx.dataId) {
                    g_dataTransferCtx.state = TRANSFER_RX_COMPLETE;
                    g_dataTransferCtx.rxComplete = 1;
                    debug_printf("[UWB] DataEnd: id=%u received\r\n", end->dataId);
                }
            }
            break;

        case FRAME_TYPE_MODE_SWITCH:
            if (len >= sizeof(ModeSwitchFrame_t)) {
                ModeSwitchFrame_t *sw = (ModeSwitchFrame_t *)data;
                UWB_SetMode((UWB_WorkMode_t)sw->targetMode);
            }
            break;

        default:
            return -1;
    }

    return 0;
}

/**
 * @brief  处理接收到的数据包
 */
int DataTransfer_HandleDataPacket(const DataPacketFrame_t *frame)
{
    /* CRC校验 */
    uint16_t calcCrc = DataTransfer_CalcCRC16(frame->data, frame->dataLen);
    if (calcCrc != frame->crc16) {
        debug_printf("[UWB] DataPacket CRC error: idx=%u\r\n", frame->packetIndex);
        UWB_SendDataAck(frame->packetIndex, 1, g_dataTransferCtx.rxExpectedPacket);
        return -1;
    }

    /* 检查包序号 */
    if (frame->packetIndex == g_dataTransferCtx.rxExpectedPacket) {
        /* 期望的包，存储数据 */
        uint16_t offset = frame->packetIndex * DATA_PACKET_MAX_LEN;
        if (offset + frame->dataLen <= DATA_TRANSFER_MAX_SIZE) {
            memcpy(&g_dataTransferCtx.rxBuffer[offset], frame->data, frame->dataLen);
            g_dataTransferCtx.rxDataLen += frame->dataLen;
            g_dataTransferCtx.rxReceivedPackets++;
            g_dataTransferCtx.rxExpectedPacket++;
            g_dataTransferCtx.lastActivityTime = TIM2->CNT;

            /* 发送ACK */
            UWB_SendDataAck(frame->packetIndex, 0, g_dataTransferCtx.rxExpectedPacket);
        }
    } else {
        /* 非期望的包，可能是重传或乱序 */
        debug_printf("[UWB] DataPacket unexpected: idx=%u expected=%u\r\n",
                     frame->packetIndex, g_dataTransferCtx.rxExpectedPacket);
        UWB_SendDataAck(g_dataTransferCtx.rxExpectedPacket - 1, 1,
                        g_dataTransferCtx.rxExpectedPacket);
    }

    return 0;
}

/**
 * @brief  处理接收到的ACK
 */
int DataTransfer_HandleAck(const DataAckFrame_t *frame)
{
    if (g_dataTransferCtx.state != TRANSFER_TX_SENDING) return -1;

    if (frame->status == 0) {
        /* ACK成功 */
        g_dataTransferCtx.txCurrentPacket = frame->nextExpectedIndex;
        g_dataTransferCtx.txAckReceived = 1;
        g_dataTransferCtx.txRetryCount = 0;
        g_dataTransferCtx.lastActivityTime = TIM2->CNT;
    } else {
        /* 需要重传 */
        g_dataTransferCtx.txCurrentPacket = frame->nextExpectedIndex;
        g_dataTransferCtx.txAckReceived = 1;
        g_dataTransferCtx.txRetryCount++;
        g_dataTransferCtx.lastActivityTime = TIM2->CNT;

        if (g_dataTransferCtx.txRetryCount > 3) {
            g_dataTransferCtx.state = TRANSFER_ERROR;
        }
    }

    return 0;
}

/**
 * @brief  检查接收是否完成
 */
uint8_t DataTransfer_IsReceiveComplete(void)
{
    return g_dataTransferCtx.rxComplete;
}

/**
 * @brief  获取接收到的数据
 */
int DataTransfer_GetReceivedData(uint8_t *buf, uint16_t *len)
{
    if (!g_dataTransferCtx.rxComplete) return -1;

    memcpy(buf, g_dataTransferCtx.rxBuffer, g_dataTransferCtx.rxDataLen);
    *len = g_dataTransferCtx.rxDataLen;

    return 0;
}

/**
 * @brief  获取传输状态
 */
TransferState_t DataTransfer_GetState(void)
{
    return g_dataTransferCtx.state;
}

/**
 * @brief  超时检查
 */
uint8_t DataTransfer_CheckTimeout(void)
{
    uint32_t currentTime = TIM2->CNT;
    uint32_t elapsed = currentTime - g_dataTransferCtx.lastActivityTime;

    if (elapsed > DATA_TRANSFER_TIMEOUT_MS * 100) {  // 转换为tick
        return 1;
    }
    return 0;
}
