/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RYDW_DEF_H
#define __RYDW_DEF_H

/*****************************************
 定义人员定位设备公用结构体
 ****************************************/
 
#include "bsp_timer.h"
#include "stdint.h"

//#include "LORA_Comm.h"
//#include "stm32l4xx.h"

/******* 缓存宏定义 *******/

#define RYDW_DIFFINFO_BUFSIZE		2048		//差分数据缓存大小


/******* 宏定义 *******/
#define RYDW_LOCATION_SURVEY		1			//自定位
#define RYDW_LOCATION_CORS			0			//CORS定位

#define RYDW_BOARDCAST_INTERVAL		3			//广播间隔
#define RYDW_BOARDCAST_OTHER_VAL	4			//广播其他数据间隔
#define RYDW_CARDTABLE_MAXNUM		10			//定位卡最大总数

/******* 变量结构体定义 *******/

/* Lora通信数据头 */
typedef struct
{
	uint8_t		Type;		//类型码 0=差分信息;1=电子围栏
	uint16_t	DestOrder;	//目标编号 0=广播
	uint16_t	SrcOrder;	//源编号
	uint8_t		PositionSta;//收敛标记 0=未收敛;1=正在收敛;2=已收敛
	uint8_t		WorkingSta;	//上下道标记 0=无;1=上道;2=下道
	uint8_t		TrainingSta;//来车标记 0=无;1=来车
} LoraHeader_t;

/* 人员卡ID */
typedef union
{
	struct
	{
		uint8_t		AdminID;		//局ID
		uint8_t		SectionID;		//段ID
		uint8_t		VrtWorkshopID;	//虚拟车间ID
		uint8_t		WorkshopID;		//车间ID
		uint8_t		WorkareaID;		//工区ID
		uint16_t    UserID;			//卡ID
	} byte;
	uint8_t array[7];
} UserCardID_t;

/* 报警标记 */
typedef union
{
	struct
	{
		uint8_t	Alram:1;	//b0:报警标记 0=正常;1=报警
		uint8_t SOS:1;		//b1:求救标记 0=正常;1=求救
		uint8_t LowPower:1;	//b2:低电量标记 0=正常;1=低电量
		uint8_t Position:1;	//b3:打点标记 0=正常;1=打点标记
		uint8_t :3;			//b4-b6:预留
		uint8_t Online:1;	//b7:在线标记 0=离线;1=在线
	} bit;
	uint8_t all;
} AlramFlag_t;

/* 报警类型 */
typedef union
{
	struct
	{
		uint16_t	DangerRegion:1;		//b0:危险区报警
		uint16_t	OverRegion:1;		//b1:超范围报警
		uint16_t	UnLeaveByTrain:1;	//b2:来车下道未下道报警
		uint16_t	UnLeave:1;			//b3:下道未下道报警
		uint16_t	UnEnter:1;			//b4:上道未上道报警
		uint16_t	Ahead:1;			//b5:提前上道报警
		uint16_t	Delay:1;			//b6:超时施工报警
		uint16_t	PosAbnormal:1;		//b7:定位异常报警
		uint16_t	:8;					//b8-b15:预留
	} bit;
	uint16_t all;
} AlramType_t;

/* 人员卡数据 共32字节 */
typedef struct
{
	UserCardID_t	ID;				//用户卡ID
	uint32_t		Latitude;		//纬度 单位=0.0000001度
	uint32_t		Longitude;		//经度 单位=0.0000001度
	uint8_t			PosQuality;		//定位质量
	uint8_t			Altitude[3];	//海拔高度
	AlramFlag_t		Alram;			//报警标记
	uint8_t			BatVoltage;		//电量
	AlramType_t		AlramType;		//报警类型
	uint8_t			FenceOrder;		//电子围栏编号
	uint8_t			WordTimeOrder;	//施工时间编号
	uint8_t			LedOrder;		//指示灯配置编号
	uint8_t			Reserver[6];	//预留6个字节
} CardData_t;


typedef struct
{
	HexDateTime_t	Time;			//时间（6）
	uint32_t		Latitude;		//纬度 单位=0.0000001度（4）
	uint32_t		Longitude;		//经度 单位=0.0000001度（4）
	uint8_t			PosQuality;		//定位质量（1）
	uint8_t			Altitude[3];	//海拔高度（3）
	AlramFlag_t		Alram;			//报警标记（1）
	uint8_t			BatVoltage;		//电量（1）
	AlramType_t		AlramType;		//报警类型（2）
	uint8_t			FenceOrder;		//电子围栏编号（1）
	uint8_t			WordTimeOrder;	//施工时间编号（1）
	uint8_t			Reserver0[1];	//预留1个字节
	uint8_t			SateOfNum;		//星数
	uint8_t			Reserver1[6];	//预留6个字节
} BaseData_t;
/* 坐标数据 */
typedef struct
{
	uint32_t		Latitude;	//纬度 单位=0.0000001度
	uint32_t		Longitude;	//经度 单位=0.0000001度
} Coordinate_t;

/* 电子围栏 */
/*  */
typedef struct
{
	uint8_t			Fence[123];	//电子围栏数据
	uint8_t			Order;		//编号
} EFence_t;

/* 施工时间 */
typedef struct
{
	HexDateTime_t	StartTime;	//施工开始时间
	HexDateTime_t	FinishTime;	//施工结束时间
	uint8_t			Order;		//施工时间编号
	uint8_t			Reserve[39];//预留
} WorkTime_t;

/* 指示灯配置 */
typedef struct
{
	uint8_t			LedSetup[27];//指示灯配置数据
	uint8_t			Order;		 //编号
} LedSetup_t;

/* 工作站参数变量 */
typedef struct
{
	uint8_t		PositionSta;	//收敛标记 0=未收敛;1=正在收敛;2=已收敛
	uint8_t		WorkingSta;		//上下道标记 0=无;1=上道;2=下道
	uint8_t		TrainingSta;	//来车标记 0=无;1=来车
	AlramFlag_t	AlramSta;		//报警标记
	
	uint8_t		Qulitaty;		//定位质量 
	uint32_t	Latitude;		//纬度 单位0.0000001度
	uint32_t	Longitude;		//经度 单位0.0000001度
	uint32_t	Altitude;		//海拔 单位cm
	
	uint8_t		BatVoltage;		//电池电量 b0-b6:电量*10 b7:充电标记 0=未充电;1=充电中
	
	uint32_t	BoardcastTimer;	//广播定时器
	uint8_t		BoardcastCnt;	//广播次数
	
	/* 回执编号 */
	uint8_t		ElecFenceOrder;	//电子围栏编号
	uint8_t		WorkTimeOrder;	//施工时间编号
	uint8_t		LedSetupOrder;	//指示灯配置编号
	
	/* 广播数据标记 */
	struct
	{
		uint8_t	Location:1;			//b0:定位信息	0=不广播;1=广播
		uint8_t	ElecFence:1;		//b1:电子围栏	0=不广播;1=广播
		uint8_t WorkTime:1;			//b2:施工时间	0=不广播;1=广播
		uint8_t LedSetup:1;			//b3:指示灯配置	0=不广播;1=广播
		uint8_t	:3;					//b4-b6:预留
		uint8_t	Emergent:1;			//b7:紧急信息	0=不广播;1=广播
	} BoardcastType;
	
	
} StationPara_t;

/* 当前状态 */
typedef struct
{
	uint8_t		CorsOK:1;		//b0:CORS状态 0=无;1=正常
	uint8_t		LoraOK:1;		//b1:Lora状态 0=无;1=正常
	uint8_t		DtuOK:1;		//b2:DTU状态 0=无;1=正常
	uint8_t		GPSPosOK:1;		//b3:GPS定位状态 0=无;1=定位
	uint8_t		RecvDiffOK:1;	//b4:接收差分数据状态 0=无;1=正常
	uint8_t		:3;				//预留
} StationStatus_t;




#endif /* __RYDW_STATIONCARD_H */
