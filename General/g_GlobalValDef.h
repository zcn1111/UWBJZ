#ifndef __G_GLOBALVALDEF_H
#define __G_GLOBALVALDEF_H

/* 引用头文件 */
#include "stm32l4xx.h"
#include <stdbool.h>

#include "BSP.h"
#include "DTU_Comm.h"


/******* 返回值状态定义 *******/
#define RET_OK					0		//正常
#define RET_ERR_READPHONENUM	-100	//读手机号错误
#define RET_ERR_DEFAULTIP		-101	//读IP错误，已返回默认IP

#define FLASH_PAGESIZE			256		//Flash页大小
/******* 变量结构体定义 *******/
/* 升级文件参数 */
typedef struct
{
	uint16_t	FileSize;	//文件大小
	uint16_t	Crc16;		//文件校验
} UpdateFilePara_t;

/*******************************************************************************/

/*0-- 产品标志类变量存放的地址 */

#define ADDR_EEPROM_IDCODE			0		    //主板ID号存放在第0字节,占用4个字节
//加CRC
/*******************************************************************************/

/* 16M FLASH 数据存放的地址分配 */

/* 	以字节为单位，在函数里面换算为字节地址 */

/*************************************第1-36个4K（系统）***********************************************************/                         
//UWB基站坐标和参数
#define FLASH_UWB_COORDINATES_START		0x00010000
#define FLASH_UWB_COORDINATES_LEN		512
#define FLASH_UWB_PARAMETER_START		0x00010200
#define FLASH_UWB_PARAMETER_LEN			512
/*手机号			*/
#define FLASH_MOBILE_START				0x23000
#define FLASH_MOBILE_LEN				1*256

/*IP端口号		   */
#define FLASH_IPPORT_START				(FLASH_MOBILE_START + FLASH_MOBILE_LEN)
#define FLASH_IPPORT_LEN				1*256

/*预留1             */
#define FLASH_RESERVED_START1 			(FLASH_IPPORT_START + FLASH_IPPORT_LEN)
#define FLASH_RESERVED_LEN1 			14*256                                       /***********1*************/ 



/*************************************第37-474个4K（人员定位）***********************************************************/
/* id */
#define FLASH_ID_START	                 0x24000
#define FLASH_ID_LEN		             16*256
/*设置工作模式 */
#define FLASH_MODE_START	             0x25000
#define FLASH_MODE_LEN		             16*256
/* 人员定位参数 */
#define FLASH_CORSACC__START             0x26000
#define FLASH_CORSACC_LEN		         1*256
/* 定位基准 */
#define FLASH_MARK_START                 0x27000
#define FLASH_MARK_LEN		             16*256
/*************************************第606个4K（版本号等）***********************************************************/

/* 004FH    升级变量    */
#define ADDR_EEPROM_UPDATA					0x0026D000//FLASH_RESERVED_START14 + FLASH_RESERVED_LEN14 //存放BIN文件大小,占用2个字节   //
#define ADDR_EEPROM_UPDATA_LEN      		2*256

#define ADDR_EEPROM_FIRMWARE_UPDATA_OFFSET  sizeof( NormalUpdateFileSize )          //存放固化BIN文件的偏移
#define ADDR_EEPROM_FIRMWARE_UPDATA_LEN		sizeof(FixUpdateFileSize)				//固化BIN文件的大小占2个字节

#define ADDR_BOOT_VERSION_OFFSET            (ADDR_EEPROM_FIRMWARE_UPDATA_OFFSET + sizeof(FixUpdateFileSize) )
#define ADDR_BOOT_VERSION_LEN               sizeof(BootFileSize)

#define ADDR_UPDATE_FLAG_OFFSET             (ADDR_BOOT_VERSION_OFFSET + sizeof(BootFileSize)) 
#define ADDR_UPDATE_FLAG_LEN                1

#define ADDR_VERSION_FLAG_OFFSET            (ADDR_UPDATE_FLAG_OFFSET+1)
#define ADDR_VERSION_FLAG_LEN               sizeof(ChangeVersion)

#define ADDR_UPDATE_VOICE_FLAG_OFFSET     	(ADDR_VERSION_FLAG_OFFSET+sizeof(ChangeVersion))
#define ADDR_UPDATE_VOICE_FLAG_LEN        	sizeof(VoiceFileSize)

#define ADDR_USER_VERSION_OFFSET           (ADDR_UPDATE_VOICE_FLAG_OFFSET + sizeof(VoiceFileSize)) 
#define ADDR_USER_VERSION_LEN              4

#define ADDR_UPDATE_DIFF_FLAG_OFFSET       (ADDR_USER_VERSION_OFFSET+4)
#define ADDR_UPDATE_DIFF_FLAG_LEN          sizeof(DiffUpdateFileSize)

/*预留15*/
#define FLASH_RESERVED_START15 			    (ADDR_EEPROM_UPDATA + ADDR_EEPROM_UPDATA_LEN)
#define FLASH_RESERVED_LEN15 			    14*256                                    /***********1*4K***************/ 

#define FLASH_VERSION_START				    0x0026E000
#define FLASH_VERSION_LEN				    1*256


/*人员定位基站打点数据*/
#define FLASH_RYDW_CONPOINT_START			0x00300000
#define FLASH_RYDW_CONPOINT_LEN				0x2A3000//(32*60*60*24)    


#define FLASH_VOICE_START	                10*1024*1024     
#define FLASH_VOICE_LEN	                    512*1024

/*boot程序区 100k*/
#define FLASH_BOOT_START                    11*1024*1024
#define FLASH_BOOT_LEN                      100*1024

/*程序空间 512k  */
#define FLASH_IAP_START1			        12*1024*1024
#define FLASH_IAP_LEN1				        512*1024

#define FLASH_FIXEDCODE_START	            0xC80000
#define FLASH_FIXEDCODE_LEN	                512*1024

//user差分升级差分包地址
#define FLASH_IAP_DIFFUSER_START1		    14*1024*1024		
#define FLASH_IAP_DIFFUSER_LEN1			    1024*1024

//user差分升级解压包地址
#define FLASH_IAP_DIFFUSER_START2		    13*1024*1024		
#define FLASH_IAP_DIFFUSER_LEN2			    1024*1024

//测试模式用变量
#define TESTMODE_FLASH_OFFSET               sizeof( GlobalSetPar )  
/******* 数据结构体定义 *******/
typedef struct
{
	uint8_t		    IDnum[7];	//地面号	
	uint16_t 		Crc16;	    //CRC16校验和
} ID_t;
/* 手机号组结构体定义 */
typedef struct
{
	MobileNum_t		DMnum;	//地面号
	MobileNum_t		WHnum;	//维护号
	MobileNum_t		DXnum;	//短信中心号	
	MobileNum_t		JSnum;	//接收号	
	MobileNum_t		BJnum;	//本机号
	
	uint16_t 		Crc16;	//CRC16校验和
} MobileNumGroups_t;

typedef struct
{

	IPAddr_t	UpdateIP;	//升级IP
	IPAddr_t	DumpIP;		//转储IP
	IPAddr_t	DataIP;		//数据IP	
	uint16_t	Crc16;		//CRC16校验和
} IPAddrGroup_t;
typedef struct
{
	uint8_t		    type;	//地面号	
	uint16_t 		Crc16;	    //CRC16校验和
}Mode_t;
typedef struct
{
    uint8_t Flag;
	AppDateTime Time; 
    uint16_t Crc16;
}ChangeVersion;
extern ChangeVersion g_ChangeVersionFlag;
typedef struct
{
	uint8_t g_ServerIPAddr[3][4];	 //0-远程升级IP，1-远程转储IP，2-数据IP
	uint16_t g_ServerPortNo[3]; 
	uint16_t Crc16;
}_SERVER_FLASH;
extern _SERVER_FLASH g_ServerInfo;
typedef struct
{
	uint8_t mobileno[5][6];	    
    uint16_t Crc16;
}_MOBILE_FLASH;
extern _MOBILE_FLASH g_MobileInFlash;
typedef struct 
{
    char     user[32];  // 鉴权用户名
    char     pass[20];  // 鉴权密码
	uint16_t Crc16;
}_CorsAcc;	
extern _CorsAcc	  g_CorsAccFlash;

#pragma pack(push, 1)  // 设置对齐到1字节
typedef struct 
{
	uint32_t      longitude;  // 纬度
	uint32_t      latitude;   // 经度
	uint32_t      fAltitude;   // 海拔高度
	uint8_t      res[8]; 
	uint16_t     Crc16;
}_Mark;	
extern  _Mark   g_MarkInfo;
#pragma pack(pop)  // 恢复默认对齐


/******* 外部变量声明 *******/
/* 手机号 */
extern const uint8_t TotalControlPhoneNum[6];		//总控
extern const uint8_t YFTotalControlPhoneNum[6];	//研发总控
//extern MobileNumGroups_t	g_MobileInFlash;
extern ID_t			    g_IDInFlash;
extern Mode_t              g_ModeInFlash;
/* IP地址 */
//extern IPAddrGroup_t		g_ServerInfo;

//extern UpdateFilePara_t	g_NormalUpdateFilePara;	//正常升级文件参数
extern UpdateFilePara_t	g_FixFilePara;			//固化文件参数
extern _CorsAcc            g_Cors;                 //自收敛CORS账号
/******* 外部函数声明 *******/
void InitDefaultIPGroup(IPAddrGroup_t *DefIP);

//int16_t WriteIPAndMobileToFlash(void);
//int16_t ReadIPAndMobileFromFlash(void);

void WriteNormalFileSizeToFlash(void);
bool ReadNormalFileSizeFromFlash(void);
void WriteDiffFileSizeToFlash(void);
bool ReadDiffFileSizeFromFlash(void);
void GetVersion(void);
int16_t ReadIDFromFlash(void);
int16_t WriteIDToFlash(void);
int16_t WriteModeToFlash(void);
int16_t ReadModeFromFlash(void);
void WriteDTUSysPar(void);
void ReadDTUSysPar(void);
bool ReadMobileNoFromFLASH(void);
void WriteMobileNoToFLASH(void);
void CmpIPAndPort(void);
void WriteServerInfoPar(void);
bool ReadServerInfoPar(void);
void WriteVoiceFileSizeToFlash(void);
bool ReadVoiceFileSizeFromFlash(void);
void ChangeVersionFormat(void);
void ReadCorsAccFromFlash(void);
void WriteCorsAccToFlash(void);
bool ReadMarkFromFlash(void);
void WriteMarkToFlash(void);
#endif

