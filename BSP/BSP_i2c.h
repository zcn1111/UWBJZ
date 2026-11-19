#ifndef __BSP_I2C_H
#define __BSP_I2C_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"


/******* GPIO管脚名称定义 *******/

/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
extern I2C_HandleTypeDef	hI2C1;
extern I2C_HandleTypeDef	hI2C2;
extern I2C_HandleTypeDef	hI2C3;


/******* 外部函数声明 *******/
void BSP_I2cSetupPara(I2C_HandleTypeDef *I2cHandler, uint8_t Addr, uint32_t AddrMode, uint32_t GenCallMode, uint32_t NoStretchMode);
void BSP_I2cBindInstance(I2C_HandleTypeDef *I2cHandler, I2C_TypeDef *Inst);
void I2C2_Init(void);
void I2C3_Init(void);
void MX_GPIO_Init(void);
#endif

