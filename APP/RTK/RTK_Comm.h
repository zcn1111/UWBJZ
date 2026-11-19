/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTK_COMM_H
#define __RTK_COMM_H

#include "BSP.h"
#include "stm32l4xx.h"

#define USE_LC29HDA		0		//使用LC29HDA
#define USE_LC29HBS		0		//使用LC29HBS
#define USE_UM980		1		//使用UM980

/******* 缓存宏定义 *******/

#define RTK_RX_BUFSIZE		3072	//接收缓存大小
#define RTK_DIFF_BUFSIZE	3072	//差分数据缓存大小

/******* RTK PQTM命令定义 *******/
#if ((USE_LC29HBS) || (USE_LC29HDA))
extern const char RTKQCMD_GetVerNo[];			//获取当前版本号
extern const char RTKQCMD_SavePara[];			//保存配置到NVM
extern const char RTKQCMD_SetSureryTime[];	//配置收敛时间300s
extern const char RTKQCMD_ShowSureryInfo[];	//配置显示测量信息(收敛) 1s
extern const char RTKQCMD_SetFixedSurery[];	//配置收敛固定位置
#endif

#if (USE_LC29HDA)
extern const char RTKQCMD_GetStationMode[];	//获取基站模式
extern const char RTKQCMD_SetBaseStation[];	//设置基站模式
extern const char RTKQCMD_SetMovingStation[];	//设置移动站模式
extern const char RTKQCMD_ColdStart[];		//模块冷启动
extern const char RTKQCMD_HotStart[];			//模块热启动
extern const char RTKQCMD_HideGeofenceSta[];	//配置隐藏地理围栏信息
#endif

/******* RTK PAIR命令定义 *******/
#if (USE_LC29HDA)
extern const char RTKACMD_SetGGAOutputEnable[];	//设置GGA按1s输出
extern const char RTKACMD_SetGGAOutputDisable[];	//设置GGA不输出
extern const char RTKACMD_SetRMCOutputEnable[];	//设置RMC按1s输出
extern const char RTKACMD_SetRMCOutputDisable[];	//设置RMC不输出
#endif

/******* 返回状态定义 *******/
typedef enum
{
	RTKRET_OK				= 0,	/* 0 = 成功 */
	RTKRET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	RTKRET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	RTKRET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	RTKRET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	RTKRET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	RTKRET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	RTKRET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	RTKRET_TYPE_ERR			= 8,	/* 8 = 类型错误 */
	RTKRET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	RTKRET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
	RTKRET_NOMATCH_ERR		= 11,	/* 11 = 不匹配 */
	RTKRET_SURVEY_NOVAILD	= 200,	/* 200 = 观测状态 无效 */
	RTKRET_SURVEY_PROGING	= 201,	/* 201 = 观测状态 进行中 */
	RTKRET_SURVEY_OK		= 202, 	/* 202 = 观测状态 有效 */
} RtkResult;


/******* 数据结构体定义 *******/


/* RTK定位标记 */

/* RTK测量信息结构体 */
typedef struct
{
	uint16_t	TOW;	//GPS周时间
	uint8_t		Valid;	//测量位置有效性标志 0=无效;1=进行中;2=有效
	uint16_t	Obs;	//测量期间使用的位置观测数
	uint16_t	CfgDur;	//配置的最小观测时间
	double		MeanX;	//沿X轴的当前测量平均位置
	double		MeanY;	//沿Y轴的当前测量平均位置
	double		MeanZ;	//沿Z轴的当前测量平均位置
	double		MeanAcc;//当前测量平均位置精度
} RtkSurveyinInfo_t;

/******* 外部变量定义 *******/
extern uint8_t		g_RtkRecvBuf[RTK_RX_BUFSIZE];	//RTK收发缓存
extern uint8_t		g_RtkDiffBuf[RTK_DIFF_BUFSIZE];	//差分数据缓存
extern uint16_t		g_RtkDiffLen;


extern RtkSurveyinInfo_t	g_RtkSurveyInfo;				//RTK测量信息
extern LEDState_t			g_RtkLED;
/******* 外部函数定义 *******/

/* RTK通信函数 */
RtkResult RTK_SendBuf(uint8_t *InBuf, uint16_t StrLen);
RtkResult RTK_RecvBuf(uint8_t *OutBuf, uint16_t *StrLen);
RtkResult RTK_InitMoudle(void);
RtkResult RTK_InitDTUMoudle(void);

RtkResult RTK_SetRoverModule_UM980(uint8_t SaveCfg);
RtkResult RTK_SetBaseFixModule_UM980(double Lat, double Lon, double Alt, uint8_t SaveCfg);


//uint8_t RTK_CalcSpiltChrNum(char *InStr, uint16_t SLen, char SpiltChr);
//void RTK_GetStrAfterSpiltChr(char *InStr, uint16_t SLen, char SpiltChr, uint8_t SpiltNum, char *OutStr);

RtkResult RTK_GetRTCMInfo(uint8_t *RecvBuf, uint16_t BufLen, uint8_t *DiffBufOut, uint16_t *DiffLen);

#if (USE_LC29HDA)
RtkResult RTK_GetSurverInfo(uint8_t *InBuf, uint16_t BufLen, RtkSurveyinInfo_t *SurveyInfo);
#endif
//void InitGPS(void);
//void GPSProcess(uint8_t *GPSchar, uint16_t Size);
//int16_t GPSInfoAnlysis(uint8_t *strGPS, uint16_t Size);

//void ConvGpsPositionInfo(GpsPositionInfo_t *GInfo, GPS_T *Gps);

//void SendGPSData(void);
#endif /* __RTK_COMM_H */
