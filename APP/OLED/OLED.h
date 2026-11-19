#ifndef __OLED_H
#define __OLED_H
#include "stdint.h"

extern uint8_t g_DispCardOrder;

void LedPowerONInit(void);
void LedPowerOFFInit(void);
void KEYProcess(void);
void LedStatus(void) ;

#endif