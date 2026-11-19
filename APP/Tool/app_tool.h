#ifndef __APP_TOOL_H
#define __APP_TOOL_H
#include "BSP.h"
#include "stdint.h"
/* 以下为主板和客服工具通信协议 */
//包头 --- 通用和专用
//通用包头
#define   RECEIVETAG_ALL                     0xc1        //通用协议的接收包头
#define   SENDTAG_ALL                        0xc2        //通用协议的发送包头
#define   GWRECEIVETAG                       0xc3        //轨温协议的发送包头
#define   GWSENDTAG                          0xc4        //轨温协议的接收包头

#define   RYDWRECEIVETAG                     0xDE        //人员定位协议的发送包头
#define   RYDWSENDTAG                        0xDD        //人员定位协议的接收包头


//类型 --- 通用和专用
//通用协议
#define   SET_MY_NUMBER_TYPE                 0x03       //设置本机号
#define   GET_MY_NUMBER_TYPE                 0x04       //索要本机号
#define   SET_IP_TYPE                        0x05       //设置IP
#define   GET_IP_TYPE                        0x06       //索要IP
#define   GET_PARAM_INFOR_TYPE               0x07       //索要参数信息
#define	  RESET_DTU_TYPE					 0x16		 //重启DTU
#define	  RESET_SYS_TYPE					 0x17		 //重启系统
//主板类型
#define   SET_Board_TYPE                     0x14       //设置主板类型
#define   GET_Board_TYPE   			         0x15       //索要主板类型

//设置打点模式
#define   GET_GPS_INFO                       0x01
#define   SET_MODE_TYPE                      0x02
#define   GET_MODE_TYPE                      0x03


/******* 状态定义 *******/
typedef enum
{
	TOOLRET_OK				= 0,	/* 0 = 成功 */
	TOOLRET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	TOOLRET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	TOOLRET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	TOOLRET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	TOOLRET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	TOOLRET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
} TOOLResult;


typedef struct
{
	uint8_t programtype;
	uint8_t mainverno;
	uint8_t secverno;	
	uint16_t programpage;
} UPDATAPROGRAM;
extern UPDATAPROGRAM g_UpdataCodePageTotal;

//typedef struct
//{
//	uint8_t g_ServerIPAddr[3][4];	 //0-远程升级IP，1-远程转储IP，2-数据IP
//	uint16_t g_ServerPortNo[3]; 
//	uint16_t Crc16;
//}_SERVER_FLASH;
//extern _SERVER_FLASH g_ServerInfo;

extern uint8_t g_ToolCodeTypeErrorFlag;		//程序程序类型错误标记
extern uint8_t g_ToolDatBuf[UART5_RX_BUFSIZE];
extern uint8_t 	g_Signal;
void CheckTool(void);
extern uint8_t g_UartUPBuf[1488];
extern uint8_t MajorVersion;      //主版本号   agj
extern uint8_t MinorVersion;      //次版本号   agj
extern uint16_t g_UartUpOffset;
extern uint16_t g_UartUpPageNo;
void ToolProcess(void);
void InitToolVar(void);
#endif