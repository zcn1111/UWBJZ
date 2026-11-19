#ifndef __UPDATE_H__
#define __UPDATE_H__

#include <stdbool.h>
#include "stm32l4xx_hal.h"
//#include "tool.h"
#include "DTU_Process.h"

#define    TOOL_UPDATE  0
#define    DTU_UPDATE   1

/********** 宏定义 **********/

//保存到Flash中的文件大小 
typedef struct
{
    uint16_t FileSize;
    uint16_t Crc16;
}UpdateFile;

#define DELAY_TOOL_UPDATE                     5           //s  客服工具下次索要升级数据包时间

#define DELAY1_DTU_UPDATE                     10          //s  等待10s切换至升级IP
#define DELAY2_DTU_UPDATE                     5           //s  等待5s后读取状态，检测是否连接成功
#define DELAY3_DTU_UPDATE                     20          //s  重新索要升级包的间隔时间

#define CONNECTIPADD_MAXNUM                   2           //连接升级IP最大次数
#define STATESCHECK_MAXNUM                    10           //切换IP后检测网络状态最大次数
#define NOTGETPACK_MAXNUM                     3           //索要升级包连续出错最大次数

/******* 数据结构体定义 *******/
#define UPD_STA_OK			(200)	//升级状态正常
#define UPD_STA_RECVCMD		(201)	//接收到升级命令
#define UPD_STA_RECVDAT		(202)	//接收到升级数据
#define UPD_STA_FINISH		(203)	//升级已结束

#define UPD_ERR_CHKSUM		(-201)	//校验和错误
#define UPD_ERR_PKTORDER	(-202)	//包序号错误
#define UPD_ERR_PKTHEAD		(-203)	//包头错误
#define UPD_ERR_TIMEOUT		(-204)	//等待超时
#define UPD_ERR_PROGTYPE	(-205)	//程序类型错误

/* 升级参数 */
//typedef struct
//{
//	uint8_t programtype;              //程序类型
//	uint8_t mainverno;                //主版本号
//	uint8_t secverno;	              //次版本号
//	uint16_t programpage;             //总包数
//	uint16_t dataLenToSave;           //已缓存并未写入flash的字节长度
//	uint16_t DownPacketNo;            //此刻应获取的包数
//	uint16_t SavePageNo;              //此刻应写入的页数
//	//注意：每一包包含496个数据，每一页包含512个数据，不要混淆这两个参数
//	TickTimerPara_t UpdateWaitDelay;          //客服工具重新索要数据包等待时间
//} UPDATAPROGRAM;
/* 升级参数 */
typedef enum
{
	UPD_PROG_NONE		=0,		/* 0 = 没有升级 */
	UPD_PROG_UPDATING	=1,		/* 1 = 正在升级 */
	UPD_PROG_FINISH		=2,		/* 2 = 升级结束 */
} UpdProg_e;
typedef struct
{
	uint8_t		ProgramType;		//程序类型
	uint8_t		MajorVersion;		//主版本号
	uint8_t		MinorVersion;		//次版本号
	uint16_t	TotalPktNbr;		//总包数
	uint16_t	RecvdPktOrder;		//已接收的包序号
	uint16_t	RequestPktOrder;	//请求的包序号
	
	uint8_t		*FlashSaveBuf;		//Flash写入缓存地址指针
	uint16_t	UnSavedDataLen;		//未保存到Flash的字节
	uint16_t	CurFlashPageNo;		//当前Flash页编号
	
	uint8_t		RetryWaiting;		//超时时间(s)
	uint8_t		RetryCnt;			//重试次数
	UpdProg_e	UpdateProgress;		//升级状态 0=未升级;1=正在升级;2=升级结束
} UpdatePara_t;

/* 升级状态 */
typedef union
{
	struct
	{	
		uint8_t Updata_DTU:1;          //使用DTU进行升级的标记
		uint8_t Updata_Tool:1;         //使用客服工具进行升级
		uint8_t UpgradeInProgress:1;   //正在升级中
		uint8_t UpdateOver:1;          //已收到全部的数据包
		uint8_t UpdateNormal:1;        //接受正常的升级数据
		uint8_t UpdateMistake:1;       //接收到异常的升级数据
		uint8_t AskAgain:1;            //出现异常包再次索要    注意：此功能仅用于DTU升级
	} bit;
	uint8_t all;
}UPDATESTATES;           

//typedef struct
//{
//	uint8_t HostUpdateFlag;     //=1正在修改IP ，=2已修改至升级IP
//	uint8_t 
//	
//}UpdateByDTU;
/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */


/******* 外部变量声明 *******/
extern UpdatePara_t g_UpdatePara;

extern UPDATESTATES UpdateStates;                  //客服工具升级的状态
//extern UPDATAPROGRAM UpdateData;                   //客服工具升级的数据
extern UpdateFile NormalUpdateFileSize;            //正常文件大小
extern UpdateFile FixUpdateFileSize;     //固件文件大小
extern UpdateFile BootFileSize;          //agj

extern uint8_t UpdateBuf[1488];                  //客服工具升级包数据储存 
extern UpdateFile 	DiffUpdateFileSize;			//增量升级主程序文件大小

extern uint8_t       g_UpdateVoiceTypeFlag;  //升级语音程序标志
extern uint8_t       g_UpFlag;
extern uint8_t       g_ShouldUpdateVoiceFlag;
extern UpdateFile    VoiceFileSize; 

int16_t Update_InitUpdatePara(UpdatePara_t *Para);
int16_t Update_RecvData(UpdatePara_t *Para, uint8_t *DatBuf);
int16_t Update_GenerateRequest(UpdatePara_t *Para, uint8_t *DatBuf);
int16_t Update_GenerateFinish(UpdatePara_t *Para, uint8_t *DatBuf);



void ToolUpdateRelay(UPDATESTATES *state);
bool UpdateHeadAnaly(uint8_t * data, bool mode);
bool UpdateBodyAnaly(uint8_t* data, bool mode);
void Update_Quest_Tool(void);
void Update_Quest_DTU(MobileNum_t PhoneNum);

#endif
