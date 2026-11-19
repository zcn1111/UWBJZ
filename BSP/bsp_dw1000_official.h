#ifndef __BSP_DW1000_OFFICIAL_H
#define __BSP_DW1000_OFFICIAL_H

#include "stm32l4xx.h"
#include "deca_device_api.h"
#include <stdint.h>

/**
 * @brief  使用官方驱动初始化DW1000
 * @retval 0=成功, -1=失败
 */
int DW1000_Official_Init(void);

/**
 * @brief  使用官方驱动发送数据
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @retval 0=成功, -1=失败
 */
int DW1000_Official_Transmit(uint8_t *data, uint16_t len);

/**
 * @brief  使用官方驱动接收数据
 * @param  buf: 接收缓冲区
 * @param  len: 接收长度指针
 * @retval 0=成功, -1=失败
 */
int DW1000_Official_Receive(uint8_t *buf, uint16_t *len);

#endif /* __BSP_DW1000_OFFICIAL_H */

int DW1000_SetGpioMode(void);
int DW1000_SetNormalMode(void);

extern dwt_config_t config;

