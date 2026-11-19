/**
  ******************************************************************************
  * @file    bsp_dw1000_official.c
  * @brief   使用DecaWave官方驱动初始化DW1000
  ******************************************************************************
  */

#include "bsp.h"
#include "bsp_dw1000.h"
#include "bsp_dw1000_official.h"
#include "deca_device_api.h"
#include "deca_regs.h"

/* 官方驱动外部函数 */
extern int dwt_initialise(uint16_t mode);
extern void dwt_configure(dwt_config_t *config);
extern int dwt_starttx(uint8 mode);
extern void dwt_writetxfctrl(uint16 txFrameLength, uint16 txBufferOffset, int ranging);
extern void dwt_write8bitoffsetreg(int regFileID, int regOffset, uint8 regval);
extern uint8 dwt_read8bitoffsetreg(int regFileID, int regOffset);

/* 配置结构体 - Channel 2, 64MHz PRF (与参考工程一致) */
dwt_config_t config = {
    .chan = 2,                              /* Channel number */
    .prf = DWT_PRF_64M,                    /* Pulse repetition frequency */
    .txPreambLength = DWT_PLEN_64,         /* Preamble length */
    .rxPAC = DWT_PAC8,                     /* Preamble acquisition chunk size */
    .txCode = 10,                          /* TX preamble code */
    .rxCode = 10,                          /* RX preamble code */
    .nsSFD = 1,                            /* 1=non-standard SFD */
    .dataRate = DWT_BR_6M8,               /* Data rate */
    .phrMode = DWT_PHRMODE_STD,           /* PHY header mode */
    .sfdTO = (65 + 16 - 8)                /* SFD timeout (DW_NS_SFD_LEN_6M8=16) */
};

/* 配置DW1000 GPIO模式 */
int DW1000_SetGpioMode(void)
{
	uint32_t gpio_mode = dwt_read32bitoffsetreg(GPIO_CTRL_ID, 0);
	debug_printf("[DW1000] GPIO_MODE before=0x%06X\r\n", gpio_mode & 0xFFFFFF);
	/* 清除GPIO2 GPIO3 GPIO4 GPIO6模式位，设为GPIO模式(00) */
	gpio_mode &= ~(GPIO_MSGP2_MASK);  /* GPIO2[11:10]=00 */
	gpio_mode &= ~(GPIO_MSGP3_MASK);  /* GPIO3[13:12]=00 */
	gpio_mode &= ~(GPIO_MSGP4_MASK);
	gpio_mode &= ~(GPIO_MSGP6_MASK);
	dwt_write32bitoffsetreg(GPIO_CTRL_ID, 0, gpio_mode);
	/* 配置GPIO输出模式 */
    dwt_setgpiodirection(GxM2 | GxM3 | GxM4 | GxM6, (~(GxP2) & ~(GxP3) & ~(GxP4) & ~(GxP6)));
}

/* 配置DW1000 正常模式 */
int DW1000_SetNormalMode(void)
{
	uint32_t gpio_mode = dwt_read32bitoffsetreg(GPIO_CTRL_ID, 0);
	debug_printf("[DW1000] GPIO_MODE before=0x%06X\r\n", gpio_mode & 0xFFFFFF);
	/* 设为工作模式(01) */
	gpio_mode |= GPIO_PIN2_RXLED;  /* GPIO2[11:10]=00 */
	gpio_mode |= GPIO_PIN3_TXLED;  /* GPIO3[13:12]=00 */
	gpio_mode |= GPIO_PIN4_EXTPA;
	gpio_mode |= GPIO_PIN6_EXTRXE;
	dwt_write32bitoffsetreg(GPIO_CTRL_ID, 0, gpio_mode);
}

/**
 * @brief  使用官方驱动初始化DW1000
 * @retval 0=成功, -1=失败
 */
int DW1000_Official_Init(void)
{
    uint32_t devID;
    int ret;

    debug_printf("[DW1000] Official init start\r\n");

    /* 1. GPIO初始化 */
    DW1000_GPIO_Init();

    /* 2. 硬件复位 */
    DW1000_HardReset();

    /* 3. SPI初始化 */
    SPI3_Init();

    debug_printf("[DW1000] Hardware reset done\r\n");

    /* 4. 使用官方驱动初始化 */
    ret = dwt_initialise(DWT_LOADUCODE);
    if (ret != DWT_SUCCESS) {
        debug_printf("[DW1000] dwt_initialise failed: %d\r\n", ret);
        return -1;
    }

    debug_printf("[DW1000] dwt_initialise OK\r\n");

    /* 5. 配置DW1000 */
    dwt_configure(&config);
	debug_printf("[CFG] ch=%d txCode=%d rxCode=%d nsSFD=%d PLEN=%d\r\n",
				config.chan, config.txCode, config.rxCode, config.nsSFD, config.txPreambLength);
    debug_printf("[DW1000] dwt_configure OK\r\n");

    /* 6. 配置天线延迟 (Channel 5 典型值) */
    dwt_settxantennadelay(16384);
    dwt_setrxantennadelay(16384);
    debug_printf("[DW1000] Antenna delay set to 16384\r\n");

//    /* 7. LED测试：只控制GPIO2和GPIO3 */
//    {
//        uint32_t gpio_mode = dwt_read32bitoffsetreg(GPIO_CTRL_ID, 0);
//        debug_printf("[DW1000] GPIO_MODE before=0x%06X\r\n", gpio_mode & 0xFFFFFF);
//        /* 清除GPIO2和GPIO3模式位，设为GPIO模式(00) */
//        gpio_mode &= ~(0x00000C00);  /* GPIO2[11:10]=00 */
//        gpio_mode &= ~(0x00003000);  /* GPIO3[13:12]=00 */
//		gpio_mode &= ~(GPIO_MSGP4_MASK);
//		gpio_mode &= ~(GPIO_MSGP6_MASK);
//        dwt_write32bitoffsetreg(GPIO_CTRL_ID, 0, gpio_mode);
//        /* 读回验证 */
//        uint32_t verify = dwt_read32bitoffsetreg(GPIO_CTRL_ID, 0);
//        debug_printf("[DW1000] GPIO_MODE after =0x%06X\r\n", verify & 0xFFFFFF);
//    }

//    /* 设置GPIO2/3方向为输出 */
//    dwt_setgpiodirection(GxM2 | GxM3 | GxM4 | GxM6, (~(GxP2) & ~(GxP3) & ~(GxP4) & ~(GxP6)));
////	dwt_setgpiodirection(GxM4 | GxM6, ~(GxP4) & ~(GxP6));
//    /* 读回GPIO_DIR寄存器(偏移0x08)验证 */
//    {
//        uint8_t dir_buf[3] = {0};
//        dwt_readfromdevice(GPIO_CTRL_ID, GPIO_DIR_OFFSET, 3, dir_buf);
//        debug_printf("[DW1000] GPIO_DIR[0..2]=0x%02X 0x%02X 0x%02X\r\n", dir_buf[0], dir_buf[1], dir_buf[2]);
//    }

//	dwt_setgpiovalue(GxM2 | GxM3, GDP2 | GDP3);
//	volatile uint32_t delay=0;
//	while (delay < 300000) delay++;
	

//    /* 延迟1秒让用户看到LED */
//    {
//        volatile uint32_t delay = 0;
//        while (delay < 8000000) delay++;
//    }
	
//	uint32_t gpio_mode = dwt_read32bitoffsetreg(GPIO_CTRL_ID, 0);
//	debug_printf("[DW1000] GPIO_MODE before=0x%06X\r\n", gpio_mode & 0xFFFFFF);
//	/* 清除已配置的LED模式位，控制管脚不变设为GPIO模式(00) */
//	gpio_mode &= ~(GPIO_MSGP2_MASK);  /* GPIO2[11:10]=00 */
//	gpio_mode &= ~(GPIO_MSGP3_MASK);  /* GPIO3[13:12]=00 */
//	gpio_mode &= ~(GPIO_MSGP4_MASK);
//	gpio_mode &= ~(GPIO_MSGP6_MASK);
//	/* 配置标准模式 */
//	gpio_mode |= GPIO_PIN2_RXLED;
//	gpio_mode |= GPIO_PIN3_TXLED;
//	gpio_mode |= GPIO_PIN4_EXTPA;
//	gpio_mode |= GPIO_PIN6_EXTRXE;
//	dwt_write32bitoffsetreg(GPIO_CTRL_ID, 0, gpio_mode);


    /* 7.5 设置RX超时 (约50ms，单位1.0256us) */
//    dwt_setrxtimeout(48750);
	dwt_setrxtimeout(0);

    /* 8. 使能RX中断 (接收好CRC帧 + 发送完成) */
    dwt_setinterrupt(DWT_INT_RFCG | DWT_INT_TFRS, 1);

    /* 8. 使能STM32 NVIC中断 (EXTI15_10 for PB14) */
    DW1000_NVIC_Init();
    debug_printf("[DW1000] NVIC interrupt enabled\r\n");

    /* 9. 配置TX参数 */
    dwt_writetxfctrl(24, 0, 0);  /* 帧长度24字节，无ranging */

    /* 10. 使能事件计数器(调试用) */
    dwt_configeventcounters(1);

    /* 11. 初始化完成，使用标准SPI速率（降低到18MHz避免通信延迟） */
    /* 注释掉快速SPI切换，使用标准速率以确保稳定性 */
    // port_set_dw1000_fastrate();
    debug_printf("[DW1000] SPI at standard rate (18MHz)\r\n");

    debug_printf("[DW1000] Official init complete!\r\n");

    return 0;
}

/**
 * @brief  使用官方驱动发送数据
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @retval 0=成功, -1=失败
 */
int DW1000_Official_Transmit(uint8_t *data, uint16_t len)
{
    int ret;

    /* 写入TX缓冲区 */
    dwt_writetxdata(len, data, 0);

    /* 设置帧长度 */
    dwt_writetxfctrl(len, 0, 0);

    /* 触发发送 */
    ret = dwt_starttx(DWT_START_TX_IMMEDIATE);

    if (ret != DWT_SUCCESS) {
        return -1;
    }

    /* 等待发送完成 */
    uint32_t timeout = 0;
    while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS)) {
        timeout++;
        if (timeout > 100000) {
            return -1;
        }
    }

    /* 清除发送完成标志 */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);

    return 0;
}

/**
 * @brief  使用官方驱动接收数据
 * @param  buf: 接收缓冲区
 * @param  len: 接收长度指针
 * @retval 0=成功, -1=失败
 */
int DW1000_Official_Receive(uint8_t *buf, uint16_t *len)
{
    uint32_t status;

    /* 使能接收 */
    dwt_rxenable(0);

    /* 等待接收完成 */
    uint32_t timeout = 0;
    while (1) {
        status = dwt_read32bitreg(SYS_STATUS_ID);
        if (status & SYS_STATUS_RXFCG) {
            break;  /* 接收成功 */
        }
        if (status & (SYS_STATUS_RXPHE | SYS_STATUS_RXFCE | SYS_STATUS_RXRFSL)) {
            return -1;  /* 接收错误 */
        }
        timeout++;
        if (timeout > 100000) {
            return -1;  /* 超时 */
        }
    }

    /* 读取帧长度 */
    *len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;

    /* 读取数据 */
    dwt_readrxdata(buf, *len, 0);

    /* 清除接收标志 */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);

    return 0;
}
