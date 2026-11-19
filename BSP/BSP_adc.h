#ifndef __BSP_ADC_H
#define __BSP_ADC_H


/* 引用头文件 */
#include "Bsp.h"
//#include "stm32h7xx_hal.h"


/******* GPIO管脚名称定义 *******/

/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
extern ADC_HandleTypeDef		hADC1;			//ADC1句柄
extern ADC_HandleTypeDef   		hADC2;          //ADC2句柄

extern ADC_ChannelConfTypeDef	hADC_Ch3;		//ADC通道句柄
extern ADC_ChannelConfTypeDef   hADC_Ch13;		//ADC通道句柄
extern float g_fBatVal;
extern float g_MainVal;
extern uint32_t g_tmrAdcDelay;
/******* 外部函数声明 *******/
/* 初始化 */
void BSP_AdcInitPara_ADC1(ADC_HandleTypeDef *AdcHandler);
void BSP_AdcBindInstance(ADC_HandleTypeDef *AdcHandler, ADC_TypeDef *AdcInst);
void BSP_AdcInitChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *AdcChConf);


/* 通道配置 */
void BSP_AdcSetupChannelPara(ADC_ChannelConfTypeDef *AdcChConf, uint32_t Chnl, uint32_t Rank, uint32_t SmplTime, uint32_t InputMode, uint32_t OffsetNbr, uint32_t Offset);

/* 功能函数 */
uint16_t ADC_GetVal(ADC_HandleTypeDef *hadc);
uint32_t ADC_GetVal_Average(ADC_HandleTypeDef *hadc,uint16_t BufLen);
void ADC_Init(void);
void ADC_Acq(void);
#endif /* __BSP_ADC_H */

