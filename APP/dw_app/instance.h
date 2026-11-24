#ifndef __INSTANCE_H
#define __INSTANCE_H

#include <stdio.h>
#include "string.h"
#include "port.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_types.h"
#include "deca_spi.h"

#include "bsp.h"
//#include "usart.h"
//#include "ssd1306.h"
//#include "kalman.h"
//#include "e2prom.h"
//#include "cw2015.h"
//#include "trilateration.h"
//#include "iwdg.h"

/********定义板卡角色*************************************/
#define LD_PA
/***********************************************************************************************/
//#define L051_DEV   		 //L051单片机定义
//#define ANCRANGE             //基站间测距，用于基站自标定
/***********************************************************************************************/

//#define UART_PORT   UART1_Handler

//#include "lis2dh12.h"
//#include "lowpower.h"


#define SOFTWARE_VER     	            "V70"

#define DEEP_SLEEP                      0       //休眠功能是否在空闲时进入低功耗休眠，主要在低功耗场景和移植情况下开启

#define MAX_AHCHOR_NUMBER               4       //系统内最大基站数量，取4或者8，比如实际3个取4，实际6个取8
#define MAX_TAG_NUMBER_SW2_OFF          5       //ULM1、LD150、ld600模块，2号拨码off时最大标签个数
#define MAX_TAG_NUMBER_SW2_ON           1       //ULM1、LD150、ld600模块，2号拨码on时最大标签个数

/* 天线延时
 * 计算距离结果比实际距离小，需要增大距离，则减小这个数
 * 计算距离结果比实际距离大，需要减小距离，则增大这个数
 */                                                                                                               

#if defined (LD_PA)
#define ANT_DLY                         16500	//默认16500
#define TX_POWER                        0X9c9c9c9c
#endif


#if(MAX_TAG_NUMBER_SW2_OFF >= MAX_TAG_NUMBER_SW2_ON)   //MAX_TAG_LIST_SIZE取系统内最大标签数量
    #define MAX_TAG_LIST_SIZE       (MAX_TAG_NUMBER_SW2_OFF)  
#else
    #define MAX_TAG_LIST_SIZE       (MAX_TAG_NUMBER_SW2_ON)   
#endif


/* 数据帧超时及延时时间*/
#define PRE_TIMEOUT                     15

#if (MAX_AHCHOR_NUMBER == 4)
#define ONE_SLOT_TIME_MS_110K           28
#define ONE_SLOT_TIME_MS_850K           12
#define ONE_SLOT_TIME_MS_6P8M           10
#elif (MAX_AHCHOR_NUMBER == 8)
#define ONE_SLOT_TIME_MS_110K           70//50
#define ONE_SLOT_TIME_MS_850K           20
#define ONE_SLOT_TIME_MS_6P8M           15
#elif (MAX_AHCHOR_NUMBER == 1)
#define ONE_SLOT_TIME_MS_110K           70//50
#define ONE_SLOT_TIME_MS_850K           20
#define ONE_SLOT_TIME_MS_6P8M           15

#endif

#define FINAL_RX_TIMEOUT_6P8M           600
#define RESP_RX_TIMEOUT_6P8M            450
#define FIRST_RESP_SEND_6P8M            900     //6.8M通信速率下，第一个resp消息发送延时
#define DATA_INTERVAL_TIME_6P8M         1100    //6.8M通信速率下，相邻消息间隔时间
#define ANC_RESP_SEND_BACK_6P8M         100     //6.8M通信速率下，基站延后发送RESP消息时间
#define TAG_FINALE_SEND_BACK_6P8M       100     //6.8M通信速率下，标签延后发送FINAL消息时间

#define FINAL_RX_TIMEOUT_110K           9000
#define RESP_RX_TIMEOUT_110K            3500
#define FIRST_RESP_SEND_110K            3000    //110K通信速率下，第一个resp消息发送延时
#define DATA_INTERVAL_TIME_110K         3900    //110K通信速率下，相邻消息间隔时间
#define ANC_RESP_SEND_BACK_110K         1080    //110K通信速率下，基站延后发送RESP消息时间
#define TAG_FINALE_SEND_BACK_110K       1080    //110K通信速率下，标签延后发送FINAL消息时间

#define FINAL_RX_TIMEOUT_850K           1300
#define RESP_RX_TIMEOUT_850K            800     
#define FIRST_RESP_SEND_850K            1250     //850K通信速率下，第一个resp消息发送延时
#define DATA_INTERVAL_TIME_850K         1650    //850K通信速率下，相邻消息间隔时间
#define ANC_RESP_SEND_BACK_850K         300     //850K通信速率下，基站延后发送RESP消息时间
#define TAG_FINALE_SEND_BACK_850K       300     //850K通信速率下，标签延后发送FINAL消息时间

#define MAX_POLL_SEND_SLEEP_COUNT       150     //MAX_POLL_SEND_SLEEP_COUNT次发送后无运动则进入休眠
#define ANC_RANGE_COUNT                 5       //自标定时每个基站测距次数
/* PAN ID */
#define PAN_ID                          0xDECA	//原来是DECA

/* 中断状态标志 */
#define RX_WAIT                         0
#define TX_WAIT                         0
#define RX_OK                           1
#define TX_OK                           1
#define RX_TIMEOUT                      2
#define RX_ERROR                        3

/* 数据帧长度 */
#define POLL_MSG_LEN                    26
#define RESP_MSG_LEN                    19
#define FIANL_MSG_LEN                   (20 + (5 * MAX_AHCHOR_NUMBER))//(20 + 5 * MAX_AHCHOR_NUMBER)
#define BLINK_MSG_LEN                   10
#define INIT_MSG_LEN                    12
#define SYNC_MSG_LEN                    15

/* 数据帧数组索引 */
#define SEQ_NB_IDX                      2
#define PANID_IDX                       3
#define RECEIVER_SHORT_ADD_IDX          5
#define SENDER_SHORT_ADD_IDX            7
#define FUNC_CODE_IDX                   9
#define RANGE_NB_IDX                    10
#define POLL_MSG_SOS_IDX                11
#define POLL_MSG_ALARM_STA_IDX          12
#define POLL_MSG_BATTERY_IDX            13
#define POLL_MSG_USER_IDX               14
#define RESP_MSG_SLEEP_COR_IDX          11
#define RESP_MSG_PREV_DIS_IDX           13
#define RESP_MSG_ALARM_IDX              17
#define RESP_MSG_GROUP_IDX              18
#define INIT_MSG_SLEEP_COR_IDX          10
#define FINAL_MSG_FINAL_VALID_IDX       11
#define FINAL_MSG_POLL_TX_TS_IDX        12
#define FINAL_MSG_FINAL_TX_TS_IDX       16
#define FINAL_MSG_A0_GROUP_ID_IDX       20
#define FINAL_MSG_A1_GROUP_ID_IDX       25
#define FINAL_MSG_A2_GROUP_ID_IDX       30
#define FINAL_MSG_A3_GROUP_ID_IDX       35
#define FINAL_MSG_A4_GROUP_ID_IDX       40
#define FINAL_MSG_A5_GROUP_ID_IDX       45
#define FINAL_MSG_A6_GROUP_ID_IDX       50
#define FINAL_MSG_A7_GROUP_ID_IDX       44
#define FINAL_MSG_RESP1_RX_TS_IDX       21
#define FINAL_MSG_RESP2_RX_TS_IDX       26
#define FINAL_MSG_RESP3_RX_TS_IDX       31
#define FINAL_MSG_RESP4_RX_TS_IDX       36
#define FINAL_MSG_RESP5_RX_TS_IDX       41
#define FINAL_MSG_RESP6_RX_TS_IDX       46
#define FINAL_MSG_RESP7_RX_TS_IDX       51
#define FINAL_MSG_RESP8_RX_TS_IDX       56
#define SYNC_MSG_TIME_IDX               10


/*  function code */
#define FUNC_CODE_POLL                  0x21
#define FUNC_CODE_RESP                  0x10
#define FUNC_CODE_FINAL                 0x23
#define FUNC_CODE_BLINK                 0x36
#define FUNC_CODE_INIT                  0x38
#define FUNC_CODE_SYNC                  0x42

/* 拨码开关键值 */
#define SWS1_IMU_MODE                   0x80	   //IMU标签 on=输出IMU数据， off=输出正常mc数据
#define SWS1_SHF_MODE                   0x40	   //默认off=10标签，100ms更新一次，on=1标签，10ms更新一次，可修改宏定义MAX_TAG_NUMBER_SW2_OFF 和 MAX_TAG_NUMBER_SW2_ON
#define SWS1_HPR_MODE                   0x20	   //外部功耗增加开关
#define SWS1_ROLE_MODE                  0x10     //工作模式0=tag 1=anchor
#define SWS1_A1A_MODE                   0x08     //anchor/tag address A1
#define SWS1_A2A_MODE                   0x04     //anchor/tag address A2
#define SWS1_A3A_MODE                   0x02     //anchor/tag address A3
#define SWS1_KAM_MODE                   0x01     //卡尔曼滤波开关

/* 状态机标志位 */
typedef enum
{
    STA_IDLE, 
    STA_SEND_POLL,
    STA_WAIT_RESP,
    STA_RECV_RESP,
    STA_SEND_FINAL,
    STA_INIT_POLL_SYNC,
    STA_WAIT_POLL_SYNC,
    STA_RECV_POLL_SYNC,
    STA_SEND_RESP,
    STA_WAIT_FINAL,
    STA_RECV_FINAL,
    STA_SORR_RESP,
    STA_SEND_BLINK,
    STA_WAIT_INIT,
    STA_RECV_INIT,
    STA_SEND_SYNC
} instStatus;


/* TWR测距状态 */
typedef enum
{
    RANGE_NULL, 
    RANGE_TWR_OK,
    RANGE_ERROR
} twrStatus;

/* 系统运行角色 */
typedef enum
{
    TAG, 
    ANCHOR
} instanceModes;

/* 基站参数 */
typedef struct
{
	double x;		//UWB坐标X
	double y;		//UWB坐标Y
	double lon;	//经度
	double lat;	//纬度
} coord_t;	//坐标类型  一个坐标32字节
extern coord_t anchor_cood[MAX_AHCHOR_NUMBER];	//基站坐标 4个
extern coord_t tag_pos;						//标签坐标

/* 定位点信息 */
typedef struct
{
	uint32_t	Tick;		//时间戳
	coord_t		TagCoord;	//定位点坐标
	double		Distance[MAX_AHCHOR_NUMBER];	//各基站测距值
} TagInfo_t;
#define V_THRE	3.0f	//速度门限 3.0m/s
#define R_THRE	5.0f	//测距变化门限 5.0m/s

/* 数据输出标记 */
typedef struct
{
	uint8_t	ShowPos:1;		//b0:输出坐标 0=不输出;1=输出
	uint8_t ShowRssi:1;		//b1:输出RSSI 0=不输出;1=输出
	uint8_t ShowDist:1;		//b2:输出距离 0=不输出;1=输出
	uint8_t ShowGPS:1;		//b3:输出GPS 0=不输出;1=输出
	uint8_t :3;				//b4-b6:预留
	uint8_t TagAbnormal:1;	//b7:异常点 0=正常;1=异常
} OutputFlag_t;
extern OutputFlag_t g_OutputFlag;	//输出标记


/* 设备配置参数 */
typedef struct
{
	uint8_t			DeviceID;		//设备ID号
	uint8_t			DeviceMode;		//设备模式 0=标签;1=基站
	uint16_t		PanID;			//网络ID
	uint16_t		AntDelay;		//天线延迟
	uint8_t			SolutionMode;	//解算模式
	uint8_t			StatisticsMode;	//统计模式
	uint8_t			ShowSolution;	//显示解算结果
	double			AnchorHeight;	//基站高度
	double			TagHeight;		//标签高度
	
	dwt_config_t	DwConfig;		//DW通信参数
} DevConfig_t;
extern DevConfig_t g_DevConfig;	//设备参数

extern uint8_t instance_mode;                         //设备运行角色
extern uint8_t dev_id;                                         //设备ID
extern uint8_t switch8;
extern uint8_t anc_id;
extern uint8_t tag_id;
extern uint8_t group_id;                                       //组ID
extern uint8_t state;
extern int32_t distance_report[8];
extern int32_t group_report[8];   	//基站组ID数组，用于打包输出
extern uint32_t range_time;
extern uint8_t inst_ch;           	//信道号Channel number
extern uint8_t inst_prf;          	//PRF
extern uint8_t frame_seq_nb;  			//每帧数据增加1
extern uint8_t range_nb;      			//每次range增加1(poll resp1~4 fianl维护一套range_nb)
extern uint8_t recv_tag_id;
extern uint8_t recv_anc_id;
extern uint8_t range_status;
extern float rx_power;
extern float rssi[8];
extern double g_tag_h;		//定位卡高度
extern double g_anchor_h;		//基站高度


extern uint16_t inst_slot_number;
extern uint8_t inst_dataRate; 
extern uint8_t inst_one_slot_time;
extern uint32_t inst_final_rx_timeout;
extern uint32_t inst_resp_rx_timeout;
extern uint32_t inst_init_rx_timeout;                          
extern uint64_t inst_poll2final_time;
extern uint32_t inst_data_interval;
extern uint16 ant_dly;
extern double distance_now_m;  
extern int32 distance_offset_cm;                               //距离校准，单位cm
extern uint8_t sos;
extern uint8_t alarm;
//extern uint8_t battery;
extern uint8_t USE_IMU;
extern int user_data[10];
extern uint8_t g_CalFlag;

extern uint8_t ancrange_flag;
extern uint8_t temp_dev_id;
extern uint8_t ancrange_count;
extern uint8_t target_ancid;
extern uint32_t volatile next_period_time;                          //标签下次测距周期开始时间，用于防冲突管理
void anchor_app(void);
void anc_diag_set_mode(uint8_t enabled, uint8_t skip_prefix, uint8_t slot);
void anc_diag_report(void);
void tag_app(void);
extern void set_instance(void);
extern void DW1000_init(void);

void tag_rx_ok_cb(const dwt_cb_data_t *cb_data);
void tag_rx_to_cb(const dwt_cb_data_t *cb_data);
void tag_rx_err_cb(const dwt_cb_data_t *cb_data);
void tag_tx_conf_cb(const dwt_cb_data_t *cb_data);


void anc_rx_ok_cb(const dwt_cb_data_t *cb_data);
void anc_rx_to_cb(const dwt_cb_data_t *cb_data);
void anc_rx_err_cb(const dwt_cb_data_t *cb_data);
void anc_tx_conf_cb(const dwt_cb_data_t *cb_data);
bool anc_tx_timeout(uint32_t timeout);

bool SaveDevConfig(DevConfig_t Cfg, uint16_t CfgLen);
bool ReadDevConfig(DevConfig_t *Cfg, uint16_t CfgLen);
void ShowAnchorCoordinates(void);
void ShowDevConfig(void);

#endif
