/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: g_GlobalValDef.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 全局变量定义
其他说明:
功能列表:
******************************************************************/

#include "BSP.h"
#include "main.h"

#include "g_GlobalValDef.h"
//#include "DTU_Comm.h"
#include "g_func.h"
#include "Update.h"
//#include "Drv_W25Qxx.h"
#include "Drv_W25Qxx_QSPI.h"
#include "string.h"
#include "DTU_Process.h"
#include "DTU_BulkData.h"
/********* 变量定义 **********/
/* 总控号 */
const uint8_t TotalControlPhoneNum[6]={0x13,0x75,0x31,0x24,0x37,0x2F};		//总控
const uint8_t YFTotalControlPhoneNum[6]={0x13,0x75,0x31,0x20,0x58,0x3F};	//研发总控
const uint8_t SMSCenterPhoneNum[6]={0x13,0x80,0x03,0x51,0x50,0x0F};			//短信中心号
/* 手机号组 */
_MOBILE_FLASH       g_MobileInFlash;
//MobileNumGroups_t	g_MobileInFlash;
ID_t			    g_IDInFlash;
Mode_t              g_ModeInFlash;
_Mark               g_MarkInfo;
/* IP地址组 */
//IPAddrGroup_t		g_ServerInfo;
_SERVER_FLASH       g_ServerInfo;
ChangeVersion       g_ChangeVersionFlag;
//UpdateFilePara_t	g_NormalUpdateFilePara;	//正常升级文件参数
UpdateFilePara_t	g_FixFilePara;			//固化文件参数
_CorsAcc	        g_CorsAccFlash;
_CorsAcc            g_Cors;                 //自收敛CORS账号
uint8_t g_CommanBuf[1024];  


void WriteDTUSysPar(void)
{
	g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2);
	g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
	W25qxx_EraseSector(FLASH_MOBILE_START/4096);    
	W25qxx_WritePages((uint8_t*)&g_ServerInfo,FLASH_IPPORT_START,sizeof(g_ServerInfo));
	W25qxx_WritePages((uint8_t*)&g_MobileInFlash,FLASH_MOBILE_START,sizeof(g_MobileInFlash));
}
void ReadDTUSysPar(void)
{
	uint8_t err = 0,i;	
	uint8_t cmpbuffer[2][4];
    memset(cmpbuffer[0],0,sizeof(cmpbuffer[0]));
    memset(cmpbuffer[1],0xff,sizeof(cmpbuffer[1]));	
	W25qxx_ReadBytes((uint8_t*)&g_MobileInFlash,FLASH_MOBILE_START,sizeof(g_MobileInFlash));
	W25qxx_ReadBytes((uint8_t*)&g_ServerInfo,FLASH_IPPORT_START,sizeof(g_ServerInfo));
	if(g_MobileInFlash.Crc16 != CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
	{
		memset(g_MobileInFlash.mobileno,0,sizeof(g_MobileInFlash.mobileno));    //agj add
        g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2);
        g_GetMyMobileOKFlag = false;        //获取本机号标识
		err = 1;
	}
	if( g_ServerInfo.Crc16 != CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2))
	{
        memset(g_ServerInfo.g_ServerIPAddr,0,sizeof(g_ServerInfo.g_ServerIPAddr));
		memset(g_ServerInfo.g_ServerPortNo,0,sizeof(g_ServerInfo.g_ServerPortNo));
		g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
		err = 1;
	}
	for(i=0;i<3;i++)
    {
        if( memcmp(g_ServerInfo.g_ServerIPAddr[i],cmpbuffer[0],sizeof(g_ServerInfo.g_ServerIPAddr[i])) == 0 || 
            memcmp(g_ServerInfo.g_ServerIPAddr[i],cmpbuffer[1],sizeof(g_ServerInfo.g_ServerIPAddr[i])) == 0 ||
            g_ServerInfo.g_ServerPortNo[i] == 0 || g_ServerInfo.g_ServerPortNo[i] == 0xff)
        {
            memcpy(g_ServerInfo.g_ServerIPAddr[i],g_IPBuffer[i],sizeof(g_ServerInfo.g_ServerIPAddr[i]));
            g_ServerInfo.g_ServerPortNo[i] = g_PortNo[i];
			err = 1;
        }
    }	
	if(err == 1)
	{
		g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
		WriteDTUSysPar();
	}
}

/**
 * @brief	存本机号
 *
 * @param	void
 *
 * @return  void
 */
void WriteMobileNoToFLASH(void)
{
	g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash, sizeof(g_MobileInFlash)-2);	
	WriteDTUSysPar();
}

/**
 * @brief	读取本机号
 *
 * @param	void
 *
 * @return  void
 */
bool ReadMobileNoFromFLASH(void)
{
    uint8_t cmpbuf[5];

	memset(cmpbuf,0,sizeof(cmpbuf));
   
    W25qxx_ReadBytes((uint8_t*)&g_MobileInFlash,FLASH_MOBILE_START,sizeof(g_MobileInFlash));
	if(g_MobileInFlash.Crc16 != CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
	{
		memset(g_MobileInFlash.mobileno,0,sizeof(g_MobileInFlash.mobileno));   
        g_MobileInFlash.Crc16 = CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2);
		WriteDTUSysPar();
        g_GetMyMobileOKFlag = false;        //获取本机号标识
        return false;
	}
    else
    {
        if(CheckNumIsBCD(g_MobileInFlash.mobileno[4],5) == true && memcmp(g_MobileInFlash.mobileno[4],cmpbuf,5)!=0)
            g_GetMyMobileOKFlag = true;
    }
    return true;
}
/**
 * @brief	存服务器信息
 *
 * @param	void
 *
 * @return  void
 */
void WriteServerInfoPar(void)
{
	g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
	WriteDTUSysPar();
}

/**
 * @brief	读取服务器信息
 *
 * @param	void
 *
 * @return  void
 */
bool ReadServerInfoPar(void)    
{    
	W25qxx_ReadBytes((uint8_t*)&g_ServerInfo,FLASH_IPPORT_START,sizeof(g_ServerInfo));
	if( g_ServerInfo.Crc16 != CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2))
	{
        memset(g_ServerInfo.g_ServerIPAddr,0,sizeof(g_ServerInfo.g_ServerIPAddr));
		memset(g_ServerInfo.g_ServerPortNo,0,sizeof(g_ServerInfo.g_ServerPortNo));
		g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
		WriteDTUSysPar();
        return false;
	}
    return true;
}
///******************************************************************
//函数名称: void InitDefaultIPGroup
//功能描述: 初始化默认IP
//参数输入: 
//参数输出: IPAddrGroup_t	*IPAddr
//返 回 值: 无
//其    他: 无
//******************************************************************/
//void InitDefaultIPGroup(IPAddrGroup_t *IPAddr)
//{
//	uint16_t crc16;
//	ConvIPStrToArray("221.204.213.61:22042", IPAddr->DataIP.all);
//	ConvIPStrToArray("211.142.31.152:2021", IPAddr->UpdateIP.all);
//	ConvIPStrToArray("211.142.31.152:2029", IPAddr->DumpIP.all);
//	crc16 = CalcCRC16((uint8_t*)&IPAddr, sizeof(IPAddrGroup_t) - 4);		//计算CRC16
//	IPAddr->Crc16 = crc16;
//	
//}

///******************************************************************
//函数名称: void InitDefaultIPGroup
//功能描述: 初始化默认IP
//参数输入: 
//参数输出: IPAddrGroup_t	*IPAddr
//返 回 值: 无
//其    他: 无
//******************************************************************/
//void InitDefaultMobileNumGroup(MobileNumGroups_t *MobileNum)
//{
//	/* 地面号 研发总控 */
//	memcpy(MobileNum->DMnum.MobNum, YFTotalControlPhoneNum, 6);
//	MobileNum->DMnum.SendMode.all = 0;
//	MobileNum->DMnum.SendMode.bit.SMScode = 2;			//默认8bit发送

//	/* 维护号 研发总控 */
//	memcpy(MobileNum->WHnum.MobNum, YFTotalControlPhoneNum, 6);
//	MobileNum->WHnum.SendMode.all = 0;
//	MobileNum->WHnum.SendMode.bit.SMScode = 2;			//默认8bit发送
//	
//	/* 短信中心号 */
//	memcpy(MobileNum->DXnum.MobNum, SMSCenterPhoneNum, 6);
//	MobileNum->DXnum.SendMode.all = 0;
//	
//	/* 接收号 总控 */
//	memcpy(MobileNum->JSnum.MobNum, TotalControlPhoneNum, 6);
//	g_MobileInFlash.JSnum.SendMode.all = 0;
//	g_MobileInFlash.JSnum.SendMode.bit.SMScode = 2;			//默认8bit发送
//}

///******************************************************************
//函数名称: int16_t WriteIPAndMobileToFlash
//功能描述: 写IP和本机号到Flash
//参数输入: 
//参数输出: 
//返 回 值: int16_t 错误代码
//其    他: 无
//******************************************************************/
//int16_t WriteIPAndMobileToFlash(void)
//{
//	uint16_t crc16;
//	
//	crc16 = CalcCRC16((uint8_t*)&g_MobileInFlash, sizeof(g_MobileInFlash) - 2);			//手机号定义Flash位置为扇区头，需要先执行
//	g_MobileInFlash.Crc16 = crc16;
//	FlashWriteData((uint8_t*)&g_MobileInFlash, FLASH_MOBILE_START, sizeof(g_MobileInFlash));
////	W25q_Write_N((uint8_t*)&g_MobileInFlash, FLASH_MOBILE_START, sizeof(g_MobileInFlash));
//	
//	crc16 = CalcCRC16((uint8_t*)&g_ServerInfo, sizeof(g_ServerInfo) - 4);
//	g_ServerInfo.Crc16 = crc16;
//	FlashWriteData((uint8_t*)&g_ServerInfo, FLASH_IPPORT_START, sizeof(g_ServerInfo));		//写IP地址
////	W25q_Write_N((uint8_t*)&g_ServerInfo, FLASH_IPPORT_START, sizeof(g_ServerInfo));
//	
//	return RET_OK;
//}

///******************************************************************
//函数名称: int16_t ReadIPAndMobileFromFlash
//功能描述: 从Flash读IP和本机号
//参数输入: 
//参数输出: 
//返 回 值: 无
//其    他: 无
//******************************************************************/
//int16_t ReadIPAndMobileFromFlash(void)
//{
//	uint8_t err = 0;
//	int16_t ret=RET_OK;
//	
//	/* 读Flash中的手机号和IP */
//	W25qxx_ReadBytes((uint8_t*)&g_MobileInFlash, FLASH_MOBILE_START, sizeof(g_MobileInFlash));
//	W25qxx_ReadBytes((uint8_t*)&g_ServerInfo, FLASH_IPPORT_START, sizeof(g_ServerInfo));
//	
//	/* 检查手机号校验和 */
//	if (g_MobileInFlash.Crc16 != CalcCRC16((uint8_t*)&g_MobileInFlash, sizeof(g_MobileInFlash) - 2))
//	{
//		memset((uint8_t*)&g_MobileInFlash, 0x00, sizeof(g_MobileInFlash));    //agj add
//        g_MobileInFlash.Crc16 = CalcCRC16((uint8_t*)&g_MobileInFlash, sizeof(g_MobileInFlash) - 2);
////		InitDefaultMobileNumGroup(&g_MobileInFlash);
////        g_DtuPara.DtuNetFlag.bit.ObtainedPhoneNum = 0;        											//本机号不正确，重新获取手机号
//		err = 1;
//		ret = RET_ERR_READPHONENUM;
//	}
//	
//	/* 检查IP校验和 */
//	if( g_ServerInfo.Crc16 != CalcCRC16((uint8_t*)&g_ServerInfo, sizeof(g_ServerInfo) - 4))
//	{
//        memset((uint8_t*)&g_ServerInfo, 0x00, sizeof(g_ServerInfo));
//		InitDefaultIPGroup(&g_ServerInfo);														//IP不正确 设置为默认IP
//		err = 1;
//		ret = RET_ERR_DEFAULTIP;
//	}
//	
//	/* 重新写入本机号和IP */
//	if(err == 1)
//	{
//		WriteIPAndMobileToFlash();
//	}
//	
//	return ret;
//	
//}





/******************************************************************
函数名称: int16_t WriteIDToFlash(void)
功能描述: 写IP和本机号到Flash
参数输入: 
参数输出: 
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t WriteIDToFlash(void)
{
	uint16_t crc16;	
	crc16 = CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2);			//手机号定义Flash位置为扇区头，需要先执行
	g_IDInFlash.Crc16 = crc16;
	FlashWriteData((uint8_t*)&g_IDInFlash, FLASH_ID_START, sizeof(g_IDInFlash));	
	return RET_OK;
}

/******************************************************************
函数名称: int16_t ReadIPAndMobileFromFlash
功能描述: 从Flash读IP和本机号
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
int16_t ReadIDFromFlash(void)
{
	uint8_t err = 0;
	int16_t ret=RET_OK;
	uint8_t cmpipbuf[2][7];												
	memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
	memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));	
	/* 读Flash中的手机号和IP */
	W25qxx_ReadBytes((uint8_t*)&g_IDInFlash, FLASH_ID_START, sizeof(g_IDInFlash));
	
	/* 检查手机号校验和 */		
	if( memcmp(g_IDInFlash.IDnum,cmpipbuf[0],7) == 0 || 
		memcmp(g_IDInFlash.IDnum,cmpipbuf[1],7) == 0 )
	{
		g_IDInFlash.IDnum[0]= 0xFF;
		g_IDInFlash.IDnum[1]= 0xF0 | (g_MobileInFlash.mobileno[4][0] >> 4);
		g_IDInFlash.IDnum[2] = ((g_MobileInFlash.mobileno[4][0] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][1] >> 4);		
		g_IDInFlash.IDnum[3] = ((g_MobileInFlash.mobileno[4][1] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][2] >> 4);	
		g_IDInFlash.IDnum[4] = ((g_MobileInFlash.mobileno[4][2] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][3] >> 4);	
		g_IDInFlash.IDnum[5] = ((g_MobileInFlash.mobileno[4][3] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][4] >> 4);	
		g_IDInFlash.IDnum[6] = ((g_MobileInFlash.mobileno[4][4] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][5] >> 4);	
		g_IDInFlash.Crc16 = CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2);
		WriteIDToFlash();
	}
	else if (g_IDInFlash.Crc16 != CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2))
	{
		g_IDInFlash.IDnum[0]= 0xFF;
		g_IDInFlash.IDnum[1]= 0xF0 | (g_MobileInFlash.mobileno[4][0] >> 4);
		g_IDInFlash.IDnum[2] = ((g_MobileInFlash.mobileno[4][0] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][1] >> 4);		
		g_IDInFlash.IDnum[3] = ((g_MobileInFlash.mobileno[4][1] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][2] >> 4);	
		g_IDInFlash.IDnum[4] = ((g_MobileInFlash.mobileno[4][2] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][3] >> 4);	
		g_IDInFlash.IDnum[5] = ((g_MobileInFlash.mobileno[4][3] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][4] >> 4);	
		g_IDInFlash.IDnum[6] = ((g_MobileInFlash.mobileno[4][4] & 0x0F) << 4) | (g_MobileInFlash.mobileno[4][5] >> 4);	
        g_IDInFlash.Crc16 = CalcCRC16((uint8_t*)&g_IDInFlash, sizeof(g_IDInFlash) - 2);
		WriteIDToFlash();
	}	
	return true;
	
}



/******************************************************************
函数名称: int16_t WriteModeToFlash(void)
功能描述: 写IP和本机号到Flash
参数输入: 
参数输出: 
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t WriteModeToFlash(void)
{
	uint16_t crc16;	
	crc16 = CalcCRC16((uint8_t*)&g_ModeInFlash, sizeof(g_ModeInFlash) - 2);			//手机号定义Flash位置为扇区头，需要先执行
	g_ModeInFlash.Crc16 = crc16;
	FlashWriteData((uint8_t*)&g_ModeInFlash, FLASH_MODE_START, sizeof(g_ModeInFlash));	
	return RET_OK;
}

/******************************************************************
函数名称: int16_t ReadIPAndMobileFromFlash
功能描述: 从Flash读IP和本机号
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
int16_t ReadModeFromFlash(void)
{
	uint8_t err = 0;
	int16_t ret=RET_OK;
	
	/* 读Flash中的手机号和IP */
	W25qxx_ReadBytes((uint8_t*)&g_ModeInFlash, FLASH_MODE_START, sizeof(g_ModeInFlash));
	
	/* 检查手机号校验和 */
	if (g_ModeInFlash.Crc16 != CalcCRC16((uint8_t*)&g_ModeInFlash, sizeof(g_ModeInFlash) - 2))
	{
		memset((uint8_t*)&g_ModeInFlash, 0x00, sizeof(g_ModeInFlash));    //agj add
        g_ModeInFlash.Crc16 = CalcCRC16((uint8_t*)&g_ModeInFlash, sizeof(g_ModeInFlash) - 2);
		WriteModeToFlash();
	}	
	return true;
	
}
/******************************************************************
函数名称: int16_t WriteCorsAccToFlash(void)
功能描述: 定位间隔到Flash
参数输入: 
参数输出: 
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
void WriteCorsAccToFlash(void)
{
	uint16_t crc16;	
	crc16 = CRC16((uint8_t*)&g_CorsAccFlash, sizeof(g_CorsAccFlash) - 2);			//手机号定义Flash位置为扇区头，需要先执行
	g_CorsAccFlash.Crc16 = crc16;
	W25qxx_EraseSector(FLASH_CORSACC__START/4096);
	W25qxx_WritePages((uint8_t*)&g_CorsAccFlash, FLASH_CORSACC__START ,  sizeof(g_CorsAccFlash));	
}
//const char* username = "qxtjtg005";     // 鉴权用户名
//const char* password = "3b3d8f6";     // 鉴权密码
/******************************************************************
函数名称: int16_t ReadIPAndMobileFromFlash
功能描述: 从Flash读定位间隔
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReadCorsAccFromFlash(void)
{
	/* 读Flash中的发送间隔 */
	W25qxx_ReadBytes((uint8_t*)&g_CorsAccFlash, FLASH_CORSACC__START, sizeof(g_CorsAccFlash));
	
	/* 检查发送间隔校验和 */
	if (g_CorsAccFlash.Crc16 != CRC16((uint8_t*)&g_CorsAccFlash, sizeof(g_CorsAccFlash) - 2))
	{
		memset((uint8_t*)&g_CorsAccFlash, 0xFF, sizeof(g_CorsAccFlash));    //agj add
        g_CorsAccFlash.Crc16 = CRC16((uint8_t*)&g_CorsAccFlash, sizeof(g_CorsAccFlash) - 2);
		WriteCorsAccToFlash();
	}	
}
/******************************************************************
函数名称: void WriteNormalFileSizeToFlash
功能描述: 更新程序参数写入到Flash
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteNormalFileSizeToFlash(void)
{
	FlashReadData(g_CommanBuf , ADDR_EEPROM_UPDATA , 512 , 0);
	NormalUpdateFileSize.Crc16 = CalcCRC16((uint8_t*)&NormalUpdateFileSize, sizeof(NormalUpdateFileSize) - 2);
	memcpy(g_CommanBuf,( uint8_t* )&NormalUpdateFileSize,sizeof(NormalUpdateFileSize));
	W25qxx_EraseSector(ADDR_EEPROM_UPDATA / w25qxx.SectorSize);
	W25qxx_WritePages(g_CommanBuf, ADDR_EEPROM_UPDATA , 512);
}

/******************************************************************
函数名称: Bool ReadNormalFileSizeFromFlash(void)
功能描述: 从Flash读更新程序参数
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
bool ReadNormalFileSizeFromFlash(void)
{
	uint16_t crc;
	
	W25qxx_ReadBytes((uint8_t*)&NormalUpdateFileSize, ADDR_EEPROM_UPDATA, sizeof(NormalUpdateFileSize));
	crc = CalcCRC16((uint8_t*)&NormalUpdateFileSize, sizeof(NormalUpdateFileSize) - 2);
	if (crc != NormalUpdateFileSize.Crc16)
	{
		NormalUpdateFileSize.FileSize = 0;
		WriteNormalFileSizeToFlash();
		return false;
	}
	return true;
}
/******************************************************************
函数名称: Bool WriteDiffFileSizeToFlash(void)
功能描述:  写入增量升级文件大小
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void WriteDiffFileSizeToFlash(void)
{
    FlashReadData(g_CommanBuf , ADDR_EEPROM_UPDATA , 512 , 0);
	DiffUpdateFileSize.Crc16 = CRC16( ( uint8_t* )&DiffUpdateFileSize , sizeof( DiffUpdateFileSize ) - 2 );		
	memcpy(g_CommanBuf + ADDR_UPDATE_DIFF_FLAG_OFFSET,( uint8_t* )&DiffUpdateFileSize,sizeof(DiffUpdateFileSize));
	W25qxx_EraseSector(ADDR_EEPROM_UPDATA / w25qxx.SectorSize);
	FlashWriteData((uint8_t*)&g_CommanBuf, ADDR_EEPROM_UPDATA , 512);
}

/******************************************************************
函数名称: Bool ReadDiffFileSizeFromFlash(void)
功能描述: 从Flash读增量升级文件大小的函数
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
bool ReadDiffFileSizeFromFlash(void)
{
	W25qxx_ReadBytes((uint8_t*)&DiffUpdateFileSize,ADDR_EEPROM_UPDATA+ADDR_UPDATE_DIFF_FLAG_OFFSET,sizeof(DiffUpdateFileSize));

	if(DiffUpdateFileSize.Crc16 != CRC16((uint8_t*)&DiffUpdateFileSize,sizeof(DiffUpdateFileSize)-2))
	{
		DiffUpdateFileSize.FileSize = 0;
        WriteDiffFileSizeToFlash();
        return false;
	}
    return true;
}

/******************************************************************
函数名称: void WriteVoiceFileSizeToFlash
功能描述: 更新程序参数写入到Flash
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteVoiceFileSizeToFlash(void)
{
	FlashReadData(g_CommanBuf , ADDR_EEPROM_UPDATA , 512 , 0);
	VoiceFileSize.Crc16 = CalcCRC16((uint8_t*)&VoiceFileSize, sizeof(VoiceFileSize) - 2);
	memcpy(g_CommanBuf + ADDR_UPDATE_VOICE_FLAG_OFFSET,( uint8_t* )&VoiceFileSize,sizeof(VoiceFileSize));	
	W25qxx_EraseSector(ADDR_EEPROM_UPDATA / w25qxx.SectorSize);
	W25qxx_WritePages(g_CommanBuf, ADDR_EEPROM_UPDATA , 512);
}

/******************************************************************
函数名称: Bool ReadVoiceFileSizeFromFlash(void)
功能描述: 从Flash读更新程序参数
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
bool ReadVoiceFileSizeFromFlash(void)
{
	uint16_t crc;
	
	W25qxx_ReadBytes((uint8_t*)&VoiceFileSize,ADDR_EEPROM_UPDATA + ADDR_UPDATE_VOICE_FLAG_OFFSET, sizeof(VoiceFileSize));
	crc = CalcCRC16((uint8_t*)&VoiceFileSize, sizeof(VoiceFileSize) - 2);
	if (crc != VoiceFileSize.Crc16)
	{
		VoiceFileSize.FileSize = 0;
		WriteVoiceFileSizeToFlash();
		return false;
	}
	return true;
}
/******************************************************************
函数名称: Bool ChangeVersionFormat(void)
功能描述: 转换版本号格式
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void ChangeVersionFormat(void)
{
	uint8_t i,flag;
	int8_t temp[7];
	
	memset(temp,0,7);
	memset(g_VersionAscii,0,sizeof(g_VersionAscii));
	memset(g_VersionHex,0,sizeof(g_VersionHex));
	
	memcpy(temp,g_Version,sizeof(g_Version));
	
	W25qxx_ReadBytes((uint8_t*)&g_VersioninFlash,FLASH_VERSION_START,sizeof(g_VersioninFlash));	//读boot版本号
	if(g_VersioninFlash.bootversion[0] == 0xFF && g_VersioninFlash.bootversion[1] == 0xFF && g_VersioninFlash.bootversion[2] == 0xFF && g_VersioninFlash.bootversion[3] == 0xFF)
	{
		memset(g_VersioninFlash.bootversion,0x30,4);
		flag = 1;
	}
	
	for(i=0;i<7;i++)    
	{
		if(temp[i] == '.')
		{
			if(i == 3)
			{
				g_VersionAscii[0] = temp[1];
				g_VersionAscii[1] = temp[2];
			}
			else if(i == 2)
			{
				g_VersionAscii[0] = 0x30;
				g_VersionAscii[1] = temp[1];
			}
			g_VersionAscii[2] = temp[i+1];
			if(temp[i+2] == 0x00)
				g_VersionAscii[3] = 0x30;
			else
				g_VersionAscii[3] = temp[i+2];
		}
	}
	g_VersionHex[0] = Asc2Hex(g_VersionAscii[0])*10+Asc2Hex(g_VersionAscii[1]);
	g_VersionHex[1] = Asc2Hex(g_VersionAscii[2])*10+Asc2Hex(g_VersionAscii[3]);
	if(memcmp(g_VersionAscii,g_VersioninFlash.userversion,4) != 0)
	{
		memcpy(g_VersioninFlash.userversion,g_VersionAscii,4);
		flag = 1;
	}
	if(flag == 1)
	{
		g_VersioninFlash.Crc16 = CRC16((uint8_t*)&g_VersioninFlash,sizeof(g_VersioninFlash)-2);
		W25qxx_EraseSector(FLASH_VERSION_START/w25qxx.SectorSize);
		W25qxx_WriteBytes((uint8_t*)&g_VersioninFlash,FLASH_VERSION_START,sizeof(g_VersioninFlash));
	}
}







/******************************************************************
函数名称: void WriteMarkToFlash
功能描述: 基准位置写入到Flash
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteMarkToFlash(void)
{
	g_MarkInfo.Crc16 = CalcCRC16((uint8_t*)&g_MarkInfo, sizeof(g_MarkInfo) - 2);
	W25qxx_EraseSector( FLASH_MARK_START/ w25qxx.SectorSize);
	W25qxx_WritePages((uint8_t*)&g_MarkInfo, FLASH_MARK_START , sizeof(g_MarkInfo));
}

/******************************************************************
函数名称: bool ReadMarkFromFlash(void)
功能描述: 从Flash读基准位置
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
bool ReadMarkFromFlash(void)
{
	uint16_t crc;
	
	W25qxx_ReadBytes((uint8_t*)&g_MarkInfo,FLASH_MARK_START, sizeof(g_MarkInfo));
	crc = CalcCRC16((uint8_t*)&g_MarkInfo, (sizeof(g_MarkInfo)-2));
	if (crc != g_MarkInfo.Crc16)
	{
		g_MarkInfo.latitude = 0xFFFFFFFF;
		g_MarkInfo.longitude = 0xFFFFFFFF;
		g_MarkInfo.fAltitude = 0xFFFFFFFF;
		memset(g_MarkInfo.res,0x00,8);
		WriteMarkToFlash();
		return false;
	}
	return true;
}