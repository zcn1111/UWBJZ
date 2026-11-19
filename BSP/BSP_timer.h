#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H


/* 引用头文件 */
//#include "BSP.h"
#include "stm32l4xx_hal.h"
#include "stdbool.h"
#define FUNC_DELAY

/******* 定时器宏定义 *******/
/* 定时器启动和停止 */
#define TTM_START		1
#define TTM_STOP		0

/* 定时器自动重计时 */
#define TTM_MODE_RESTART	1
#define TTM_MODE_SINGLE		0

#define TICKTIMER_LIST_MAX	20


/******* 校时函数返回值宏定义 *******/
#define ADJTIME_NONE		0			//未校时
#define ADJTIME_SUCCESS		1			//校时成功
#define ADJTIME_HADTIME		2			//已有时间不需要校时




/*******************************************/


#define Delay1mSec          1
#define Delay5mSec          5
#define Delay3Sec           3000
#define Delay10Sec          10000
#define Delay30Sec          30000
#define Delay3Min           180000

/******* 数据结构体定义 *******/
/* 定时器运行流程状态 */
typedef enum
{
	TICKTIMER_STOPPED	= 0,	/* 0 = 定时器已停止 */
	TICKTIMER_RUNNNING	= 1,	/* 1 = 定时器正在运行 */
	TICKTIMER_TIMEOUT	= 2,	/* 2 = 定时时间到 */
	TICKTIMER_OVERMAX	= 3,	/* 3 = 超过重新开始次数 */
} TimerRunProg_e;

/* 滴答定时器标记参数 */
typedef struct
{
	uint8_t Actived:1;			//b0:定时器激活 0=不活动;1=活动
	uint8_t AutoRestart:1;		//b1:自动重开始 0=关闭;1=自动重开始
	uint8_t :4;					//b2-b5:预留
	uint8_t ReStartMaxFlag:1;	//b6:定时器超过最大重启次数
	uint8_t TimeOutFlag:1;		//b7:定时时间到标记
} TimerFlag_t;

/* 滴答定时器参数 */
typedef struct
{
	TimerFlag_t		TimerFlag;		//定时器标记状态
	TimerRunProg_e	TimerProg;		//定时器流程状态
	uint32_t		StartTickVal;	//起始滴答值
	uint32_t		DelayTick;		//定时滴答值
	
	uint8_t			RestartCnt;		//重开始次数
	uint8_t			RestartMax;		//最大重开始次数
	
	volatile	uint32_t	*BaseTick;			//定时器对应滴答时基

	void		(*cbTimeoutFunc)(void);			//定时时间到回调函数
	void		(*cbOverMaxFunc)(void);			//超过最大重开始次数回调函数
} TickTimerPara_t;

/* 定时器列表组 */
typedef struct
{
	uint16_t			ListNbr;
	TickTimerPara_t*	TimerList[TICKTIMER_LIST_MAX];
}TickTimerGroups_t;

/* BCD码时间日期 */
typedef struct
{
	uint8_t	Year;	//年
	uint8_t Month;	//月
	uint8_t Day;	//日
	uint8_t	Hour;	//时
	uint8_t	Minute;	//分
	uint8_t	Second;	//秒
} BcdDateTime_t;

/* Hex码时间日期 */
typedef struct
{
	uint8_t	Year;	//年
	uint8_t Month;	//月
	uint8_t Day;	//日
	uint8_t	Hour;	//时
	uint8_t	Minute;	//分
	uint8_t	Second;	//秒
} HexDateTime_t;

/* Hex码时间日期(带校时标记) */
typedef struct
{
	uint8_t	Year;	//年
	uint8_t Month;	//月
	uint8_t Day;	//日
	uint8_t	Hour;	//时
	uint8_t	Minute;	//分
	uint8_t	Second;	//秒
	struct
	{
		uint8_t SrcDTU:1;		//b0:DTU时间源 0=未获取时间;1=已获取时间
		uint8_t SrcTAX:1;		//b1:TAX时间源 0=未获取时间;1=已获取时间
		uint8_t SrcGPS:1;		//b2:GPS时间源 0=未获取时间;1=已获取时间
		uint8_t :4;				//b3-b6:预留
		uint8_t AdjustedTime:1;	//b7:已校时
	} AdjFlag;					//校时标记
} SysDateTime_t;

/* 32位时间戳 */
typedef union
{
	struct
	{
		uint32_t Second:6;	//b0-b5:秒 		范围0-59
		uint32_t Minute:6;	//b6-b11:分钟	范围0-59
		uint32_t Hour:5;	//b12-b16:小时	范围0-23
		uint32_t Day:5;		//b17-b21:日	范围1-31
		uint32_t Month:4;	//b22-b25:月	范围1-12
		uint32_t Year:6;	//b26-b31:年	范围0-63
	} bit;
	uint32_t all;
} TimeStampSec_t;
typedef struct
{
	uint8_t   Year;	
	uint8_t   Month;	 
	uint8_t   Date;		
	uint8_t   Hour;	
	uint8_t   Minute;
}AppDateTime;

/******* 外部变量声明 *******/

extern TIM_HandleTypeDef hTIM7;
extern TIM_HandleTypeDef hTIM6;
extern TIM_HandleTypeDef hTIM3;
extern TIM_HandleTypeDef hTIM16;

extern volatile uint32_t g_Tick10ms;	//计时间隔为10ms
extern volatile uint32_t g_Tick1s;	//计时间隔为1s
extern volatile uint32_t g_Timeout1ms;	
extern HexDateTime_t	g_BCDSysTime;
extern HexDateTime_t	g_SysTime;
extern HexDateTime_t	g_BCDTime;
/******* 外部函数声明 *******/
/* 定时器初始化 */
void TIM7Init(uint16_t arr, uint16_t psc);
void TIM6Init(uint16_t arr, uint16_t psc);
void TIM2Init(void);
void TIM3Init(uint16_t arr, uint16_t psc);
void TIM16Init(uint16_t arr, uint16_t psc);


/* 滴答 */
void TickInc10ms(void);
void TickInc1s(void);

#if defined(FUNC_DELAY)
void delay_init(uint8_t SYSCLK);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);
#endif


/* 滴答延时 */
void TickDelay10ms(uint32_t DelayTime);
void Delay1us(uint32_t DelayTime);
void Delay1ms(uint32_t DelayTime);

/* 滴答软件定时器 */
void InitTickTimerPara(TickTimerPara_t *Para, volatile uint32_t *BaseTick);
void SetupTickTimerPara(uint32_t DelayTick, uint8_t AutoRestart, uint8_t RestartMax, void (*cbTimeoutFunc)(void), void (*cbOvermaxFunc)(void), TickTimerPara_t *Para);
void StopTickTimer(TickTimerPara_t *Para);
void StartTickTimer(TickTimerPara_t *Para);
void ResetTickTimer(TickTimerPara_t *Para);
void TickTimerProcess(TickTimerPara_t *TmrPara);

/* 滴答定时器组 */
void InitTickTimerListGroup(TickTimerGroups_t *TmrList);
void RegisterTickTimer(TickTimerPara_t *TmrPara, TickTimerGroups_t *TmrList);
void UnRegisterTickTimer(TickTimerPara_t *TmrPara, TickTimerGroups_t *TmrList);
void TickTimerGroupsProcess(TickTimerGroups_t *TmrGrpPara);

/* 计时校时函数 */
uint8_t IsLeapYear(uint16_t Year);
void CalcSysTimeBySoftware(HexDateTime_t *Time);
uint8_t AdjustSysTime(HexDateTime_t SrcTime, HexDateTime_t *SysTime);

uint32_t ElapsedTimeInSecond(HexDateTime_t StTime, HexDateTime_t EdTime);

void convertToUTC8(HexDateTime_t *time);
bool JudgeSomeDelayInProOk(uint32_t Timerval, uint32_t DelayN, uint32_t BaseTick);
bool JudgeMilSecondDelayInPro(uint32_t Timerval, uint32_t DelayN);
bool JudgeTick(uint32_t Timerval,uint32_t DelayN);
#endif

