#ifndef __BSP_DW1000_DIAG_H
#define __BSP_DW1000_DIAG_H

#include "stm32l4xx.h"
#include <stdint.h>

/* ==================== DW1000 Register Addresses ==================== */
#define DW1000_REG_DEV_ID           0x00
#define DW1000_REG_EUI_64           0x01
#define DW1000_REG_PANADR           0x03
#define DW1000_REG_SYS_CFG          0x04
#define DW1000_REG_SYS_TIME         0x06
#define DW1000_REG_TX_FCTRL         0x08
#define DW1000_REG_SYS_STATUS       0x0F
#define DW1000_REG_RX_FINFO         0x10
#define DW1000_REG_RX_BUFFER        0x11
#define DW1000_REG_RX_TIME          0x15
#define DW1000_REG_TX_TIME          0x17
#define DW1000_REG_TX_ANTD          0x18
#define DW1000_REG_SYS_STATE        0x19
#define DW1000_REG_AGCCONFIG        0x23
#define DW1000_REG_AGC_STAT1        0x25
#define DW1000_REG_DIG_DIAG         0x2F
#define DW1000_REG_PMSC_CTRL0       0x36

/* RX_TIME sub-register offsets */
#define RX_TIME_RX_STAMP_OFFSET     0
#define RX_TIME_FP_INDEX_OFFSET     5
#define RX_TIME_FP_AMPL1_OFFSET     7
#define RX_TIME_FP_RAWST_OFFSET     9

/* AGC_STAT1 sub-register offsets */
#define AGC_STAT1_OFFSET            0x1E
#define AGC_STAT1_LEN               3

/* DIG_DIAG sub-register offsets */
#define EVC_CTRL_OFFSET             0x00
#define EVC_PHE_OFFSET              0x04
#define EVC_RSE_OFFSET              0x06
#define EVC_FCG_OFFSET              0x08
#define EVC_FCE_OFFSET              0x0A
#define EVC_FFR_OFFSET              0x0C
#define EVC_OVR_OFFSET              0x0E
#define EVC_STO_OFFSET              0x10
#define EVC_PTO_OFFSET              0x12
#define EVC_FWTO_OFFSET             0x14
#define EVC_TXFS_OFFSET             0x16

/* ==================== Diagnostic Data Structure ==================== */
typedef struct {
    /* Device Info */
    uint32_t    deviceId;
    uint8_t     eui64[8];
    uint16_t    panId;
    uint16_t    shortAddr;
    
    /* System Status */
    uint32_t    sysStatus;
    
    /* RX Timestamp & Diagnostics */
    uint64_t    rxTimestamp;
    uint16_t    fpIndex;
    uint16_t    fpAmpl1;
    uint64_t    rxRawTimestamp;
    uint16_t    stdNoise;
    uint16_t    fpAmpl2;
    uint16_t    fpAmpl3;
    uint16_t    cirMaxGrowth;
    
    /* TX Timestamp */
    uint64_t    txTimestamp;
    
    /* AGC Status */
    uint8_t     agcGain1;
    uint16_t    agcGain2;
    
    /* Event Counters */
    uint16_t    evcPHR_Error;
    uint16_t    evcRSE_Error;
    uint16_t    evcGoodCRC;
    uint16_t    evcBadCRC;
    uint16_t    evcFrameFiltered;
    uint16_t    evcOverrun;
    uint16_t    evcSFD_Timeout;
    uint16_t    evcPreamble_Timeout;
    uint16_t    evcFrameWait_Timeout;
    uint16_t    evcTX_FramesSent;
    
    /* Calculated values */
    float       rxPower;
    float       fpPower;
    float       noiseLevel;
    float       fpSnr;
} DW1000_Diag_t;

/* ==================== Function Declarations ==================== */
int         DW1000_Diag_ReadAll(DW1000_Diag_t *diag);
void        DW1000_Diag_Print(const DW1000_Diag_t *diag);
void        DW1000_Diag_DumpAll(void);
void        DW1000_Diag_CalcPower(DW1000_Diag_t *diag);
void        DW1000_Diag_PrintEventCounters(void);
void        DW1000_Diag_PrintRxTimestamp(void);
void        DW1000_Diag_ClearEventCounters(void);

#endif /* __BSP_DW1000_DIAG_H */
