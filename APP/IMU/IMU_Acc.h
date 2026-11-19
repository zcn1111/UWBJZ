#ifndef __IMU_ACC_H
#define __IMU_ACC_H

#include "lsm6dsl_reg.h"


/******* 返回状态定义 *******/
typedef enum
{
	IMURET_OK				= 0,	/* 0 = 成功 */
	IMURET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	IMURET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	IMURET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	IMURET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	IMURET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	IMURET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	IMURET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	IMURET_TYPE_ERR			= 8,	/* 8 = 类型错误 */
	IMURET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	IMURET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
} IMUResult;

/******* IMU参数定义 *******/
typedef struct
{
	float AccX;		//加速度 - X轴 mg
	float AccY;		//加速度 - Y轴 mg
	float AccZ;		//加速度 - Z轴 mg
	float GyoX;		//角速率 - X轴 mdps
	float GyoY;		//角速率 - Y轴 mdps
	float GyoZ;		//角速率 - Z轴 mdps
	float TmpC;		//温度 摄氏度
} IMU_Val_t;


/******* 外部变量定义 *******/

extern stmdev_ctx_t	g_IMUDevCtx;
extern IMU_Val_t		g_IMUVal;


extern uint32_t g_tmrIMUDelay;				//IMU采样定时器
extern uint32_t g_tmrIMUDispDelay;			//IMU输出定时器
/******* 外部函数定义 *******/

//void lsm6dsl_read_data_polling(void);

/* 驱动初始化 */
void IMU_InitDriver(stmdev_ctx_t *pDevCtx);

/* 获取采样数据 */
IMUResult IMU_GetAccelerationInMMg(stmdev_ctx_t *pDevCtx, float *AccX, float *AccY, float *AccZ);
IMUResult IMU_GetAngularRateInMMdps(stmdev_ctx_t *pDevCtx, float *GyoX, float *GyoY, float *GyoZ);
IMUResult IMU_GetTempratureInCelsius(stmdev_ctx_t *pDevCtx, float *Temp);


#endif

