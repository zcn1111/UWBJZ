#ifndef __BSP_WATCHDOG_H
#define __BSP_WATCHDOG_H

#include "stdint.h"
#include "stdbool.h"
#include "bsp.h"

#define BEEP(n)	(n?HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_RESET))
//#define BEEP_TogglePin	HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_2)


#define WDEN		0x00000001
#define WDRESET		0x00000002
#define WDTOF		0x00000004
#define WDINT		0x00000008

#define WDT_FEED_VALUE	0x186a00

#define WDT_PORT 1
#define WDT 14


extern uint32_t g_BeepTimer;
extern uint8_t BeepEnable;
extern uint32_t BeepFrelev;
void WatchDogInit(void);
void WDTInFeed( void );
void WDTExFeed(void);

void BeepInit(void);
void BeepStatus(uint8_t Enable);

void IWDG_Init(uint8_t prer, uint16_t rlr);




#endif


