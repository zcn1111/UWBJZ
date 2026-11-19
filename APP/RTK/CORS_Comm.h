/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CORS_COMM_H
#define __CORS_COMM_H

#include "BSP.h"
#include "stm32l4xx.h"

#define USE_ML307A		1		//使用ML307A模块

/******* 缓存宏定义 *******/

#define CORS_RX_BUFSIZE		3072
/******* ML307A命令定义 *******/
#if (USE_ML307A)
extern const char CORSMCMD_SetNtripServerp[];	//设置Ntrip服务器地址和端口号 TCP模式 rtk.ntrip.qxwz.com:8002
extern const char CORSMCMD_UserName[];		//设置Ntrip账号用户名、密码和挂载点
//extern const char CORSMCMD_SetDataServerp[];	//设置数据服务器地址和端口号 TCP模式 111.53.192.10:2028
extern const char CORSMCMD_GetNtripServerp[];	//获取Ntrip服务器配置
extern const char CORSMCMD_GetDataServerp[];	//获取数据服务器配置
extern const char CORSMCMD_GetSignalQuality[];//获取信号质量
extern const char CORSMCMD_GetGSMStatus[];	//获取连接状态
extern const char CORSMCMD_Reset[];			//复位模块
extern const char CORSMCMD_SetOffline[];		//设置模块离线
#endif
/******* RTK PAIR命令定义 *******/

/******* 返回状态定义 *******/
typedef enum
{
	CORSRET_OK				= 0,	/* 0 = 成功 */
	CORSRET_MEM_FAIL		= 1,	/* 1 = 申请内存失败 */
	CORSRET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	CORSRET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	CORSRET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	CORSRET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	CORSRET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	CORSRET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	CORSRET_TYPE_ERR		= 8,	/* 8 = 类型错误 */
	CORSRET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	CORSRET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
	CORSRET_NOMATCH_ERR		= 11,	/* 11 = 不匹配 */
	CORSRET_CONNECT_FAIL	= 100,	/* 100 = 联网失败 */
	CORSRET_AUTH_FAIL		= 101, 	/* 101 = 鉴权失败 */
} CorsResult;


/******* 数据结构体定义 *******/
/* DTU连接状态 */
/* 服务器连接状态 0=未连接;1=已连接;2=未注册成功;3=未配置 */
typedef struct
{
	uint8_t	SvrSta1:2;		//b0-b1:服务器1连接状态
	uint8_t SvrSta2:2;		//b2-b3:服务器2连接状态
	uint8_t SvrSta3:2;		//b4-b5:服务器3连接状态
	uint8_t SvrSta4:2;		//b6-b7:服务器4连接状态
} ConnStatus_t;

/******* 外部变量定义 *******/
extern uint8_t			g_CorsRecvBuf[CORS_RX_BUFSIZE];		//Cors缓存

/******* 外部函数定义 *******/
CorsResult CORS_SendBuf(uint8_t *StrBuf, uint16_t StrLen);
CorsResult CORS_RecvBuf(uint8_t *OutBuf, uint16_t *StrLen);
CorsResult CORS_SetupCommand(char *SetCmd, char *SuccRetStr, uint8_t RetryCnt, uint16_t Timeout);
CorsResult CORS_GetStatusCode_ML307A(uint16_t *StatusCode, uint32_t TimeoutMs);


#if (USE_ML307A)
CorsResult CORS_GetStatusCode_ML307A(uint16_t *StatusCode, uint32_t TimeoutMs);
CorsResult CORS_ConnectNtrip_ML307A(uint16_t *StatusCode, uint8_t TimeoutSec);
CorsResult CORS_SetupNtrip_ML307A(char*user,char*pass);
CorsResult CORS_SetupDataSvr_ML307A(void);
CorsResult CORS_Disconnect_ML307A(void);
CorsResult CORS_DisconnectNtrip_ML307A(void);
CorsResult CORS_GetNetSignalQuality_ML307A(uint8_t *Rssi);
CorsResult CORS_GetConnectionStatus_ML307A(ConnStatus_t *Sta);

#endif

#endif /* __CORS_COMM_H */
