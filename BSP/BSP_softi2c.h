/******************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称: III型雨量
文件名称: BSP_softi2c.h
作    者: // 作者
版    本: // 版本
完成日期: // 完成日期
功能描述: // 用于详细说明此程序文件完成的主要功能，与其他模块
          // 或函数的接口，输出值、取值范围、含义及参数间的控
	      // 制、顺序、独立或依赖等关系
其他说明: // 其它内容的说明
功能列表: // 主要函数列表，每条记录应包括函数名及功能简要说明
          // 1. ....
历史记录: // 修改历史记录列表，每条修改记录应包括修改日期、修改
          // 作者及修改内容简述

1. 日期:
作者:
修改内容:
2. ..
******************************************************************/
#ifndef __BSP_SOFTI2C_H__
#define __BSP_SOFTI2C_H__

#include "BSP.h"
//#include "stm32h7xx.h"

/********** 宏定义 ***********/

/* 软件模拟I2C返回状态 */
#define SI2C_RET_OK			(0)		//成功
#define SI2C_RET_NOACK		(-1)	//错误 否应答
#define SI2C_RET_NODEVICE	(-2)	//错误 设备地址不存在
#define SI2C_RET_TIMEOUT	(-3)	//错误 设备超时

/* 数据读写方向 */
#define SI2C_DIR_READ	0x01	//软件I2C数据方向 读
#define SI2C_DIR_WRITE	0x00	//软件I2C数据方向 写

/********** 数据结构体声明 ***********/

/* 软件模拟I2C协议参数结构体 */
typedef struct
{
	GPIO_TypeDef*	SCLPort;			//SCL端口号
	uint16_t 		SCLPin;				//SCL管脚号
	GPIO_TypeDef*	SDAPort;			//SDA端口号
	uint16_t		SDAPin;				//SDA管脚号
	uint16_t 		SCLHighHoldDelayUs;	//时钟高保持时间 us
	uint16_t		SCLLowHoldDelayUs;	//时钟低保持时间 us
	uint16_t		SDASampleDelayUs;	//数据采样保持时间 us
	uint16_t		SDAChangeDelayUs;	//数据变化保持时间 us
	
	void			(*cbDelayUs)(uint32_t n);	//微秒延迟回调函数
	uint32_t		DelayTimer;					//延迟定时器
} SoftI2cPar_t;



/********** 外部变量声明 ***********/

extern SoftI2cPar_t Softi2c1;

/*********** 外部函数声明 **********/

/* 初始化 */
void SI2C_Init(SoftI2cPar_t *Par);

/* 读写操作 */
void SI2C_Start(SoftI2cPar_t *Par);
void SI2C_Stop(SoftI2cPar_t *Par);
void SI2C_WriteByte(SoftI2cPar_t *Par, uint8_t Byte, uint8_t *Ack);
void SI2C_ReadByte(SoftI2cPar_t *Par, uint8_t *Byte, uint8_t Ack);

int16_t SI2C_WriteDataByReg(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t RegAddr, uint8_t *wData, uint16_t Len);
int16_t SI2C_ReadDataByReg(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t RegAddr, uint8_t *rData, uint16_t Len);
int16_t SI2C_WriteDataDirect(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t *wData, uint16_t Len);
int16_t SI2C_ReadDataDirect(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t *rData, uint16_t Len);


#endif

