/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LORA_CARD_H
#define __LORA_CARD_H

#include "stdint.h"
#include "BSP_timer.h"
#include "LORA_Comm.h"

#include "RYDW_Def.h"
//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/

/******* 宏定义 *******/

/******* 返回状态定义 *******/


/******* 外部变量定义 *******/


/******* 外部函数定义 *******/

/* 定位卡广播和接收函数 */
LoraResult LORA_Card_BoardcastFence(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo, EFence_t FenceInfo);
LoraResult LORA_Card_BoardcastCardInfo(LoraPara_t *Para, LoraHeader_t Header, CardData_t *SelfCardInfo, CardData_t *OtherCardInfo[], uint8_t CardNum);
LoraResult LORA_Card_RecvBaseFence(LoraPara_t *Para, LoraHeader_t *Header, EFence_t *FenceInfo);
LoraResult LORA_Card_RecvBaseDiffInfo(LoraPara_t *Para, LoraHeader_t *Header, uint8_t *DiffInfo, uint16_t *DiffLen);
LoraResult LORA_Card_RecvBaseDiffByBlock(LoraPara_t *Para, LoraHeader_t *Header, uint8_t *DiffInfo, uint16_t *DiffLen, uint32_t Timeout);








#endif /* __LORA_CARD_H */
