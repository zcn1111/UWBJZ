/*
 ******************************************************************************
 * @file    read_data_simple.c
 * @author  MEMS Software Solution Team
 * @date    30-November-2017
 * @brief   This file show the simplest way to get data from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "IMU_Acc.h"
#include "lsm6dsl_reg.h"
#include <string.h>
#include <stdio.h>

#include "BSP.h"
/* OLED */
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_fonts.h"
#include "RYDW_Init.h"
typedef union{
  int16_t i16bit[3];
  uint8_t u8bit[6];
} axis3bit16_t;

typedef union{
  int16_t i16bit;
  uint8_t u8bit[2];
} axis1bit16_t;

/* Private macro -------------------------------------------------------------*/
#define CS_SPI2_GPIO_Port   GPIOD
#define CS_SPI2_Pin         GPIO_PIN_0

#define TX_BUF_DIM          1000

/* Private variables ---------------------------------------------------------*/
static axis3bit16_t data_raw_acceleration;
static axis3bit16_t data_raw_angular_rate;
static axis1bit16_t data_raw_temperature;
static uint8_t whoamI, rst;



uint32_t g_tmrIMUDelay;				//IMU采样定时器
uint32_t g_tmrIMUDispDelay;			//IMU输出定时器

/* Extern variables ----------------------------------------------------------*/
stmdev_ctx_t	g_IMUDevCtx;
IMU_Val_t		g_IMUVal;

/* Private functions ---------------------------------------------------------*/

/*
 *   Replace the functions "platform_write" and "platform_read" with your
 *   platform specific read and write function.
 *   This example use an STM32 evaluation board and CubeMX tool.
 *   In this case the "*handle" variable is useful in order to select the
 *   correct interface but the usage of "*handle" is not mandatory.
 *   使用自己平台的读写函数替换 platform_write 和 platform_read 函数
 */

static int32_t platform_write(void *handle, uint8_t Reg, uint8_t *Bufp,
                              uint16_t len)
{
  if (handle == &hSPI2)
  {
    HAL_GPIO_WritePin(CS_SPI2_GPIO_Port, CS_SPI2_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &Reg, 1, 1000);
    HAL_SPI_Transmit(handle, Bufp, len, 1000);
    HAL_GPIO_WritePin(CS_SPI2_GPIO_Port, CS_SPI2_Pin, GPIO_PIN_SET);
  }
  
  return 0;
}

static int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp,
                             uint16_t len)
{
  if (handle == &hSPI2)
  {
    Reg |= 0x80;
    HAL_GPIO_WritePin(CS_SPI2_GPIO_Port, CS_SPI2_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(handle, &Reg, 1, 1000);
    HAL_SPI_Receive(handle, Bufp, len, 1000);
    HAL_GPIO_WritePin(CS_SPI2_GPIO_Port, CS_SPI2_Pin, GPIO_PIN_SET);
  }
  
  return 0;
}

/*
 *  Function to print messages
 *  打印信息函数 
 */
//static void tx_com( uint8_t *tx_buffer, uint16_t len )
//{
//	HAL_UART_Transmit(&hUSART2, tx_buffer, len, 1000);
//}

/******************************************************************
函数名称: void IMU_InitDriver
功能描述: IMU驱动初始化
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void IMU_InitDriver(stmdev_ctx_t *pDevCtx)
{
	/* 配置驱动读写函数 */
	pDevCtx->handle = &hSPI2;
	pDevCtx->write_reg = platform_write;
	pDevCtx->read_reg = platform_read;
	
	/* 检查设备ID */
	whoamI = 0;
	lsm6dsl_device_id_get(pDevCtx, &whoamI);
	if (whoamI != LSM6DSL_ID)
	{
		ssd1306_ZhengChang(36, 27, White,0);
	}
	else 
	{
		/* 等待设备复位 */
		lsm6dsl_reset_set(pDevCtx, PROPERTY_ENABLE);
		do
		{
			lsm6dsl_reset_get(pDevCtx, &rst);
		} while (rst);
		
		/*
		*  Enable Block Data Update
		*/
		lsm6dsl_block_data_update_set(pDevCtx, PROPERTY_ENABLE);
		/*
		* Set Output Data Rate
		*/
		lsm6dsl_xl_data_rate_set(pDevCtx, LSM6DSL_XL_ODR_12Hz5);
		lsm6dsl_gy_data_rate_set(pDevCtx, LSM6DSL_GY_ODR_12Hz5);
		/*
		* Set full scale
		*/ 
		lsm6dsl_xl_full_scale_set(pDevCtx, LSM6DSL_2g);
		lsm6dsl_gy_full_scale_set(pDevCtx, LSM6DSL_2000dps);

		/*
		* Configure filtering chain(No aux interface)
		*/ 
		/* Accelerometer - analog filter */
		lsm6dsl_xl_filter_analog_set(pDevCtx, LSM6DSL_XL_ANA_BW_400Hz);

		/* Accelerometer - LPF1 path ( LPF2 not used )*/
		//lsm6dsl_xl_lp1_bandwidth_set(&dev_ctx, LSM6DSL_XL_LP1_ODR_DIV_4);

		/* Accelerometer - LPF1 + LPF2 path */  
		lsm6dsl_xl_lp2_bandwidth_set(pDevCtx, LSM6DSL_XL_LOW_NOISE_LP_ODR_DIV_100);

		/* Accelerometer - High Pass / Slope path */
		//lsm6dsl_xl_reference_mode_set(&dev_ctx, PROPERTY_DISABLE);
		//lsm6dsl_xl_hp_bandwidth_set(&dev_ctx, LSM6DSL_XL_HP_ODR_DIV_100);

		/* Gyroscope - filtering chain */
		lsm6dsl_gy_band_pass_set(pDevCtx, LSM6DSL_HP_260mHz_LP1_STRONG);
		g_InitSuccflag |= 0x01;
		ssd1306_ZhengChang(36, 27, White,1);
	}
}

/******************************************************************
函数名称: uint16_t IMU_GetAccelerationInMMg
功能描述: IMU获取加速度值(毫g)
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_GetAccelerationInMMg(stmdev_ctx_t *pDevCtx, float *AccX, float *AccY, float *AccZ)
{
	lsm6dsl_reg_t	reg;
	float			acc_mg[3];
	
	IMUResult		ret;

	lsm6dsl_status_reg_get(pDevCtx, &reg.status_reg);	//获取寄存器状态
	ret = IMURET_NODATA_ERR;
	
	/* 加速度有数据 */
	if (reg.status_reg.xlda)
	{
		/* Read magnetic field data */
		memset(data_raw_acceleration.u8bit, 0x00, 3*sizeof(int16_t));
		lsm6dsl_acceleration_raw_get(pDevCtx, data_raw_acceleration.u8bit);
		acc_mg[0] = lsm6dsl_from_fs2g_to_mg(data_raw_acceleration.i16bit[0]);
		acc_mg[1] = lsm6dsl_from_fs2g_to_mg(data_raw_acceleration.i16bit[1]);
		acc_mg[2] = lsm6dsl_from_fs2g_to_mg(data_raw_acceleration.i16bit[2]);

		*AccX = acc_mg[0];
		*AccY = acc_mg[1];
		*AccZ = acc_mg[2];
		
		/* 输出加速度值 */
//		sprintf((char*)tx_buffer, "Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
//			  acc_mg[0], acc_mg[1], acc_mg[2]);
//		tx_com(tx_buffer, strlen((char const*)tx_buffer));
		ret = IMURET_OK;
	}
	
	return ret;
}

/******************************************************************
函数名称: uint16_t IMU_GetAngularRateInMMdps
功能描述: IMU获取角速率值(毫弧度每秒mdps)
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_GetAngularRateInMMdps(stmdev_ctx_t *pDevCtx, float *GyoX, float *GyoY, float *GyoZ)
{
	lsm6dsl_reg_t	reg;
	float			gyo_mdps[3];
	
	IMUResult		ret;
	
	lsm6dsl_status_reg_get(pDevCtx, &reg.status_reg);	//获取寄存器状态
	ret = IMURET_NODATA_ERR;
	
	/* 陀螺仪有数据 */
	if (reg.status_reg.gda)
	{
		/* Read magnetic field data */
		memset(data_raw_angular_rate.u8bit, 0x00, 3*sizeof(int16_t));
		lsm6dsl_angular_rate_raw_get(pDevCtx, data_raw_angular_rate.u8bit);
		gyo_mdps[0] = lsm6dsl_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[0]);
		gyo_mdps[1] = lsm6dsl_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[1]);
		gyo_mdps[2] = lsm6dsl_from_fs2000dps_to_mdps(data_raw_angular_rate.i16bit[2]);

		*GyoX = gyo_mdps[0];
		*GyoY = gyo_mdps[1];
		*GyoZ = gyo_mdps[2];
		
		/* 输出角速率值 */
//		sprintf((char*)tx_buffer, "Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
//			  gyo_mdps[0], gyo_mdps[1], gyo_mdps[2]);
//		tx_com(tx_buffer, strlen((char const*)tx_buffer));
		ret = IMURET_OK;
	}
	
	return ret;
}

/******************************************************************
函数名称: uint16_t IMU_GetTemperatureInCelsius
功能描述: IMU获取温度值(摄氏度)
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
IMUResult IMU_GetTempratureInCelsius(stmdev_ctx_t *pDevCtx, float *Temp)
{
	lsm6dsl_reg_t	reg;
	float			tempc;
	
	IMUResult		ret;
	
	lsm6dsl_status_reg_get(pDevCtx, &reg.status_reg);	//获取寄存器状态
	ret = IMURET_NODATA_ERR;
	
	/* 温度计有数据 */
    if (reg.status_reg.tda)
    {  
		/* Read temperature data */
		memset(data_raw_temperature.u8bit, 0x00, sizeof(int16_t));
		lsm6dsl_temperature_raw_get(pDevCtx, data_raw_temperature.u8bit);
		tempc = lsm6dsl_from_lsb_to_celsius(data_raw_temperature.i16bit);

		*Temp = tempc;
		
		/* 输出温度值 */
//		sprintf((char*)tx_buffer, "Temperature [degC]:%6.2f\r\n", tempc);
//		tx_com(tx_buffer, strlen((char const*)tx_buffer));
		ret = IMURET_OK;
    }
	
	return ret;
}

/************************************** IMU数据处理函数 ************************************************/

