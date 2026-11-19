#include "main.h"
#include "BSP.h"
#include "String.h"

#include "g_func.h"

#include "DTU_Process.h"
//#include "BLE_Boardcast.h"

/******** 定时器句柄变量 ********/
TIM_HandleTypeDef hTIM7;
TIM_HandleTypeDef hTIM6;
TIM_HandleTypeDef hTIM2;
TIM_HandleTypeDef hTIM3;
TIM_HandleTypeDef hTIM16;

static uint32_t fac_us;

/******** 滴答计时器 ********/
volatile uint32_t g_Tick10ms=0;//计时间隔为10ms
volatile uint32_t g_Tick1s=0;	//计时间隔为1s

volatile uint32_t g_Timeout1ms=0;	

/******** 系统时钟 ********/
HexDateTime_t	g_SysTime;
HexDateTime_t	g_BCDSysTime;
HexDateTime_t 	g_BCDTime;

/******************************************************************
函数名称: void TIM7Init
功能描述: 定时器7初始化
参数输入: uint16_t	arr	自动装载值
		  uint16_t	psc	分频系数
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void TIM7Init(uint16_t arr, uint16_t psc)
{
	__HAL_RCC_TIM7_CLK_ENABLE();	//使能TIM7时钟
	
	hTIM7.Instance = TIM7;
	hTIM7.Init.Prescaler = psc - 1;						//预分频系数
	hTIM7.Init.CounterMode = TIM_COUNTERMODE_UP;		//向上计数
	hTIM7.Init.Period = arr - 1;						//自动装载
	hTIM7.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;	//时钟分频因子
	
	HAL_TIM_Base_Init(&hTIM7);	//初始化
	HAL_NVIC_SetPriority(TIM7_IRQn, 6, 0);	//配置中断优先级 6/0
	
	HAL_NVIC_EnableIRQ(TIM7_IRQn);	//开启TIM7中断
	HAL_TIM_Base_Start_IT(&hTIM7);	//启动定时器(中断方式)
}

/******************************************************************
函数名称: void TIM6Init
功能描述: 定时器6初始化
参数输入: uint16_t	arr		自动装载值
		  uint16_t	psc		分频系数
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void TIM6Init(uint16_t arr, uint16_t psc)
{
	__HAL_RCC_TIM6_CLK_ENABLE();	//使能TIM6时钟
	
	hTIM6.Instance = TIM6;
	hTIM6.Init.Prescaler = psc - 1;						//预分频系数
	hTIM6.Init.CounterMode = TIM_COUNTERMODE_UP;		//向上计数
	hTIM6.Init.Period = arr - 1;						//自动装载
	hTIM6.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;	//时钟分频因子
	
	HAL_TIM_Base_Init(&hTIM6);	//初始化
	HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 6, 0);		//配置中断优先级 6/0
	
	HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);	//开启TIM6中断
	HAL_TIM_Base_Start_IT(&hTIM6);		//启动定时器(中断方式)
}

/******************************************************************
函数名称: void TIM2Init
功能描述: 定时器2初始化 固定配置为1us 不启动中断
参数输入: 
参数输出: 无
返 回 值: 
其    他: 用于NAND读写延时
******************************************************************/
void TIM2Init(void)
{
	__HAL_RCC_TIM2_CLK_ENABLE();	//使能TIM2时钟
	
	hTIM2.Instance = TIM2;
	hTIM2.Init.Prescaler = 720 - 1;						//预分频系数
	hTIM2.Init.CounterMode = TIM_COUNTERMODE_UP;		//向上计数
	hTIM2.Init.Period = 0xFFFFFFFF;						//自动装载
	hTIM2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;	//时钟分频因子
	
	HAL_TIM_Base_Init(&hTIM2);		//初始化
	HAL_TIM_Base_Start(&hTIM2);		//启动定时器
}

/******************************************************************
函数名称: void TIM3Init
功能描述: 定时器3初始化
参数输入: uint16_t	arr		自动装载值
		  uint16_t	psc		分频系数
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void TIM3Init(uint16_t arr, uint16_t psc)
{
	__HAL_RCC_TIM3_CLK_ENABLE();	//使能TIM3时钟
	
	hTIM3.Instance = TIM3;
	hTIM3.Init.Prescaler = psc - 1;						//预分频系数
	hTIM3.Init.CounterMode = TIM_COUNTERMODE_UP;		//向上计数
	hTIM3.Init.Period = arr - 1;						//自动装载
	hTIM3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;	//时钟分频因子
	
	HAL_TIM_Base_Init(&hTIM3);	//初始化
	HAL_NVIC_SetPriority(TIM3_IRQn, 6, 0);		//配置中断优先级 6/0
	
	HAL_NVIC_EnableIRQ(TIM3_IRQn);	//开启TIM3中断
	HAL_TIM_Base_Start_IT(&hTIM3);	//启动定时器(中断方式)
}


/******************************************************************
函数名称: static void TickInc10ms
功能描述: 滴答自增10ms
参数输入: 
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
static void TickInc10ms(void)
{
	g_Tick10ms++;
}

/******************************************************************
函数名称: void TickInc1s
功能描述: 滴答自增1s
参数输入: 
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
static void TickInc1s(void)
{
	g_Tick1s++;
}

#if defined(FUNC_DELAY)
/**
 * @brief	初始化延迟函数,SYSTICK的时钟固定为AHB时钟
 * @param   SYSCLK	系统时钟频率
 * @return  void
 */
void delay_init(uint8_t SYSCLK)
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);//SysTick频率为HCLK
    fac_us = SYSCLK;						
}

/**
 * @brief	延时微秒(us)函数
 * @remark	nus:0~190887435(最大值即2^32/fac_us@fac_us=22.5)
 * @param   nus		需要延时多少微秒
 * @return  void
 */
void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;			//LOAD的值
    ticks = nus * fac_us; 						//需要的节拍数
    told = SysTick->VAL;        				//刚进入时的计数器值

    while(1)
    {
        tnow = SysTick->VAL;

        if(tnow != told)
        {
            if(tnow < told)
				tcnt += told - tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
            else 
				tcnt += reload - tnow + told;
			
            told = tnow;
            if(tcnt >= ticks)
				break;			//时间超过/等于要延迟的时间,则退出.
        }
    }
}

/**
 * @brief	延时毫秒(ms)函数
 * @param   nms		需要延时多少毫秒
 * @return  void
 */
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for(i = 0; i < nms; i++)
	{
		delay_us(1000);
	}	

}

#endif
/******************************************************************
函数名称: void InitTickTimerPara
功能描述: 初始化滴答定时器参数
参数输入: 
参数输出: TickTimerPara_t *Para			定时器参数
返 回 值: 
其    他: 
******************************************************************/
void InitTickTimerPara(TickTimerPara_t *Para, volatile uint32_t *BaseTick)
{
	Para->TimerFlag.Actived = 0;
	Para->TimerFlag.AutoRestart = 0;
	Para->TimerFlag.TimeOutFlag = 0;
	Para->TimerFlag.ReStartMaxFlag = 0;
	
	Para->RestartCnt = 0;
	Para->RestartMax = 0;
	
	Para->TimerProg = TICKTIMER_STOPPED;
	Para->cbTimeoutFunc = NULL;
	Para->cbOverMaxFunc = NULL;
	
	Para->BaseTick = BaseTick;
}

/******************************************************************
函数名称: void SetupTickTimerPara
功能描述: 配置滴答定时器参数
参数输入: uint32_t	DelayTick			定时滴答时间
		  uint8_t	RestartMax			最大重开始次数
		  void		(*cbCallback)(void)	定时器到时间回调函数
参数输出: TickTimerPara_t *Para			定时器参数
返 回 值: 
其    他: 
******************************************************************/
void SetupTickTimerPara(uint32_t DelayTick, uint8_t AutoRestart, uint8_t RestartMax, void (*cbTimeoutFunc)(void), void (*cbOvermaxFunc)(void), TickTimerPara_t *Para)
{
	/* 设置参数 */
	Para->DelayTick = DelayTick;
	Para->cbTimeoutFunc = cbTimeoutFunc;
	Para->cbOverMaxFunc = cbOvermaxFunc;
	
	/* 计时器停止状态配置参数 */
	if (Para->TimerProg == TICKTIMER_STOPPED)
	{
		Para->TimerFlag.Actived = 0;
		Para->StartTickVal = *(Para->BaseTick);
		if (AutoRestart == TTM_MODE_RESTART)
		{
			Para->TimerFlag.AutoRestart = 1;
			Para->RestartMax = RestartMax;
		}
		else if (AutoRestart == TTM_MODE_SINGLE)
		{
			Para->TimerFlag.AutoRestart = 0;
			Para->RestartMax = 0;
		}
		Para->RestartCnt = 0;
	}
}

/******************************************************************
函数名称: void StartTickTimer
功能描述: 启动定时器
参数输入: TickTimerPara_t *Para			定时器参数
参数输出: TickTimerPara_t *Para			定时器参数
返 回 值: 
其    他: 
******************************************************************/
void StartTickTimer(TickTimerPara_t *Para)
{
	/* 当前不是运行状态 重新启动定时器 */
	if (Para->TimerProg != TICKTIMER_RUNNNING)
	{
		Para->StartTickVal = *(Para->BaseTick);
		Para->RestartCnt = 0;
		Para->TimerFlag.TimeOutFlag = 0;
		Para->TimerFlag.ReStartMaxFlag = 0;
	}
	Para->TimerFlag.Actived = 1;
	Para->TimerProg = TICKTIMER_RUNNNING;
}

/******************************************************************
函数名称: void ResetTickTimer
功能描述: 启动定时器
参数输入: TickTimerPara_t *Para			定时器参数
参数输出: TickTimerPara_t *Para			定时器参数
返 回 值: 
其    他: 
******************************************************************/
void ResetTickTimer(TickTimerPara_t *Para)
{
	Para->StartTickVal = *(Para->BaseTick);
	Para->RestartCnt = 0;
	Para->TimerFlag.Actived = 1;
	/* 当前不是运行状态 重置标记 */
	if (Para->TimerProg != TICKTIMER_RUNNNING)
	{
		Para->TimerFlag.TimeOutFlag = 0;
		Para->TimerFlag.ReStartMaxFlag = 0;
	}
	/* 当前是 超时状态 或 超过次数状态 重置后切换为停止状态 */
	if ((Para->TimerProg == TICKTIMER_TIMEOUT) || (Para->TimerProg == TICKTIMER_OVERMAX))
	{
		Para->TimerProg = TICKTIMER_STOPPED;
	}
}

/******************************************************************
函数名称: void StopTickTimer
功能描述: 停止定时器
参数输入: TickTimerPara_t *Para			定时器参数
参数输出: TickTimerPara_t *Para			定时器参数
返 回 值: 
其    他: 
******************************************************************/
void StopTickTimer(TickTimerPara_t *Para)
{
	Para->StartTickVal = *(Para->BaseTick);
	Para->RestartCnt = 0;
	Para->TimerFlag.TimeOutFlag = 0;
	Para->TimerFlag.ReStartMaxFlag = 0;
	Para->TimerProg = TICKTIMER_STOPPED;
}

/******************************************************************
函数名称: int16_t TickTimerProcess
功能描述: 滴答定时器处理
参数输入: TickTimerPara_t *Para		定时器参数
参数输出: TickTimerPara_t *Para		定时器参数
返 回 值: 
其    他: 1. 参数中的超时标记 如果执行回调函数后会自动清除 否则需要手动清除
		  2. 回调函数不要太长
		  3. 最长定时时间
******************************************************************/
void TickTimerProcess(TickTimerPara_t *TmrPara)
{
	uint32_t now;
	uint32_t st;
	uint32_t diff;
	uint8_t ret=0;
	
	/* 非活动的定时器 */
	if (TmrPara->TimerFlag.Actived == 0)
	{
		return;
	}
	
	/* 根据定时器当前状态处理 */
	switch (TmrPara->TimerProg)
	{
		/* 当前状态 = 停止 */
		case TICKTIMER_STOPPED:
			TmrPara->RestartCnt = 0;
			TmrPara->StartTickVal = *(TmrPara->BaseTick);	//初始化起始
			TmrPara->TimerFlag.ReStartMaxFlag = 0;
			TmrPara->TimerFlag.TimeOutFlag = 0;
			TmrPara->TimerProg = TICKTIMER_STOPPED;
			break;
		/* 当前状态 = 运行中 */
		case TICKTIMER_RUNNNING:
			st = TmrPara->StartTickVal;
			now = *(TmrPara->BaseTick);
			if (st <= now)
			{
				diff = now - st;
			}
			else
			{
				diff = 0xFFFFFFFF - st + now;
			}
			
			/* 计算超时 */
			if (diff >= TmrPara->DelayTick)
			{
				TmrPara->TimerFlag.TimeOutFlag = 1;			//超时标记
				if (TmrPara->TimerFlag.AutoRestart == 1)	//自动重开始
				{
					TmrPara->RestartCnt++;
					TmrPara->TimerFlag.Actived = 1;
					TmrPara->StartTickVal = *(TmrPara->BaseTick);
					/* 重试次数等于0表示无限重启动 */
					if (TmrPara->RestartMax > 0)
					{
						if (TmrPara->RestartCnt > TmrPara->RestartMax)
						{
							TmrPara->TimerFlag.ReStartMaxFlag = 1;
							TmrPara->TimerFlag.TimeOutFlag = 1;
							TmrPara->TimerProg = TICKTIMER_OVERMAX;
						}
					}
				}
				else
				{
					TmrPara->TimerProg = TICKTIMER_TIMEOUT;
				}
			}
			break;
		/* 当前状态 = 定时器超时 */
		case TICKTIMER_TIMEOUT:
			TmrPara->TimerFlag.Actived = 0;				//单次执行的定时器 到时间后休眠
			if (TmrPara->cbTimeoutFunc != NULL)
			{
				TmrPara->cbTimeoutFunc();				//有回调函数执行回调函数
				TmrPara->TimerFlag.TimeOutFlag = 0;		//执行回调函数后清除超时标记
			}
			break;
		/* 当前状态 = 定时器超过最大重启次数 */
		case TICKTIMER_OVERMAX:
			TmrPara->TimerFlag.Actived = 0;				//执行后休眠
			if (TmrPara->cbOverMaxFunc != NULL)
			{
				TmrPara->cbOverMaxFunc();
				TmrPara->TimerFlag.TimeOutFlag = 0;
				TmrPara->TimerFlag.ReStartMaxFlag = 0;	//执行回调函数后清除标记
			}
			break;
		default:
			break;
	}
}

/******************************************************************
函数名称: void InitTickTimerListGroup
功能描述: 初始化定时器参数组
参数输入: TickTimerPara_t	*TmrPara	定时器变量参数
		  TickTimerGroups_t	*TmrList	定时器组
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void InitTickTimerListGroup(TickTimerGroups_t *TmrList)
{
	uint8_t i;
	
	for (i=0; i<TICKTIMER_LIST_MAX; i++)
	{
		TmrList->TimerList[i] = NULL;
	}
	TmrList->ListNbr = 0;
}

/******************************************************************
函数名称: void RegisterTickTimer
功能描述: 注册定时器参数到参数组中
参数输入: TickTimerPara_t	*TmrPara	定时器变量参数
		  TickTimerGroups_t	*TmrList	定时器组
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void RegisterTickTimer(TickTimerPara_t *TmrPara, TickTimerGroups_t *TmrList)
{
	uint8_t tmrpos;
	LEDPara_t para;
	
	tmrpos = TmrList->ListNbr;
	if (tmrpos >= TICKTIMER_LIST_MAX)
	{
		return;
	}
	
	TmrList->TimerList[tmrpos] = TmrPara;
	TmrList->ListNbr = tmrpos + 1;
}

/******************************************************************
函数名称: void UnRegisterTickTimer
功能描述: 反注册定时器
参数输入: LEDGroups_t	*ptrLED	LED灯组变量指针
参数输出: 无
返 回 值: 无
其    他: 
******************************************************************/
void UnRegisterTickTimer(TickTimerPara_t *TmrPara, TickTimerGroups_t *TmrList)
{
	uint8_t i, j;
	uint8_t tmrnbr;
	
	tmrnbr = TmrList->ListNbr;
	
	/* 反注册所有列表 */
	if (TmrPara == NULL)
	{
		InitTickTimerListGroup(TmrList);
	}
	
	/* 查找指定定时器参数指针 */
	for (i=0; i<tmrnbr; i++)
	{
		if (TmrList->TimerList[i] == TmrPara)
		{
			for (j=i; j<(tmrnbr - 1); j++)
			{
				TmrList->TimerList[j] = TmrList->TimerList[j + 1];
			}
			TmrList->TimerList[tmrnbr-1] = NULL;
			TmrList->ListNbr = tmrnbr - 1;
		}
	}
}

/******************************************************************
函数名称: void TickTimerGroupsProcess
功能描述: 滴答定时器组处理
参数输入: TickTimerPara_t *Para		定时器参数
参数输出: TickTimerPara_t *Para		定时器参数
返 回 值: 
其    他: 1. 参数中的超时标记 如果执行回调函数后会自动清除 否则需要手动清除
		  2. 回调函数不要太长
		  3. 最长定时时间
******************************************************************/
void TickTimerGroupsProcess(TickTimerGroups_t *TmrGrpPara)
{
	uint8_t i;
	uint8_t listnbr;
	
	listnbr = TmrGrpPara->ListNbr;
	for (i=0; i<listnbr; i++)
	{
		if (TmrGrpPara->TimerList[i]->TimerFlag.Actived == 1)
		{
			TickTimerProcess(TmrGrpPara->TimerList[i]);
		}
	}
}


/******************************************************************
函数名称: void TickDelay10ms
功能描述: 延时10ms
参数输入: uint32_t	DelayTime		定时时间(10ms)
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void TickDelay10ms(uint32_t DelayTime)
{
	uint32_t sttick;
	uint32_t diff;
	
	diff = 0;
	sttick = g_Tick10ms;
	while (DelayTime != 0)
	{
		if (g_Tick10ms >= sttick)
		{
			diff = g_Tick10ms - sttick;
		}
		else
		{
			diff = 0xFFFFFFFF - sttick + g_Tick10ms;
		}
		if (diff >= DelayTime)
			break;
	}
	
	diff = 0;
}

/******************************************************************
函数名称: void Delay1us
功能描述: 延时1us TIM2延时
参数输入: uint32_t	DelayTime		定时时间(1us)
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void Delay1us(uint32_t DelayTime)
{
	uint32_t sttick;
	uint32_t diff;
	
	diff = 0;
	sttick = TIM2->CNT;
	while (DelayTime != 0)
	{
		if (TIM2->CNT >= sttick)
		{
			diff = TIM2->CNT - sttick;
		}
		else
		{
			diff = 0xFFFFFFFF - sttick + TIM2->CNT;
		}
		if (diff >= DelayTime)
			break;
	}
	
}

/******************************************************************
函数名称: void Delay1ms
功能描述: 延时1ms TIM2延时
参数输入: uint32_t	DelayTime		定时时间(1ms)
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void Delay1ms(uint32_t DelayTime)
{
	uint32_t i;
	
	for (i=0; i<DelayTime; i++)
	{
		Delay1us(1000);
	}
}

/******************************************************************
函数名称: bool JudgeSomeDelayInProOk
功能描述: 判断延时
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
bool JudgeSomeDelayInProOk(uint32_t Timerval, uint32_t DelayN, uint32_t BaseTick)
{
	bool ret;
	uint32_t xx,tt;

	ret = false;
	xx = BaseTick;
	
	if(Timerval <= xx)	
		tt = xx - Timerval;	
	else
		tt = 0xffffffff - Timerval + xx;
	if( tt  > DelayN )
	{
		ret = true;		
	}		
	return ret;
}

/******************************************************************
函数名称: uint8_t IsLeapYear(uint8_t Year)
功能描述: 计算闰年
参数输入: 
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
uint8_t IsLeapYear(uint16_t Year)
{
	uint8_t ret;
	
	ret = 0;
	if (Year % 100 == 0)
	{
		if (Year % 400 == 0) ret = 1;
	}
	else
	{
		if (Year % 4 == 0) ret = 1;
	}
	return ret;
}

/******************************************************************
函数名称: void CalcSysTimeBySoftware
功能描述: 用软件计算系统时钟
参数输入: 
参数输出: 
返 回 值: 
其    他: 放在1s定时器中断里
******************************************************************/
void CalcSysTimeBySoftware(HexDateTime_t *Time)
{
	uint8_t monthday[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	
	/* 计算闰年 */
	if (IsLeapYear(Time->Year + 2000))
	{
		monthday[1] = 29;
	}
	/* 计算时间 */
	if (Time->Second > 59)
	{
		Time->Second -= 60;
		Time->Minute++;
	}
	if (Time->Minute > 59)
	{
		Time->Minute -= 60;
		Time->Hour++;
	}
	if (Time->Hour > 23)
	{
		Time->Hour = 0;
		Time->Day++;
	}
	if (Time->Day > monthday[Time->Month-1])
	{
		Time->Day = 1;
		Time->Month++;
	}
	if (Time->Month > 12)
	{
		Time->Month = 1;
		Time->Year++;
	}
}

/******************************************************************
函数名称: uint8_t AdjustSysTime
功能描述: 系统时间校时
参数输入: 
参数输出: 
返 回 值: uint8_t	校时成功标记
其    他: 
******************************************************************/
uint8_t AdjustSysTime(HexDateTime_t SrcTime, HexDateTime_t *SysTime)
{
	uint8_t ret=0;
	uint8_t monthday[12]={31,28,31,30,31,30,31,31,30,31,30,31};
	
	uint32_t src_in_sec;
	uint32_t now_in_sec;
	
	char strtime[32];

	ret = ADJTIME_NONE;
	/* 检查源数据有效性 */
	if (SrcTime.Year < 99)
	{
		if ((SrcTime.Month > 0) && (SrcTime.Month <= 12))
		{
			if ((SrcTime.Day > 0) && (SrcTime.Day <= monthday[SrcTime.Month-1]))
			{
				if ((SrcTime.Hour <= 23) && (SrcTime.Minute <= 59) && (SrcTime.Second <= 59))
				{
					/* 当前有时间 判断时间差超过5s校时 */
					if ((SysTime->Month != 0x00) && (SysTime->Day != 0x00))
					{
						src_in_sec = HexTime2ElapsedSecond(SrcTime);
						now_in_sec = HexTime2ElapsedSecond(*SysTime);
						if (abs(((int)src_in_sec - (int)now_in_sec)) >= 5)
						{
							*SysTime = SrcTime;
							sprintf(strtime, "20%02d/%02d/%02d %02d:%02d:%02d", SysTime->Year, SysTime->Month, SysTime->Day, SysTime->Hour, SysTime->Minute, SysTime->Second);
							debug_msg("Adjust Time: %s\r\n", strtime);
							ret = ADJTIME_SUCCESS;
						}
						else
						{
							ret = ADJTIME_HADTIME;
						}
					}
					else
					{
						*SysTime = SrcTime;
						sprintf(strtime, "20%02d/%02d/%02d %02d:%02d:%02d", SysTime->Year, SysTime->Month, SysTime->Day, SysTime->Hour, SysTime->Minute, SysTime->Second);
						debug_msg("Adjust Time: %s\r\n", strtime);
						ret = ADJTIME_SUCCESS;
					}
				}
			}
		}
	}
	
	return ret;
}

/******************************************************************
函数名称: uint32_t ElapsedTimeInSecond
功能描述: 已经经过时间
参数输入: 
参数输出: 
返 回 值: uint32_t	已经过时间(秒)
其    他: 
******************************************************************/
uint32_t ElapsedTimeInSecond(HexDateTime_t StTime, HexDateTime_t EdTime)
{
	uint32_t stsec, edsec;
	
	stsec = HexTime2ElapsedSecond(StTime);
	edsec = HexTime2ElapsedSecond(EdTime);
	return (edsec - stsec);
}

bool JudgeMilSecondDelayInPro(uint32_t Timerval, uint32_t DelayN)
{
	bool ret;
	uint32_t xx,tt;

	ret = false;
	xx = uwTick;
	
	if(Timerval <= xx)	
		tt = xx - Timerval;	
	else
		tt = 0xffffffff - Timerval + xx;
	if( tt  > DelayN )
	{
		ret = true;		
	}		
	return ret;
}

/****************************************  中断回调函数 ***********************************************************/

/******************************************************************
函数名称: void HAL_TIM_PeriodElapsedCallback
功能描述: 定时器中断回调
参数输入: TIM_HandleTypeDef	*htim	定时器句柄
参数输出: 无
返 回 值: 
其    他: HAL库中产生定时器中断时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{		
	/* 1s 1s定时器 */
	if (htim->Instance == TIM7)
	{
		TickInc1s();
		g_SysTime.Second++;
		CalcSysTimeBySoftware(&g_SysTime);						//计算时间
//		printf("2mcnt in sec=%d", g_2mCntInSecond);
		TickTimerProcess(&g_AskSignalDelay);
		TickTimerProcess(&g_AskDateTimeDelay);
		TickTimerProcess(&g_AskPhoneNumDelay);
		TickTimerProcess(&g_DtuResetDelay);
		TickTimerProcess(&g_DtuPowerUpDelay);
		TickTimerProcess(&g_DtuPowerDownDelay);
//		TickTimerProcess(&g_FileCloseTimer);
		TickTimerProcess(&g_UpdateRetryDelay);
	}
	
	/* 10ms 10ms定时器 */
	if (htim->Instance == TIM6)
	{
		TickInc10ms();
//		LED_ProcessInInterrupt(&g_LEDGroup, g_Tick10ms);		//中断中处理LED灯组显示状态
	}
		
}
// 获取月份天数函数
uint8_t getMonthDays(uint16_t year, uint8_t month)
{
    uint8_t days;
    uint8_t leap = 0;
    
    // 判断闰年
    if(((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0))
        leap = 1;
    
    switch(month)
    {
        case 1: case 3: case 5: case 7: case 8: case 10: case 12:
            days = 31;
            break;
        case 4: case 6: case 9: case 11:
            days = 30;
            break;
        case 2:
            days = leap ? 29 : 28;
            break;
        default:
            days = 31;
    }
    return days;
}
// 时间转换函数
void convertToUTC8(HexDateTime_t *time)
{
    time->Hour += 8;
    
    if(time->Hour >= 24)
    {
        time->Hour -= 24;
        time->Day += 1;
        
        if(time->Day > getMonthDays((time->Year+2000), time->Month))
        {
            time->Day = 1;
            time->Month += 1;
            
            if(time->Month > 12)
            {
                time->Month = 1;
                time->Year += 1;
                // 如果年份超过99，回绕到0（根据您的两位数年份格式）
                if(time->Year > 99)
                    time->Year = 0;				
            }
        }
    }
}


bool JudgeTick(uint32_t Timerval,uint32_t DelayN)
{
	bool ret;
	uint32_t xx,tt;

	ret = false;
	xx = uwTick;
	
	if(Timerval <= xx)	
		tt = xx - Timerval;	
	else
		tt = 0xffffffff - Timerval + xx;
	if( tt  > DelayN )
	{
		ret = true;		
	}		
	return ret;
}