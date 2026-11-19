#ifndef __BSP_LED_H
#define __BSP_LED_H


/* 引用头文件 */
#include "BSP.h"
//#include "stm32h7xx_hal.h"

#define LED_MAX		10

/******* GPIO管脚名称定义 *******/


/******* 指示灯类型定义 *******/
#define LEDT_OFF		0x00	//熄灭
#define LEDT_ON			0x01	//长亮
#define LEDT_FILP		0x02	//翻转
#define LEDT_BLINK_S	0x03	//慢闪
#define LEDT_BLINK_F	0x04	//快闪


/******* 数据结构体定义 *******/

/* 指示灯状态 */
typedef struct
{
	uint8_t	DispType:3;		//b0-b2:LED亮灯类型
	uint8_t Toogle:1;		//b3:LED翻转切换 0=不触发;1=触发
	uint8_t :3;				//b4-b6:预留
	uint8_t Power:1;		//b7:LED状态 0=灭;1=亮
} LEDState_t;

/* 指示灯参数 */
typedef struct
{
	GPIO_TypeDef	*GPIOx;		//LED对应端口
	uint16_t		Pinx;		//LED对应管脚
	LEDState_t		*Sta;		//LED状态
} LEDPara_t;

/* 指示灯组 */
typedef struct
{
	uint8_t			LedTotalNbr;
	LEDPara_t		LedPara[LED_MAX];
} LEDGroups_t;

/******* 外部变量声明 *******/
extern LEDGroups_t g_LEDGroup;	//指示灯组


/******* 外部函数声明 *******/
void RegisterLEDState(LEDGroups_t *ptrLED, GPIO_TypeDef *GPIOx, uint16_t Pin, LEDState_t *ptrSta);
void UnRegisterLEDState(LEDGroups_t *ptrLED, GPIO_TypeDef *GPIO, uint16_t Pin);


/* LED指示灯处理 放在中断里 */
void LED_ProcessInInterrupt(LEDGroups_t *ptrLED, uint32_t Tick);

#endif

