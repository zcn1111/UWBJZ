/*! ----------------------------------------------------------------------------
 * @file	port.h
 * @brief	HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2015 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */


#ifndef PORT_H_
#define PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include "compiler.h"

#include "stm32l4xx.h"
#include "stm32l4xx_hal.h"
#include "bsp_dw1000.h"

#define BUFFLEN 	(127)   //(4096+128)
#define BUF_SIZE	(127)

/* 官方驱动和板级初始化共用 UWBJZ 的 DW1000 引脚定义。 */
#define DW_IRQn_Pin         DW1000_IRQ_PIN
#define DW_IRQn_GPIO_Port   DW1000_IRQ_GPIO_PORT
#define DW_RESET_Pin        DW1000_RST_PIN
#define DW_RESET_GPIO_Port  DW1000_RST_GPIO_PORT
#define DW_NSS_Pin          DW1000_CS_PIN
#define DW_NSS_GPIO_Port    DW1000_CS_GPIO_PORT
#define DW_IRQn_EXTI_IRQn   DW1000_IRQ_EXTI_IRQn

/* ACC_IRQ引脚 - 如果没有可以定义为任意引脚 */
#ifndef ACC_IRQ_Pin
#define ACC_IRQ_Pin         GPIO_PIN_0
#define ACC_IRQ_GPIO_Port   GPIOA
#endif

/* EXPR引脚 - 如果没有可以定义为任意引脚 */
#ifndef EXPR_Pin
#define EXPR_Pin            GPIO_PIN_1
#define EXPR_GPIO_Port      GPIOA
#endif

typedef uint64_t        uint64;
typedef int64_t         int64;

/* led_t和flag_acc_irq在项目的bsp_gpio.h和bsp_key.h中已定义，这里不再重复定义 */
/* 避免与项目原有代码冲突 */

extern volatile int32_t sys_time_diff;

#define DECAIRQ_EXTI_IRQn           DW_IRQn_EXTI_IRQn

#define DW1000_RSTn					DW_RESET_Pin
#define DW1000_RSTn_GPIO	        DW_RESET_GPIO_Port

#define DECAIRQ                     DW_IRQn_Pin
#define DECAIRQ_GPIO                DW_IRQn_GPIO_Port



#define GPIO_ResetBits(x,y)				    HAL_GPIO_WritePin(x,y, RESET)
#define GPIO_SetBits(x,y)					HAL_GPIO_WritePin(x,y, SET)
#define GPIO_ReadInputDataBit(x,y) 		    HAL_GPIO_ReadPin (x,y)


/* NSS pin is SW controllable */
#define port_SPIx_set_chip_select()			HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_SET)
#define port_SPIx_clear_chip_select()		HAL_GPIO_WritePin(DW_NSS_GPIO_Port, DW_NSS_Pin, GPIO_PIN_RESET)


void Sleep(uint32_t Delay);
unsigned long portGetTickCnt(void);

int switch_is_on(uint16_t GPIOpin);

void port_wakeup_IC(void);
void port_wakeup_IC_fast(void);

void port_set_dw1000_slowrate(void);
void port_set_dw1000_fastrate(void);

void process_dwRSTn_irq(void);
void process_deca_irq(void);

//void led_on(led_t led);
//void led_off(led_t led);
//void led_toggle(led_t led);

//void motor_on(void);
//void motor_off(void);

int  peripherals_init(void);
void spi_peripheral_init(void);
void setup_DW1000RSTnIRQ(int enable);
void reset_DW1000(void);

ITStatus EXTI_GetITEnStatus(uint32_t x);

uint32_t port_GetEXT_IRQStatus(void);
uint32_t port_CheckEXT_IRQ(void);
void port_DisableEXT_IRQ(void);
void port_EnableEXT_IRQ(void);
extern uint32_t		HAL_GetTick(void);
HAL_StatusTypeDef 	flush_report_buff(void);


#ifdef __cplusplus
}
#endif


#endif 

