/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RYDW_INIT_H
#define __RYDW_INIT_H


#include "stdint.h"

//#include "LORA_Comm.h"
//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/

/******* 宏定义 *******/

/******* 外部变量定义 *******/
extern uint8_t g_OledContrast;
extern uint8_t g_OpenOLEDPower;
extern uint32_t g_OLEDPowerOffDelay;
extern uint8_t g_InitSuccflag;   //初始化成功标记
extern uint8_t  g_NeedInitDTU;
extern uint8_t  g_NeedInitRTK;
extern uint32_t g_NeedInitRtkDelay;
extern uint32_t g_NeedInitDTUDelay;
extern uint32_t g_ControlMsgDelay;
extern uint8_t  g_DataNetOk;
extern uint8_t  g_CorsNetOk;
/******* 外部函数定义 *******/
void RYDW_OLEDInit(void);
void RYDW_LoraInit(void);
void RYDW_RtkInit(void);
void RYDW_DtuInit(void);
void RYDW_BleInit(void);
void RYDW_IMUInit(void);
void RYDW_Init(void);
void RtkMode_Init(void);

//void SX126x_ChangeRadioPara(RadioEvents_t *Event, uint8_t Mode);



#endif /* __RYDW_INIT_H */
