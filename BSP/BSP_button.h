#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H


/* 引用头文件 */
#include "BSP_gpio.h"
//#include "stm32h7xx_hal.h"

/******* 宏定义 *******/
#define BTN_DEBOUNCING_MS	10		//默认消抖时间 10ms
#define BTN_LONGPRESS_MS	2000	//默认长按时间 2000ms
#define BTN_RELEASE_MS		300		//默认释放时间 300ms

/******* 按键状态宏定义 *******/
#define BTN_STA_RELEASE		0x00	//0=按键释放
#define BTN_STA_PRE_DOWN	0x01	//0x01=预按下
#define BTN_STA_PRESSDOWN	0x11	//0x11=按键按下
#define BTN_STA_PRE_UP		0x02	//0x02=预抬起
#define BTN_STA_PULLUP		0x12	//0x12=按键抬起

/******* 数据结构体定义 *******/
typedef enum
{
	BTN_EVT_NONE	=0,	/* 0=无 */
	BTN_EVT_CLICK	=1,	/* 1=单击事件 */
	BTN_EVT_DOUBLE	=2,	/* 2=双击事件 */
	BTN_EVT_LONG	=3,	/* 3=长按事件 */
} BtnEvent_e;

typedef struct
{
	struct
	{
		GPIO_TypeDef	*GPIOx;			//按键Port
		uint16_t		Pinx;			//按键Pin
		uint8_t			Direction;		//按键方向 0=低有效;1=高有效
		uint32_t		DebouncingMs;	//消抖时间(ms)
		uint32_t		LongPressMs;	//长按时间(ms)
		uint32_t		ReleaseMs;		//释放时间(ms)
	} PinPara;
	
	struct
	{
		BtnEvent_e	eEvent;				//按键事件
		BtnEvent_e	eLastEvent;			//上次按键状态
		uint8_t		uClickCnt;			//按键次数
		uint8_t		uBtnStatus;			//按键状态
		uint8_t		uBtnNowSta;			//当前按键状态
		uint8_t		uBtnLastSta;		//上次按键状态
		uint32_t	tmrDebouncing;		//消抖计时器
		uint32_t	tmrLongPress;		//长按计时器
		uint32_t	tmrRelease;			//释放计时器
	} EventPara;
	
	struct
	{
		void		(*cbBtnClickFunc)(void);	//按键单击回调函数
		void		(*cbBtnDoubleFunc)(void);	//按键双击回调函数
		void		(*cbBtnLongFunc)(void);		//按键长按回调函数
	} BtnCallback;
} BtnPara_t;


/******* 外部变量声明 *******/

extern BtnPara_t g_BtnOK;
extern BtnPara_t g_BtnPower;
extern BtnPara_t g_BtnUp;
extern BtnPara_t g_BtnDown;


/******* 外部函数声明 *******/
/* 初始化 */
void BSP_BtnInitPara(BtnPara_t *BtnPara, GPIO_TypeDef *GPIOx, uint16_t Pinx, uint8_t Dir);
void BSP_BtnSetupTimePara(BtnPara_t *BtnPara, uint32_t DebouncingTimeMs, uint32_t LongPressTimeMs, uint32_t ReleaseTimeMs);
void BSP_BtnSetupCallbackPara(BtnPara_t *BtnPara, void (*ClickCallback)(void), void (*DoubleCallback)(void), void (LongCallback)(void));

/* 功能函数 */
uint8_t BSP_BtnGetGpioSta(BtnPara_t *BtnPara);
BtnEvent_e BSP_BtnGetButtonEvent(BtnPara_t *BtnPara);
void BSP_BtnEventProcess(BtnPara_t *BtnPara);

/******* 回调函数声明 *******/

void ButtonClick_OK(void);
void ButtonDouble_OK(void);
void ButtonLongPress_OK(void);

#endif /* __BSP_BUTTON_H */

