#ifndef __WT588F02B_H
#define __WT588F02B_H

#include "stdint.h"
#include "stdbool.h"

#include "BSP.h"
/*
*/
#define CLK_2A(n)		(n ? HAL_GPIO_WritePin(VOICE_CLK_PORT, VOICE_CLK_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(VOICE_CLK_PORT, VOICE_CLK_PIN, GPIO_PIN_RESET))
#define DATA_2A(n)		(n ? HAL_GPIO_WritePin(VOICE_SDA_PORT, VOICE_SDA_PIN, GPIO_PIN_SET) : HAL_GPIO_WritePin(VOICE_SDA_PORT, VOICE_SDA_PIN, GPIO_PIN_RESET))

 
#define   KJZSBZC         1           /*开机中，设备正常*/
#define   KJZSBYC         2           /*开机中，设备异常*/
#define   KJZSBDLD        3           /*开机中，设备电量低，请及时充电*/
#define   ZJZSBZC         4           /*自检中，设备正常*/
#define   ZJZSBYC         5           /*自检中，设备异常*/
#define   ZJZSBDLD        6           /*自检中，设备电量低，请及时充电*/
#define   ZACD            7           /*正在充电*/
#define   LCXD            8           /*来车下道*/
#define   QXD             9           /*请下道*/
#define   WXQQZY         10           /*进入危险区，请注意*/
#define   CFWSG          11           /*超范围施工*/
#define   DLD            12           /*设备电量低，请及时充电*/
#define   SBZC           13           /*设备正常*/



void Line_2A_WT588F(uint8_t DDATA);


#endif



