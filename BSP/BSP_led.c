#include "main.h"
#include "BSP_gpio.h"
#include "BSP_led.h"

/******** 定义 ********/
LEDGroups_t g_LEDGroup;		//指示灯组

/******************************************************************
函数名称: void InitLEDPara
功能描述: 初始化LED参数
参数输入: LEDPara_t	*ptrLEDPara	LED参数指针
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void InitLEDPara(LEDPara_t *ptrLEDPara)
{
	ptrLEDPara->GPIOx = NULL;
	ptrLEDPara->Pinx = 0;
	ptrLEDPara->Sta = NULL;
}

/******************************************************************
函数名称: void RegisterLEDState
功能描述: 注册指定LED状态变量到LED灯组中
参数输入: LEDGroups_t	*ptrLED	LED灯组变量指针
		  GPIO_TypeDef	*GPIOx	GPIO端口号
		  uint16_t		Pin		GPIO管脚号
		  LEDState_t	*ptrSta	LED状态变量指针
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void RegisterLEDState(LEDGroups_t *ptrLED, GPIO_TypeDef *GPIO, uint16_t Pin, LEDState_t *ptrSta)
{
	uint8_t ledpos;
	LEDPara_t para;
	
	ledpos = ptrLED->LedTotalNbr;
	if (ledpos >= LED_MAX)
	{
		return;
	}
	
	para.GPIOx = GPIO;
	para.Pinx = Pin;
	para.Sta = ptrSta;
	
	/* 初始化默认关灯 并重置状态 */
	para.Sta->DispType = 0x00;
	para.Sta->Power = 0;
	para.Sta->Toogle = 0;
	
	ptrLED->LedPara[ledpos] = para;
	ptrLED->LedTotalNbr = ledpos + 1;
	
}

/******************************************************************
函数名称: void UnRegisterLEDState
功能描述: 反注册LED灯组中的LED状态变量
参数输入: LEDGroups_t	*ptrLED	LED灯组变量指针
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void UnRegisterLEDState(LEDGroups_t *ptrLED, GPIO_TypeDef *GPIO, uint16_t Pin)
{
	uint8_t i, j;
	uint8_t lednbr;
	
	lednbr = ptrLED->LedTotalNbr;
	
	if ((GPIO == NULL) && (Pin == 0))
	{
		for (i=0; i<lednbr; i++)
		{
			/* 反初始化默认关灯 但不重置状态 */
			HAL_GPIO_WritePin(ptrLED->LedPara[i].GPIOx, ptrLED->LedPara[i].Pinx, GPIO_PIN_SET);
			InitLEDPara(&(ptrLED->LedPara[i]));
		}
		ptrLED->LedTotalNbr = 0;
		return;
	}
	
	for (i=0; i<lednbr; i++)
	{
		if ((ptrLED->LedPara[i].GPIOx == GPIO) && (ptrLED->LedPara[i].Pinx == Pin))
		{
			HAL_GPIO_WritePin(ptrLED->LedPara[i].GPIOx, ptrLED->LedPara[i].Pinx, GPIO_PIN_SET);
			for (j=i; j<(lednbr - 1); j++)
			{
				ptrLED->LedPara[j] = ptrLED->LedPara[j + 1];
			}
			InitLEDPara(&(ptrLED->LedPara[lednbr - 1]));
			ptrLED->LedTotalNbr = lednbr - 1;
			break;
		}
	}
}

/******************************************************************
函数名称: void LED_SetON
功能描述: 点亮LED
参数输入: LEDPara_t	*LedPara	LED灯参数
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void LED_SetON(LEDPara_t *LedPara)
{
	HAL_GPIO_WritePin(LedPara->GPIOx, LedPara->Pinx, GPIO_PIN_RESET);
	LedPara->Sta->Power = 1;
}

/******************************************************************
函数名称: void LED_SetOFF
功能描述: 熄灭LED
参数输入: LEDPara_t	*LedPara	LED灯参数
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void LED_SetOFF(LEDPara_t *LedPara)
{
	HAL_GPIO_WritePin(LedPara->GPIOx, LedPara->Pinx, GPIO_PIN_SET);
	LedPara->Sta->Power = 0;
}

/******************************************************************
函数名称: void LED_SetFilp
功能描述: 翻转LED
参数输入: LEDPara_t	*LedPara	LED灯参数
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void LED_SetFilp(LEDPara_t *LedPara)
{
	if (LedPara->Sta->Power == 0)
	{
		HAL_GPIO_WritePin(LedPara->GPIOx, LedPara->Pinx, GPIO_PIN_RESET);
		LedPara->Sta->Power = 1;
	}
	else if (LedPara->Sta->Power == 1)
	{
		HAL_GPIO_WritePin(LedPara->GPIOx, LedPara->Pinx, GPIO_PIN_SET);
		LedPara->Sta->Power = 0;
	}
}

/******************************************************************
函数名称: void LED_ProcessInInterrupt
功能描述: LED点亮中断处理
参数输入: LEDGroups_t	*ptrLED	LED灯组
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
/*****************************************************************
LED低三位表示显示状态
LED第四位表示翻转触发状态，执行翻转后会自己清除

******************************************************************/
void LED_ProcessInInterrupt(LEDGroups_t *ptrLED, uint32_t Tick)
{
	uint8_t i;
	uint8_t lednbr;
	uint8_t interval;
	
	lednbr = ptrLED->LedTotalNbr;
	for (i=0; i<lednbr; i++)
	{
		/* 判断已注册的指示灯 */
		if (ptrLED->LedPara[i].Sta != NULL)
		{
			/* 根据LED类型处理 */
			switch (ptrLED->LedPara[i].Sta->DispType)
			{
				/* 熄灭 */
				case LEDT_OFF:
					LED_SetOFF(&(ptrLED->LedPara[i]));
					break;
				/* 长亮 */
				case LEDT_ON:
					LED_SetON(&(ptrLED->LedPara[i]));
					break;
				/* 翻转 */
				case LEDT_FILP:
					if (ptrLED->LedPara[i].Sta->Toogle == 1)
					{
						LED_SetFilp(&(ptrLED->LedPara[i]));
						ptrLED->LedPara[i].Sta->Toogle = 0;
					}
					break;
				/* 快慢闪 */
				case LEDT_BLINK_S:
				case LEDT_BLINK_F:
					if (ptrLED->LedPara[i].Sta->DispType == LEDT_BLINK_F) interval = 5;
					if (ptrLED->LedPara[i].Sta->DispType == LEDT_BLINK_S) interval = 15;
					if ((Tick % interval) == 0)
					{
						LED_SetFilp(&(ptrLED->LedPara[i]));
					}
					break;
				/* 其他情况 */
				default:
					break;
			}
		}
	}
}
