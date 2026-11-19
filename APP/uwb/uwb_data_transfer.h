#ifndef __UWB_DATA_TRANSFER_H
#define __UWB_DATA_TRANSFER_H

#include "stm32l4xx.h"
#include <stdint.h>

/* ==================== Configuration ==================== */
#define DATA_TRANSFER_MAX_SIZE      3072    // 最大数据传输大小
#define DATA_PACKET_MAX_LEN         120     // 单包最大数据长度
#define DATA_TRANSFER_TIMEOUT_MS    5000    // 传输超时时间(ms)
#define DATA_ACK_TIMEOUT_MS         50      // ACK等待超时(ms)

/* ==================== Transfer Status ==================== */
typedef enum {
    TRANSFER_IDLE = 0,          // 空闲
    TRANSFER_TX_SENDING,        // 发送中
    TRANSFER_TX_COMPLETE,       // 发送完成
    TRANSFER_RX_RECEIVING,      // 接收中
    TRANSFER_RX_COMPLETE,       // 接收完成
    TRANSFER_ERROR,             // 错误
    TRANSFER_TIMEOUT            // 超时
} TransferState_t;

/* ==================== Transfer Context ==================== */
typedef struct {
    /* Common */
    TransferState_t state;              // 传输状态
    uint16_t        dataId;             // 数据标识
    uint32_t        lastActivityTime;   // 最后活动时间

    /* TX (Sender) */
    const uint8_t   *txData;            // 发送数据指针
    uint16_t        txDataLen;          // 发送数据总长度
    uint8_t         txTotalPackets;     // 发送总包数
    uint8_t         txCurrentPacket;    // 当前发送包序号
    uint8_t         txAckReceived;      // 收到ACK标志
    uint8_t         txRetryCount;       // 重试计数

    /* RX (Receiver) */
    uint8_t         rxBuffer[DATA_TRANSFER_MAX_SIZE];   // 接收缓冲区
    uint16_t        rxDataLen;          // 已接收数据长度
    uint8_t         rxTotalPackets;     // 接收总包数
    uint8_t         rxReceivedPackets;  // 已接收包数
    uint8_t         rxExpectedPacket;   // 期望的下一个包序号
    uint8_t         rxComplete;         // 接收完成标志
} DataTransferCtx_t;

/* ==================== Global Variables ==================== */
extern DataTransferCtx_t g_dataTransferCtx;

/* ==================== Function Declarations ==================== */

/**
 * @brief  数据传输模块初始化
 */
void DataTransfer_Init(void);

/**
 * @brief  CRC16校验计算
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @retval CRC16值
 */
uint16_t DataTransfer_CalcCRC16(const uint8_t *data, uint16_t len);

/**
 * @brief  开始发送数据
 * @param  dataId: 数据标识
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @retval 0=成功, -1=失败
 */
int DataTransfer_StartSend(uint16_t dataId, const uint8_t *data, uint16_t len);

/**
 * @brief  发送处理函数(在主循环中调用)
 * @retval TransferState_t当前状态
 */
TransferState_t DataTransfer_ProcessSend(void);

/**
 * @brief  处理接收到的数据帧
 * @param  data: 接收到的数据
 * @param  len: 数据长度
 * @retval 0=成功, -1=失败
 */
int DataTransfer_ProcessReceive(const uint8_t *data, uint16_t len);

/**
 * @brief  处理接收到的数据包
 * @param  frame: 数据包帧指针
 * @retval 0=成功, -1=失败
 */
int DataTransfer_HandleDataPacket(const DataPacketFrame_t *frame);

/**
 * @brief  处理接收到的ACK
 * @param  frame: ACK帧指针
 * @retval 0=成功, -1=失败
 */
int DataTransfer_HandleAck(const DataAckFrame_t *frame);

/**
 * @brief  检查接收是否完成
 * @retval 1=完成, 0=未完成
 */
uint8_t DataTransfer_IsReceiveComplete(void);

/**
 * @brief  获取接收到的数据
 * @param  buf: 输出缓冲区
 * @param  len: 输出数据长度
 * @retval 0=成功, -1=失败
 */
int DataTransfer_GetReceivedData(uint8_t *buf, uint16_t *len);

/**
 * @brief  获取传输状态
 * @retval TransferState_t状态
 */
TransferState_t DataTransfer_GetState(void);

/**
 * @brief  重置传输上下文
 */
void DataTransfer_Reset(void);

/**
 * @brief  超时检查
 * @retval 1=超时, 0=未超时
 */
uint8_t DataTransfer_CheckTimeout(void);

#endif /* __UWB_DATA_TRANSFER_H */
