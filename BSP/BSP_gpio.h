#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"


/******* GPIO管脚名称定义 *******/
/* SYS3.3 系统电源 */
#define LY_PWR_PORT			GPIOB
#define LY_PWR_PIN			GPIO_PIN_15

#define SYS_PWR_PORT		GPIOB
#define SYS_PWR_PIN			GPIO_PIN_14

#define Flash_PWR_PORT		GPIOB
#define Flash_PWR_PIN		GPIO_PIN_13



/* 按键管脚 */
#define KEY1INT_PORT		GPIOE
#define KEY1INT_PIN			GPIO_PIN_5
#define KEY2INT_PORT		GPIOE
#define KEY2INT_PIN			GPIO_PIN_0
#define OFFINT_PORT			GPIOE
#define OFFINT_PIN			GPIO_PIN_1
#define KEY1_UP_PORT		GPIOA
#define KEY1_UP_PIN			GPIO_PIN_11
#define KEY2_DOWN_PORT		GPIOE
#define KEY2_DOWN_PIN		GPIO_PIN_9
#define KEY3_OK_PORT		GPIOA
#define KEY3_OK_PIN			GPIO_PIN_12
#define KILL_OFF_PORT		GPIOE
#define KILL_OFF_PIN		GPIO_PIN_6

/* IMU中断 */
#define IMUINT1_PORT		GPIOD
#define IMUINT1_PIN			GPIO_PIN_6
#define IMUINT2_PORT		GPIOD
#define IMUINT2_PIN			GPIO_PIN_5

/* DTU电源控制 */
#define GSM_PWR_PORT		GPIOB
#define GSM_PWR_PIN			GPIO_PIN_12
#define GSM_RESET_PORT		GPIOC
#define GSM_RESET_PIN		GPIO_PIN_5



/* RTK控制 */
#define RTK_PWR_PORT		GPIOB
#define RTK_PWR_PIN			GPIO_PIN_0
#define RTK_RESET_PORT		GPIOE
#define RTK_RESET_PIN		GPIO_PIN_2
#define RTK_PPS_PORT		GPIOE
#define RTK_PPS_PIN			GPIO_PIN_3
#define RTK_LED_PORT		GPIOC
#define RTK_LED_PIN			GPIO_PIN_13

/* LoRa控制 */
#define LORA_PWR_PORT		GPIOB
#define LORA_PWR_PIN		GPIO_PIN_2
#define LORA_RESET_PORT		GPIOA
#define LORA_RESET_PIN		GPIO_PIN_1
#define LORA_BUSY_PORT		GPIOA
#define LORA_BUSY_PIN		GPIO_PIN_0
#define LORA_RXEN_PORT		GPIOC
#define LORA_RXEN_PIN		GPIO_PIN_3
#define LORA_TXEN_PORT		GPIOD
#define LORA_TXEN_PIN		GPIO_PIN_11
#define LORA_DIO1_PORT		GPIOC
#define LORA_DIO1_PIN		GPIO_PIN_6
#define LORA_LED_PORT		GPIOA
#define LORA_LED_PIN		GPIO_PIN_10

/* OLED显示屏控制 */
#define OLED_CS2_PORT		GPIOA
#define OLED_CS2_PIN		GPIO_PIN_9
#define OLED_RES_PORT		GPIOC
#define OLED_RES_PIN		GPIO_PIN_7
#define OLED_PWR_PORT		GPIOC
#define OLED_PWR_PIN		GPIO_PIN_8
#define OLED_DC_PORT		GPIOC
#define OLED_DC_PIN			GPIO_PIN_9

/* 蓝牙控制 */
#define BT_MODESW_PORT		GPIOD
#define BT_MODESW_PIN		GPIO_PIN_10
#define BT_LINK_PORT		GPIOD
#define BT_LINK_PIN			GPIO_PIN_9
#define BT_HOLDWAKE_PORT	GPIOD
#define BT_HOLDWAKE_PIN		GPIO_PIN_8

/* 语音控制管脚 */
#define VOICE_MOSI_PORT		GPIOD
#define VOICE_MOSI_PIN		GPIO_PIN_15
#define VOICE_MISO_PORT		GPIOD
#define VOICE_MISO_PIN		GPIO_PIN_14
#define VOICE_CLK_PORT		GPIOD
#define VOICE_CLK_PIN		GPIO_PIN_13
#define VOICE_NSS_PORT		GPIOD
#define VOICE_NSS_PIN		GPIO_PIN_12
#define VOICE_BUSY_PORT		GPIOE
#define VOICE_BUSY_PIN		GPIO_PIN_7
#define VOICE_PWR_PORT		GPIOE
#define VOICE_PWR_PIN		GPIO_PIN_8

/* 电池ADC采样 */
#define BAT_ADC_PORT		GPIOC
#define BAT_ADC_PIN			GPIO_PIN_2

#define AC12_ADC_PORT		GPIOC
#define AC12_ADC_PIN		GPIO_PIN_4

/*客服工具*/
#define TOOL_PWR_PORT		GPIOE
#define TOOL_PWR_PIN		GPIO_PIN_4
#define TOOL_INDET_PORT		GPIOA
#define TOOL_INDET_PIN		GPIO_PIN_8
/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
extern uint8_t g_CheckTool;

/******* 外部函数声明 *******/

void InitCelluarGpio(void);
void DeInitCelluarGpio(void);
void BSP_GpioInit(void);
void GPIO_SetupPin(GPIO_TypeDef *GPIOx, uint32_t Pin, uint32_t Mode, uint32_t Pull, uint32_t Speed, uint32_t Alter);

#endif

