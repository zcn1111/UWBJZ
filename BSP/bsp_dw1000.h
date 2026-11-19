/**
  ******************************************************************************
  * @file    bsp_dw1000.h
  * @brief   DW1000 UWB Transceiver Driver Header
  * @note    SPI2: PB10=SCK, PC2=MISO, PC3=MOSI
  *          CSN=PB2, nRST=PB11, IRQn=PB14
  ******************************************************************************
  */
#ifndef __BSP_DW1000_H
#define __BSP_DW1000_H

#include "stm32l4xx.h"
#include <stdint.h>
#include <string.h>


/*DW1000_1 UWB模组控制引脚*/
#define DW1000_CS_GPIO_PORT             GPIOD
#define DW1000_CS_PIN                   GPIO_PIN_7
#define DW1000_CS_LOW()                 HAL_GPIO_WritePin(DW1000_CS_GPIO_PORT, DW1000_CS_PIN, GPIO_PIN_RESET)
#define DW1000_CS_HIGH()                HAL_GPIO_WritePin(DW1000_CS_GPIO_PORT, DW1000_CS_PIN, GPIO_PIN_SET)

#define DW1000_RST_GPIO_PORT            GPIOE
#define DW1000_RST_PIN                  GPIO_PIN_5
#define DW1000_RST_LOW()                HAL_GPIO_WritePin(DW1000_RST_GPIO_PORT, DW1000_RST_PIN, GPIO_PIN_RESET)
#define DW1000_RST_HIGH()               HAL_GPIO_WritePin(DW1000_RST_GPIO_PORT, DW1000_RST_PIN, GPIO_PIN_SET)

#define DW1000_IRQ_GPIO_PORT            GPIOE
#define DW1000_IRQ_PIN                  GPIO_PIN_1
#define DW1000_IRQ_EXTI_IRQn            EXTI1_IRQn




/*DW1000_2*/
#define DW1000_PA_GPIO_PORT				GPIOA
#define DW1000_PA_GPIO_PIN				GPIO_PIN_9
#define DW1000_PA_LOW()					HAL_GPIO_WritePin(DW1000_PA_GPIO_PORT, DW1000_PA_GPIO_PIN, GPIO_PIN_RESET)
#define DW1000_PA_HIGH()				HAL_GPIO_WritePin(DW1000_PA_GPIO_PORT, DW1000_PA_GPIO_PIN, GPIO_PIN_SET)





/* ==================== DW1000 Device ID ==================== */
#define DW1000_DEVICE_ID            0xDECA0130
#define DW1000_DEV_ID_LEN           4

/* ==================== DW1000 Register Addresses ==================== */
/* 与官方驱动 deca_regs.h 保持一致 */
#define DW1000_REG_DEV_ID           0x00    /* Device ID register (read-only) */
#define DW1000_REG_DEV_CFG          0x01    /* Device Configuration register */
#define DW1000_REG_PANADR           0x03    /* PAN ID and Short Address */
#define DW1000_REG_SYS_CFG          0x04    /* System Configuration */
#define DW1000_REG_SYS_TIME         0x06    /* System Time (read-only) */
#define DW1000_REG_TX_FCTRL         0x08    /* Transmit Frame Control */
#define DW1000_REG_TX_BUFFER        0x09    /* Transmit Data Buffer */
#define DW1000_REG_DX_TIME          0x0A    /* Delayed Transmit Time */
#define DW1000_REG_SYS_CTRL         0x0D    /* System Control register */
#define DW1000_REG_SYS_MASK         0x0E    /* System Event Mask register */
#define DW1000_REG_SYS_STATUS       0x0F    /* System Event Status register (read) / Clear (write) */

/* RX Registers */
#define DW1000_REG_RX_FWINFO        0x10    /* Receive Frame Info register */
#define DW1000_REG_RX_BUFFER        0x11    /* Receive Data Buffer */

/* System Configuration */
#define DW1000_REG_DRX_CONF         0x27    /* Digital Receiver Configuration */
#define DW1000_REG_TX_POWER         0x1E    /* Transmit Power Control */
#define DW1000_REG_CHAN_CTRL         0x1F    /* Channel Control register */
#define DW1000_REG_LRCKGHD_T        0x23    /* LDE Configuration */
#define DW1000_REG_RX_SNIFF         0x21    /* Receiver SNIFF Mode */
#define DW1000_REG_AGC_CTRL         0x23    /* AGC Configuration */
#define DW1000_REG_EXT_SYNC         0x24    /* External Synchronization Control */
#define DW1000_REG_TX_ANTD          0x18    /* TX Antenna Delay */
#define DW1000_REG_RX_ANTD          0x19    /* RX Antenna Delay */
#define DW1000_REG_TX_TIMESTAMP     0x17    /* TX Timestamp (sub-register) */
#define DW1000_REG_RX_TIMESTAMP     0x15    /* RX Timestamp (sub-register) */

/* OTP (One-Time Programmable) Memory */
#define DW1000_REG_OTP_ADDR         0x04    /* OTP Address register */
#define DW1000_REG_OTP_DATA         0x06    /* OTP Data register */
#define DW1000_REG_OTP_CTRL        0x04    /* OTP Control (write to sub-field) */

/* Power Management */
#define DW1000_REG_TX_CTRL          0x12    /* Transmit Control */
#define DW1000_REG_TX_DATA          0x14    /* Data Buffer Access */

/* PMSC (Power Management and System Control) */
#define DW1000_REG_PMSC_CTRL0       0x36    /* PMSC Control 0 */
#define DW1000_REG_PMSC_CTRL1       0x36    /* PMSC Control 1 (sub-register offset) */
#define DW1000_REG_PMSC_SNOZT       0x36    /* PMSC Snooze Timer */

/* GPIO */
#define DW1000_REG_GPIO_MODE         0x26    /* GPIO Mode register */
#define DW1000_REG_GPIO_DIR          0x26    /* GPIO Direction (sub-register offset) */
#define DW1000_REG_GPIO_IRQE         0x2A    /* GPIO IRQ Enable */
#define DW1000_REG_GPIO_ISEL         0x2B    /* GPIO IRQ Sense (edge/level select) */
#define DW1000_REG_GPIO_IB           0x2C    /* GPIO IRQ Clear / Drive Strength */

/* ==================== System Status Register Bits ==================== */
/* 与官方驱动 deca_regs.h SYS_STATUS 位定义保持一致 */
#define DW1000_SYS_STATUS_IRQS      (1 << 0)    /* Interrupt Request Status */
#define DW1000_SYS_STATUS_CPLOCK    (1 << 1)    /* Clock PLL Lock */
#define DW1000_SYS_STATUS_ESYNCR    (1 << 2)    /* External Sync Reset */
#define DW1000_SYS_STATUS_AAT       (1 << 3)    /* Automatic Acknowledge Trigger */
#define DW1000_SYS_STATUS_TXFRB     (1 << 4)    /* TX Frame Begins */
#define DW1000_SYS_STATUS_TXPRS     (1 << 5)    /* TX Preamble Sent */
#define DW1000_SYS_STATUS_TXPHS     (1 << 6)    /* TX PHY Header Sent */
#define DW1000_SYS_STATUS_TXFRS     (1 << 7)    /* TX Frame Sent */
#define DW1000_SYS_STATUS_RXPRD     (1 << 8)    /* RX Preamble Detected */
#define DW1000_SYS_STATUS_RXSFDD    (1 << 9)    /* RX SFD Detected */
#define DW1000_SYS_STATUS_LDEDONE   (1 << 10)   /* LDE Processing Done */
#define DW1000_SYS_STATUS_RXPHD     (1 << 11)   /* RX PHY Header Detected */
#define DW1000_SYS_STATUS_RXPHE     (1 << 12)   /* RX PHY Header Error */
#define DW1000_SYS_STATUS_RXDFR     (1 << 13)   /* RX Data Ready */
#define DW1000_SYS_STATUS_RXFCG     (1 << 14)   /* RX FCS Good (frame received with good CRC) */
#define DW1000_SYS_STATUS_RXFCE     (1 << 15)   /* RX FCS Error */
#define DW1000_SYS_STATUS_RXRFSL    (1 << 16)   /* RX Reed Solomon Frame Sync Loss */
#define DW1000_SYS_STATUS_RXRFTO    (1 << 17)   /* RX Frame Wait Timeout */
#define DW1000_SYS_STATUS_LDEERR    (1 << 18)   /* LDE Error */
#define DW1000_SYS_STATUS_TXPUTE    (1 << 21)   /* TX Power Up Time Error */

/* System Interrupt Clear register bits (same position as status) */
#define DW1000_CLEAR_ALL            0xFFFFFFFF

/* ==================== Channel Control Register Bits ==================== */
#define DW1000_CH_CTRL_RXFPRF_SHIFT 16          /* RX Preamble Repetition */
#define DW1000_CH_CTRL_PHR_6M8      (1 << 20)   /* PHR Rate (6.8Mbps) */
#define DW1000_CH_CTRL_SFD_MODE_48  (0 << 21)   /* SFD Mode */
#define DW1000_CH_CTRL_SFD_MODE_16  (1 << 21)

/* Channel numbers */
#define DW1000_CHANNEL_0            0
#define DW1000_CHANNEL_1            1
#define DW1000_CHANNEL_2            2
#define DW1000_CHANNEL_3            3
#define DW1000_CHANNEL_4            4
#define DW1000_CHANNEL_5            5
#define DW1000_CHANNEL_6            6
#define DW1000_CHANNEL_7            7

/* PRF (Pulse Repetition Frequency) */
#define DW1000_PRF_16M              0
#define DW1000_PRF_64M              1

/* ==================== Preamble Length ==================== */
#define DW1000_PREAMBLE_LEN_64      0   /* 64 symbols */
#define DW1000_PREAMBLE_LEN_128     1
#define DW1000_PREAMBLE_LEN_256     2
#define DW1000_PREAMBLE_LEN_512     3
#define DW1000_PREAMBLE_LEN_1024    4
#define DW1000_PREAMBLE_LEN_1536    5
#define DW1000_PREAMBLE_LEN_2048    6

/* ==================== Data Rate ==================== */
#define DW1000_DATA_RATE_110K       0
#define DW1000_DATA_RATE_850K       1
#define DW1000_DATA_RATE_6M8        2

/* ==================== PAC Size ==================== */
#define DW1000_PAC_8                8
#define DW1000_PAC_16               16
#define DW1000_PAC_32               32
#define DW1000_PAC_64               64

/* ==================== LDE Configuration ==================== */
#define DW1000_LDE_CFG1             0x1802
#define DW1000_LDE_CFG2             0x1607

/* ==================== Operation Mode Config ==================== */
typedef struct
{
    uint8_t  channel;           /* Channel: 1~7 */
    uint8_t  prf;               /* Pulse Repetition Frequency: DW1000_PRF_16M or DW1000_PRF_64M */
    uint8_t  dataRate;          /* Data Rate: DW1000_DATA_RATE_110K/850K/6M8 */
    uint8_t  preambleLen;       /* Preamble Length */
    uint8_t  pacSize;           /* PAC Size */
    uint8_t  txPreambleCode;    /* TX Preamble Code */
    uint8_t  rxPreambleCode;    /* RX Preamble Code */
    uint8_t  sfdMode;           /* SFD Mode: 0=standard, 1=decawave */
} DW1000_OpMode_t;

/* ==================== DW1000 Error Codes ==================== */
typedef enum
{
    DW1000_OK = 0,
    DW1000_ERR_COMM,            /* SPI communication error */
    DW1000_ERR_INIT,            /* Initialization error */
    DW1000_ERR_TIMEOUT,         /* Operation timeout */
    DW1000_ERR_PARAM,           /* Parameter error */
} DW1000_Status_t;

/* ==================== Global Variables ==================== */
//extern SPI_HandleTypeDef hSPI1;
extern volatile uint8_t  g_dw1000_irq_flag;
extern uint8_t battery;
extern volatile uint8_t g_recvirq;
/* ==================== Function Declarations ==================== */

/* Hardware Abstraction Layer */
void     DW1000_GPIO_Init(void);
void     DW1000_NVIC_Init(void);

/* SPI Low-Level Functions */
void     DW1000_SPI_Read(uint16_t addr, uint8_t *buf, uint32_t len);
void     DW1000_SPI_Write(uint16_t addr, const uint8_t *buf, uint32_t len);
uint8_t  DW1000_SPI_ReadByte(uint16_t addr);
void     DW1000_SPI_WriteByte(uint16_t addr, uint8_t val);
void     DW1000_SPI_ReadSub(uint16_t addr, uint8_t offset, uint8_t *buf, uint32_t len);
void     DW1000_SPI_WriteSub(uint16_t addr, uint8_t offset, const uint8_t *buf, uint32_t len);

/* Register Access */
uint32_t DW1000_ReadDevID(void);
void     DW1000_ReadUID(uint8_t uid[8]);
uint32_t DW1000_ReadSysStatus(void);
void     DW1000_ClearSysStatus(uint32_t mask);
void     DW1000_SetSysMask(uint32_t mask);

/* Configuration */
void     DW1000_Reset(void);
void     DW1000_HardReset(void);
//DW1000_Status_t DW1000_Init(const DW1000_OpMode_t *mode);
void     DW1000_EnterSleep(void);
void     DW1000_ExitSleep(void);

/* Channel Configuration */
void     DW1000_SetChannel(uint8_t channel, uint8_t prf);
void     DW1000_SetDataRate(uint8_t rate);
void     DW1000_SetPreambleLen(uint8_t len);
void     DW1000_SetTxPower(uint8_t power);
void     DW1000_SetAntennaDelay(uint16_t txDelay, uint16_t rxDelay);

void	DW1000_Delay_us(uint32_t us);

/* Interrupt */
uint8_t  DW1000_IRQ_Status(void);
uint16_t WriteUwbCoordinatesToFlash(uint8_t *wDat, uint16_t wLen);
uint16_t ReadUwbCoordinatesFromFlash(uint8_t *rDat, uint16_t rLen);
uint16_t WriteDataToFlashWithCRC16(uint8_t *wDat, uint16_t wLen, uint32_t FlashAddr, uint32_t FlashLen);
uint16_t ReadDataFromFlashWithCRC16(uint8_t *rDat, uint16_t rLen, uint32_t FlashAddr, uint32_t FlashLen);

#endif /* __BSP_DW1000_H */
