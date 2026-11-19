#include "BSP.h"

uint8_t BeepEnable;
uint32_t g_BeepTimer;
uint32_t BeepFrelev;

uint8_t g_WatchDogStatus;
volatile uint32_t wdt_counter;

IWDG_HandleTypeDef hIWDG;		//独立看门狗句柄

/**
 * @brief	初始化独立看门狗函数
 *			时间计算(大概):Tout=((2^prer)*rlr)/32 (ms)
 *
 * @param   prer	分频数:IWDG_PRESCALER_4~IWDG_PRESCALER_256
 * @param   rlr		自动重装载值,0~0XFFF
 *
 * @return  void
 */
void IWDG_Init(uint8_t prer, uint16_t rlr)
{
    hIWDG.Instance = IWDG;
    hIWDG.Init.Prescaler = prer;				//设置IWDG分频系数
    hIWDG.Init.Reload = rlr;					//重装载值
    hIWDG.Init.Window = IWDG_WINDOW_DISABLE;	//关闭窗口模式
    HAL_IWDG_Init(&hIWDG);						//初始化IWDG并开启独立看门狗
	
	BeepEnable = false;
	BeepFrelev = 2000;	
	
/*********************调试模式下禁用独立看门狗IWDG**********************************/
//	__HAL_RCC_DBGMCU_CLK_ENABLE();  //打开时钟
//	__HAL_DBGMCU_FREEZE_IWDG();	  //调试模式下，冻结看门狗计数器时钟
//    __HAL_RCC_DBGMCU_CLK_DISABLE();   //关闭时钟	
}



