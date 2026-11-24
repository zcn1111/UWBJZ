/**
  ******************************************************************************
  * @file    bsp_dw1000.c
  * @brief   DW1000 UWB Transceiver Driver
  * @note    SPI2: PB10=SCK, PC2=MISO, PC3=MOSI
  *          CSN=PB2, nRST=PB11, IRQn=PB14
  ******************************************************************************
  */
  
#include "bsp.h"
#include "bsp_dw1000.h"
#include "deca_device_api.h"
#include "Drv_W25Qxx_QSPI.h"
#include "g_GlobalValDef.h"
/* Debug print macro */
#define DW1000_DEBUG  1
#if DW1000_DEBUG
#define dw1000_debug_printf(format, args...)  debug_printf(format, ##args)
#else
#define dw1000_debug_printf(...)
#endif

/* ==================== Global Variables ==================== */
volatile uint8_t g_dw1000_irq_flag = 0;
uint8_t battery = 0;
volatile uint8_t g_recvirq=0;

/**
  * @brief  EXTI10-15 ?��??????
  * @note   KEY1=PC0(??EXTI0), DW1000_IRQ=PB14(EXTI14)
  */
void EXTI1_IRQHandler(void)
{	
    HAL_GPIO_EXTI_IRQHandler(DW1000_IRQ_PIN);
}

/**
  * @brief  �ⲿ�жϻص�����
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == DW1000_IRQ_PIN)
    {
        g_dw1000_irq_flag = 1;
        dwt_isr();  /* 调用官方驱动中断处理，触发TX/RX回调 */
		g_recvirq = 0;
    }
}





/* ==================== Delay Helper ==================== */
void DW1000_Delay_us(uint32_t us)
{
    /* 使用 TIM2 的高精度定时器 (10us resolution) 或简单循环延时 */
    volatile uint32_t i;
    for (i = 0; i < us * (SystemCoreClock / 1000000 / 4); i++)
    {
        __NOP();
    }
}

/* ==================== Hardware Init ==================== */

/**
  * @brief  DW1000 控制引脚初始化 (CSN, nRST, IRQ)
  * @note   CSN=PB2, nRST=PB11, IRQn=PB14
  *         SPI2 的 GPIO 在 HAL_SPI_MspInit 中配置
  */
void DW1000_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    /* 使能 GPIOB 时钟 */
    __HAL_RCC_GPIOD_CLK_ENABLE();
	/* 使能 GPIOA 时钟 */
    __HAL_RCC_GPIOE_CLK_ENABLE();
    /* PB2 = CSN (软件控制, 推挽输出, 默认高电平) */
    GPIO_Initure.Pin   = DW1000_CS_PIN;
    GPIO_Initure.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_Initure.Pull  = GPIO_PULLUP;
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DW1000_CS_GPIO_PORT, &GPIO_Initure);
    DW1000_CS_HIGH();   /* CSN 默认拉高 (不选中) */

    /* PB11 = nRST (推挽输出, 默认高电平) */
    GPIO_Initure.Pin   = DW1000_RST_PIN;
    GPIO_Initure.Pull  = GPIO_NOPULL;
    HAL_GPIO_Init(DW1000_RST_GPIO_PORT, &GPIO_Initure);
    DW1000_RST_HIGH();  /* nRST 默认拉高 (非复位状态) */

    /* PB14 = IRQn (下拉输入, 上升沿触发中断) */
    GPIO_Initure.Pin   = DW1000_IRQ_PIN;
    GPIO_Initure.Mode  = GPIO_MODE_IT_RISING;
    GPIO_Initure.Pull  = GPIO_PULLDOWN;
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(DW1000_IRQ_GPIO_PORT, &GPIO_Initure);
	

	/* PA9 = UWB2_POW_CTR */
	GPIO_Initure.Pin = UWB2_POWER_GPIO_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Initure.Pull = GPIO_PULLDOWN;
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(UWB2_POWER_GPIO_PORT, &GPIO_Initure);
}

void UWB2_SetPower(uint8_t enabled)
{
    HAL_GPIO_WritePin(UWB2_POWER_GPIO_PORT, UWB2_POWER_GPIO_PIN,
        enabled ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t UWB2_IsPowerEnabled(void)
{
    return HAL_GPIO_ReadPin(UWB2_POWER_GPIO_PORT, UWB2_POWER_GPIO_PIN) == GPIO_PIN_SET;
}

/**
  * @brief  DW1000 中断优先级配置
  */
void DW1000_NVIC_Init(void)
{
    /* 清除 EXTI15_10 中断标志 */
    __HAL_GPIO_EXTI_CLEAR_IT(DW1000_IRQ_PIN);
    /* EXTI15_10 IRQ 优先级 (抢占优先级 1, 子优先级 1) */
    HAL_NVIC_SetPriority(DW1000_IRQ_EXTI_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DW1000_IRQ_EXTI_IRQn);
}

/* ==================== SPI Low-Level Functions ==================== */

/**
  * @brief  构建 DW1000 SPI 头部并发送
  * @param  addr: 寄存器地址 (11位: bit[10:0])
  * @param  isWrite: 1=写, 0=读
  * @param  buf: 数据缓冲区
  * @param  len: 数据长度
  * @param  readDir: 1=读方向(SPI全双工接收), 0=写方向
  * @note   DW1000 SPI协议:
  *         1字节头: [R/W][E=0][A5][A4][A3][A2][A1][A0]  (地址0x00~0x3F)
  *         2字节头: [R/W][E=1][A10..A6][A5..A0 + SubAddr3..0]  (地址0x40~0x7F)
  */
static void DW1000_SPI_TransferHeader(uint16_t addr, uint8_t isWrite)
{
    uint8_t header[2];
    uint8_t headerLen;

    if (addr < 0x40)
    {
        /* 短地址模式 (1字节头) */
        header[0] = (uint8_t)(addr & 0x3F);
        if (isWrite)
            header[0] &= ~0x80;    /* bit[7] = 0: Write */
        else
            header[0] |= 0x80;     /* bit[7] = 1: Read */
        headerLen = 1;
    }
    else
    {
        /* 扩展地址模式 (2字节头) */
        header[0] = 0x7F | 0x80;   /* bit[6] = 1 (extended), bit[7] = 1 (read) */
        if (isWrite)
            header[0] = 0x7F;      /* bit[7] = 0: Write, bit[6] = 1: Extended */
        header[1] = (uint8_t)(addr & 0x3F);   /* 低6位地址 */
        headerLen = 2;
    }

    DW1000_CS_LOW();
    HAL_SPI_Transmit(&hSPI3, header, headerLen, 100);
}

/**
  * @brief  DW1000 SPI 读数据
  * @param  addr: 寄存器地址 (11位)
  * @param  buf: 接收数据缓冲区
  * @param  len: 要读取的字节数
  */
void DW1000_SPI_Read(uint16_t addr, uint8_t *buf, uint32_t len)
{
    uint8_t header[2];
    uint8_t headerLen;

    /* DW1000 SPI header格式 (与官方驱动一致):
     * file 0x00: 1字节头 [fileID]
     * 其他文件:  2字节头 [fileID|0x40, subAddr]
     * 其中 bit6=1 表示有子地址跟随
     */
    if (addr == 0) {
        header[0] = (uint8_t)(addr & 0x3F);  // 1字节头，读操作
        headerLen = 1;
    } else {
        header[0] = (uint8_t)(addr & 0x3F) | 0x40;  // 2字节头，bit6=1
        header[1] = 0x00;  // 子地址偏移=0
        headerLen = 2;
    }

    DW1000_CS_LOW();
    HAL_SPI_Transmit(&hSPI3, header, headerLen, 100);
    HAL_SPI_Receive(&hSPI3, buf, (uint16_t)len, 500);
    DW1000_CS_HIGH();
}

/**
  * @brief  DW1000 SPI 写数据
  * @param  addr: 寄存器地址 (11位)
  * @param  buf: 发送数据缓冲区
  * @param  len: 要写入的字节数
  */
void DW1000_SPI_Write(uint16_t addr, const uint8_t *buf, uint32_t len)
{
    uint8_t header[2];
    uint8_t headerLen;

    /* DW1000 SPI header格式 (与官方驱动一致):
     * file 0x00: 1字节头 [fileID|0x80]
     * 其他文件:  2字节头 [fileID|0xC0, subAddr]
     * 其中 bit7=1写, bit6=1有子地址跟随
     */
    if (addr == 0) {
        header[0] = (uint8_t)(addr & 0x3F) | 0x80;  // 1字节头，写操作
        headerLen = 1;
    } else {
        header[0] = (uint8_t)(addr & 0x3F) | 0xC0;  // 2字节头，写+子地址
        header[1] = 0x00;  // 子地址偏移=0
        headerLen = 2;
    }

    DW1000_CS_LOW();
    HAL_SPI_Transmit(&hSPI3, header, headerLen, 100);
    HAL_SPI_Transmit(&hSPI3, (uint8_t *)buf, (uint16_t)len, 500);
    DW1000_CS_HIGH();
}

/**
  * @brief  DW1000 SPI 读单字节
  */
uint8_t DW1000_SPI_ReadByte(uint16_t addr)
{
    uint8_t val;
    DW1000_SPI_Read(addr, &val, 1);
    return val;
}

/**
  * @brief  DW1000 SPI 写单字节
  */
void DW1000_SPI_WriteByte(uint16_t addr, uint8_t val)
{
    DW1000_SPI_Write(addr, &val, 1);
}

/**
  * @brief  DW1000 SPI 带子地址读
  * @param  addr: 寄存器地址
  * @param  offset: 子地址偏移
  * @param  buf: 接收数据缓冲区
  * @param  len: 数据长度
  */
void DW1000_SPI_ReadSub(uint16_t addr, uint8_t offset, uint8_t *buf, uint32_t len)
{
    uint16_t regAddr = (uint16_t)((addr & 0x7F) | ((uint16_t)(offset & 0x03) << 6));
    DW1000_SPI_Read(regAddr, buf, len);
}

/**
  * @brief  DW1000 SPI 带子地址写
  * @param  addr: 寄存器地址
  * @param  offset: 子地址偏移
  * @param  buf: 发送数据缓冲区
  * @param  len: 数据长度
  */
void DW1000_SPI_WriteSub(uint16_t addr, uint8_t offset, const uint8_t *buf, uint32_t len)
{
    uint16_t regAddr = (uint16_t)((addr & 0x7F) | ((uint16_t)(offset & 0x03) << 6));
    DW1000_SPI_Write(regAddr, buf, len);
}

/* ==================== Register Access ==================== */

/**
  * @brief  读取 DW1000 Device ID
  * @return 设备ID (应为 0xDECA0130)
  */
uint32_t DW1000_ReadDevID(void)
{
    uint8_t buf[4];
    DW1000_SPI_Read(DW1000_REG_DEV_ID, buf, 4);
    return ((uint32_t)buf[3] << 24) | ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[1] << 8)  | (uint32_t)buf[0];
}

/**
  * @brief  读取 DW1000 唯一 ID (UID)
  */
void DW1000_ReadUID(uint8_t uid[8])
{
    DW1000_SPI_Read(DW1000_REG_DEV_ID + 4, uid, 8);
}

/**
  * @brief  读取系统状态寄存器
  * @return 32位状态值
  */
uint32_t DW1000_ReadSysStatus(void)
{
    uint8_t buf[4];
    DW1000_SPI_Read(DW1000_REG_SYS_STATUS, buf, 4);
    return ((uint32_t)buf[3] << 24) | ((uint32_t)buf[2] << 16) |
           ((uint32_t)buf[1] << 8)  | (uint32_t)buf[0];
}

/**
  * @brief  清除系统状态标志
  * @param  mask: 要清除的标志位
  */
void DW1000_ClearSysStatus(uint32_t mask)
{
    uint8_t buf[4];
    buf[0] = (uint8_t)(mask & 0xFF);
    buf[1] = (uint8_t)((mask >> 8) & 0xFF);
    buf[2] = (uint8_t)((mask >> 16) & 0xFF);
    buf[3] = (uint8_t)((mask >> 24) & 0xFF);
    DW1000_SPI_Write(DW1000_REG_SYS_STATUS, buf, 4);
}

/**
  * @brief  设置系统中断掩码
  * @param  mask: 中断掩码值
  */
void DW1000_SetSysMask(uint32_t mask)
{
    uint8_t buf[4];
    buf[0] = (uint8_t)(mask & 0xFF);
    buf[1] = (uint8_t)((mask >> 8) & 0xFF);
    buf[2] = (uint8_t)((mask >> 16) & 0xFF);
    buf[3] = (uint8_t)((mask >> 24) & 0xFF);
    DW1000_SPI_Write(DW1000_REG_SYS_MASK, buf, 4);
}

/* ==================== Hardware Reset ==================== */

/**
  * @brief  DW1000 软复位 (通过 SPI 写 PMSC Control 寄存器)
  */
void DW1000_Reset(void)
{
    uint8_t val;

    /* 写 PMSC_CTRL0 寄存器 bit[0] = 1 触发软复位 */
    val = DW1000_SPI_ReadByte(DW1000_REG_PMSC_CTRL0);
    val &= 0xF0;           /* 清除低4位 */
    val |= 0x01;           /* bit[0] = 1: 复位 */
    DW1000_SPI_WriteByte(DW1000_REG_PMSC_CTRL0, val);

    DW1000_Delay_us(200);

    /* 恢复 PMSC_CTRL0 */
    val = DW1000_SPI_ReadByte(DW1000_REG_PMSC_CTRL0);
    val &= 0xF0;
    DW1000_SPI_WriteByte(DW1000_REG_PMSC_CTRL0, val);

    DW1000_Delay_us(500);
}

/**
  * @brief  DW1000 硬件复位 (nRST 引脚低脉冲)
  */
void DW1000_HardReset(void)
{
    DW1000_RST_LOW();
    DW1000_Delay_us(100);       /* 至少保持500ns, 这里用100us */
    DW1000_RST_HIGH();
    DW1000_Delay_us(5000);      /* 等待芯片启动, 典型需要3ms */
}

/* ==================== Channel Configuration ==================== */

/**
  * @brief  设置DW1000工作通道和PRF
  */
void DW1000_SetChannel(uint8_t channel, uint8_t prf)
{
    uint8_t buf[4];

    DW1000_SPI_Read(DW1000_REG_CHAN_CTRL, buf, 4);

    /* 清除 CHAN_CTRL[3:0] 通道号 */
    buf[0] = (buf[0] & 0xF0) | (channel & 0x0F);

    /* 设置 PRF: bit[17:16] */
    buf[2] = (buf[2] & 0xFC) | ((prf & 0x01) << 1);

    DW1000_SPI_Write(DW1000_REG_CHAN_CTRL, buf, 4);
}

/**
  * @brief  设置数据速率
  */
void DW1000_SetDataRate(uint8_t rate)
{
    uint8_t val;

    val = DW1000_SPI_ReadByte(DW1000_REG_TX_FCTRL);
    val = (val & 0x83) | ((rate & 0x03) << 1);  /* bits[2:1] = Data Rate */
    DW1000_SPI_WriteByte(DW1000_REG_TX_FCTRL, val);
}

/**
  * @brief  设置前导码长度
  */
void DW1000_SetPreambleLen(uint8_t len)
{
    uint8_t val;

    val = DW1000_SPI_ReadByte(DW1000_REG_TX_FCTRL);
    val = (val & 0xC7) | ((len & 0x07) << 3);   /* bits[5:3] = TX Preamble Length */
    DW1000_SPI_WriteByte(DW1000_REG_TX_FCTRL, val);
}

/**
  * @brief  设置TX功率
  */
void DW1000_SetTxPower(uint8_t power)
{
    uint8_t buf[4];
    buf[0] = power;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;
    DW1000_SPI_Write(DW1000_REG_TX_POWER, buf, 4);
}

/**
  * @brief  设置天线延迟补偿
  */
void DW1000_SetAntennaDelay(uint16_t txDelay, uint16_t rxDelay)
{
    uint8_t buf[2];

    /* TX Antenna Delay (16-bit) */
    buf[0] = (uint8_t)(txDelay & 0xFF);
    buf[1] = (uint8_t)((txDelay >> 8) & 0xFF);
    DW1000_SPI_Write(DW1000_REG_TX_ANTD, buf, 2);

    /* RX Antenna Delay (16-bit) */
    buf[0] = (uint8_t)(rxDelay & 0xFF);
    buf[1] = (uint8_t)((rxDelay >> 8) & 0xFF);
    DW1000_SPI_Write(DW1000_REG_RX_ANTD, buf, 2);
}

/* ==================== Power Management ==================== */

/**
  * @brief  DW1000 进入低功耗睡眠模式
  */
void DW1000_EnterSleep(void)
{
    uint8_t val;

    /* 设置 PMSC_CTRL0 的 SNOZE 位进入深度睡眠 */
    /* 先关闭收发器 */
    val = DW1000_SPI_ReadByte(DW1000_REG_PMSC_CTRL0);
    val = (val & 0xFC) | 0x02;  /* bits[1:0] = 0b10: 睡眠模式 */
    DW1000_SPI_WriteByte(DW1000_REG_PMSC_CTRL0, val);
}

/**
  * @brief  DW1000 退出睡眠模式
  */
void DW1000_ExitSleep(void)
{
    uint8_t val;

    val = DW1000_SPI_ReadByte(DW1000_REG_PMSC_CTRL0);
    val = (val & 0xFC);         /* bits[1:0] = 0b00: 正常模式 */
    DW1000_SPI_WriteByte(DW1000_REG_PMSC_CTRL0, val);
    DW1000_Delay_us(500);
}

/* ==================== Interrupt ==================== */

/**
  * @brief  读取 DW1000 IRQ 引脚电平
  * @return 1=有中断, 0=无中断
  */
uint8_t DW1000_IRQ_Status(void)
{
    return (HAL_GPIO_ReadPin(DW1000_IRQ_GPIO_PORT, DW1000_IRQ_PIN) == GPIO_PIN_SET) ? 1 : 0;
}

/* ==================== Initialization ==================== */

/**
  * @brief  DW1000 初始化
  * @param  mode: 工作模式配置
  * @return DW1000_Status_t
  * @note   默认配置: Channel 5, 6.8Mbps, PRF 64MHz, Preamble 128
  *         常用配置: Channel 5, 110Kbps, PRF 16MHz, Preamble 1024 (长距离)
  *         常用配置: Channel 9 (802.15.4z HRP), 6.8Mbps, PRF 64MHz
  */
DW1000_Status_t DW1000_Init(const DW1000_OpMode_t *mode)
{
    uint32_t devID;
    uint8_t  retry = 5;

    dw1000_debug_printf("[DW1000] Init start\r\n");

    /* 1. GPIO 初始化 (先初始化CS/RST/IRQ引脚) */
    dw1000_debug_printf("[DW1000] GPIO init...\r\n");
    DW1000_GPIO_Init();

    /* 2. 硬件复位 (GPIO初始化后再复位) */
    dw1000_debug_printf("[DW1000] Hard reset...\r\n");
    DW1000_HardReset();

    /* 3. SPI2 初始化 */
    dw1000_debug_printf("[DW1000] SPI2 init...\r\n");
    SPI3_Init();

    /* 测试CS引脚控制 */
    dw1000_debug_printf("[DW1000] Test CS pin...\r\n");
    DW1000_CS_HIGH();
    DW1000_Delay_us(10);
    dw1000_debug_printf("[DW1000] CS HIGH = %d\r\n",
    HAL_GPIO_ReadPin(DW1000_CS_GPIO_PORT, DW1000_CS_PIN));
	
    DW1000_CS_LOW();
    DW1000_Delay_us(10);
    dw1000_debug_printf("[DW1000] CS LOW = %d\r\n",
    HAL_GPIO_ReadPin(DW1000_CS_GPIO_PORT, DW1000_CS_PIN));
    DW1000_CS_HIGH();

    /* 4. 读取 Device ID, 验证通信 */
    dw1000_debug_printf("[DW1000] Read Device ID...\r\n");
    while (retry--)
    {
        devID = DW1000_ReadDevID();
        dw1000_debug_printf("[DW1000] DevID = 0x%08X (retry=%d)\r\n", devID, retry);
        if (devID == DW1000_DEVICE_ID)
            break;
        DW1000_Delay_us(500);
    }

    if (devID != DW1000_DEVICE_ID)
    {
        dw1000_debug_printf("[DW1000] ERROR: Init failed! DevID=0x%08X\r\n", devID);
        return DW1000_ERR_INIT;
    }

    dw1000_debug_printf("[DW1000] Device ID OK: 0x%08X\r\n", devID);

    /* 5. 清除所有中断状态 */
    DW1000_ClearSysStatus(DW1000_CLEAR_ALL);

    /* 6. 配置通道 */
    if (mode != NULL)
    {
        DW1000_SetChannel(mode->channel, mode->prf);
        DW1000_SetDataRate(mode->dataRate);
        DW1000_SetPreambleLen(mode->preambleLen);
    }
    else
    {
        /* 默认配置: Channel 5, 6.8Mbps, PRF 64MHz, Preamble 128 */
        DW1000_SetChannel(5, DW1000_PRF_64M);
        DW1000_SetDataRate(DW1000_DATA_RATE_6M8);
        DW1000_SetPreambleLen(DW1000_PREAMBLE_LEN_128);
    }

    /* 7. 设置默认TX功率 (最大) */
    DW1000_SetTxPower(0);

    /* 8. 使能中断 (默认关闭) */
    DW1000_SetSysMask(0);

    return DW1000_OK;
}

/******************************************************************
函数名称: void WriteUwbCoordinatesToFlash(void)
功能描述: 向Flash写UWB坐标
参数输入: uint8_t	*wDat	写入数据
		  uint16_t	wLen	写入数据长度
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t WriteUwbCoordinatesToFlash(uint8_t *wDat, uint16_t wLen)
{
	uint16_t crc16=0;
	uint32_t addr=0;
	bool wret=false, wret2=false;
	
	crc16 = CalcCRC16(wDat, wLen);	//计算CRC16
	
	wret = FlashWriteData(wDat, FLASH_UWB_COORDINATES_START, wLen);							//写UWB坐标
	wret2 = FlashWriteData((uint8_t*)&crc16, FLASH_UWB_COORDINATES_START + wLen + 2, 2);	//写校验
	if ((wret == true) && (wret2 == true))
	{
		return wLen;
	}
	
	return 0;
}

/******************************************************************
函数名称: uint16_t ReadUwbCoordinatesFromFlash(void)
功能描述: 从Flash读UWB坐标
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t ReadUwbCoordinatesFromFlash(uint8_t *rDat, uint16_t rLen)
{
	uint16_t crc16=0;
	uint16_t readcrc=0;
	
	/* 读坐标和crc校验 */
	W25qxx_ReadBytes(rDat, FLASH_UWB_COORDINATES_START, rLen);
	W25qxx_ReadBytes((uint8_t*)&readcrc, FLASH_UWB_COORDINATES_START + rLen + 2, 2);
	
	crc16 = CalcCRC16(rDat, rLen);	//计算CRC16
	if (crc16 == readcrc)			//读取校验成功
	{
		return rLen;
	}
	
	return 0;
}
/******************************************************************
函数名称: void WriteDataToFlashWithCRC16
功能描述: 向Flash写数据
参数输入: uint8_t	*wDat		写入数据
		  uint16_t	wLen		写入数据长度
		  uint32_t	FlashAddr	Flash地址
		  uint32_t	FlashLen	Flash长度
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t WriteDataToFlashWithCRC16(uint8_t *wDat, uint16_t wLen, uint32_t FlashAddr, uint32_t FlashLen)
{
	uint16_t crc16=0;
	uint32_t crc_addr=0;
	bool wdret=false;
	bool wcret=false;
	
	/* 数据有效性确认 */
	if (wLen >= FlashLen)
	{
		return 0;		//数据长度错误
	}
	
	crc16 = CalcCRC16(wDat, wLen);		//计算CRC16
	wdret = FlashWriteData(wDat, FlashAddr, wLen);		//写数据
	if (wdret == true)
	{
		crc_addr = FlashAddr + wLen + 2;	//计算CRC地址
		wcret = FlashWriteData((uint8_t*)&crc16, crc_addr, 2);	//写CRC16
		if (wcret == true)
		{
			return wLen;	//写入成功返回写入字节 不包含crc字节数
		}
	}

	return 0;
}

/******************************************************************
函数名称: void ReadDataFromFlashWithCRC16
功能描述: 从Flash读数据
参数输入: uint8_t	*rDat		读出数据
		  uint16_t	rLen		读出数据长度
		  uint32_t	FlashAddr	Flash地址
		  uint32_t	FlashLen	Flash长度
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint16_t ReadDataFromFlashWithCRC16(uint8_t *rDat, uint16_t rLen, uint32_t FlashAddr, uint32_t FlashLen)
{
	uint16_t crc16=0;
	uint16_t readcrc=0;
	uint32_t crc_addr=0;
	bool rdret=false;
	bool rcret=false;
	
	/* 数据有效性确认 */
	if (rLen >= FlashLen)
	{
		return 0;		//数据长度错误
	}
	
	/* 读坐标和crc校验 */
	W25qxx_ReadBytes(rDat, FlashAddr, rLen);
	crc_addr = FlashAddr + rLen + 2;
	W25qxx_ReadBytes((uint8_t*)&readcrc, crc_addr, 2);
	
	crc16 = CalcCRC16(rDat, rLen);		//计算CRC16
	if (crc16 == readcrc)
	{
		return rLen;	//成功读取数据 返回读出字节长度
	}

	return 0;
}

