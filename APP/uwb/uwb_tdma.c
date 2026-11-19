/**
  ******************************************************************************
  * @file    uwb_tdma.c
  * @brief   UWB TDMA mechanism implementation (Tag side)
  ******************************************************************************
  */

#include "bsp.h"
#include "uwb_tdma.h"
#include "bsp_dw1000.h"
#include "bsp_timer.h"

/* ==================== Global Variables ==================== */
TDMA_Context_t g_tdmaCtx;
UWB_ModeCtrl_t g_modeCtrl;

/* ==================== Internal Functions ==================== */
static void TDMA_ResetContext(void);
static uint8_t TDMA_ValidateBeacon(const BeaconFrame_t *beacon);
static void TDMA_UpdateSlotTiming(uint64_t slotStartTime);

/**
 * @brief  从DW1000读取接收数据
 * @param  buf: 接收缓冲区
 * @param  len: 数据长度指针
 * @retval 0=成功, -1=失败
 */
static int TDMA_ReadRxData(uint8_t *buf, uint16_t *len)
{
    uint8_t rxfl;

    /* 检查是否有接收到的数据 */
    if (!(DW1000_ReadSysStatus() & DW1000_SYS_STATUS_RXPRD)) return -1;

    /* 读取帧长度 */
    DW1000_SPI_Read(DW1000_REG_RX_FWINFO, &rxfl, 1);
    *len = rxfl;

    /* 长度检查 */
    if (*len > 64 || *len == 0) return -1;

    /* 读取数据 */
    DW1000_SPI_Read(DW1000_REG_RX_BUFFER, buf, *len);

    /* 清除接收状态 */
    DW1000_ClearSysStatus(DW1000_SYS_STATUS_RXPRD);

    return 0;
}

/* ==================== Implementation ==================== */

int TDMA_Init(uint16_t tagId, uint16_t slotIndex, uint16_t totalSlots)
{
    if (slotIndex >= totalSlots || totalSlots > TDMA_MAX_TAGS) {
        return -1;
    }

    TDMA_ResetContext();

    g_tdmaCtx.tagId = tagId;
    g_tdmaCtx.slotIndex = slotIndex;
    g_tdmaCtx.totalSlots = totalSlots;
    g_tdmaCtx.panId = 0x1234;

    DW1000_SetSysMask(DW1000_SYS_STATUS_RXPRD);

    return 0;
}

static void TDMA_ResetContext(void)
{
    memset((uint8_t *)&g_tdmaCtx, 0, sizeof(TDMA_Context_t));
    g_tdmaCtx.state = TDMA_STATE_IDLE;
    g_tdmaCtx.isSynced = 0;
}

void TDMA_Process(void)
{
    uint8_t rxData[64];
    uint16_t rxLen;
    uint64_t currentTime;

    if (g_dw1000_irq_flag) {
        g_dw1000_irq_flag = 0;

        /* 从DW1000读取接收到的数据 */
        if (TDMA_ReadRxData(rxData, &rxLen) == 0) {

            if (rxLen >= sizeof(BeaconFrame_t)) {
                uint8_t frameType = rxData[0];
                switch (frameType) {
                    case FRAME_TYPE_BEACON:
                        TDMA_ProcessBeacon(rxData, rxLen);
                        break;
                    case FRAME_TYPE_RESPONSE:
                        /* Handled by ranging module */
                        break;
                    default:
                        break;
                }
            }
        }
    }

    currentTime = TDMA_GetUWBTime();

    switch (g_tdmaCtx.state) {
        case TDMA_STATE_IDLE:
            if (g_tdmaCtx.isSynced) {
                g_tdmaCtx.state = TDMA_STATE_WAIT_SLOT;
            }
            break;

        case TDMA_STATE_SYNCING:
            break;

        case TDMA_STATE_WAIT_SLOT:
            if (TDMA_IsMySlot()) {
                g_tdmaCtx.state = TDMA_STATE_TX_POLL;
                g_tdmaCtx.retryCount = 0;
            }
            if (currentTime > g_tdmaCtx.slotEndTime) {
                g_tdmaCtx.state = TDMA_STATE_WAIT_SLOT;
                g_tdmaCtx.rangeFailCount++;
            }
            break;

        case TDMA_STATE_TX_POLL:
            g_tdmaCtx.state = TDMA_STATE_WAIT_RESPONSE;
            break;

        case TDMA_STATE_WAIT_RESPONSE:
            if (currentTime > g_tdmaCtx.slotEndTime) {
                if (g_tdmaCtx.retryCount < 3) {
                    g_tdmaCtx.retryCount++;
                    g_tdmaCtx.state = TDMA_STATE_TX_POLL;
                } else {
                    g_tdmaCtx.state = TDMA_STATE_WAIT_SLOT;
                    g_tdmaCtx.rangeFailCount++;
                }
            }
            break;

        case TDMA_STATE_TX_FINAL:
            g_tdmaCtx.state = TDMA_STATE_COMPLETE;
            break;

        case TDMA_STATE_COMPLETE:
            g_tdmaCtx.rangeSuccessCount++;
            g_tdmaCtx.state = TDMA_STATE_WAIT_SLOT;
            break;

        case TDMA_STATE_ERROR:
            TDMA_ResetContext();
            break;

        default:
            g_tdmaCtx.state = TDMA_STATE_IDLE;
            break;
    }
}

static uint8_t TDMA_ValidateBeacon(const BeaconFrame_t *beacon)
{
    if (beacon->frameType != FRAME_TYPE_BEACON) return 0;
    if (beacon->panId != g_tdmaCtx.panId) return 0;
    if (beacon->totalSlots != g_tdmaCtx.totalSlots) return 0;
    return 1;
}

int TDMA_ProcessBeacon(const uint8_t *data, uint16_t len)
{
    BeaconFrame_t *beacon;
    uint64_t rxTimestamp;

    if (len < sizeof(BeaconFrame_t)) return -1;

    beacon = (BeaconFrame_t *)data;
    if (!TDMA_ValidateBeacon(beacon)) return -1;

    rxTimestamp = TDMA_GetUWBTime();
    TDMA_SyncClock(beacon->txTimestamp);
    TDMA_UpdateSlotTiming(beacon->slotStartTime);

    g_tdmaCtx.isSynced = 1;
    g_tdmaCtx.lastBeaconSeq = beacon->seqNum;
    g_tdmaCtx.currentCycle = beacon->cycleNum;
    g_tdmaCtx.beaconCount++;

    if (g_tdmaCtx.state == TDMA_STATE_IDLE) {
        g_tdmaCtx.state = TDMA_STATE_WAIT_SLOT;
    }

    return 0;
}

uint64_t TDMA_GetUWBTime(void)
{
    uint8_t buf[5];
    uint64_t timestamp = 0;

    DW1000_SPI_Read(0x06, buf, 5);

    timestamp = (uint64_t)buf[0] |
                ((uint64_t)buf[1] << 8) |
                ((uint64_t)buf[2] << 16) |
                ((uint64_t)buf[3] << 24) |
                ((uint64_t)buf[4] << 32);

    return timestamp;
}

void TDMA_SyncClock(uint64_t beaconTimestamp)
{
    uint64_t localTime;
    int64_t offset;

    localTime = TDMA_GetUWBTime();
    offset = (int64_t)beaconTimestamp - (int64_t)localTime;

    if (g_tdmaCtx.isSynced) {
        g_tdmaCtx.localClockOffset = (g_tdmaCtx.localClockOffset * 7 + offset) / 8;
    } else {
        g_tdmaCtx.localClockOffset = offset;
    }
}

static void TDMA_UpdateSlotTiming(uint64_t slotStartTime)
{
    uint64_t slotDurationTicks;
    uint64_t mySlotStart;

    slotDurationTicks = UWB_MS_TO_TICKS(TDMA_SLOT_DURATION_MS);
    mySlotStart = slotStartTime + (uint64_t)g_tdmaCtx.slotIndex * slotDurationTicks;

    g_tdmaCtx.cycleStartTime = slotStartTime;
    g_tdmaCtx.slotStartTime = mySlotStart;
    g_tdmaCtx.slotEndTime = mySlotStart + slotDurationTicks;
}

uint8_t TDMA_IsMySlot(void)
{
    uint64_t currentTime;
    uint64_t adjustedTime;

    if (!g_tdmaCtx.isSynced) return 0;

    currentTime = TDMA_GetUWBTime();
    adjustedTime = currentTime + g_tdmaCtx.localClockOffset;

    if (adjustedTime >= g_tdmaCtx.slotStartTime && adjustedTime < g_tdmaCtx.slotEndTime) {
        return 1;
    }

    return 0;
}

TDMA_State_t TDMA_GetState(void)
{
    return g_tdmaCtx.state;
}

void TDMA_SetState(TDMA_State_t state)
{
    g_tdmaCtx.state = state;
}

uint8_t TDMA_IsSynced(void)
{
    return g_tdmaCtx.isSynced;
}

double TDMA_GetDistance(void)
{
    return g_tdmaCtx.distance;
}

void TDMA_GetStatistics(uint32_t *beaconCount, uint32_t *successCount, uint32_t *failCount)
{
    if (beaconCount) *beaconCount = g_tdmaCtx.beaconCount;
    if (successCount) *successCount = g_tdmaCtx.rangeSuccessCount;
    if (failCount) *failCount = g_tdmaCtx.rangeFailCount;
}
