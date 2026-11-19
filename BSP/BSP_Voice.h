#ifndef __BSP_VOICE_H
#define __BSP_VOICE_H



#include "stdint.h"
#include "stdbool.h"
#include "BSP.h"

extern uint8_t DATA_SONG[512]; //¥Ê∑≈“Ù∆µ ˝æ›

#define VOICE_CS(n)		    (n?HAL_GPIO_WritePin(VOICE_NSS_PORT,VOICE_NSS_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(VOICE_NSS_PORT,VOICE_NSS_PIN,GPIO_PIN_RESET))
#define VOICE_SI(n)		    (n?HAL_GPIO_WritePin(VOICE_MOSI_PORT,VOICE_MOSI_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(VOICE_MOSI_PORT,VOICE_MOSI_PIN,GPIO_PIN_RESET))
#define VOICE_SCK(n)	    (n?HAL_GPIO_WritePin(VOICE_CLK_PORT,VOICE_CLK_PIN,GPIO_PIN_SET):HAL_GPIO_WritePin(VOICE_CLK_PORT,VOICE_CLK_PIN,GPIO_PIN_RESET))

#define VOICE_SO()	        HAL_GPIO_ReadPin(VOICE_MISO_PORT,VOICE_MISO_PIN)

bool DATA_CHANGE();
void Get512Byte(uint16_t num);
void WT588E_PLAY(uint8_t DDATA);
void JudgeUpdateVoice(void);
#endif

