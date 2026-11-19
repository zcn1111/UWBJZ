/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS_COMM_H
#define __GPS_COMM_H

#include "BSP.h"


/******* 宏定义 *******/
#define GPS_RX_BUFSIZE		2048
#define GPS_NMEA_BUFSIZE	1024

/******* GPS NMEA数据标记定义 *******/
#define GPSMASK_GGA		0x0001		//卫星定位信息
#define GPSMASK_GLL		0x0002		//地理位置信息
#define GPSMASK_GNS		0x0004		//定位数据输出
#define GPSMASK_GRS		0x0008		//解算的卫星残差
#define GPSMASK_GSA		0x0010		//参与计算的卫星信息
#define GPSMASK_GST		0x0020		//伪距观测误差
#define GPSMASK_GSV		0x0040		//可视卫星信息
#define GPSMASK_RMC		0x0080		//卫星定位信息
#define GPSMASK_VTG		0x0100		//地面航向与速度信息

#define GPSMASK_ALL		0xFFFF		//获取所有信息
#define GPSMASK_NONE	0x0000		//不获取信息

/******* 返回状态定义 *******/
typedef enum
{
	GPSRET_OK				= 0,	/* 0 = 成功 */
	GPSRET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	GPSRET_SEND_FAIL		= 2,	/* 2 = 发送失败 */
	GPSRET_RECV_FAIL		= 3,	/* 3 = 接收失败 */
	GPSRET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	GPSRET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	GPSRET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	GPSRET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	GPSRET_TYPE_ERR			= 8,	/* 8 = 类型错误 */
	GPSRET_INCOMPLETE_ERR	= 9,	/* 9 = 未完成 */
	GPSRET_TIMEOUT_ERR		= 10,	/* 10 = 超时 */
	GPSRET_NOMATCH_ERR		= 11,	/* 11 = 不匹配 */
	GPSRET_SURVEY_NOVAILD	= 200,	/* 200 = 观测状态 无效 */
	GPSRET_SURVEY_PROGING	= 201,	/* 201 = 观测状态 进行中 */
	GPSRET_SURVEY_OK		= 202, 	/* 202 = 观测状态 有效 */
} GpsResult;



/******* 数据结构体定义 *******/
typedef struct
{
	uint8_t UartOk;		/* 1表示串口数据接收正常，即可以收到GPS的命令字符串 */
	
	/* 从GGA 命令 : 时间、位置、定位类型  (位置信息可有 RMC命令获得) */

	/* 定位有效标志, 0:未定位 1:SPS模式，定位有效 2:差分，SPS模式，定位有效 3:PPS模式，定位有效 */
	uint8_t PositionOk;
	uint32_t Altitude;	/* 海拔高度，相对海平面多少米   x.x  */

	/* GLL：经度、纬度、UTC时间 */
	/* 样例数据：$GPGLL,3723.2475,N,12158.3416,W,161229.487,A*2C */

	/* GSA：GPS接收机操作模式，定位使用的卫星，DOP值 */
	/* 样例数据：$GPGSA,A,3,07,02,26,27,09,04,15, , , , , ,1.8,1.0,1.5*33 */
	uint8_t ModeAM;				/* M=手动（强制操作在2D或3D模式），A=自动 */
	uint8_t Mode2D3D;			/* 定位类型 1=未定位，2=2D定位，3=3D定位 */
	uint8_t SateID[12];			/* ID of 1st satellite used for fix */
	uint16_t PDOP;				/* 位置精度, 0.1米 */
	uint16_t HDOP;				/* 水平精度, 0.1米 */
	uint16_t VDOP;				/* 垂直精度, 0.1米 */

	/* GSV：可见GPS卫星信息、仰角、方位角、信噪比（SNR） */
	/*
		$GPGSV,2,1,07,07,79,048,42,02,51,062,43,26,36,256,42,27,27,138,42*71
		$GPGSV,2,2,07,09,23,313,42,04,19,159,41,15,12,041,42*41
	*/
	uint8_t ViewNumber;			/* 可见卫星个数 */
	uint8_t Elevation[12];		/* 仰角 elevation in degrees  度,最大90°*/
	uint16_t Azimuth[12];		/* 方位角 azimuth in degrees to true  度,0-359°*/
	uint8_t SNR[12];			/* 载噪比（C/No）  信噪比？ ， dBHz   范围0到99，没有跟踪时为空 */

	/* RMC：时间、日期、位置	、速度 */
	/* 样例数据：$GPRMC,161229.487,A,3723.2475,N,12158.3416,W,0.13,309.62,120598, ,*10 */
	uint16_t WeiDu_Du;			/* 纬度，度 */
	uint32_t WeiDu_Fen;			/* 纬度，分. 232475；  小数点后4位, 表示 23.2475 分 */
	char     NS;				/* 纬度半球N（北半球）或S（南半球） */

	uint16_t JingDu_Du;			/* 经度，单位度 */
	uint32_t JingDu_Fen;		/* 经度，单位度 */
	char     EW;				/* 经度半球E（东经）或W（西经） */

	uint16_t Year;				/* 日期 120598 ddmmyy */
	uint8_t  Month;
	uint8_t  Day;
	uint8_t  Hour;				/* UTC 时间。 hhmmss.sss */
	uint8_t  Min;				/* 分 */
	uint8_t  Sec;				/* 秒 */
	uint16_t mSec;				/* 毫秒 */
	char   TimeOk;				/* A=UTC时间数据有效；V=数据无效 */
	char	ModeInd;			/* 模式指示 A=自主模式, D=差分模式, E=估算模式, F=RTK浮点解, N=无效, R=RTK固定解 */

	/* VTG：地面速度信息 */
	/* 样例数据：$GPVTG,309.62,T, ,M,0.13,N,0.2,K*6E */
	uint16_t TrackDegTrue;		/* 以真北为参考基准的地面航向（000~359度，前面的0也将被传输） */
	uint16_t TrackDegMag;		/* 以磁北为参考基准的地面航向（000~359度，前面的0也将被传输） */
	uint32_t SpeedKnots;		/* 地面速率（000.0~999.9节，前面的0也将被传输） */
	uint32_t SpeedKM;			/* 地面速率（000.0~999.9节，前面的0也将被传输） */

}GPS_T;


typedef struct{
    int year; 
    int month; 
    int day;
    int hour;
    int minute;
    int second;
}DATE_TIME;

/* GPS定位标记 */
typedef union
{
	struct
	{
		uint8_t PositionActived:1;		//b0:定位标记 0=未定位;1=已定位
		uint8_t MileageActived:1;		//b1:里程有效 0=里程无效;1=里程有效
		uint8_t :2;						//b2-b3:预留
		uint8_t SatelliteNum:4;			//b4-b7:星数
	} bit;
	uint8_t all;
} GpsPositionFlag;

/* GPS坐标信息 WGS84度分格式 */
typedef struct
{
	double Latitude;	//经度原始值 度分格式 ddmm.mmmmmm
	double Longitude;	//纬度原始值 度分格式 dddmm.mmmmmm
	double Altitude;	//海拔高度原始值 单位米 mmmmm.m
} GpsWGS84DM_t;
typedef struct
{
	uint32_t		g_Latitude;          //纬度十六进制
	uint32_t		g_Longitude;         //经度十六进制
	uint32_t        g_Altitude;          //海拔高度
	uint16_t        g_Speed;
}Gpscoort_t;
extern Gpscoort_t g_gpsco;
/* GPS定位信息结构体 */
typedef  struct
{
	uint8_t			uActived;			//定位标记	u8	0=未定位;1=已定位
	uint8_t			uMode;				//定位模式	u8
	char			cMode;				//模式指示	chr	定位模式
	uint8_t			uSatelliteNum;		//星数		u8
	uint8_t			uEWFlag;			//东西经		u8	0=无;1=东经;2=西经
    double			dLatitudeInDegree;	//经度		dbl	单位度 格式 dd.ddddddd
	double			dLatitudeInDegMin;	//经度		dbl	单位度分 格式 ddmm.mmmmmm
	uint8_t			uNSFlag;			//南北纬		u8	0=无;1=北纬;2=南纬
    double			dLongitudeInDegree;	//纬度		dbl	单位度 格式 ddd.ddddddd
	double			dLongitudeInDegMin;	//纬度		dbl	单位度分 格式 dddmm.mmmmmm
	float			fAltitudeInMeter;	//海拔高度	flt	单位m
    float			fSpeedInKmh;		//速度		flt 单位Km/h
	float			fTrackTrueInDegree;	//地面航向	flt	单位度(从北向顺时针)
    HexDateTime_t	DateTimeInUTC;		//日期时间		单位UTC
	HexDateTime_t	DateTimeInLocate;	//日期时间		单位当地时间
	uint16_t		MilSecond;			//毫秒
}GpsLocationInfo_t;


/******* 外部变量定义 *******/

extern GPS_T g_tGPS;
extern uint8_t g_GPSSendFlag;
extern GpsLocationInfo_t	g_GPSInfo;
extern char		g_SingleGpsBuf[256];		//GPS单条信息
extern char		g_chGpsSaveInfo[384];
extern uint8_t		g_LatitudeBCD[4];
extern uint8_t		g_LongitudeBCD[5];
extern uint8_t		g_SatelliteNum;			//卫星数
extern uint8_t		g_PosQuality;			//信号质量
extern uint8_t      g_GpsGGABuf[255];
extern uint8_t      g_GpsGGABufLen;
extern char		g_GpsNMEAMessageBuf[GPS_NMEA_BUFSIZE];	//GPS-NMEA报文接收缓存


/******* 外部函数定义 *******/

void InitGPS(void);
//void GPSProcess(uint8_t *GPSchar, uint16_t Size);
int16_t GPSInfoAnlysis(uint8_t *strGPS, uint16_t Size);

void ConvGpsPositionInfo(GpsLocationInfo_t *GInfo, GPS_T *Gps);

void SendGPSData(void);

/* 初始化 */
GpsResult GPS_InitGPSInfo(GpsLocationInfo_t *GpsInfo);

/* 接收函数 */
GpsResult GPS_RecvBuf(char *OutBuf, uint16_t *BufLen, uint16_t Mask);


/* 数据解析 */
GpsResult GPS_AnlysisRMC(char *GpsStr, uint16_t StrLen, GpsLocationInfo_t *GpsInfo);
GpsResult GPS_AnlysisGGA(char *GpsStr, uint16_t StrLen, GpsLocationInfo_t *GpsInfo);
GpsResult GPS_GetNMEAInfo(uint8_t *InBuf, uint16_t BufLen, char *GpsStr, uint16_t *StrLen, uint32_t Mask);

#endif /* __APP_GPS_H */
