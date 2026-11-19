#ifndef IP2365_H
#define IP2365_H

#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// IP2365 I2C地址 (7位地址)
#define IP2365_I2C_ADDR        0x75  // 0xEA >> 1 = 0x75

// 寄存器地址定义
#define IP2365_REG_SYS_CTL1    0x01  // 系统控制寄存器1
#define IP2365_REG_VINOV_SET   0x04  // 输入过压设置
#define IP2365_REG_NTC_CTL     0x05  // NTC控制
#define IP2365_REG_RJSET       0x0F  // 电流设置使能
#define IP2365_REG_VIN_ISET    0x19  // 输入端充电电流设置
#define IP2365_REG_LED_REG     0x1A  // LED控制
#define IP2365_REG_BAT_ISET    0x1F  // 电池端充电电流设置
#define IP2365_REG_CHG_FULL    0x2B  // 充电状态和充满标志
#define IP2365_REG_NTC_STAT    0x22  // NTC状态和输入过压状态

// 错误代码
#define IP2365_OK             0
#define IP2365_ERROR          -1
#define IP2365_TIMEOUT        -2

// 充电状态定义
typedef enum {
    IP2365_CHG_STATUS_TRICKLE = 0,  // 涓流充电
    IP2365_CHG_STATUS_CC = 1,           // 恒流充电
    IP2365_CHG_STATUS_CV = 3            // 恒压充电
} IP2365_ChargingStatus;

// NTC状态定义
typedef enum {
    IP2365_NTC_HIGH_TEMP = 0,       // 高温
    IP2365_NTC_MID_TEMP = 4,        // 中温
    IP2365_NTC_NORMAL_TEMP = 6,     // 正常温度
    IP2365_NTC_LOW_TEMP = 7         // 低温
} IP2365_NTCStatus;

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint8_t address;
} IP2365_HandleTypeDef;
extern IP2365_HandleTypeDef hip2365;

extern uint32_t g_DetectChaDelay;
extern uint8_t  g_IP2365Init;
extern uint8_t  g_IP2365ErrNum;
// 函数声明
int8_t IP2365_Init(IP2365_HandleTypeDef *hip, I2C_HandleTypeDef *hi2c);
int8_t IP2365_ReadRegister(IP2365_HandleTypeDef *hip, uint8_t reg, uint8_t *data);
int8_t IP2365_WriteRegister(IP2365_HandleTypeDef *hip, uint8_t reg, uint8_t data);

// 充电控制函数
int8_t IP2365_EnableCharging(IP2365_HandleTypeDef *hip, bool enable);
int8_t IP2365_SetInputCurrent(IP2365_HandleTypeDef *hip, float current);
int8_t IP2365_SetBatteryCurrent(IP2365_HandleTypeDef *hip, float current);
int8_t IP2365_EnableResistorCurrentSetting(IP2365_HandleTypeDef *hip, bool bat_enable, bool vin_enable);

// NTC功能函数
int8_t IP2365_EnableNTC(IP2365_HandleTypeDef *hip, bool enable);
int8_t IP2365_SetNTCTresholds(IP2365_HandleTypeDef *hip, uint8_t high_temp, uint8_t mid_temp, uint8_t low_temp);
int8_t IP2365_GetNTCStatus(IP2365_HandleTypeDef *hip, IP2365_NTCStatus *status);

// LED控制函数
int8_t IP2365_SetLEDMode(IP2365_HandleTypeDef *hip, uint8_t charging_mode, uint8_t full_mode);

// 状态读取函数
int8_t IP2365_GetChargingStatus(IP2365_HandleTypeDef *hip, uint8_t *status);
int8_t IP2365_IsChargingFull(IP2365_HandleTypeDef *hip, bool *full);
int8_t IP2365_IsInputOvervoltage(IP2365_HandleTypeDef *hip, bool *ovp);

void IP2365_Detect(void);
#endif // IP2365_H