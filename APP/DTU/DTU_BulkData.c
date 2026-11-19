/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: DTU_BulkData.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: DTU批量数据收发函数
其他说明:
功能列表:
******************************************************************/

#include "DTU_Comm.h"

#include "g_func.h"
#include "g_GlobalValDef.h"
#include "string.h"
#include "iap.h"
#include "BSP.h"
#include "Drv_W25Qxx_QSPI.h"
#include "RYDW_StationCard.h"
#include "RYDW_Postion.h"
#include "DTU_BulkData.h"
#include "DTU_Process.h"
#include "APP_STORE.h"
#include "RYDW_Init.h"
#include "RTK_Comm.h"
#include "RYDW_StationCard.h"
#include "Update.h"
#include "main.h"
#include "RYDW_Locate.h"
#include "ssd1306.h"
#include "app_dtu.h"
/********* 变量定义 **********/
uint8_t	g_BulkRecvBuf[4096];
uint32_t g_SendPunCoorDelay;
RespServ_t      g_ReplyData;
/********* 函数声明 **********/
static void DTU_DispBulkHeaderInfo(BulkHeader_t *Header);
static void DTU_DispBulkRecordInfo(BulkRecord_t Record);


static int16_t DTU_SendBulkData(BulkHeader_t *Header, uint8_t *Record, uint16_t RecLen);
static int16_t DTU_RecvBulkData(BulkHeader_t *Header, uint8_t *Record, uint16_t *RecLen);

void InsertDARecord(void);
uint8_t g_SendDiffMark=1;             //基站是否发送差分数据标记
int8_t g_ResendTimes;
uint8_t g_ResendFlag;
uint32_t g_ResendDelay = 0;
uint8_t g_FileSendDataToDtu=0;
uint32_t g_tmrUploadBasePosDelay;	//基站定位上传定时器
uint8_t g_BasePosUploaded=0;		//基站定位上传标识
//uint8_t g_PunCoorSendFlag=0;          //打点数据发送标记
//uint8_t g_DtuSendOk=0;                //DTU发送成功
//uint32_t g_DTUSendDataDelay;
//uint8_t 	g_NetAskTime;
//uint8_t 	g_NetAskTimeCnt;
//uint8_t g_AskAuthorityFlag;       /* 收到返回控制权限的数据包 */
//uint8_t 	g_NetAskIPAndPortFlag;
//uint8_t		g_AskIDFlag;
//uint8_t     g_AskParFlag = 0;
//uint8_t     g_InitAskParFlag = 0;
//bool 		g_Send5ASuc;
//bool 		g_SendD6Suc;
//uint32_t 	g_SendD6BagNo;	//发送D6数据包序号
//uint32_t 	g_RecD6BagNo;	//接收D6数据包序号
//uint8_t 	g_UpdateCodeTypeErrorFlag;		//程序程序类型错误标
//uint8_t 	g_D6Phone[6];
//uint32_t 	g_Send5ABagNo;	//发送正常数据包序号
//uint8_t g_ClearFlashFlag;			//清空FLASH  1-整片擦除，2-指定地址删除 
//uint32_t g_ClearFlashStartAddr;	    //清空FLASH起始位置
//uint32_t g_ClearFlashLen;			//清空FLASH长度
//uint8_t g_ChangeIPFlag;                   //修改IP的标志
//uint8_t 	g_SendToNetFailCount;
//uint8_t g_RturnFaultMessageFlag;  /* 返回故障信息 agj */
//uint8_t g_ErrCode;
//bool 		g_UpdateSuc;
//uint8_t 	tmpsendbuf[560];
//uint8_t 	CelluarStep=0;
//uint32_t 	g_SendD6Delay;
/***** 远程升级**************************************************************/
const uint8_t TotalControlMobleNo[2][6]={{0x13,0x75,0x31,0x24,0x37,0x2F},{0x13,0x75,0x31,0x20,0x58,0x3F}};
uint8_t 	g_IPBuffer[3][4] = {{211,142,31,152},{211,142,31,152},{221,204,213,61}};//0-远程升级IP，1-远程转储IP，2-数据IP   221.204.213.61:22042
uint16_t 	g_PortNo[3] = {2021,2029,22042};
uint8_t 	g_DataIPBack[4];      //数据IP备份
uint16_t 	g_DataPortBack;      //数据端口号备份
bool        g_GetMyMobileOKFlag;
uint8_t 	g_ShouldJumpToStartFlag;
uint8_t 	g_RemoteUpdateGCFlag; 		//=1建立升级socket =2获取升级类型 =4获取升级数据包
uint16_t 	g_RemoteUpdatePackNo;
uint16_t 	g_RemoteUpdatePackTotal;
uint8_t 	g_UpdateType;	//0-全量升级 1-增量升级
uint8_t 	g_checkbuf[512];
uint8_t		LastMajorVersion = 0;
uint8_t		LastMinorVersion = 0;
uint32_t 	LastTotalBagNum;                //上次的包总数 
uint8_t	    g_VersionHex[2];				//16进制版本号
uint8_t	    g_VersionAscii[4];				//ASCII版本号
_VERSION_FLASH g_VersioninFlash;
UPDATAPROGRAM  g_UpdateCodePageTotal;
///******************************************************************
//函数名称: void SendRecSuc(void)
//功能描述: 向服务器返回接收成功指令
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：hly
//******************************************************************/
//void SendRecSuc(void)
//{
//	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
//	tmpsendbuf[0] = 0xDF;
//	tmpsendbuf[1] = g_RecD6BagNo&0xFF;
//	tmpsendbuf[2] = (g_RecD6BagNo>>8)&0xFF;
//	tmpsendbuf[3] = (g_RecD6BagNo>>16)&0xFF;
//	tmpsendbuf[4] = (g_RecD6BagNo>>24)&0xFF;
//	tmpsendbuf[5] = CheckSum(5,tmpsendbuf);		
//	HAL_UART_Transmit(&hUART4, tmpsendbuf, 6, 1000);		//
//}
///******************************************************************
//函数名称: void SendUpdateBag(void)
//功能描述: 向服务器请求升级数据包
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：hly
//******************************************************************/
//void SendUpdateBag(void)
//{
//	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
//	
//	tmpsendbuf[0] = 0x5A;
//	tmpsendbuf[1] = 0xA5;
//	tmpsendbuf[2] = 0x20;
//	tmpsendbuf[3] = 0x00;
//	tmpsendbuf[4] = 0xEF;	
//	
//	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
//	memcpy(tmpsendbuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);
//	tmpsendbuf[14] = ( g_UpdateCodePageTotal.programpage & 0xff );		
//	tmpsendbuf[15] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
//	tmpsendbuf[16] = ( g_RemoteUpdatePackNo & 0xff );		
//	tmpsendbuf[17] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
//	if(g_UpdateType == 0xDC)
//		tmpsendbuf[18] = 0;	
//	else
//		tmpsendbuf[18] = g_UpdateType;
//	if(g_UpdateType == 1)
//	{
//		tmpsendbuf[19] = g_VersionHex[0];
//		tmpsendbuf[20] = g_VersionHex[1];
//	}
//	tmpsendbuf[35] = CheckSum(31,tmpsendbuf+4);	
//	tmpsendbuf[40] = CheckSum(40,tmpsendbuf);		
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,41, 1000);		//
//}
///******************************************************************
//函数名称: void SendFullUpdateBag(void)
//功能描述: 向服务器请求升级数据包(全量升级)
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：hly
//******************************************************************/
//void SendFullUpdateBag(void)
//{
//	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
//	
//	tmpsendbuf[0] = 0x5A;
//	tmpsendbuf[1] = 0xA5;
//	tmpsendbuf[2] = 0x10;
//	tmpsendbuf[3] = 0x00;
//	tmpsendbuf[4] = 0xEE;	
//	
//	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
//	memcpy(tmpsendbuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);
//	tmpsendbuf[14] = ( g_UpdateCodePageTotal.programpage & 0xff );		
//	tmpsendbuf[15] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
//	tmpsendbuf[16] = ( g_RemoteUpdatePackNo & 0xff );		
//	tmpsendbuf[17] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
//	tmpsendbuf[19] = CheckSum(15,tmpsendbuf+4);	
//	tmpsendbuf[24] = CheckSum(24,tmpsendbuf);		
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,25, 1000);		//
//}
/******************************************************************
函数名称: void SendUpdateBag(void)
功能描述: 向服务器应答参数包
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendReplydateBag(void)
{
	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
	
	tmpsendbuf[0] = 0x5A;
	tmpsendbuf[1] = 0xA5;
	tmpsendbuf[2] = 0x20;
	tmpsendbuf[3] = 0x00;
	tmpsendbuf[4] = 0x01;	
	tmpsendbuf[5] = g_ReplyData.type;		
	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
	memcpy(tmpsendbuf + 12, g_IDInFlash.IDnum, 7);                /*ID号*/
	tmpsendbuf[19] = 0x02;
	if(g_ReplyData.type == 0x07)
	{
		tmpsendbuf[20] = g_SendDiffMark;
	}
	else if(g_ReplyData.type == 0x09)
	{
		tmpsendbuf[20] = g_ModeInFlash.type;
	}
	tmpsendbuf[35] = CheckSum(31,tmpsendbuf+4);	
	memcpy(tmpsendbuf+36,g_ReplyData.seri,4);
	tmpsendbuf[40] = CheckSum(40,tmpsendbuf);		
	HAL_UART_Transmit(&hUART4, tmpsendbuf,41, 1000);		
}
/******************************************************************
函数名称: void SendUpdateBag(void)
功能描述: 向服务器应答参数包
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendPardateBag(void)
{
//	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
//	
//	tmpsendbuf[0] = 0xFA;
//	tmpsendbuf[1] = 0x00;
//	tmpsendbuf[2] = g_ReplyData.type;
//	tmpsendbuf[30] = 0x1F;
//	tmpsendbuf[31] = 0x00;	
//	tmpsendbuf[32] = 0x5A;
//	tmpsendbuf[33] = 0xA5;
//	if(g_ReplyData.type == 0x07)
//	{
//		tmpsendbuf[34] = g_SendDiffMark;
//	}
//	else if(g_ReplyData.type == 0x09)
//	{
//		tmpsendbuf[34] = g_ModeInFlash.type;
//	}
//	tmpsendbuf[63] = CheckSum(63,tmpsendbuf);	
//	memcpy(tmpsendbuf+64,g_ReplyData.seri,4);	
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,68, 1000);
}

///******************************************************************
//函数名称: Bool SendD6Data(void)
//功能描述: 发送D6数据
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：hly
//******************************************************************/
//void SendD6Data(uint8_t *data, uint8_t type)
//{
//    uint8_t i,cmpbuf[5],tmpmobile[6],gsm7bit[140],tmptime[6];

//	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
//    memset(cmpbuf,0,sizeof(cmpbuf));
//	
//	tmpsendbuf[0] = 0xD6;
//	memcpy(tmpsendbuf + 1,g_D6Phone,6);	//接收手机号
//	tmpsendbuf[7] = g_SendD6BagNo;						//D6数据包
//	if(g_MobileInFlash.Crc16 == CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
//    {
//        if((CheckNumIsBCD((uint8_t*)g_MobileInFlash.mobileno[4],5) == true)&&
//            (memcmp((uint8_t*)g_MobileInFlash.mobileno[4],cmpbuf,5)!=0))
//		{
//			for(i=0;i<6;i++)
//			{
//				tmpmobile[i]=CharSwap(g_MobileInFlash.mobileno[4][i]);	//手机号每个字节高低位交换
//			}
//			for( i = 0 ; i < 6  ; i++ )
//			{
//				Bcd2Asc(tmpmobile[i],tmpsendbuf+8+2*i);				//转换ASCII码
//			}
//			if(type == D67bit)
//			{
//				memset(tmpsendbuf+20,0x30,4);							//协议标识为0，编码方式为00-7bit					
//				TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
//				tmptime[0] = CharSwap(g_BCDTime.Year);
//				tmptime[1] = CharSwap(g_BCDTime.Month);
//				tmptime[2] = CharSwap(g_BCDTime.Day);
//				tmptime[3] = CharSwap(g_BCDTime.Hour);
//				tmptime[4] = CharSwap(g_BCDTime.Minute);
//				tmptime[5] = CharSwap(g_BCDTime.Second);
//				for( i = 0 ; i < 6  ; i++ )
//				{
//					Bcd2Asc(tmptime[i],tmpsendbuf+24+2*i);		//年月日时分秒
//				}
//				i=0;														//时区
//				Bcd2Asc(i,tmpsendbuf+36);
//				i=160;														//短信长度
//				Bcd2Asc(i,tmpsendbuf+38);
//				
//				for( i = 0 ; i < 80  ; i++ )								//80字节转换ASCII码-160
//				{
//					Bcd2Asc(*data,g_tmpBuf+2*i);
//					data++;
//				}
//				gsmEncode7bit((char*)g_tmpBuf,gsm7bit,160);					//转换7位编码-140
//				for( i = 0 ; i < 140  ; i++ )								//140字节转换ASCII码-280
//				{
//					Bcd2Asc(gsm7bit[i],tmpsendbuf+40+2*i);
//				}
//				tmpsendbuf[320] = CheckSum(320,tmpsendbuf);
//				HAL_UART_Transmit(&hUART4, tmpsendbuf,321, 1000);
//			}
//			else
//			{
//				memset(tmpsendbuf+20,0x30,2);							//协议标识为0
//				tmpsendbuf[22] = 0x30;									//编码方式为04-8bit
//				tmpsendbuf[23] = 0x34;
//				TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
//				tmptime[0] = CharSwap(g_BCDTime.Year);
//				tmptime[1] = CharSwap(g_BCDTime.Month);
//				tmptime[2] = CharSwap(g_BCDTime.Day);
//				tmptime[3] = CharSwap(g_BCDTime.Hour);
//				tmptime[4] = CharSwap(g_BCDTime.Minute);
//				tmptime[5] = CharSwap(g_BCDTime.Second);
//				for( i = 0 ; i < 6  ; i++ )
//				{
//					Bcd2Asc(tmptime[i],tmpsendbuf+24+2*i);		//年月日时分秒
//				}
//				i=0;														//时区
//				Bcd2Asc(i,tmpsendbuf+36);
//				i=140;														//短信长度
//				Bcd2Asc(i,tmpsendbuf+38);
//				
//				for( i = 0 ; i < 140  ; i++ )								//140字节转换ASCII码-280
//				{
//					Bcd2Asc(data[i],tmpsendbuf+40+2*i);
//				}
//				tmpsendbuf[320] = CheckSum(320,tmpsendbuf);
//				HAL_UART_Transmit(&hUART4, tmpsendbuf,321, 1000);
//			}
//			g_SendD6Suc = false;
//			g_SendD6Delay = uwTick;
//		}
//	}
//}
///******************************************************************
//函数名称: void ReturnMobileNoToNet(void)
//功能描述: 向服务器返回控制权限号
//参数输入: 无
//参数输出: 
//返 回 值: 无
//其    他: 无
//******************************************************************/
//void ReturnMobileNoToNet(void)
//{
//    uint8_t i,tmpbuf[80];

//    memset(tmpbuf,0,sizeof(tmpbuf));
//	
//    memset(tmpbuf,0xA8,6);
//    tmpbuf[6] = 0x01;

//    for(i=0;i<2;i++)
//    {
//        memcpy(tmpbuf+7+6*i,g_MobileInFlash.mobileno[i],6);    
//    }
//    memset(tmpbuf+19,0xff,12);                        //路局号和工务段号
//    memset(tmpbuf+31,0,4);            
//    memcpy(tmpbuf+35,g_MobileInFlash.mobileno[4],6);  //本机号
//    memcpy(tmpbuf+41,g_MobileInFlash.mobileno[3],6);  //短信接收号
//    memset(tmpbuf+47,0,33);
//    SendD6Data(tmpbuf,D67bit);
//}
///******************************************************************
//函数名称: void ReturnIDtoNet(void)
//功能描述: 向服务器返回ID版本号
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//******************************************************************/
//void ReturnIDtoNet(void)
//{
//	uint8_t tmpbuf[80];
//	
//    memset(tmpbuf,0,sizeof(tmpbuf));

//    tmpbuf[0] = 0xCF;
//	tmpbuf[1] = 0x80;	
//	tmpbuf[2] = 0x0B;
//	tmpbuf[3] = 0x01;
//	memcpy(tmpbuf+36,g_VersionAscii,4);
//	memcpy(tmpbuf+40,g_VersioninFlash.bootversion, 4); 
//    memset(tmpbuf + 44,0,35);	
//	tmpbuf[79] = CheckSum(79,tmpbuf);	
//	SendD6Data(tmpbuf,D67bit);
//}
//void ReturnClearFlashToGsm(void)
//{
//    uint8_t data[80];
//    
//    memset(data,0,80);
//    data[0] = 0xCF;
//    data[1] = 0x80;
//    data[2] = 0x12;
//	data[3] = g_ClearFlashFlag;
//	if(g_ClearFlashFlag == 2)
//	{		
//		data[4] = g_ClearFlashStartAddr&0xFF;
//		data[5] = (g_ClearFlashStartAddr>>8)&0xFF;
//		data[6] = (g_ClearFlashStartAddr>>16)&0xFF;
//		data[7] = (g_ClearFlashStartAddr>>24)&0xFF;
//		data[8] = g_ClearFlashLen&0xFF;
//		data[9] = (g_ClearFlashLen>>8)&0xFF;
//		data[10] = (g_ClearFlashLen>>16)&0xFF;
//		data[11] = (g_ClearFlashLen>>24)&0xFF;
//	}
//    data[79] = CheckSum(79,data);
//    SendD6Data(data,D67bit);
//}
///******************************************************************
//函数名称: void IIIRain_ReturnFaultMessageToGsm(void)
//功能描述: III_RAIN 主板给DTU返回故障信息
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：agj
//******************************************************************/
//void ReturnFaultMessageToGsm(void)
//{
//	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
//	
//	tmpsendbuf[0] = 0x5a;
//	tmpsendbuf[1] = 0xa5;
//	tmpsendbuf[2] = 0x10;
//	tmpsendbuf[3] = 0;
//	tmpsendbuf[4] = 0xe0;
//	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
//	memcpy(tmpsendbuf+11,(uint8_t *)&g_BCDSysTime,5);
//	tmpsendbuf[16] = g_ErrCode;                      //故障信息编码
//	tmpsendbuf[17] = 0;
//	tmpsendbuf[18] = 0;
//	tmpsendbuf[19] = CheckSum(15,tmpsendbuf+4);
//	tmpsendbuf[20] = g_Send5ABagNo&0xFF;
//	tmpsendbuf[21] = (g_Send5ABagNo>>8)&0xFF;
//	tmpsendbuf[22] = (g_Send5ABagNo>>16)&0xFF;
//	tmpsendbuf[23] = (g_Send5ABagNo>>24)&0xFF;
//	tmpsendbuf[24] = CheckSum(24,tmpsendbuf);
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,25, 1000);		//
//}
//void ReturnPartoNet(void)
//{
//	uint8_t tmpbuf[140];
//	
//    memset(tmpbuf,0,sizeof(tmpbuf));

//    tmpbuf[0] = 0xCF;
//	tmpbuf[1] = 0x91;	
//	tmpbuf[2] = 0x01;
//	tmpbuf[3] = g_ModeInFlash.type;
//	tmpbuf[4] = g_SendDiffMark;
//	memcpy(tmpbuf+5,g_CorsAccFlash.user,32);
//	memcpy(tmpbuf+37,g_CorsAccFlash.pass,16);	
//	tmpbuf[139] = CheckSum(139,tmpbuf);	
//	SendD6Data(tmpbuf,D68bit);
//}
///******************************************************************
//函数名称: void ControlMsg(void)
//功能描述: 控制信息
//参数输入: 无
//参数输出: 
//返 回 值: 无
//其    他: 无
//******************************************************************/
//void ControlMsg(void)
//{
////	if(g_ReplyData.flag == 1)
////	{
////		SendReplydateBag();
////		memset((uint8_t *)&g_ReplyData,0x00,sizeof(g_ReplyData));
////	}
////	else if(g_ReplyData.flag == 2)
////	{
////		SendPardateBag();
////		memset((uint8_t *)&g_ReplyData,0x00,sizeof(g_ReplyData));
////	}
//	if(g_UpdateCodeTypeErrorFlag == 1)
//	{
//		if(g_UpdateVoiceTypeFlag == 1)
//		{					
//			SendFullUpdateBag();
//		}
//		else
//		{
//			SendUpdateBag();
//		}			
//		g_UpdateCodeTypeErrorFlag = 0;
//		g_CtrlMsgFlag = 0;
//	}
//	else if(g_AskAuthorityFlag == 1)
//	{
//		ReturnMobileNoToNet();
//		g_AskAuthorityFlag = 0;
//		g_CtrlMsgFlag = 0;
//	}
//	else if(g_AskIDFlag == 1)
//	{
//		ReturnIDtoNet();
//		g_AskIDFlag = 0;
//		g_CtrlMsgFlag = 0;
//	}
//	else if(g_ClearFlashFlag == 1 || g_ClearFlashFlag == 2)
//	{
//		ReturnClearFlashToGsm();
//		g_ClearFlashFlag = 0;
//		g_CtrlMsgFlag = 0;
//		SoftReset();
//	}		
//    else if(g_RturnFaultMessageFlag == 1)   //返回故障信息
//    {
//        if(g_GetMyMobileOKFlag == true)
//        {
//            ReturnFaultMessageToGsm();
//            g_RturnFaultMessageFlag = 0;
//            g_CtrlMsgFlag = 0;
//        }
//    }
//	else if(g_AskParFlag == 1)   //返回基站基础信息
//    {
//		ReturnPartoNet();
//		g_AskParFlag = 0;
//		g_CtrlMsgFlag = 0;
//	}	
//	else
//		g_CtrlMsgFlag = 0;
//	g_ControlMsgDelay = uwTick;
//}

/******************************************************************
函数名称: int16_t DTU_BulkParaInit
功能描述: 上传批量数据
参数输入: BulkPara_t		*Para		批量参数
参数输出: 
返 回 值: int16_t 成功=0;错误<0
其    他: 无
******************************************************************/
int16_t DTU_BulkParaInit(BulkPara_t *Para)
{
	Para->Step = BULK_PROG_NONE;
	Para->uRecordCnt = 0;
	Para->iMinRecordOrder = 0;
	Para->iMaxRecordOrder = 0;
	Para->iRawRecordSize = 0;
	memcpy(Para->cID, "18406503524", 11);
	memcpy(Para->cSN, "18406503524", 11);
	
//	g_qhBulkUploadRecords = xQueueCreate(10, sizeof(BulkRecord_t));	//初始化队列
//	g_qhBulkDownloadRecords = xQueueCreate(10, sizeof(BulkRecord_t));	//初始化队列
	
	return BULK_STA_OK;
}
/************************************** DTU透传通信函数 ************************************************/

/******************************************************************
函数名称: static void DTU_DispBulkHeaderInfo
功能描述: 显示批量数据头信息
参数输入: BulkHeader_t	*Header		数据头
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
static void DTU_DispBulkHeaderInfo(BulkHeader_t *Header)
{
	char str[64];
	
	sprintf(str, "Cmd=%04X, Count=%d, Min=%d, Max=%d", Header->byte.uCmd, Header->byte.uRecordCnt, Header->byte.iMinRecordOrder, Header->byte.iMaxRecordOrder);
	debug_msg("Header: %s\r\n", str);
}

/******************************************************************
函数名称: static void DTU_DispBulkReocrdInfo
功能描述: 显示批量记录信息
参数输入: BulkRecord_t	Record		数据头
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
static void DTU_DispBulkRecordInfo(BulkRecord_t Record)
{
	uint16_t i;
	
	debug_info(&Record.RecordBody[i], Record.uLength, "->BULK");
}



///******************************************************************
//函数名称: void ProcessDtuNoAnswerFunc(void)
//功能描述: 处理DTU无应答
//参数输入: 无
//参数输出: 
//返  回  值 : 无
//其       他: 
//******************************************************************/
//void ProcessDtuNoAnswerFunc(void)
//{
//    if(g_FileSendDataToDtu >= 180)
//    {
//		g_FileSendDataToDtu = 0;		
//    }
//}
///******************************************************************
//函数名称: void SendC3Func(void)
//功能描述: 发送打点数据的函数
//参数输入: 无
//参数输出: 
//返  回  值 : 无
//其       他: 无
//******************************************************************/
//bool SendPunCoorFunc(void)
//{
//    uint8_t tempdata;
//    uint8_t cmpbuf[5];
//	uint16_t i;
//	uint32_t tmpoffset;
//	
//	memset(cmpbuf,0,sizeof(cmpbuf));
//    
//    if(g_PunCoorSendFlag == 0 || (g_PunCoorSendFlag == 1 && g_DtuSendOk == 1)) //数据未发送，或者有数据发送且已发送成功    
//    {
//        if(g_PunCoorSendFlag == 1 && g_DtuSendOk == 1)
//        {
//            for(i=0;i<g_PUNCOORAddnumb;i++)
//			{
//				tempdata = INDEX_SEND_SUC;            
//				W25qxx_WriteByte(tempdata,g_ReadAddrPar[i].Readaddr);
//				g_PUNCOORDatainFlash.needsendcnt--;								
//			}
//			g_PUNCOORAddnumb = 0;
//			memset(g_ReadAddrPar,0xFF,sizeof(g_ReadAddrPar));
//			g_PunCoorSendFlag = 0;			
//            return true;        //20150120
//        }
//        else if(g_PunCoorSendFlag == 0 && g_PUNCOORDatainFlash.needsendcnt > 0)
//        {
//			if(RYDW_GeneratePostionRecord() == true)    //发送 c3
//			{
//				g_PunCoorSendFlag = 1;
//				g_DtuSendOk = 2;
//				g_SendPunCoorDelay = uwTick;
//			}
//			else
//				return true;				
//        }
//        else if(g_PUNCOORDatainFlash.needsendcnt == 0)
//            return true;				
//    }
//    else if(g_PunCoorSendFlag == 1 && (g_DtuSendOk == 0 || g_DtuSendOk == 2))
//    {/*重发*/
//        if(JudgeMilSecondDelayInPro(g_SendPunCoorDelay , 30*1000) == true)
//        {
//		
//			if(RYDW_GeneratePostionRecord() == true)    //重新发送D5
//			{
//				g_DtuSendOk = 2;
//				g_SendPunCoorDelay = uwTick;
//				g_FileSendDataToDtu++;
//				ProcessDtuNoAnswerFunc();					
//			}
//        }
//    }
//    return false;
//}


///******************************************************************
//函数名称: void NetGetUpdateType(void)
//功能描述: 向服务器请求升级类型
//参数输入: 无
//参数输出: 
//返  回  值: 无
//其      他: 无
//编写者：hly
//******************************************************************/
//void GetUpdateType(void)
//{
//	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
//	
//	tmpsendbuf[0] = 0x5A;
//	tmpsendbuf[1] = 0xA5;
//	tmpsendbuf[2] = 0x10;
//	tmpsendbuf[3] = 0x00;
//	tmpsendbuf[4] = 0xEA;	
//	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
//	memcpy(tmpsendbuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);	
//	tmpsendbuf[14] = g_VersionHex[0];
//	tmpsendbuf[15] = g_VersionHex[1];	
//	tmpsendbuf[19] = CheckSum(15,tmpsendbuf+4);	
//	tmpsendbuf[24] = CheckSum(24,tmpsendbuf);		
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,25, 1000);		//
//}



/******************************************************************
函数名称: void DTU_BulkProgress
功能描述: 批量数据处理流程
参数输入: BulkPara_t	*Para		批量参数
参数输出: 
返 回 值: 
其    他: 无
******************************************************************/
void DTU_BulkProgrss(void)
{
//	int16_t recvret;
//	int16_t sendret;
//	uint16_t reclen;
	uint16_t size,i;
	uint32_t recd6bagno,rec5abagno;
	uint16_t increase1,inphth,disincrease;	
	/*解析接收*/
	/* 检查接收数据长度 */
	size = GetRingUsedLen(&g_Uart4RxRing);
	if(size > 0)
	{	
		/* 检查包头 */
		memset(g_BulkRecvBuf, 0x00, sizeof(g_BulkRecvBuf));
		RingBufPop(&g_Uart4RxRing, g_BulkRecvBuf, size);
		switch(g_BulkRecvBuf[0])
		{
			case 0xD2:
				{
					if(g_BulkRecvBuf[31] == CheckSum(31,g_BulkRecvBuf))
					{
						if(memcmp(g_MobileInFlash.mobileno[4],g_BulkRecvBuf+1,6)==0)
						{					
							if(g_BulkRecvBuf[14] == 0x02)
							{//基站
					
							}				
						}
					}
				}			
				break;		
			case 0x5A:
				{
					uint8_t tmpmobile[6],typebuf[32],cmpbuf[8],tmp,type,equalflag;
					uint16_t i=0,j=0,k=0;
					uint8_t recbuf[300];
					uint8_t gsmcode7[140];
					uint8_t cmdbuf[140];
					uint8_t mobile_rxflag = 0;
					uint8_t tempmobile[5][6]; 
					
					if(g_BulkRecvBuf[1] == 0xA5 && g_BulkRecvBuf[2] == 0x5A && g_BulkRecvBuf[3] == 0xA5)
					{
						if(g_BulkRecvBuf[4] == 0xEE)
						{
							memset(cmpbuf,0xEE,8);
							if(memcmp(g_BulkRecvBuf+4, cmpbuf, 8) == 0)
							{
								g_RecD6BagNo = g_BulkRecvBuf[20]+(g_BulkRecvBuf[21]<<8)+(g_BulkRecvBuf[22]<<16)+(g_BulkRecvBuf[23]<<24);
								SendRecSuc();
								memcpy((uint8_t*)&g_UpdateCodePageTotal,g_BulkRecvBuf+12,3);
								g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(g_BulkRecvBuf,15);
								if(g_UpdateCodePageTotal.programtype == CODETYPE)
								{
									g_UpdateCodeTypeErrorFlag = 0;
									g_RemoteUpdateGCFlag = 2;								
									
									g_RemoteUpdatePackNo = 0;			//重置升级包序号
									g_UpdateType = 0;					//重置升级类型（0-全量升级 1-增量升级），后续由AnalysisRemoteUpgradeTypeData设置
									g_ResendFlag = 0;
									g_ResendTimes = 5;	
									MajorVersion = g_BulkRecvBuf[13];                        		//主版本号
									MinorVersion = g_BulkRecvBuf[14];                        		//次版本号
									LastMajorVersion = MajorVersion;                        	//备份主版本号
									LastMinorVersion = MinorVersion;                        	//备份次版本号
									LastTotalBagNum = Get16ValueFrom8Buf(g_BulkRecvBuf,15);  		//包总数
									NormalUpdateFileSize.FileSize = 0;
									WriteNormalFileSizeToFlash();
									DiffUpdateFileSize.FileSize = 0;
									WriteDiffFileSizeToFlash();
									CelluarStep = 0;
									g_UpFlag =1;									
								}
								else if(g_UpdateCodePageTotal.programtype == VOICECODETYPE && g_ShouldUpdateVoiceFlag == 0)
								{
									g_UpdateCodeTypeErrorFlag = 0;
									g_RemoteUpdateGCFlag = 2;								
									g_RemoteUpdatePackNo = 0;			//重置升级包序号
									g_UpdateType = 0;					//重置升级类型（0-全量升级 1-增量升级），后续由AnalysisRemoteUpgradeTypeData设置
									g_ResendFlag = 0;
									g_ResendTimes = 5;										
									MajorVersion = g_BulkRecvBuf[13];                        		//主版本号
									MinorVersion = g_BulkRecvBuf[14];                        		//次版本号
									LastMajorVersion = MajorVersion;                        	//备份主版本号
									LastMinorVersion = MinorVersion;                        	//备份次版本号
									LastTotalBagNum = Get16ValueFrom8Buf(g_BulkRecvBuf,15);  		//包总数
									NormalUpdateFileSize.FileSize = 0;
									WriteNormalFileSizeToFlash();
									DiffUpdateFileSize.FileSize = 0;
									WriteDiffFileSizeToFlash();
									CelluarStep = 0;
									g_UpFlag =1;
									g_UpdateVoiceTypeFlag = 1;									
								}	
							}
						}
						else if(g_BulkRecvBuf[4] == 0xDF)
						{
							memset(cmpbuf,0xDF,8);
							if(memcmp(g_BulkRecvBuf+4, cmpbuf, 8) == 0)
							{
								g_NetAskTime = 1;
								g_NetAskTimeCnt = 0;						
							}
						}					
						else
						{
							j=0;
							for(i=5;i<37;i+=2)
							{
								tmp = Asc2Hex(g_BulkRecvBuf[i]);
								tmp = (tmp<<4);
								tmp += Asc2Hex(g_BulkRecvBuf[i+1]);									
								typebuf[j++] = tmp;
							}
							
							type = typebuf[7];
							if(g_BulkRecvBuf[4] == 0xD6 && g_BulkRecvBuf[317] == CheckSum(313,g_BulkRecvBuf+4) && g_SendD6Suc)
							{
								g_RecD6BagNo = g_BulkRecvBuf[318]+(g_BulkRecvBuf[319]<<8)+(g_BulkRecvBuf[320]<<16)+(g_BulkRecvBuf[321]<<24);
								SendRecSuc();
								
								memset(cmpbuf,0,6);
								for(i=0;i<6;i++)
								{
									tmpmobile[i]=CharSwap(typebuf[i]);
								}
								j=0;
								for(i=37;i<317;i+=2)
								{
									tmp = Asc2Hex(g_BulkRecvBuf[i]);
									tmp = (tmp<<4);
									tmp += Asc2Hex(g_BulkRecvBuf[i+1]);									
									gsmcode7[j++] = tmp;
								}
								if(type == 0x00)
								{
									gsmDecode7bit(gsmcode7,(char*)recbuf,140);		
								}					
//								if(memcmp(tmpmobile,g_MobileInFlash.mobileno[0],6)&& 
//								   memcmp(tmpmobile,g_MobileInFlash.mobileno[1],6)&&
//								   memcmp(tmpmobile,TotalControlMobleNo[0],6)&&
//								   memcmp(tmpmobile,TotalControlMobleNo[1],6))
//								{
//									return;
//								}
//								else
								{
									memcpy(g_D6Phone,tmpmobile,6);
									if(type == 0x00)//7bit
									{
										g_RecD6BagNo = g_BulkRecvBuf[318]+(g_BulkRecvBuf[319]<<8)+(g_BulkRecvBuf[320]<<16)+(g_BulkRecvBuf[321]<<24);
										j=0;
										for(i=37;i<317;i+=2)
										{
											tmp = Asc2Hex(g_BulkRecvBuf[i]);
											tmp = (tmp<<4);
											tmp += Asc2Hex(g_BulkRecvBuf[i+1]);									
											gsmcode7[j++] = tmp;
										}
										gsmDecode7bit(gsmcode7,(char*)recbuf,140); 
										
										for(i=0;i<80;i++)
										{
											cmdbuf[i] = (Asc2Hex(recbuf[i*2])<<4) + Asc2Hex(recbuf[i*2+1]);
										}
										
										if(cmdbuf[0] == 0xA8)
										{
											memset(cmpbuf,0xA8,6);
											if(memcmp(cmdbuf, cmpbuf, 6) == 0)
												mobile_rxflag = 1;
											else
												mobile_rxflag = 0;											
										}
										else if(cmdbuf[0] == 0xA9)
										{
											memset(cmpbuf,0xA9,6);
											if(memcmp(cmdbuf, cmpbuf, 6) == 0)
											{
												if(cmdbuf[9] == 0x08)
													mobile_rxflag = 3;	
											}
											
										}
										if(mobile_rxflag == 1)
										{
											mobile_rxflag = 0;
											for(k=0;k<5;k++)
											{
												for(i=0;i<6;i++)
												{
													tempmobile[k][i] = cmdbuf[10+i+8*k];
												}
											}
											if(ReadMobileNoFromFLASH() == true)
											{
												;
											}
											else
											{
												return;
											}
											for(i=0;i<5;i++)
											{
												if(CheckNumIsBCD((uint8_t*)tempmobile[i],5) == true && memcmp((uint8_t*)tempmobile[i],cmpbuf,5)!=0)
												{
													if(i == 4)
													{
														if(0 != memcmp(g_MobileInFlash.mobileno[4],tempmobile[i],5) || ((g_MobileInFlash.mobileno[4][5]&0xF0) != (tempmobile[4][5]&0xF0)))
														{
															equalflag = 1;
															tempmobile[i][5] |= 0x0F;
															memcpy(g_MobileInFlash.mobileno[4],tempmobile[i],sizeof(tempmobile[i]));
														}
													}
													else
														memcpy(g_MobileInFlash.mobileno[i],tempmobile[i],sizeof(tempmobile[i]));												
												}
											}   
											WriteMobileNoToFLASH();										
											if(ReadMobileNoFromFLASH() == true)
											{
												if(equalflag == 1)
												{
													equalflag = 0;
													CelluarStep = 1;
												}
												g_GetMyMobileOKFlag = true;
												g_AskAuthorityFlag = 1;
												g_CtrlMsgFlag = 1;
												g_SendD6Suc = false;
												g_SendD6Delay = uwTick;
												return;
											}
											g_AskAuthorityFlag = 0;
										}
										else if(mobile_rxflag == 3 && g_CtrlMsgFlag == 0)
										{                                            
											mobile_rxflag = 0;
											if(ReadMobileNoFromFLASH() == true)
											{
												g_AskAuthorityFlag = 1;             
												g_CtrlMsgFlag = 1;
												g_SendD6Suc = false;
												g_SendD6Delay = uwTick;
											}
										}
										if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0x0F)		//重启命令
										{
										   if(0 == g_CtrlMsgFlag)
										   {
											g_CtrlMsgFlag = 1;											   
											if(cmdbuf[2] == 0x01)			//重启485
											{
	//												g_Reset485Flag = 1; 
	//												g_PowerOff485Time = TIM2->CNT;												
											}
											if(cmdbuf[2] == 0x02)			//重启主板
											{
	//												g_ResetSystemFlag = 1;
	//												g_ResetSystemTime = TIM2->CNT;												
											}
											if(cmdbuf[2] == 0x03)			//重启4G模块
											{
	//												g_ResetDTUFlag = 1; 
	//												g_DTUResetTime = TIM2->CNT;												
											}
											g_SendD6Suc = false;
											g_SendD6Delay = uwTick;
										   }											
										}
										if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0x80 && cmdbuf[2] == 0x09)
										{
											if(0 == g_CtrlMsgFlag)
											{
												j = 0;
												ReadServerInfoPar();  												
												uint8_t cmpipbuf[2][6];												
												memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
												memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));												
												
												for( k = 0 ; k < 3 ; k++ )
												{
													for( i = 0 ; i < 6 ;i++ )             
													{
														tempmobile[k][i] = cmdbuf[3+6*k+i];
													}
													if( memcmp(tempmobile[k],cmpipbuf[0],4)== 0 || memcmp(tempmobile[k]+4,cmpipbuf[0],2)== 0 
														|| memcmp(tempmobile[k],cmpipbuf[1],4) == 0 || memcmp(tempmobile[k]+4,cmpipbuf[1],2)== 0 )

													{
													}													
													else
													{
														j = 1;
														if( k == 0)                                                 //数据
														{
															memcpy(g_DataIPBack,g_ServerInfo.g_ServerIPAddr[2],4);  //备份数据IP和端口号
															g_DataPortBack = g_ServerInfo.g_ServerPortNo[2];
															memcpy(g_ServerInfo.g_ServerIPAddr[2],tempmobile[k],4);
															g_ServerInfo.g_ServerPortNo[2] = tempmobile[k][4]*100+tempmobile[k][5];
														}
														else if(k == 1 )                                            //升级
														{
															memcpy(g_ServerInfo.g_ServerIPAddr[0],tempmobile[k],4);
															g_ServerInfo.g_ServerPortNo[0] = tempmobile[k][4]*100+tempmobile[k][5];
														}
														else if( k == 2)                                            //转储
														{
															memcpy(g_ServerInfo.g_ServerIPAddr[1],tempmobile[k],4);
															g_ServerInfo.g_ServerPortNo[1] = tempmobile[k][4]*100+tempmobile[k][5];
														}
													}
												}
												if(j == 1)
												{
													WriteServerInfoPar(); 
													if(ReadServerInfoPar() == true)                             //
													{                                       //
														CmpIPAndPort();                                         //若有全零或全F的替换为默认值
														if((memcmp(g_DataIPBack,g_ServerInfo.g_ServerIPAddr[2],4) != 0) || (g_DataPortBack != g_ServerInfo.g_ServerPortNo[2]))  //若数据IP修改了
														{
															g_ChangeIPFlag = 1;
															CelluarStep = 1;
														}
													}
													g_NetAskIPAndPortFlag = 1;
													g_CtrlMsgFlag = 1; 
													g_SendD6Suc = false;
													g_SendD6Delay = uwTick;
												}
											}
										}
										else if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0xFF && cmdbuf[2] == 0x80 && cmdbuf[3] == 0x09)
										{
											if(0 == g_CtrlMsgFlag)
											{
												g_NetAskIPAndPortFlag = 1;
												g_CtrlMsgFlag = 1;
												g_SendD6Suc = false;
												g_SendD6Delay = uwTick;
											}
										}
										else if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0xFF && cmdbuf[2] == 0x80 && cmdbuf[3] == 0x0B)
										{
											if(0 == g_CtrlMsgFlag)
											{
												g_AskIDFlag = 1;
												g_CtrlMsgFlag = 1;
												g_SendD6Suc = false;
												g_SendD6Delay = uwTick;
											}									
										}
										else if(cmdbuf[0] == 0xCF && cmdbuf[1]== 0x80 && cmdbuf[2] == 0x12)
										{
											uint16_t k;
											if(0 == g_CtrlMsgFlag)
											{
												g_ClearFlashFlag = cmdbuf[3];
												if(g_ClearFlashFlag == 1)
												{
													for(i=0;i<4095;i++)   //16M，扇区擦除（4K）
													{
														W25qxx_EraseSector( i );						
													}										
													g_CtrlMsgFlag = 1;		
												}
												else if(g_ClearFlashFlag == 2)
												{
													g_ClearFlashStartAddr = cmdbuf[4]+(cmdbuf[5]<<8)+(cmdbuf[6]<<16)+(cmdbuf[7]<<24);

													g_ClearFlashLen = cmdbuf[8]+(cmdbuf[9]<<8)+(cmdbuf[10]<<16)+(cmdbuf[11]<<24);
													if(g_ClearFlashStartAddr < 32768 && g_ClearFlashLen < 32768)
													{																					
														if(g_ClearFlashStartAddr%8 > 0 || (g_ClearFlashStartAddr+g_ClearFlashLen)%8>0)
														{												
															W25qxx_ReadBytes((uint8_t*)&g_tmpBuf , g_ClearFlashStartAddr*512- (g_ClearFlashStartAddr*512)%4096, (g_ClearFlashStartAddr*512)%4096);
															W25qxx_ReadBytes((uint8_t*)&g_tmpBuf+(g_ClearFlashStartAddr*512)%4096, (g_ClearFlashStartAddr+g_ClearFlashLen)*512, 4096-((g_ClearFlashStartAddr+g_ClearFlashLen)*512)%4096);	
															
															if(g_ClearFlashStartAddr/8 == (g_ClearFlashStartAddr+g_ClearFlashLen)/8)
															{
																W25qxx_EraseSector( g_ClearFlashStartAddr/8 );
																W25qxx_WritePages((uint8_t*)&g_tmpBuf , g_ClearFlashStartAddr*512- (g_ClearFlashStartAddr*512)%4096 , (g_ClearFlashStartAddr*512)%4096);		
																W25qxx_WritePages((uint8_t*)&g_tmpBuf+(g_ClearFlashStartAddr*512)%4096, (g_ClearFlashStartAddr+g_ClearFlashLen)*512 , 4096-((g_ClearFlashStartAddr+g_ClearFlashLen)*512)%4096 );													
															}
															else
															{
															  k = (g_ClearFlashStartAddr+g_ClearFlashLen)/8 - g_ClearFlashStartAddr/8;										
															  for(i=0;i<k;i++)
															  {
																W25qxx_EraseSector( g_ClearFlashStartAddr/8 + i);
															  }
															  W25qxx_WritePages((uint8_t*)&g_tmpBuf , g_ClearFlashStartAddr*512- (g_ClearFlashStartAddr*512)%4096 , (g_ClearFlashStartAddr*512)%4096);		
															  W25qxx_WritePages((uint8_t*)&g_tmpBuf+(g_ClearFlashStartAddr*512)%4096, (g_ClearFlashStartAddr+g_ClearFlashLen)*512 , 4096-((g_ClearFlashStartAddr+g_ClearFlashLen)*512)%4096 );														
															}																					

														}
														else
														{
															if(g_ClearFlashLen/8 ==0)
															{
																
															}
															else
															{
																for(i=0;i<(g_ClearFlashLen/8);i++)   //16M，扇区擦除（4K）
																{
																	W25qxx_EraseSector( g_ClearFlashStartAddr/8 + i );						
																}												
															}																																				
														}
													}	
												}											
											}											
										}
										else if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0xFF && cmdbuf[2] == 0x91 && cmdbuf[3] == 0x01)
										{//索要基站参数
											if(0 == g_CtrlMsgFlag)
											{
												g_AskParFlag = 1;
												g_CtrlMsgFlag = 1;
												g_SendD6Suc = false;
												g_SendD6Delay = uwTick;												
											}											
										}
									}
									else if(type == 0x08)
									{
										g_RecD6BagNo = g_BulkRecvBuf[318]+(g_BulkRecvBuf[319]<<8)+(g_BulkRecvBuf[320]<<16)+(g_BulkRecvBuf[321]<<24);
										j=0;									
										for(i=0;i<140;i++)
										{
											cmdbuf[i] = (Asc2Hex(g_BulkRecvBuf[37+i*2])<<4) + Asc2Hex(g_BulkRecvBuf[i*2+38]);
										}
										if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0x91 && cmdbuf[2] == 0x01)
										{//设置基站参数
											if(0 == g_CtrlMsgFlag)
											{												
												uint8_t cmpipbuf[2][48];												
												memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
												memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));												
												if(cmdbuf[3] !=0xFF && cmdbuf[3] != g_ModeInFlash.type)
												{//设置基站工作模式												
													g_ModeInFlash.type = cmdbuf[3];
													g_LocationProg = 0;	
												    WriteModeToFlash();
													ssd1306_Fill(Black);														
													if(g_ModeInFlash.type == 1)
													{																												
														g_MarkInfo.latitude = 0xFFFFFFFF;
														g_MarkInfo.longitude = 0xFFFFFFFF;
														g_MarkInfo.fAltitude = 0xFFFFFFFF;
														memset(g_MarkInfo.res,0x00,8);														
														WriteMarkToFlash();
														memset((uint8_t*)&g_Cors, 0x00, sizeof(g_Cors)); 
														g_NeedInitDTU = 1;
														g_NeedInitRTK = 1;
													}
													else if(g_ModeInFlash.type == 0)
													{
														memset((uint8_t*)&g_Cors, 0x00, sizeof(g_Cors)); 
														memset((uint8_t*)&g_CorsAccFlash, 0xFF, sizeof(g_CorsAccFlash));
														WriteCorsAccToFlash();
														g_MarkWarnFlag = 2;
														g_NeedInitDTU = 1;
														g_NeedInitRTK = 1;
													}	
												}
												if(cmdbuf[4] !=0xFF && cmdbuf[4] != g_SendDiffMark)
												{//差分数据转发标记
													g_SendDiffMark = cmdbuf[4];
												}
												if( memcmp(cmdbuf+5,cmpipbuf[0],48) == 0 || 
													memcmp(cmdbuf+5,cmpipbuf[1],48) == 0 )
													;												
												else
												{
													if(g_ModeInFlash.type == 0)
													{//自收敛
														memset((uint8_t*)&g_Cors, 0x00, sizeof(g_Cors)); 
														memcpy(g_Cors.user,cmdbuf+5,32);
														memcpy(g_Cors.pass,cmdbuf+37,16);															
														CelluarStep = 1;
														g_NeedInitDTU = 1;	
														g_SelfContCORS = 0x01;														
														g_LocationProg	= 0;
														g_NeedInitRTK = 1;	
													}
													else
													{//CORS
														if(memcmp(cmdbuf+5,g_CorsAccFlash.user,32) == 0 && memcmp(cmdbuf+37,g_CorsAccFlash.pass,16) == 0 )
															;
														else
														{
															memset((uint8_t*)&g_Cors, 0x00, sizeof(g_Cors)); 
															memcpy(g_CorsAccFlash.user,cmdbuf+5,32);
															memcpy(g_CorsAccFlash.pass,cmdbuf+37,16);								
															WriteCorsAccToFlash();
															//需重新连接
															CelluarStep = 1;
															g_NeedInitDTU = 1;								
															g_NeedInitRTK = 1;															
														}													
													}
												}
												if(memcmp(cmdbuf+53,cmpipbuf[0],7) == 0 || 
													memcmp(cmdbuf+53,cmpipbuf[1],7) == 0)
													;//设置ID
												else
												{				
													memcpy(g_IDInFlash.IDnum,cmdbuf+53,7);						
													WriteIDToFlash();
													CelluarStep = 1;	
												}																								
												g_AskParFlag = 1;
												g_CtrlMsgFlag = 1;
												g_SendD6Suc = false;
												g_SendD6Delay = uwTick;														
											}
										}									
									}
								}
							}
						
						}
					}
					else if(g_BulkRecvBuf[1] == 0xA5 && g_BulkRecvBuf[2] == 0x06 && g_BulkRecvBuf[3] == 0x00)
					{
						if(g_BulkRecvBuf[4] == 0xD8 && g_BulkRecvBuf[10] == CheckSum(10,g_BulkRecvBuf))
						{
							rec5abagno = g_BulkRecvBuf[5]+(g_BulkRecvBuf[6]<<8)+(g_BulkRecvBuf[7]<<16)+(g_BulkRecvBuf[8]<<24);
							if(rec5abagno == g_Send5ABagNo)//判断包序号、是否发送成功
							{
								g_Send5ASuc = true;
								g_Send5ABagNo++;
//								g_SendToNetFailCount = 0;
							}
						}
					}			
					else if(g_BulkRecvBuf[1] == 0xA5 && g_BulkRecvBuf[2] == 0x10 && g_BulkRecvBuf[3] == 0x00 && g_BulkRecvBuf[4] == 0xEB)
					{
						if(g_BulkRecvBuf[24] == CheckSum(24,g_BulkRecvBuf))
						{
							memcpy((uint8_t*)&g_UpdateCodePageTotal,g_BulkRecvBuf+11,3);							
							if(g_BulkRecvBuf[14] == g_VersionHex[0] && g_BulkRecvBuf[15] == g_VersionHex[1])
							{
								g_UpdateType = g_BulkRecvBuf[16];
								g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(g_BulkRecvBuf,17);
								g_RemoteUpdateGCFlag = 4;
								g_ResendFlag = 0;
							}
						}
					}
					else if(g_BulkRecvBuf[1] == 0xA5 && g_BulkRecvBuf[4] == 0x00)
					{//服务器应答FA00
						if((g_BulkRecvBuf[5] == 0x02 || g_BulkRecvBuf[5] == 0x05) && g_BulkRecvBuf[16] == CheckSum(16,g_BulkRecvBuf))
						{//应答基站定位数据
							g_DtuSendOk = 1;
						}
						else if(g_BulkRecvBuf[5] == 0x03 && g_BulkRecvBuf[16] == CheckSum(16,g_BulkRecvBuf))
						{//应答基站差分数据						
						}
						g_FileSendDataToDtu = 0;
					}
				}
				break;
			case 0xD7:
				if(g_BulkRecvBuf[8] == CheckSum(8,g_BulkRecvBuf))
				{
					recd6bagno = g_BulkRecvBuf[7];
					if(recd6bagno == g_SendD6BagNo)//判断包序号、是否发送成功
					{
						g_SendD6Suc = true;
						g_SendD6BagNo++;
//						g_SendD6ToNetFailCount = 0;
					}
				}
				break;
			case 0xDC:
				{
					uint8_t tmpbuf1[3];
					
					memset(tmpbuf1,0xDC,3);

					if(memcmp(g_BulkRecvBuf+1,tmpbuf1,3)==0)
					{
						if(g_BulkRecvBuf[511] == CheckSum(511,g_BulkRecvBuf))
						{
							if((g_UpdateCodePageTotal.programpage == Get16ValueFrom8Buf(g_BulkRecvBuf,10))&&(g_RemoteUpdatePackNo ==Get16ValueFrom8Buf(g_BulkRecvBuf,12)))
							{
								memcpy(g_UartUPBuf,g_BulkRecvBuf+14,496);
								
								if(g_UpdateVoiceTypeFlag == 1)
								{
									W25q_Write_N(g_UartUPBuf,FLASH_VOICE_START+496*g_RemoteUpdatePackNo,496);								
									W25qxx_ReadBytes(g_checkbuf,FLASH_VOICE_START+496*g_RemoteUpdatePackNo,496);	
										
									if(memcmp(g_UartUPBuf,g_checkbuf,496) == 0)
										g_RemoteUpdatePackNo++;

									if( g_RemoteUpdatePackNo == g_UpdateCodePageTotal.programpage )
									{	
										VoiceFileSize.FileSize = g_UpdateCodePageTotal.programpage;
										WriteVoiceFileSizeToFlash();
										g_UpdateVoiceTypeFlag = 0;			
										g_UpdateSuc = false;
										/*开始给语音模块升级*/										
										g_RemoteUpdateGCFlag = 0;   
										g_ResendFlag = 0;									
										g_UpdateSuc = true;
										g_UpFlag = 0;

									}
									else
									{
										g_RemoteUpdateGCFlag = 4;	// 3	
										g_ResendFlag = 0;											
									}								
								}
								else
								{
									g_UpdateType = g_BulkRecvBuf[4];
									if(g_UpdateType == 1)
									{
										W25q_Write_N(g_UartUPBuf,FLASH_IAP_DIFFUSER_START1+496*g_RemoteUpdatePackNo,496);
										W25qxx_ReadBytes(g_checkbuf,FLASH_IAP_DIFFUSER_START1+496*g_RemoteUpdatePackNo,496);
									}
									else
									{
										W25q_Write_N(g_UartUPBuf,FLASH_IAP_START1+496*g_RemoteUpdatePackNo,496);								
										W25qxx_ReadBytes(g_checkbuf,FLASH_IAP_START1+496*g_RemoteUpdatePackNo,496);	
									}
										
									if(memcmp(g_UartUPBuf,g_checkbuf,496) == 0)
										g_RemoteUpdatePackNo++;

									if( g_RemoteUpdatePackNo == g_UpdateCodePageTotal.programpage )
									{
										if(g_UpdateType == 1)
										{
											DiffUpdateFileSize.FileSize = g_UpdateCodePageTotal.programpage;
											WriteDiffFileSizeToFlash();										
										}									
										else
										{	
											NormalUpdateFileSize.FileSize = g_UpdateCodePageTotal.programpage;
											WriteNormalFileSizeToFlash();
										}
										g_UpFlag = 0;
										g_RemoteUpdateGCFlag = 6;   
										g_ResendFlag = 0;									
										g_UpdateSuc = true;																				
									}
									else
									{
										g_RemoteUpdateGCFlag = 4;	// 3	
										g_ResendFlag = 0;									
									}								
								}							
							}
						}
					}				
				}
				break;
			case 0xFA:
				{
//					if(g_BulkRecvBuf[2] == 0x00 && g_BulkRecvBuf[2] == 0x07)
//					{//=7基站开始结束发送差分数据
//						if(g_BulkRecvBuf[63] == CheckSum(63,g_BulkRecvBuf))
//						{//校验和
//							if(g_BulkRecvBuf[3]== 0x00)
//							{//设置
//								g_SendDiffMark = g_BulkRecvBuf[34];
//								g_ReplyData.flag = 0x01;  
//								g_ReplyData.type = g_BulkRecvBuf[1];
//								memcpy(g_ReplyData.seri,g_BulkRecvBuf+64,4);						//序列号需保存   								
//							}
//							else if(g_BulkRecvBuf[3]== 0x01)
//							{//索要
//								g_ReplyData.flag = 0x01;  
//								g_ReplyData.type = g_BulkRecvBuf[2];
//								memcpy(g_ReplyData.seri,g_BulkRecvBuf+64,4);						//序列号需保存   							
//							}
//						}
//					}
//					else if(g_BulkRecvBuf[2] == 0x00 && g_BulkRecvBuf[2] == 0x09)
//					{//服务器设置基站工作模式
//						if(g_BulkRecvBuf[64] == CheckSum(64,g_BulkRecvBuf))
//						{							
//							if(g_BulkRecvBuf[3]== 0x00)							
//							{
//								g_ModeInFlash.type = g_BulkRecvBuf[4];
//								WriteModeToFlash();
//								g_LocationProg = 0;	
//								g_ReplyData.flag = 0x01;  
//								g_ReplyData.type = g_BulkRecvBuf[1];
//								memcpy(g_ReplyData.seri,g_BulkRecvBuf+64,4);						//序列号需保存							
//							}
//							else if(g_BulkRecvBuf[3]== 0x01)
//							{
//								g_ReplyData.flag = 0x01;  
//								g_ReplyData.type = g_BulkRecvBuf[2];
//								memcpy(g_ReplyData.seri,g_BulkRecvBuf+64,4);						//序列号需保存   							
//							}
//						}							
//					}
				}				
				break;	
				
				
			default:
				break;
		}
	}
	if(CelluarStep == 1)	
	{	
		if(g_RemoteUpdateGCFlag == 1)//判断是否升级程序
		{
			if(g_UpdateVoiceTypeFlag == 1)					
				g_RemoteUpdateGCFlag = 4;
			else
			{
//				g_RemoteUpdateGCFlag = 2;
//				g_UpFlag = 0;
			}				
		}
		g_InitAskParFlag = 0;
		CelluarStep = 0;
		g_NeedInitDTU = 1;
	}
	else if(JudgeMilSecondDelayInPro(g_DTUSendDataDelay, 50) == true)
	{
		if(g_RemoteUpdateGCFlag > 0)
		{
			if(g_RemoteUpdateGCFlag == 2)
			{
				GetUpdateType();
				g_ResendFlag = 1;
				g_RemoteUpdateGCFlag = 3;
				g_ResendDelay = uwTick;
				g_ResendTimes = 6;
			}
			else if(g_RemoteUpdateGCFlag == 3 && g_ResendFlag == 1 && JudgeMilSecondDelayInPro(g_ResendDelay, 4000) == true )
			{
				if(g_ResendTimes > 0 )
				{
					GetUpdateType();
					g_ResendTimes--;
					g_ResendDelay = uwTick;
				}
				else
				{
					g_ResendFlag = 0;
					g_ResendTimes = 5;
					g_RemoteUpdateGCFlag = 0;
					CelluarStep	= 0; //重启
					g_UpFlag = 0;
				}
			}
			else if(g_RemoteUpdateGCFlag == 4)
			{
				if(g_UpdateVoiceTypeFlag == 1)
				{					
					SendFullUpdateBag();
				}
				else
				{
					SendUpdateBag();
				}
				g_ResendFlag = 1;
				g_RemoteUpdateGCFlag = 5;
				g_ResendDelay = uwTick;
				g_ResendTimes = 6;				
			}
			else if(g_RemoteUpdateGCFlag == 5 && g_ResendFlag == 1 && JudgeMilSecondDelayInPro(g_ResendDelay, 4000) == true )
			{
				if(g_ResendTimes > 0 )
				{
					if(g_UpdateVoiceTypeFlag == 1)
					{					
						SendFullUpdateBag();
					}
					else
					{
						SendUpdateBag();
					}						
					g_ResendTimes--;
					g_ResendDelay = uwTick;
				}
				else
				{
					g_ResendFlag = 0;
					g_ResendTimes = 5;
					g_RemoteUpdateGCFlag = 0;
					CelluarStep	= 0; //重启
					g_UpFlag = 0;
				}
			}			
			else if(g_RemoteUpdateGCFlag == 6)
			{
				if(g_UpdateSuc)
				{
					if(g_UpdateType == 1)
					{
						g_UpdateType = 0;
						DiffProgramUserData();
					}
					g_UpdateSuc = false;
					HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);		//关闭DTU电源/*DTU断电*/
					g_InitAskParFlag = 0;
					SoftReset();
				}			
			}
		}
		else if(!g_SendD6Suc && JudgeMilSecondDelayInPro(g_SendD6Delay,10000) == true)
		{
			g_SendD6Suc = true;
			g_CtrlMsgFlag = 0;
		}	
		else
		{
			if(g_CtrlMsgFlag > 0 && g_InitAskParFlag > 0)
				ControlMsg();		
			/*发送差分数据至服务器 */
			if(g_PUNCOORDatainFlash.needsendcnt>0)//判断是否发送定位数据
			{/*发送打点数据*/
				SendPunCoorFunc();
				if(g_InitAskParFlag == 0)
				{
					g_InitAskParFlag = 1;
					g_AskParFlag = 1;
					g_CtrlMsgFlag = 1;
				}
			}
			else if(g_RecvRTKCnt >= RYDW_BOARDCAST_INTERVAL && WorkMode == 0 && g_RtkDiffLen > 0 && g_recvdiff == 1)//接收差分数据完整
			{
				if(g_GPSInfo.uMode != 7)
					HAL_UART_Transmit(&hUSART1, g_RtkDiffBuf, g_RtkDiffLen, 1000);		//转发RTK差分数据给GPS模块
				if(g_SendDiffMark == 1)
				{
					RYDW_GenerateDiffData();
					if(g_InitAskParFlag == 0)
					{
						g_InitAskParFlag = 1;
						g_AskParFlag = 1;
						g_CtrlMsgFlag = 1;
					}					
				}
				g_RecvRTKCnt =0;
			}	
			else if ((JudgeMilSecondDelayInPro(g_tmrUploadBasePosDelay, 30000) == true) || (g_BasePosUploaded == 1))
			{//发送定位数据
				g_BasePosUploaded = 0;
				g_tmrUploadBasePosDelay = uwTick;
				memset(g_UploadRecord, 0x00, sizeof(g_UploadRecord));
				RYDW_GenerateBaseStationPosRecord(g_UploadRecord, &g_UploadRecordLen,1);		//生成基站定位数据
				if(g_InitAskParFlag == 0)
				{
					g_InitAskParFlag = 1;
					g_AskParFlag = 1;
					g_CtrlMsgFlag = 1;
				}				
			}
			ProcessDtuNoAnswerFunc();		
		}
	}
}


/********** 测试函数 ************/
void InsertDARecord()
{
	g_CardTable.CardNbr = 1;
	g_CardTable.Info[0].ID.byte.AdminID = 1;
	g_CardTable.Info[0].ID.byte.SectionID = 1;
	g_CardTable.Info[0].ID.byte.VrtWorkshopID = 0;
	g_CardTable.Info[0].ID.byte.WorkshopID = 0;
	g_CardTable.Info[0].ID.byte.WorkareaID = 0;
	g_CardTable.Info[0].ID.byte.UserID = 2;
	/* lat=37.8043823 long=112.5540619 */ 
	g_CardTable.Info[0].Latitude = 378043823;
	g_CardTable.Info[0].Longitude = 1125540619;
	g_CardTable.Info[0].PosQuality = 5;
	g_CardTable.Info[0].Altitude[0] = 0;
	g_CardTable.Info[0].Altitude[1] = 0;
	g_CardTable.Info[0].Altitude[2] = 0;
	g_CardTable.Info[0].Alram.all = 0x80;
	g_CardTable.Info[0].BatVoltage = 37;
	RYDW_GenerateCardRecord(g_CardTable, g_UploadRecord, &g_UploadRecordLen);		//生成定位卡数据
}
//void InsertC3Record(xQueueHandle RecordQue)
//{
//	char	tmp[256];
//	uint16_t i;
//	char ascstr[2];
//	
//	BulkRecord_t rq;
//	
//	memcpy(tmp, "C340626126329F24041000000000000000000000000000000000000000000000000000FFFFFFFFFFFFFFFFFFFF00000000000000780082000000000052FC7F4E", 128);
//	for (i=0; i<128; i++)
//	{
//		memcpy(ascstr, tmp + (i * 2), 2);
//		rq.RecordBody[i] = Asc2HexInString((uint8_t*)&ascstr);
//	}
//	
//	rq.uLength = 64;
//	xQueueSend(RecordQue, &rq, 0);		//入队列
//}

