/**
  ******************************************************************************
  * @file    bsp_dw1000_diag.c
  * @brief   DW1000 Diagnostic Functions - Read and print diagnostic info via UART
  ******************************************************************************
  */

#include "bsp.h"
#include "bsp_dw1000.h"
#include "bsp_dw1000_diag.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ==================== Helper Functions ==================== */

/**
 * @brief  Read 5-byte timestamp from DW1000
 */
static uint64_t Diag_ReadTimestamp(uint16_t regAddr, uint8_t offset)
{
    uint8_t buf[5];
    DW1000_SPI_ReadSub(regAddr, offset, buf, 5);
    
    return (uint64_t)buf[0] |
           ((uint64_t)buf[1] << 8) |
           ((uint64_t)buf[2] << 16) |
           ((uint64_t)buf[3] << 24) |
           ((uint64_t)buf[4] << 32);
}

/**
 * @brief  Read 16-bit value from sub-register
 */
static uint16_t Diag_ReadU16(uint16_t regAddr, uint8_t offset)
{
    uint8_t buf[2];
    DW1000_SPI_ReadSub(regAddr, offset, buf, 2);
    return (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
}

/* ==================== Main Diagnostic Functions ==================== */

/**
 * @brief  Read all DW1000 diagnostic information
 */
int DW1000_Diag_ReadAll(DW1000_Diag_t *diag)
{
    uint8_t buf[8];
    
    if (diag == NULL) return -1;
    
    memset(diag, 0, sizeof(DW1000_Diag_t));
    
    /* 1. Read Device ID */
    DW1000_SPI_Read(DW1000_REG_DEV_ID, buf, 4);
    diag->deviceId = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
                     ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    
    /* 2. Read EUI-64 */
    DW1000_SPI_Read(DW1000_REG_EUI_64, diag->eui64, 8);
    
    /* 3. Read PAN ID and Short Address */
    DW1000_SPI_Read(DW1000_REG_PANADR, buf, 4);
    diag->shortAddr = (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
    diag->panId = (uint16_t)(buf[2] | ((uint16_t)buf[3] << 8));
    
    /* 4. Read System Status */
    DW1000_SPI_Read(DW1000_REG_SYS_STATUS, buf, 4);
    diag->sysStatus = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) |
                      ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    
    /* 5. Read RX Timestamp */
    diag->rxTimestamp = Diag_ReadTimestamp(DW1000_REG_RX_TIME, RX_TIME_RX_STAMP_OFFSET);
    
    /* 6. Read First Path Index */
    diag->fpIndex = Diag_ReadU16(DW1000_REG_RX_TIME, RX_TIME_FP_INDEX_OFFSET);
    
    /* 7. Read First Path Amplitude 1 */
    diag->fpAmpl1 = Diag_ReadU16(DW1000_REG_RX_TIME, RX_TIME_FP_AMPL1_OFFSET);
    
    /* 8. Read RX Raw Timestamp */
    diag->rxRawTimestamp = Diag_ReadTimestamp(DW1000_REG_RX_TIME, RX_TIME_FP_RAWST_OFFSET);
    
    /* 9. Read RX_EQUAL register (0x2E) */
    DW1000_SPI_ReadSub(0x2E, 0, buf, 8);
    diag->stdNoise = (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
    diag->fpAmpl2 = (uint16_t)(buf[2] | ((uint16_t)buf[3] << 8));
    diag->fpAmpl3 = (uint16_t)(buf[4] | ((uint16_t)buf[5] << 8));
    diag->cirMaxGrowth = (uint16_t)(buf[6] | ((uint16_t)buf[7] << 8));
    
    /* 10. Read TX Timestamp */
    diag->txTimestamp = Diag_ReadTimestamp(DW1000_REG_TX_TIME, 0);
    
    /* 11. Read AGC Status */
    DW1000_SPI_ReadSub(DW1000_REG_AGCCONFIG, AGC_STAT1_OFFSET, buf, AGC_STAT1_LEN);
    diag->agcGain1 = (buf[0] >> 2) & 0x1F;
    diag->agcGain2 = (uint16_t)(((buf[0] & 0x03) << 7) | (buf[1] >> 1));
    
    /* 12. Read Event Counters */
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_PHE_OFFSET, buf, 2);
    diag->evcPHR_Error = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_RSE_OFFSET, buf, 2);
    diag->evcRSE_Error = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_FCG_OFFSET, buf, 2);
    diag->evcGoodCRC = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_FCE_OFFSET, buf, 2);
    diag->evcBadCRC = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_FFR_OFFSET, buf, 2);
    diag->evcFrameFiltered = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_OVR_OFFSET, buf, 2);
    diag->evcOverrun = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_STO_OFFSET, buf, 2);
    diag->evcSFD_Timeout = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_PTO_OFFSET, buf, 2);
    diag->evcPreamble_Timeout = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_FWTO_OFFSET, buf, 2);
    diag->evcFrameWait_Timeout = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_TXFS_OFFSET, buf, 2);
    diag->evcTX_FramesSent = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    /* 13. Calculate power values */
    DW1000_Diag_CalcPower(diag);
    
    return 0;
}

/**
 * @brief  Calculate RX power and FP power from raw values
 */
void DW1000_Diag_CalcPower(DW1000_Diag_t *diag)
{
    if (diag == NULL) return;
    
    /* Calculate First Path Power */
    if (diag->fpAmpl1 > 0 && diag->fpAmpl2 > 0 && diag->stdNoise > 0) {
        float A = (float)diag->fpAmpl1 * (float)diag->fpAmpl1;
        float B = (float)diag->fpAmpl2 * (float)diag->fpAmpl2;
        diag->fpPower = 10.0f * log10f(A + B) - 121.74f;
    }
    
    /* Calculate RX Power */
    if (diag->cirMaxGrowth > 0 && diag->stdNoise > 0) {
        diag->rxPower = 10.0f * log10f((float)diag->cirMaxGrowth * (float)diag->cirMaxGrowth / 
                        (float)diag->stdNoise) - 121.74f;
    }
    
    /* Calculate Noise Level */
    if (diag->stdNoise > 0) {
        diag->noiseLevel = 10.0f * log10f((float)diag->stdNoise) - 121.74f;
    }
    
    /* Calculate First Path SNR */
    if (diag->fpPower > -200.0f && diag->noiseLevel > -200.0f) {
        diag->fpSnr = diag->fpPower - diag->noiseLevel;
    }
}

/**
 * @brief  Print diagnostic information to UART using debug_printf
 */
void DW1000_Diag_Print(const DW1000_Diag_t *diag)
{
    if (diag == NULL) return;
    
    debug_printf("\r\n");
    debug_printf("=== DW1000 Diagnostic Info ===\r\n");
    debug_printf("\r\n");
    
    /* Device Info */
    debug_printf("[Device Info]\r\n");
    debug_printf("  Device ID:    0x%08X\r\n", diag->deviceId);
    debug_printf("  EUI-64:       %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
                diag->eui64[0], diag->eui64[1], diag->eui64[2], diag->eui64[3],
                diag->eui64[4], diag->eui64[5], diag->eui64[6], diag->eui64[7]);
    debug_printf("  PAN ID:       0x%04X\r\n", diag->panId);
    debug_printf("  Short Addr:   0x%04X\r\n", diag->shortAddr);
    debug_printf("\r\n");
    
    /* System Status */
    debug_printf("[System Status]\r\n");
    debug_printf("  SYS_STATUS:   0x%08X\r\n", diag->sysStatus);
    debug_printf("    TXFRS:      %s\r\n", (diag->sysStatus & 0x00008000) ? "YES" : "NO");
    debug_printf("    RXPRD:      %s\r\n", (diag->sysStatus & 0x00004000) ? "YES" : "NO");
    debug_printf("    RXSFDTO:    %s\r\n", (diag->sysStatus & 0x00000400) ? "YES" : "NO");
    debug_printf("    LDEDONE:    %s\r\n", (diag->sysStatus & 0x00000040) ? "YES" : "NO");
    debug_printf("\r\n");
    
    /* RX Timestamp */
    debug_printf("[RX Timestamp]\r\n");
    debug_printf("  RX Timestamp: %016llX\r\n", diag->rxTimestamp);
    debug_printf("  RX Raw Time:  %016llX\r\n", diag->rxRawTimestamp);
    debug_printf("  FP Index:     %u\r\n", diag->fpIndex);
    debug_printf("  FP Ampl1:     %u\r\n", diag->fpAmpl1);
    debug_printf("  FP Ampl2:     %u\r\n", diag->fpAmpl2);
    debug_printf("  Std Noise:    %u\r\n", diag->stdNoise);
    debug_printf("  CIR Max:      %u\r\n", diag->cirMaxGrowth);
    debug_printf("\r\n");
    
    /* TX Timestamp */
    debug_printf("[TX Timestamp]\r\n");
    debug_printf("  TX Timestamp: %016llX\r\n", diag->txTimestamp);
    debug_printf("\r\n");
    
    /* Power Measurements */
    debug_printf("[Power Measurements]\r\n");
    debug_printf("  RX Power:     %.2f dBm\r\n", diag->rxPower);
    debug_printf("  FP Power:     %.2f dBm\r\n", diag->fpPower);
    debug_printf("  Noise Level:  %.2f dBm\r\n", diag->noiseLevel);
    debug_printf("  FP SNR:       %.2f dB\r\n", diag->fpSnr);
    debug_printf("\r\n");
    
    /* AGC Status */
    debug_printf("[AGC Status]\r\n");
    debug_printf("  AGC Gain1:    %u\r\n", diag->agcGain1);
    debug_printf("  AGC Gain2:    %u\r\n", diag->agcGain2);
    debug_printf("\r\n");
    
    /* Event Counters */
    debug_printf("[Event Counters]\r\n");
    debug_printf("  Good CRC:     %u\r\n", diag->evcGoodCRC);
    debug_printf("  Bad CRC:      %u\r\n", diag->evcBadCRC);
    debug_printf("  TX Sent:      %u\r\n", diag->evcTX_FramesSent);
    debug_printf("  PHR Error:    %u\r\n", diag->evcPHR_Error);
    debug_printf("  RS Error:     %u\r\n", diag->evcRSE_Error);
    debug_printf("  Filtered:     %u\r\n", diag->evcFrameFiltered);
    debug_printf("  Overrun:      %u\r\n", diag->evcOverrun);
    debug_printf("  SFD Timeout:  %u\r\n", diag->evcSFD_Timeout);
    debug_printf("  Preamble TO:  %u\r\n", diag->evcPreamble_Timeout);
    debug_printf("  Frame Wait TO:%u\r\n", diag->evcFrameWait_Timeout);
    debug_printf("\r\n");
    
    debug_printf("==============================\r\n");
}

/**
 * @brief  Read and print all diagnostics (convenience function)
 */
void DW1000_Diag_DumpAll(void)
{
    DW1000_Diag_t diag;
    
    if (DW1000_Diag_ReadAll(&diag) == 0) {
        DW1000_Diag_Print(&diag);
    } else {
        debug_printf("DW1000 Diag: Read failed!\r\n");
    }
}

/**
 * @brief  Print event counters only
 */
void DW1000_Diag_PrintEventCounters(void)
{
    uint8_t buf[2];
    uint16_t evcGoodCRC, evcBadCRC, evcTX_Sent;
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_FCG_OFFSET, buf, 2);
    evcGoodCRC = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_FCE_OFFSET, buf, 2);
    evcBadCRC = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    DW1000_SPI_ReadSub(DW1000_REG_DIG_DIAG, EVC_TXFS_OFFSET, buf, 2);
    evcTX_Sent = (uint16_t)((buf[0] | ((uint16_t)buf[1] << 8)) & 0x0FFF);
    
    debug_printf("[EVC] Good:%u Bad:%u TX:%u\r\n", evcGoodCRC, evcBadCRC, evcTX_Sent);
}

/**
 * @brief  Print RX timestamp and first path info
 */
void DW1000_Diag_PrintRxTimestamp(void)
{
    uint8_t buf[14];
    uint64_t rxTime;
    uint16_t fpIndex, fpAmpl1, stdNoise, fpAmpl2;
    
    DW1000_SPI_Read(DW1000_REG_RX_TIME, buf, 14);
    
    rxTime = (uint64_t)buf[0] | ((uint64_t)buf[1] << 8) |
             ((uint64_t)buf[2] << 16) | ((uint64_t)buf[3] << 24) |
             ((uint64_t)buf[4] << 32);
    
    fpIndex = (uint16_t)(buf[5] | ((uint16_t)buf[6] << 8));
    fpAmpl1 = (uint16_t)(buf[7] | ((uint16_t)buf[8] << 8));
    
    DW1000_SPI_ReadSub(0x2E, 0, buf, 4);
    stdNoise = (uint16_t)(buf[0] | ((uint16_t)buf[1] << 8));
    fpAmpl2 = (uint16_t)(buf[2] | ((uint16_t)buf[3] << 8));
    
    float fpPower = -200.0f;
    if (fpAmpl1 > 0 && fpAmpl2 > 0 && stdNoise > 0) {
        float A = (float)fpAmpl1 * (float)fpAmpl1;
        float B = (float)fpAmpl2 * (float)fpAmpl2;
        fpPower = 10.0f * log10f(A + B) - 121.74f;
    }
    
    debug_printf("[RX] T:%010llX FP:%u A1:%u A2:%u N:%u P:%.1fdBm\r\n",
                rxTime, fpIndex, fpAmpl1, fpAmpl2, stdNoise, fpPower);
}

/**
 * @brief  Clear event counters
 */
void DW1000_Diag_ClearEventCounters(void)
{
    uint8_t buf[4];
    
    buf[0] = 0x02;  /* EVC_CLR */
    DW1000_SPI_WriteSub(DW1000_REG_DIG_DIAG, EVC_CTRL_OFFSET, buf, 1);
    
    buf[0] = 0x01;  /* EVC_EN */
    DW1000_SPI_WriteSub(DW1000_REG_DIG_DIAG, EVC_CTRL_OFFSET, buf, 1);
}
