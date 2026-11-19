#ifndef __UWB_ANCHOR_H
#define __UWB_ANCHOR_H

#include "stm32l4xx.h"
#include <stdint.h>

/* ==================== Anchor Parameters ==================== */
#define ANCHOR_MAX_TAGS         5       // Max tags to track (减小以节省RAM)
#define ANCHOR_BEACON_INTERVAL  1000    // Beacon broadcast interval (ms)

/* ==================== Anchor State ==================== */
typedef enum {
    ANCHOR_STATE_IDLE = 0,
    ANCHOR_STATE_TX_BEACON,
    ANCHOR_STATE_WAIT_POLL,
    ANCHOR_STATE_TX_RESPONSE,
    ANCHOR_STATE_WAIT_FINAL,
    ANCHOR_STATE_CALC_DISTANCE,
    ANCHOR_STATE_ERROR
} Anchor_State_t;

/* ==================== Tag Info Structure ==================== */
typedef struct {
    uint16_t    tagId;              // Tag ID
    uint64_t    pollRxTime;        // POLL receive timestamp
    uint64_t    respTxTime;        // RESPONSE transmit timestamp
    uint64_t    finalRxTime;       // FINAL receive timestamp
    double      distance;           // Calculated distance (m)
    uint32_t    lastUpdateTime;    // Last update tick
    uint8_t     valid;              // Data valid flag
} Anchor_TagInfo_t;

/* ==================== Anchor Context ==================== */
typedef struct {
    uint16_t    anchorId;           // Anchor ID
    uint16_t    panId;              // PAN ID
    uint16_t    totalSlots;         // Total slots

    Anchor_State_t state;           // Current state
    uint32_t    beaconSeqNum;       // Beacon sequence number
    uint32_t    lastBeaconTime;     // Last beacon send time

    uint16_t    activeTagCount;     // Active tag count
    Anchor_TagInfo_t tags[ANCHOR_MAX_TAGS]; // Tag info array

    uint32_t    beaconCount;        // Beacons sent
    uint32_t    rangingCount;       // Ranging completions
} Anchor_Context_t;

/* ==================== Global Variables ==================== */
extern Anchor_Context_t g_anchorCtx;

/* ==================== Function Declarations ==================== */

/**
 * @brief  Anchor module initialization
 * @param  anchorId: This anchor ID
 * @param  panId: PAN ID
 * @param  totalSlots: Total slots in TDMA cycle
 * @retval 0=success, -1=fail
 */
int Anchor_Init(uint16_t anchorId, uint16_t panId, uint16_t totalSlots);

/**
 * @brief  Anchor main process function (call in main loop)
 */
void Anchor_Process(void);

/**
 * @brief  Send Beacon frame
 * @retval 0=success, -1=fail
 */
int Anchor_SendBeacon(void);

/**
 * @brief  Process received POLL frame
 * @param  data: Received data
 * @param  len: Data length
 * @retval 0=success, -1=fail
 */
int Anchor_ProcessPoll(const uint8_t *data, uint16_t len);

/**
 * @brief  Send RESPONSE frame to tag
 * @param  tagId: Target tag ID
 * @param  pollRxTime: POLL receive timestamp
 * @retval 0=success, -1=fail
 */
int Anchor_SendResponse(uint16_t tagId, uint64_t pollRxTime);

/**
 * @brief  Process received FINAL frame
 * @param  data: Received data
 * @param  len: Data length
 * @retval 0=success, -1=fail
 */
int Anchor_ProcessFinal(const uint8_t *data, uint16_t len);

/**
 * @brief  Find tag info by ID
 * @param  tagId: Tag ID to find
 * @retval Pointer to tag info, or NULL if not found
 */
Anchor_TagInfo_t* Anchor_FindTag(uint16_t tagId);

/**
 * @brief  Get tag distance
 * @param  tagId: Tag ID
 * @retval Distance in meters, -1.0 if invalid
 */
double Anchor_GetTagDistance(uint16_t tagId);

/**
 * @brief  Get active tag count
 * @retval Number of active tags
 */
uint16_t Anchor_GetActiveTagCount(void);

/**
 * @brief  Get anchor state
 * @retval Current anchor state
 */
Anchor_State_t Anchor_GetState(void);

/**
 * @brief  Enable DW1000 receive mode
 */
void Anchor_EnableRx(void);

/**
 * @brief  Read received data from DW1000
 * @param  buf: Receive buffer
 * @param  len: Data length pointer
 * @retval 0=success, -1=fail
 */
int Anchor_ReadRxData(uint8_t *buf, uint16_t *len);

/**
 * @brief  Get DW1000 transmit timestamp
 * @retval 64-bit timestamp (ticks)
 */
uint64_t Anchor_GetTxTimestamp(void);

/**
 * @brief  Get DW1000 receive timestamp
 * @retval 64-bit timestamp (ticks)
 */
uint64_t Anchor_GetRxTimestamp(void);

#endif /* __UWB_ANCHOR_H */
