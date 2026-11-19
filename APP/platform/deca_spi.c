/*! ----------------------------------------------------------------------------
 * @file	deca_spi.c
 * @brief	SPI access functions - 使用我们的SPI驱动
 *
 * @attention
 *
 * Copyright 2015 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include "deca_spi.h"
#include "deca_device_api.h"
#include "bsp_dw1000.h"
#include "bsp_gpio.h"

/****************************************************************************//**
 *
 * 								DW1000 SPI section
 *
 *******************************************************************************/

int openspi(void)
{
	return 0;
}

int closespi(void)
{
	return 0;
}

/**
 * @brief  写数据到DW1000 SPI
 * @param  headerLength: 头部长度
 * @param  headerBuffer: 头部数据
 * @param  bodyLength: 数据长度
 * @param  bodyBuffer: 数据内容
 * @retval 0=成功
 */
int writetospi(uint16_t headerLength,
			   const uint8_t *headerBuffer,
			   uint32_t bodyLength,
			   const uint8_t *bodyBuffer)
{
	/* 直接使用SPI总线，保留官方驱动的header格式 */
	DW1000_CS_LOW();
	HAL_SPI_Transmit(&hSPI3, (uint8_t *)headerBuffer, headerLength, 100);
	HAL_SPI_Transmit(&hSPI3, (uint8_t *)bodyBuffer, (uint16_t)bodyLength, 500);
	DW1000_CS_HIGH();

	return 0;
}

/**
 * @brief  从DW1000 SPI读取数据
 * @param  headerLength: 头部长度
 * @param  headerBuffer: 头部数据
 * @param  readlength: 读取长度
 * @param  readBuffer: 读取缓冲区
 * @retval 0=成功
 */
int readfromspi(uint16_t headerLength,
				const uint8_t *headerBuffer,
				uint32_t readlength,
				uint8_t *readBuffer)
{
	/* 直接使用SPI总线，保留官方驱动的header格式 */
	DW1000_CS_LOW();
	HAL_SPI_Transmit(&hSPI3, (uint8_t *)headerBuffer, headerLength, 100);
	HAL_SPI_Receive(&hSPI3, readBuffer, (uint16_t)readlength, 500);
	DW1000_CS_HIGH();

	return 0;
}

/****************************************************************************//**
 *
 * 								END OF DW1000 SPI section
 *
 *******************************************************************************/
