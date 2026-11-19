#ifndef __UWB_TDMA_H
#define __UWB_TDMA_H

#include "stm32l4xx.h"
#include <stdint.h>

/* ==================== Device Type Configuration ==================== */
/* 设备类型定义 */
#define UWB_DEVICE_ANCHOR    1    // Base Station (基站)
#define UWB_DEVICE_TAG       2    // Positioning Tag (定位卡)

/* 选择当前设备类型: 在Keil项目设置中定义 UWB_DEVICE_TYPE */
/* 例如: UWB_DEVICE_TYPE=UWB_DEVICE_TAG 或 UWB_DEVICE_TYPE=UWB_DEVICE_ANCHOR */
#ifndef UWB_DEVICE_TYPE
#define UWB_DEVICE_TYPE        UWB_DEVICE_TAG      // 默认: 定位卡
#endif

/* Device configuration */
#define UWB_PAN_ID             0x1234          // PAN ID
#define UWB_ANCHOR_ID          2               // This anchor ID (for anchor mode)
#define UWB_TAG_ID             2               // This tag ID (for tag mode)
#define UWB_TAG_SLOT           0               // Tag slot index (for tag mode)
#define UWB_TOTAL_SLOTS        50              // Total TDMA slots

/* ==================== TDMA Configuration ==================== */
#define TDMA_MAX_TAGS           50
#define TDMA_SLOT_DURATION_MS   12
#define TDMA_CYCLE_MS           600
#define TDMA_BEACON_INTERVAL_MS 1000

/* ==================== Frame Types ==================== */
#define FRAME_TYPE_BEACON       0x01
#define FRAME_TYPE_POLL         0x02
#define FRAME_TYPE_RESPONSE     0x03
#define FRAME_TYPE_FINAL        0x04

/* Data Transfer Frame Types */
#define FRAME_TYPE_DATA_START   0x20    // 数据传输开始
#define FRAME_TYPE_DATA_PACKET  0x21    // 数据包
#define FRAME_TYPE_DATA_ACK     0x22    // 确认帧
#define FRAME_TYPE_DATA_END     0x23    // 数据传输结束
#define FRAME_TYPE_MODE_SWITCH  0x24    // 模式切换命令

/* ==================== Work Mode Configuration ==================== */
typedef enum {
    UWB_MODE_RANGING = 0,       // 测距模式（默认）
    UWB_MODE_COMM = 1           // 通信模式（仅数据传输）
} UWB_WorkMode_t;

/* Mode Control Context */
typedef struct {
    UWB_WorkMode_t  currentMode;    // 当前模式
    UWB_WorkMode_t  requestMode;    // 请求切换的模式
    uint8_t         switchPending;  // 切换请求标志
    uint32_t        switchTime;     // 切换时间戳
    uint32_t        commModeTimeout;// 通信模式超时时间
} UWB_ModeCtrl_t;

/* ==================== Time Conversion Macros ==================== */
#define UWB_US_TO_TICKS(us)     ((uint64_t)(us) * 63898)
#define UWB_MS_TO_TICKS(ms)     ((uint64_t)(ms) * 63897952)
#define UWB_TICKS_TO_US(ticks)  ((uint64_t)(ticks) / 63898)

#define SPEED_OF_LIGHT          299792458.0
#define UWB_TICK_TO_S           (1.0/63897952.0)

/* ==================== TDMA State Machine ==================== */
typedef enum {
    TDMA_STATE_IDLE = 0,
    TDMA_STATE_SYNCING,
    TDMA_STATE_WAIT_SLOT,
    TDMA_STATE_TX_POLL,
    TDMA_STATE_WAIT_RESPONSE,
    TDMA_STATE_TX_FINAL,
    TDMA_STATE_COMPLETE,
    TDMA_STATE_ERROR
} TDMA_State_t;

/* ==================== Beacon Frame ==================== */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;
    uint8_t     seqNum;
    uint16_t    panId;
    uint16_t    coordinatorId;
    uint16_t    totalSlots;
    uint32_t    cycleNum;
    uint64_t    txTimestamp;
    uint64_t    slotStartTime;
    uint8_t     reserved[6];
} BeaconFrame_t;

/* ==================== Poll Frame ==================== */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;          // 帧类型
    uint8_t     seqNum;             // 序列号
    uint16_t    panId;              // PAN ID
    uint16_t    tagId;              // 标签ID
    uint16_t    slotIndex;          // 时隙索引
    uint64_t    txTimestamp;        // 发送时间戳
    /* 自定义数据 */
    uint32_t    mileage;            // 里程
    uint16_t    lineName;           // 线名
    uint8_t     reserved[2];        // 保留
} PollFrame_t;                      // 共30字节

/* ==================== Response Frame ==================== */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;
    uint8_t     seqNum;
    uint16_t    panId;
    uint16_t    tagId;
    uint16_t    anchorId;
    uint64_t    rxTimestamp;
    uint64_t    txTimestamp;
    uint8_t     reserved[4];
} ResponseFrame_t;

/* ==================== Final Frame ==================== */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;
    uint8_t     seqNum;
    uint16_t    panId;
    uint16_t    tagId;
    uint16_t    anchorId;
    uint64_t    pollTxTime;
    uint64_t    respRxTime;
    uint64_t    finalTxTime;
} FinalFrame_t;

/* ==================== TDMA Context ==================== */
typedef struct {
    uint16_t    tagId;
    uint16_t    slotIndex;
    uint16_t    panId;
    uint16_t    totalSlots;

    uint8_t     isSynced;
    uint32_t    lastBeaconSeq;
    uint64_t    localClockOffset;

    uint64_t    cycleStartTime;
    uint64_t    slotStartTime;
    uint64_t    slotEndTime;
    uint32_t    currentCycle;

    TDMA_State_t state;
    uint8_t     retryCount;

    uint64_t    pollTxTimestamp;
    uint64_t    responseRxTimestamp;
    uint64_t    finalTxTimestamp;
    double      distance;

    uint32_t    beaconCount;
    uint32_t    rangeSuccessCount;
    uint32_t    rangeFailCount;
} TDMA_Context_t;

/* ==================== Global Variables ==================== */
extern TDMA_Context_t g_tdmaCtx;
extern UWB_ModeCtrl_t g_modeCtrl;

/* ==================== Data Transfer Structures ==================== */

/* 数据传输开始帧 */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;          // FRAME_TYPE_DATA_START
    uint8_t     reserved;
    uint16_t    dataId;             // 数据标识
    uint8_t     totalPackets;       // 总包数
    uint16_t    totalDataLen;       // 总数据长度
    uint8_t     reserved2[7];
} DataStartFrame_t;                 // 共14字节

/* 数据包帧 */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;          // FRAME_TYPE_DATA_PACKET
    uint8_t     packetIndex;        // 当前包序号 (0~254)
    uint8_t     totalPackets;       // 总包数
    uint8_t     dataLen;            // 本包数据长度
    uint16_t    dataId;             // 数据标识
    uint16_t    crc16;              // 本包数据CRC校验
    uint8_t     data[120];          // 数据内容 (最大120字节)
} DataPacketFrame_t;                // 共128字节

/* 数据确认帧 */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;          // FRAME_TYPE_DATA_ACK
    uint8_t     ackPacketIndex;     // 确认的包序号
    uint8_t     status;             // 0=成功, 1=需要重传
    uint8_t     nextExpectedIndex;  // 下一个期望的包序号
} DataAckFrame_t;                   // 共4字节

/* 数据传输结束帧 */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;          // FRAME_TYPE_DATA_END
    uint8_t     reserved;
    uint16_t    dataId;             // 数据标识
    uint16_t    totalCrc16;         // 整体数据CRC
    uint8_t     reserved2[8];
} DataEndFrame_t;                   // 共14字节

/* 模式切换帧 */
typedef struct __attribute__((packed)) {
    uint8_t     frameType;          // FRAME_TYPE_MODE_SWITCH
    uint8_t     targetMode;         // 目标模式 (UWB_WorkMode_t)
    uint8_t     reserved[12];
} ModeSwitchFrame_t;                // 共14字节

/* ==================== Function Declarations ==================== */
int         TDMA_Init(uint16_t tagId, uint16_t slotIndex, uint16_t totalSlots);
void        TDMA_Process(void);
int         TDMA_ProcessBeacon(const uint8_t *data, uint16_t len);
uint64_t    TDMA_GetUWBTime(void);
void        TDMA_SyncClock(uint64_t beaconTimestamp);
uint8_t     TDMA_IsMySlot(void);
TDMA_State_t TDMA_GetState(void);
void        TDMA_SetState(TDMA_State_t state);
uint8_t     TDMA_IsSynced(void);
double      TDMA_GetDistance(void);
void        TDMA_GetStatistics(uint32_t *beaconCount, uint32_t *successCount, uint32_t *failCount);

/* Mode Control Functions */
void        UWB_SetMode(UWB_WorkMode_t mode);
UWB_WorkMode_t UWB_GetMode(void);
uint8_t     UWB_IsCommMode(void);
uint8_t     UWB_IsRangingMode(void);

/* Data Transfer Functions */
int         UWB_SendDataStart(uint16_t dataId, uint8_t totalPackets, uint16_t totalLen);
int         UWB_SendDataPacket(uint16_t dataId, uint8_t packetIndex, uint8_t totalPackets,
                               const uint8_t *data, uint8_t dataLen);
int         UWB_SendDataAck(uint8_t ackPacketIndex, uint8_t status, uint8_t nextExpected);
int         UWB_SendDataEnd(uint16_t dataId, uint16_t totalCrc);
int         UWB_SendModeSwitch(UWB_WorkMode_t targetMode);

#endif /* __UWB_TDMA_H */
