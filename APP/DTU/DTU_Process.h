#ifndef __DTU_PROCESS_H
#define __DTU_PROCESS_H

/* 引用头文件 */
#include "stm32l4xx.h"

#include "BSP.h"
#include "DTU_Comm.h"

/******* 状态定义 *******/


/* 宏定义 */
#define DELAY_ASKSIGNAL_DISCONN		10		//未连接索要信号强度间隔(秒)
#define DELAY_ASKSIGNAL_NORMAL		10		//正常索要信号强度间隔(秒)
#define DELAY_ASKTIME				30		//索要时间间隔(秒)
#define DELAY_ASKPHONENUM			30		//索要本机号间隔(秒)
#define DELAY_RESETPOWER_DISCONN	1200	//未连接重启DTU间隔(秒)

#define DELAY_DTU_POWERDOWN			10		//DTU断电前等待时间(秒)
#define DELAY_DTU_POWERUP			30		//DTU上电前等待时间(秒)

#define DELAY_TRANS_DISCONN			120		//透传模式连接等待时间(秒)

#define DELAY_UPDATE_RETRY			20		//等待数据超时(秒)

#define RETRYMAX_PHONENUM			10		//索要本机号次数上限
#define RETRYMAX_CHANGEIP			6		//切换IP次数上限

/******* 数据结构体定义 *******/

/******* 外部变量声明 *******/
extern TickTimerPara_t	g_AskSignalDelay;
extern TickTimerPara_t	g_AskDateTimeDelay;
extern TickTimerPara_t	g_AskPhoneNumDelay;
extern TickTimerPara_t	g_DtuResetDelay;
extern TickTimerPara_t	g_DtuPowerDownDelay;
extern TickTimerPara_t	g_DtuPowerUpDelay;

extern TickTimerPara_t	g_TransConnectDelay;		//透传模式连接延迟
extern TickTimerPara_t	g_UpdateRetryDelay;			//升级重试延迟


extern DtuPara_t		g_DtuPara;			//DTU参数
/******* 外部函数声明 *******/

void DTU_TimerInit(void);

/* 连接流程 */
int16_t DTU_ConnectionProgress(DtuPara_t *Para);
int16_t DTU_CommProgress(DtuPara_t *Para);

/* 外部函数注册 */
void DTU_GenericExtFuncInit(void);

#endif

