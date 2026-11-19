
#include "BSP_gpio.h"
#include "BSP_button.h"

/******** 外部变量定义 ********/
BtnPara_t g_BtnPower;
BtnPara_t g_BtnUp;
BtnPara_t g_BtnDown;
BtnPara_t g_BtnOK;



/******** 外部函数定义 ********/

/****************************************  初始化及管脚配置函数 ***********************************************************/
/******************************************************************
函数名称: void BSP_BtnInitGpioPara
功能描述: 初始化按键参数配置
参数输入: BtnPara_t		*BtnPara	按键参数
		  GPIO_TypeDef	*GPIOx		按键管脚
		  uint16_t		Pinx
		  uint8_t		Dir			按键方向 0=低有效;1=高有效
参数输出: BtnPara_t		*BtnPara	按键参数
返 回 值: 
其    他: 
******************************************************************/
void BSP_BtnInitPara(BtnPara_t *BtnPara, GPIO_TypeDef *GPIOx, uint16_t Pinx, uint8_t Dir)
{
	BtnPara->PinPara.GPIOx = GPIOx;
	BtnPara->PinPara.Pinx = Pinx;
	if (Dir == 0)
		BtnPara->PinPara.Direction = 0;		//按键低有效
	else
		BtnPara->PinPara.Direction = 1;		//按键高有效
	
	BtnPara->EventPara.eEvent = BTN_EVT_NONE;
	BtnPara->EventPara.eLastEvent = BTN_EVT_NONE;
	BtnPara->EventPara.uBtnNowSta = BTN_STA_RELEASE;
	BtnPara->EventPara.uBtnLastSta = BTN_STA_RELEASE;
	BtnPara->EventPara.uClickCnt = 0;
	BtnPara->EventPara.tmrDebouncing = 0xFFFFFFFF;
	BtnPara->EventPara.tmrLongPress = 0xFFFFFFFF;
	BtnPara->EventPara.tmrRelease = 0xFFFFFFFF;
	
	/* 传入默认时间 */
	BtnPara->PinPara.DebouncingMs = BTN_DEBOUNCING_MS;
	BtnPara->PinPara.LongPressMs = BTN_LONGPRESS_MS;
	BtnPara->PinPara.ReleaseMs = BTN_RELEASE_MS;
	
	/* 回调函数 */
	BtnPara->BtnCallback.cbBtnClickFunc = NULL;
	BtnPara->BtnCallback.cbBtnDoubleFunc = NULL;
	BtnPara->BtnCallback.cbBtnLongFunc = NULL;
}

/******************************************************************
函数名称: void BSP_BtnSetupTimePara
功能描述: 配置按键事件参数
参数输入: BtnPara_t		*BtnPara	按键参数
		  uint32_t		DebouncingTimeMs	消抖时间ms
		  uint32_t		LongPressTimeMs		长按时间ms
		  uint32_t		ReleaseTimeMs		释放时间ms
参数输出: BtnPara_t		*BtnPara	按键参数
返 回 值: 
其    他: 
******************************************************************/
void BSP_BtnSetupTimePara(BtnPara_t *BtnPara, uint32_t DebouncingTimeMs, uint32_t LongPressTimeMs, uint32_t ReleaseTimeMs)
{
	/* 配置定时时间 */
	if (DebouncingTimeMs == 0)
		BtnPara->PinPara.DebouncingMs = BTN_DEBOUNCING_MS;
	else
		BtnPara->PinPara.DebouncingMs = DebouncingTimeMs;
	
	if (LongPressTimeMs == 0)
		BtnPara->PinPara.LongPressMs = BTN_LONGPRESS_MS;
	else
		BtnPara->PinPara.LongPressMs = LongPressTimeMs;
	
	if (ReleaseTimeMs == 0)
		BtnPara->PinPara.ReleaseMs = BTN_RELEASE_MS;
	else
		BtnPara->PinPara.ReleaseMs = ReleaseTimeMs;
}

/******************************************************************
函数名称: void BSP_BtnSetupCallbackPara
功能描述: 配置按键事件回调
参数输入: BtnPara_t		*BtnPara	按键参数
		  void 			*ClickCallback(void)	单击回调函数指针
		  void			*DoubleCallback(void)	双击回调函数指针
		  void			*LongCallback(void)		长按回调函数指针
参数输出: BtnPara_t		*BtnPara	按键参数
返 回 值: 
其    他: 
******************************************************************/
void BSP_BtnSetupCallbackPara(BtnPara_t *BtnPara, void (*ClickCallback)(void), void (*DoubleCallback)(void), void (LongCallback)(void))
{
	BtnPara->BtnCallback.cbBtnClickFunc = ClickCallback;
	BtnPara->BtnCallback.cbBtnDoubleFunc = DoubleCallback;
	BtnPara->BtnCallback.cbBtnLongFunc = LongCallback;
}

/****************************************  按键事件处理函数 ***********************************************************/
/******************************************************************
函数名称: uint8_t BSP_BtnGetGpioSta
功能描述: 获取GPIO状态
参数输入: BtnPara_t		*BtnPara	按键参数
参数输出: BtnPara_t		*BtnPara	按键参数
返 回 值: uint8_t	0=抬起;1=按下
其    他: 
******************************************************************/
uint8_t BSP_BtnGetGpioSta(BtnPara_t *BtnPara)
{
	uint8_t btnpress=0xFF;	//0=抬起;1=按下;0xFF=无
	
	/* 根据按键方向 获取GPIO状态 */
	if (BtnPara->PinPara.Direction == 0)
	{
		/* 按键低有效 */
		if (HAL_GPIO_ReadPin(BtnPara->PinPara.GPIOx, BtnPara->PinPara.Pinx) == GPIO_PIN_RESET)
			btnpress = 1;
		else
			btnpress = 0;
	}
	else
	{
		/* 按键高有效 */
		if (HAL_GPIO_ReadPin(BtnPara->PinPara.GPIOx, BtnPara->PinPara.Pinx) == GPIO_PIN_SET)
			btnpress = 1;
		else
			btnpress = 0;
	}
	
	return btnpress;
}

/******************************************************************
函数名称: BtnEvent_e BSP_BtnGetButtonEvent
功能描述: 获取按键事件
参数输入: BtnPara_t		*BtnPara	按键参数
参数输出: BtnPara_t		*BtnPara	按键参数
返 回 值: 
其    他: 需要提供1ms计时时间 默认是uwTick
******************************************************************/
BtnEvent_e BSP_BtnGetButtonEvent(BtnPara_t *BtnPara)
{
	uint8_t btnsta=BTN_STA_RELEASE;
	BtnEvent_e btnevt=BTN_EVT_NONE;
	uint8_t btnpress=0xFF;				
	
//	/* 当前已经产生事件 不改变事件 */
//	if (BtnPara->EventPara.eEvent != BTN_EVT_NONE)
//		return BtnPara->EventPara.eEvent;

	/* 根据当前按键状态确定 */
	btnpress = BSP_BtnGetGpioSta(BtnPara);		//获取按键状态
	btnsta = BtnPara->EventPara.uBtnStatus;
	
	switch (btnsta)
	{
		/* 按键状态 = 释放 */
		case BTN_STA_RELEASE:
			/* 判断按键事件 */
			if (BtnPara->EventPara.uClickCnt == 1)
			{
				btnevt = BTN_EVT_CLICK;
			}
			else if (BtnPara->EventPara.uClickCnt >= 2)
			{
				btnevt = BTN_EVT_DOUBLE;
			}
			/* 判断按键按下 */
			if (btnpress == 1)
			{
				BtnPara->EventPara.uBtnStatus = BTN_STA_PRE_DOWN;
			}
			else
			{
				BtnPara->EventPara.tmrDebouncing = uwTick;
				BtnPara->EventPara.tmrLongPress = uwTick;
				BtnPara->EventPara.tmrRelease = uwTick;
				BtnPara->EventPara.uClickCnt = 0;
			}
			break;
		/* 按键状态 = 预按下 */
		case BTN_STA_PRE_DOWN:
			/* 按键按下 判断按键消抖时间 */
			if (JudgeMilSecondDelayInPro(BtnPara->EventPara.tmrDebouncing, BtnPara->PinPara.DebouncingMs) == true)
			{
				/* 已经经过消抖时间 判断按键按下 */
				if (btnpress == 1)
				{
					BtnPara->EventPara.uBtnStatus = BTN_STA_PRESSDOWN;
				}
				else
				{
					BtnPara->EventPara.uBtnStatus = BTN_STA_RELEASE;
				}
			}
			BtnPara->EventPara.tmrLongPress = uwTick;
			BtnPara->EventPara.tmrRelease = uwTick;
			break;
		/* 按键状态 = 按键按下 */
		case BTN_STA_PRESSDOWN:
			/* 判断按键按下 */
			if (btnpress == 1)
			{
				BtnPara->EventPara.tmrDebouncing = uwTick;
				BtnPara->EventPara.tmrRelease = uwTick;
				if (JudgeMilSecondDelayInPro(BtnPara->EventPara.tmrLongPress, BtnPara->PinPara.LongPressMs) == true)
				{
					/* 超过长按时间 */
					btnevt = BTN_EVT_LONG;
					BtnPara->EventPara.uClickCnt = 0;
				}
			}
			if (btnpress == 0)
			{
				BtnPara->EventPara.uBtnStatus = BTN_STA_PRE_UP;
				BtnPara->EventPara.tmrDebouncing = uwTick;
				BtnPara->EventPara.tmrRelease = uwTick;
			}
			break;
		/* 按键状态 = 预抬起 */
		case BTN_STA_PRE_UP:
			/* 按键抬起 判断按键消抖时间 */
			if (JudgeMilSecondDelayInPro(BtnPara->EventPara.tmrDebouncing, BtnPara->PinPara.DebouncingMs) == true)
			{
				/* 已经经过消抖时间 判断按键抬起 */
				if (btnpress == 0)
				{
					BtnPara->EventPara.uBtnStatus = BTN_STA_PULLUP;
					BtnPara->EventPara.tmrDebouncing = uwTick;
					BtnPara->EventPara.tmrRelease = uwTick;
					BtnPara->EventPara.uClickCnt++;						//完成一次单击
				}
			}
			break;
		/* 按键状态 = 按键抬起 */
		case BTN_STA_PULLUP:
			/* 判断按键释放时间 */
			if (JudgeMilSecondDelayInPro(BtnPara->EventPara.tmrRelease, BtnPara->PinPara.ReleaseMs) == true)
			{
				/* 已经经过释放时间 */
				if (btnpress == 0)
				{
					BtnPara->EventPara.uBtnStatus = BTN_STA_RELEASE;
				}
			}
			if (btnpress == 1)
			{
				BtnPara->EventPara.uBtnStatus = BTN_STA_PRE_DOWN;
				BtnPara->EventPara.tmrDebouncing = uwTick;
				BtnPara->EventPara.tmrRelease = uwTick;
			}
			/* 判断按键释放 */
			break;
		/* 其他 */
		default:
			break;
	}
		
	return btnevt;
}

/******************************************************************
函数名称: void BSP_BtnEventProcess
功能描述: 按键事件处理
参数输入: BtnPara_t		*BtnPara	按键参数
参数输出: BtnPara_t		*BtnPara	按键参数
返 回 值: 
其    他: 需要提供1ms计时时间 默认是uwTick
******************************************************************/
void BSP_BtnEventProcess(BtnPara_t *BtnPara)
{
	BtnEvent_e btnevt;
	uint8_t btnsta;
	uint8_t btnpress;
	uint8_t longpress=0;
	
	/* 获取按键事件 */
	btnevt = BSP_BtnGetButtonEvent(BtnPara);
			
	/* 处理事件回调 */
	if (btnevt != BTN_EVT_NONE)
	{
		/* 产生单击事件 执行单击回调 */
		if (btnevt == BTN_EVT_CLICK)
		{
			BtnPara->EventPara.eEvent = btnevt;
			/* 判断上次是否是长按 长按不再触发单击事件 */
			if (BtnPara->EventPara.eLastEvent != BTN_EVT_LONG)
			{
				debug_printf("Click Event\r\n");
				BtnPara->EventPara.eLastEvent = BTN_EVT_CLICK;
				if (BtnPara->BtnCallback.cbBtnClickFunc != NULL)
				{
					BtnPara->BtnCallback.cbBtnClickFunc();
					BtnPara->EventPara.eLastEvent = BtnPara->EventPara.eEvent;
					BtnPara->EventPara.eEvent = BTN_EVT_NONE;
				}
			}
			else
			{
				BtnPara->EventPara.eEvent = BTN_EVT_NONE;
				BtnPara->EventPara.eLastEvent = BTN_EVT_NONE;
			}
		}
		/* 产生双击事件 执行双击回调 */
		else if (btnevt == BTN_EVT_DOUBLE)
		{
			BtnPara->EventPara.eEvent = btnevt;
			debug_printf("Double Event\r\n");
			BtnPara->EventPara.eLastEvent = BTN_EVT_DOUBLE;
			if (BtnPara->BtnCallback.cbBtnDoubleFunc != NULL)
			{
				BtnPara->BtnCallback.cbBtnDoubleFunc();
				BtnPara->EventPara.eLastEvent = BtnPara->EventPara.eEvent;
				BtnPara->EventPara.eEvent = BTN_EVT_NONE;
			}
		}
		/* 产生长按事件 执行长按回调 */
		else if (btnevt == BTN_EVT_LONG)
		{
			if (BtnPara->EventPara.eLastEvent != BTN_EVT_LONG)
			{
				BtnPara->EventPara.eEvent = btnevt;
				debug_printf("LongPress Event\r\n");
				BtnPara->EventPara.eLastEvent = BTN_EVT_LONG;
				if (BtnPara->BtnCallback.cbBtnLongFunc != NULL)
				{
					BtnPara->BtnCallback.cbBtnLongFunc();
					BtnPara->EventPara.eLastEvent = BtnPara->EventPara.eEvent;
					BtnPara->EventPara.eEvent = BTN_EVT_NONE;
				}
			}
			else
			{
				/* 上次长按未清除 不重复产生长按事件 */
				BtnPara->EventPara.eEvent = BTN_EVT_NONE;
			}
		}
	}
	else
	{
		/* 无事件 */
		BtnPara->EventPara.eEvent = BTN_EVT_NONE;
	}
	
}


/****************************************  按键回调函数 ***********************************************************/
void ButtonClick_OK(void)
{
	
}

void ButtonLongPress_OK(void)
{
	
}

void ButtonDouble_OK(void)
{
	
}


