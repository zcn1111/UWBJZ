#ifndef __DTU_UPDATE_H
#define __DTU_UPDATE_H

/* 引用头文件 */
#include "stm32l4xx.h"

#include "g_GlobalValDef.h"
#include "BSP_timer.h"
#include "BSP_gpio.h"

/******* 宏定义 *******/

/******* 数据结构定义 *******/



/******* 外部变量声明 *******/


/******* 外部函数声明 *******/
int16_t DTU_RecvUpdateCmd(void);
int16_t DTU_RecvUpdateData(void);

int16_t DTU_SendUpdateRequest(void);
int16_t DTU_SendUpdateFinish(void);


#endif /* __DTU_UPDATE_H */

