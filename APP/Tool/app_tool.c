#include "app_tool.h"
#include "RTK_Comm.h"
#include "bsp.h"
#include "main.h"
#include "Drv_W25Qxx_QSPI.h"
#include "g_GlobalValDef.h"
#include "Update.h"
#include "DTU_BulkData.h"
#include "iap.h"
#include "GPS_Comm.h"
#include "RYDW_Locate.h"
#include "ssd1306.h"
#include "RYDW_Init.h"
#include "app_dtu.h"
uint8_t g_ToolDatBuf[UART5_RX_BUFSIZE];

uint8_t g_ToolCommandFlag;
uint8_t g_ToolBaowenFlag;        /* =0 结束或者没有开始，=1 收到头，=2 收到中间*/
uint16_t g_LocalUart5SendBuf[20];
/*-----本地数据转储-----------------------------------------------------*/
/*=0 初始值，收到0xe0  =1 ，收到0xe1 =2 ,收到页号= 3 ，收到0xe3 = 4 删除雨量数据 */
uint8_t g_LocalZCFlag;
uint8_t g_LocalZCStartFlashPage;  /* 每64K 数据中页的序号 */
uint8_t g_LocalZCTotal64kPage;    /* 共有多少个64K 页 */
uint8_t g_LocalZCRec64KPageNo;    /* 数据转储包的页号 */
uint8_t g_LocalZC64kSum;          /* 每次发送数据进行计算的校验和 */

/*-----本地程序升级-----------------------------------------------------*/
/* =0 初始状态，= 1 收到客服工具下发的命令，=2 收到客服下发的升级数据包, =3 发送了数据等待下一个命令*/
uint8_t g_LocalUpdateFlag;
uint16_t g_LocalUpdateQuestPageNo;/* 雨量主板请求数据包的序号，从0开始 */
uint8_t g_Uart2UpdateFlag;        /* 临时判断升级数据是命令还是数据的标志 */
uint32_t g_ToolRecTimer;         /* 本地升级数据定时75MS收不到数据的定时器，重发 */
uint8_t g_ToolResendTimers;     /* 本地升级数据重发次数 */
uint8_t g_ToolResendFlag;         /* 本地数据升级重发的标志位 */
//uint8_t g_TransferWithToolFlag;   /* 和客服工具通信标识 */
uint8_t g_ToolCodeTypeErrorFlag;		//程序程序类型错误标记


uint16_t g_UartUpOffset;
uint16_t g_UartUpPageNo;
uint8_t MajorVersion;                     //主版本号   
uint8_t MinorVersion;                     //次版本号
uint8_t g_UartUPBuf[1488];

__align(4) uint8_t IAP_Tmp[512];		    // 定义4K空间，编程Flash时使用


uint8_t g_ToolCodeTypeErrorFlag;		//程序程序类型错误标记
UPDATAPROGRAM g_UpdataCodePageTotal;
uint8_t 	g_Signal;

void InitToolVar(void)
{
	g_ToolCommandFlag = 0;
	g_ToolBaowenFlag = 0;
	g_LocalZCFlag = 0;

	g_LocalZCStartFlashPage = 0;/* 每64K 数据中页的序号 */
	g_LocalZCTotal64kPage = 64; /* 共有多少个64K 页 */
	g_LocalZCRec64KPageNo =0;  /* 收到64K 页的序号 */

	g_LocalUpdateFlag = 0;
	g_ToolRecTimer = uwTick;	
}
/******************************************************************
函数名称: void CheckTool(void)
功能描述: 检测客服工具是否插入功能函数
参数输入: 无
参数输出: 
返 回 值: 无
******************************************************************/
void CheckTool(void)
{
	if(HAL_GPIO_ReadPin(TOOL_INDET_PORT,TOOL_INDET_PIN) == GPIO_PIN_SET && g_CheckTool == 0)
	{
		g_CheckTool = 1;
		HAL_GPIO_WritePin(TOOL_PWR_PORT, TOOL_PWR_PIN, GPIO_PIN_SET);		//关闭客服工具电源
	}
	else if(HAL_GPIO_ReadPin(TOOL_INDET_PORT,TOOL_INDET_PIN) == GPIO_PIN_RESET && g_CheckTool == 1)
	{
		g_CheckTool = 0;
		HAL_GPIO_WritePin(TOOL_PWR_PORT, TOOL_PWR_PIN, GPIO_PIN_RESET);		//打开客服工具电源
		InitToolVar();
	}

}
/******************************************************************
函数名称: void Local_Update_Quest(void)
功能描述: 通过客服工具升级程序的应答包
参数输入: 无
参数输出: 
返  回  值 : 无
其           他: 无
******************************************************************/
void Local_Update_Quest(void )
{
	memset(g_TxBuf5,0,sizeof(g_TxBuf5));

	g_TxBuf5[0]=0xEE;
	g_TxBuf5[1] = g_MobileInFlash.mobileno[4][0];
	g_TxBuf5[2] = g_MobileInFlash.mobileno[4][1];
	g_TxBuf5[3] = g_MobileInFlash.mobileno[4][2];
	g_TxBuf5[4] = g_MobileInFlash.mobileno[4][3];
	g_TxBuf5[5] = g_MobileInFlash.mobileno[4][4];
	g_TxBuf5[6] = g_MobileInFlash.mobileno[4][5];
	g_TxBuf5[7] = g_UpdataCodePageTotal.programtype;
	g_TxBuf5[8] = g_UpdataCodePageTotal.mainverno;
	g_TxBuf5[9] = g_UpdataCodePageTotal.secverno;
    g_TxBuf5[10] = ( g_UpdataCodePageTotal.programpage & 0xFF );
	g_TxBuf5[11] = ( g_UpdataCodePageTotal.programpage>>8);
	
	if( g_LocalUpdateQuestPageNo == g_UpdataCodePageTotal.programpage || g_ToolCodeTypeErrorFlag >0)
	{
		g_TxBuf5[12] = 0xEE;
		g_TxBuf5[13] = 0xEE;
		g_LocalUpdateFlag = 0;
		g_ToolResendFlag = 0;
		
        if(g_ToolCodeTypeErrorFlag == 0)
		{
			if(g_UartUpOffset != 0)
			{
				memset(g_UartUPBuf + g_UartUpOffset,0xff,1488 - g_UartUpOffset);				
				if(g_UpdateVoiceTypeFlag == 1)
				{	
					W25qxx_WritePages((uint8_t*)&g_UartUPBuf,FLASH_VOICE_START+g_UartUpPageNo*2*256,256);
					W25qxx_WritePages((uint8_t*)&g_UartUPBuf+256,FLASH_VOICE_START+g_UartUpPageNo*2*256+256,256);					
				}
				else 
				{
					W25qxx_WritePages((uint8_t*)&g_UartUPBuf,FLASH_IAP_START1+g_UartUpPageNo*2*256,256);
					W25qxx_WritePages((uint8_t*)&g_UartUPBuf+256,FLASH_IAP_START1+g_UartUpPageNo*2*256+256,256);	
				}					
			}				
			if( g_UpdateVoiceTypeFlag == 1)
			{
				VoiceFileSize.FileSize = g_UpdataCodePageTotal.programpage;
				WriteVoiceFileSizeToFlash();
				g_UpdateVoiceTypeFlag = 0;				
			}
			else 
			{	
				NormalUpdateFileSize.FileSize = g_UpdataCodePageTotal.programpage;
				WriteNormalFileSizeToFlash();
				g_ShouldJumpToStartFlag = 2;
			}					
		}
		else if(g_ToolCodeTypeErrorFlag == 2)
			g_TxBuf5[14] = 2;
		g_UpFlag = 0;			
	}
	else
	{
		g_TxBuf5[12] = ( g_LocalUpdateQuestPageNo & 0xff ) ;
		g_TxBuf5[13] = (( g_LocalUpdateQuestPageNo>>8) & 0xff ) ;
		g_LocalUpdateFlag = 3;
	}	
	g_TxBuf5[15] = CheckSum( 15*2, (uint8_t*)&g_TxBuf5);	
	UartSend(&hUART5,(uint8_t*)&g_TxBuf5,16);	
}

/******************************************************************
函数名称: RtkResult RTK_RecvBuf
功能描述: 接收客服工具数据
参数输入: 
参数输出: 
返 回 值: RtkResult 返回值
其    他: 无
******************************************************************/
TOOLResult Tool_RecvBuf(uint8_t *OutBuf, uint16_t *StrLen)
{
	uint16_t rlen;

	/* 接收客服工具数据 */
	rlen = GetRingUsedLen(&g_Uart5RxRing);
	if (rlen > 0)
	{
		memset(OutBuf, 0x00, UART5_RX_BUFSIZE);
		if (rlen > UART5_RX_BUFSIZE)
		{
			RingBufClear(&g_Uart5RxRing);
			*StrLen = 0;
			return TOOLRET_LEN_ERR;
		}
		else
		{
			RingBufPop(&g_Uart5RxRing, OutBuf, rlen);
			*StrLen = rlen;
			return TOOLRET_OK;
		}
	}
	else
	{
		*StrLen = 0;	//无数据
	}
	
	return TOOLRET_NODATA_ERR;
}
/******************************************************************
函数名称: void WriteMyNumberFromTool(uint8_t *buf)
功能描述: 用客服工具给主板写本机号
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteMyNumberFromTool(uint8_t *buf)
{
    uint8_t tempmobile[6];
    uint8_t cmpbuf[5];

	memset(cmpbuf,0,sizeof(cmpbuf));
    memcpy(tempmobile, buf+3, 6);
    if(CheckNumIsBCD((uint8_t*)tempmobile,5) == true && memcmp((uint8_t*)tempmobile,cmpbuf,5)!=0)
    {
        if(0 != memcmp(g_MobileInFlash.mobileno[4],tempmobile,sizeof(tempmobile)))
        {
            memcpy(g_MobileInFlash.mobileno[4],tempmobile,sizeof(tempmobile));                                                    
        }
    }
    WriteMobileNoToFLASH();
	if(ReadMobileNoFromFLASH() == true)
	{
		g_GetMyMobileOKFlag = true;
		CelluarStep = STA_CLOSE_TCP;//重连服务器
	}  
}
/******************************************************************
函数名称: void ReturnMyMobileToTool(void)
功能描述: 给客服工具返回本机号
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnMyMobileToTool(void)
{    
	uint8_t i;
	memset(g_TxBuf5,0,sizeof(g_TxBuf5));
    g_TxBuf5[0] = SENDTAG_ALL;
    g_TxBuf5[1] = GET_MY_NUMBER_TYPE;
    g_TxBuf5[2] = 12;
    if(ReadMobileNoFromFLASH() == true)
    {       						
		
		g_TxBuf5[3] = g_MobileInFlash.mobileno[4][0];
		g_TxBuf5[4] = g_MobileInFlash.mobileno[4][1];
		g_TxBuf5[5] = g_MobileInFlash.mobileno[4][2];
		g_TxBuf5[6] = g_MobileInFlash.mobileno[4][3];
		g_TxBuf5[7] = g_MobileInFlash.mobileno[4][4];
		g_TxBuf5[8] = g_MobileInFlash.mobileno[4][5];		
    }
    else
	{	
        for(i=0;i<6;i++)
		{
			g_TxBuf5[3 + i] = 0xff;
		}       	
	}
	for(i=0;i<6;i++)
	{
		g_TxBuf5[9] = 0x00;
	}	
    g_TxBuf5[15] = CheckSum(15*2,(uint8_t*)&g_TxBuf5);
	UartSend(&hUART5,(uint8_t*)&g_TxBuf5,16);	
}

/******************************************************************
函数名称: void CmpIPAndPort(void)
功能描述: 判断IP和端口号若为0或者FF，替换为默认的值
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void CmpIPAndPort(void)
{
    uint8_t cmpbuffer[2][4];
    uint8_t i,err=0;
    
    memset(cmpbuffer[0],0,sizeof(cmpbuffer[0]));
    memset(cmpbuffer[1],0xff,sizeof(cmpbuffer[1]));
    for(i=0;i<3;i++)
    {
        if( memcmp(g_ServerInfo.g_ServerIPAddr[i],cmpbuffer[0],sizeof(g_ServerInfo.g_ServerIPAddr[i])) == 0 || 
            memcmp(g_ServerInfo.g_ServerIPAddr[i],cmpbuffer[1],sizeof(g_ServerInfo.g_ServerIPAddr[i])) == 0 ||
            g_ServerInfo.g_ServerPortNo[i] == 0 || g_ServerInfo.g_ServerPortNo[i] == 0xff)
        {
            memcpy(g_ServerInfo.g_ServerIPAddr[i],g_IPBuffer[i],sizeof(g_ServerInfo.g_ServerIPAddr[i]));
            g_ServerInfo.g_ServerPortNo[i] = g_PortNo[i];
			g_ServerInfo.Crc16 = CRC16((uint8_t*)&g_ServerInfo,sizeof(g_ServerInfo)-2);
			err = 1;
        }
    }
	if(err == 1)
	{
		WriteDTUSysPar();
	}		

}

/******************************************************************
函数名称: void WriteIPFromTool(uint8_t *buf)
功能描述: 用客服工具给主板设置IP
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void WriteIPFromTool(uint8_t *buf)
{
    uint8_t tempmobile[3][6];
    uint8_t j,k;
    memcpy(tempmobile,buf+3,sizeof(tempmobile));
    j = 0;
    ReadServerInfoPar();                                                //读Flash

    for( k = 0 ; k < 3 ; k++ )
    {
        if((tempmobile[k][0] == 0 || tempmobile[k][1] == 0 || tempmobile[k][2] == 0 || tempmobile[k][3] == 0) || (tempmobile[k][4] == 0 && tempmobile[k][5] == 0)||
            (tempmobile[k][0] == 0xff || tempmobile[k][1] == 0xff || tempmobile[k][2] == 0xff || tempmobile[k][3] == 0xff) || tempmobile[k][4] == 0xff || tempmobile[k][5] == 0xff)
            ;
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
		ReadServerInfoPar();                             //从Flash读出

		CmpIPAndPort();                                         //若有全零或全F的替换为默认值
		if((memcmp(g_DataIPBack,g_ServerInfo.g_ServerIPAddr[2],4) != 0) || (g_DataPortBack != g_ServerInfo.g_ServerPortNo[2]))  //若数据IP修改了
		{
			CelluarStep = STA_CLOSE_TCP;//重连服务器
		}
    }
}  
/******************************************************************
函数名称: void ReturnIPToTool(void)
功能描述: 给客服工具返回IP
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnIPToTool(void)
{
    memset(g_TxBuf5,0,sizeof(g_TxBuf5));
    g_TxBuf5[0] = SENDTAG_ALL;
    g_TxBuf5[1] = GET_IP_TYPE;
    g_TxBuf5[2] = 28;
	g_TxBuf5[3] = g_ServerInfo.g_ServerIPAddr[2][0];					 //数据IP
	g_TxBuf5[4] = g_ServerInfo.g_ServerIPAddr[2][1];	
	g_TxBuf5[5] = g_ServerInfo.g_ServerIPAddr[2][2];	
	g_TxBuf5[6] = g_ServerInfo.g_ServerIPAddr[2][3];    
    g_TxBuf5[7] = (uint8_t)(g_ServerInfo.g_ServerPortNo[2]/100);         //数据端口
    g_TxBuf5[8] = (uint8_t)(g_ServerInfo.g_ServerPortNo[2]%100);
    
	g_TxBuf5[9] = g_ServerInfo.g_ServerIPAddr[0][0];                     //升级IP
	g_TxBuf5[10] = g_ServerInfo.g_ServerIPAddr[0][1];	
	g_TxBuf5[11] = g_ServerInfo.g_ServerIPAddr[0][2];	
	g_TxBuf5[12] = g_ServerInfo.g_ServerIPAddr[0][3];    
    g_TxBuf5[13] = (uint8_t)(g_ServerInfo.g_ServerPortNo[0]/100);        //升级端口
    g_TxBuf5[14] = (uint8_t)(g_ServerInfo.g_ServerPortNo[0]%100);   
    
	g_TxBuf5[15] = g_ServerInfo.g_ServerIPAddr[1][0];	                 //转储IP
	g_TxBuf5[16] = g_ServerInfo.g_ServerIPAddr[1][1];	
	g_TxBuf5[17] = g_ServerInfo.g_ServerIPAddr[1][2];	
	g_TxBuf5[18] = g_ServerInfo.g_ServerIPAddr[1][3];	
    g_TxBuf5[19] = (uint8_t)(g_ServerInfo.g_ServerPortNo[1]/100);        //转储端口
    g_TxBuf5[20] = (uint8_t)(g_ServerInfo.g_ServerPortNo[1]%100);
    g_TxBuf5[31] = CheckSum(31*2,(uint8_t*)&g_TxBuf5);
	UartSend(&hUART5,(uint8_t*)&g_TxBuf5,32);	
}

/******************************************************************
函数名称: void ReturnParamInforToTool(void)
功能描述: 给客服工具返回参数信息
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnParamInforToTool(void)
{
	uint8_t i;
    
    memset(g_TxBuf5,0,sizeof(g_TxBuf5));
    
    g_TxBuf5[0] = SENDTAG_ALL;
    g_TxBuf5[1] = GET_PARAM_INFOR_TYPE;
    g_TxBuf5[2] = 64;
	g_TxBuf5[3] = g_Signal;    					//DTU信号强度
	g_TxBuf5[4] = g_DataNetOk;							//GPRS连接状态
    TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
	g_TxBuf5[5] = g_BCDTime.Year;
	g_TxBuf5[6] = g_BCDTime.Month;
	g_TxBuf5[7] = g_BCDTime.Day;
	g_TxBuf5[8] = g_BCDTime.Hour;
	g_TxBuf5[9] = g_BCDTime.Minute;
	g_TxBuf5[10] = g_BCDTime.Second;	
	g_TxBuf5[11] = g_ServerInfo.g_ServerIPAddr[2][0];							//数据IP
	g_TxBuf5[12] = g_ServerInfo.g_ServerIPAddr[2][1];	
	g_TxBuf5[13] = g_ServerInfo.g_ServerIPAddr[2][2];	
	g_TxBuf5[14] = g_ServerInfo.g_ServerIPAddr[2][3];	
	g_TxBuf5[15] = (uint8_t)(g_ServerInfo.g_ServerPortNo[2]/100);              //数据端口
	g_TxBuf5[16] = (uint8_t)(g_ServerInfo.g_ServerPortNo[2]%100);
	g_TxBuf5[17] = g_ServerInfo.g_ServerIPAddr[0][0];                           //升级IP
	g_TxBuf5[18] = g_ServerInfo.g_ServerIPAddr[0][1];	
	g_TxBuf5[19] = g_ServerInfo.g_ServerIPAddr[0][2];	
	g_TxBuf5[20] = g_ServerInfo.g_ServerIPAddr[0][3];	
	g_TxBuf5[21] = (uint8_t)(g_ServerInfo.g_ServerPortNo[0]/100);               //升级端口
	g_TxBuf5[22] = (uint8_t)(g_ServerInfo.g_ServerPortNo[0]%100);
	g_TxBuf5[23] = g_ServerInfo.g_ServerIPAddr[1][0];	                           //转储IP
	g_TxBuf5[24] = g_ServerInfo.g_ServerIPAddr[1][1];	
	g_TxBuf5[25] = g_ServerInfo.g_ServerIPAddr[1][2];	
	g_TxBuf5[26] = g_ServerInfo.g_ServerIPAddr[1][3];	
	g_TxBuf5[27] = (uint8_t)(g_ServerInfo.g_ServerPortNo[1]/100);                //转储端口
	g_TxBuf5[28] = (uint8_t)(g_ServerInfo.g_ServerPortNo[1]%100);
	
	for(i=0;i<5;i++)
	{
		g_TxBuf5[29+6*i] = g_MobileInFlash.mobileno[i][0];
		g_TxBuf5[30+6*i] = g_MobileInFlash.mobileno[i][1];
		g_TxBuf5[31+6*i] = g_MobileInFlash.mobileno[i][2];
		g_TxBuf5[32+6*i] = g_MobileInFlash.mobileno[i][3];
		g_TxBuf5[33+6*i] = g_MobileInFlash.mobileno[i][4];
		g_TxBuf5[34+6*i] = g_MobileInFlash.mobileno[i][5];
	}	
	
	g_TxBuf5[59] = g_VersionAscii[0];
	g_TxBuf5[60] = g_VersionAscii[1];
	g_TxBuf5[61] = g_VersionAscii[2];
	g_TxBuf5[62] = g_VersionAscii[3];
	g_TxBuf5[63] = 0;	
    g_TxBuf5[67] = CheckSum(67*2,(uint8_t*)&g_TxBuf5);
	UartSend(&hUART5,(uint8_t*)&g_TxBuf5,68);	
}

/******************************************************************
函数名称: void ReturnGPSInfoToTool(void)
功能描述: 给客服工具返回定位信息
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnGPSInfoToTool(void)
{
	uint8_t i;
    
    memset(g_TxBuf5,0,sizeof(g_TxBuf5));
    
    g_TxBuf5[0] = RYDWRECEIVETAG;
    g_TxBuf5[1] = GET_GPS_INFO;
    g_TxBuf5[2] = 28;	
	g_TxBuf5[3] = (uint8_t)(g_gpsco.g_Latitude & 0xFF);             //纬度
	g_TxBuf5[4] = (uint8_t)((g_gpsco.g_Latitude>>8) & 0xFF);	
	g_TxBuf5[5] = (uint8_t)((g_gpsco.g_Latitude>>16) & 0xFF);	
	g_TxBuf5[6] = (uint8_t)((g_gpsco.g_Latitude>>24) & 0xFF);

	g_TxBuf5[7] = (uint8_t)(g_gpsco.g_Longitude & 0xFF);            //经度
	g_TxBuf5[8] = (uint8_t)((g_gpsco.g_Longitude>>8) & 0xFF);	
	g_TxBuf5[9] = (uint8_t)((g_gpsco.g_Longitude>>16) & 0xFF);	
	g_TxBuf5[10] = (uint8_t)((g_gpsco.g_Longitude>>24) & 0xFF);
	
	g_TxBuf5[11] = (uint8_t)(g_gpsco.g_Altitude & 0xFF);           //高程
	g_TxBuf5[12] = (uint8_t)((g_gpsco.g_Altitude >> 8) & 0xFF);
	g_TxBuf5[13] = (uint8_t)((g_gpsco.g_Altitude >> 16) & 0xFF);   	
	
	g_TxBuf5[14] = g_SatelliteNum;                                //星数
	g_TxBuf5[15] = g_GPSInfo.uMode;                               //定位质量
	
    TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);                //时间
	g_TxBuf5[16] = g_BCDTime.Year;
	g_TxBuf5[17] = g_BCDTime.Month;
	g_TxBuf5[18] = g_BCDTime.Day;
	g_TxBuf5[19] = g_BCDTime.Hour;
	g_TxBuf5[20] = g_BCDTime.Minute;
	g_TxBuf5[21] = g_BCDTime.Second;
	
	g_TxBuf5[22] = (uint8_t)(g_fBatVal*10);                      //电池电压
	if(g_MarkWarnFlag == 0x01)
		g_TxBuf5[23] = 0x01; 
	else
		g_TxBuf5[23] = 0x00; 
	g_TxBuf5[31] = CheckSum(31*2,(uint8_t*)&g_TxBuf5);
	UartSend(&hUART5,(uint8_t*)&g_TxBuf5,32);		
}

/******************************************************************
函数名称: void ReturnModeTypeToTool(void)
功能描述: 给客服工具返回工作模式
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnModeTypeToTool(void)
{
	uint8_t i;
    
    memset(g_TxBuf5,0,sizeof(g_TxBuf5));
    
    g_TxBuf5[0] = RYDWRECEIVETAG;
    g_TxBuf5[1] = GET_MODE_TYPE;
    g_TxBuf5[2] = 12;	
	g_TxBuf5[3] = g_ModeInFlash.type;

	g_TxBuf5[15] = CheckSum(15*2,(uint8_t*)&g_TxBuf5);
	UartSend(&hUART5,(uint8_t*)&g_TxBuf5,16);		
}
void ProcessDataWithTool(uint8_t *indata, uint8_t inlen)
{
	uint16_t cnt1,cnt2,crc;
	uint32_t pageaddr[4];
	
    switch(indata[0])                           //判断包头
    {
        case RECEIVETAG_ALL:                    //若是通用包头
        {
            switch(indata[1])
            {                           
				case SET_MY_NUMBER_TYPE:                //设置本机号
                    WriteMyNumberFromTool(indata);
                    break;
                
                case GET_MY_NUMBER_TYPE:                //索要本机号
                    ReturnMyMobileToTool();
                    break;
                
                case SET_IP_TYPE:                       //设置IP
                    WriteIPFromTool(indata);
                    break;
                
                case GET_IP_TYPE:                       //索要IP
                    ReturnIPToTool();
                    break;
                    
                case GET_PARAM_INFOR_TYPE:              //索要参数信息
                    ReturnParamInforToTool();
                    break;						                                      												
//	
//				case RESET_DTU_TYPE:
//					g_ResetDTUFlag = 2;
//					g_ToolResetDtu = 1;
//					PWR_DTU_DISABLE();                            //给DTU断电
//					g_DTUResetTime = TIM2->CNT;				
//					break;
//				case RESET_SYS_TYPE:
//					WritePowerFlagToFlash(4);
//					delay_ms(500);
//					PWR_SYS3_3V_DISABLE();		
//					break;			
                default:
                    break;
            }
            break;
        }
		case RYDWSENDTAG:                    //若是通用包头
		{
			switch(indata[1])
			{
				case GET_GPS_INFO:                       //索要IP
					ReturnGPSInfoToTool();
					break;
				case SET_MODE_TYPE:                       //索要IP
					g_ModeInFlash.type = indata[3];
					g_LocationProg = 0;	
					WriteModeToFlash();
					ssd1306_Fill(Black);														
					if(g_ModeInFlash.type == 1)
					{																												
						g_NeedInitDTU = 1;
						g_NeedInitRTK = 1;
					}	
					break;
				case GET_MODE_TYPE:                       //索要IP
					ReturnModeTypeToTool();
					break;								
				default:
					break;				
			}
		}
		default:
		break;
    }
    
}



void ToolProcess(void)
{
	uint16_t pos,len;
	uint32_t i,j;
	int16_t dbgret;

	if(JudgeMilSecondDelayInPro(g_Uart5TxTimer,5) == true && g_Uart5DirFlag == 2)
	{
		Tool485_RX_EN();	
		g_Uart5DirFlag = 1;	
	}
	else
	{
		memset(g_ToolDatBuf, 0x00, sizeof(g_ToolDatBuf));
		dbgret = Tool_RecvBuf(g_ToolDatBuf,&len);

		if(dbgret == TOOLRET_OK)
		{	
			if(g_ToolDatBuf[0] == 0xE0)
			{
				if(g_ToolCommandFlag == 0)     //agj 20150207
				{
					g_LocalZCRec64KPageNo = 0;  //agj
					
					g_ToolCommandFlag = 2;
					g_LocalZCFlag = 1;
					g_ToolBaowenFlag = 2;
				}
				
			}
			else if(g_ToolDatBuf[0] == 0xE2)
			{																														
				if(g_ToolCommandFlag == 0 || g_ToolCommandFlag == 2)
				{
					g_ToolCommandFlag = 2;
					g_LocalZCFlag = 2;
					g_ToolBaowenFlag = 0;
					
					if(( g_ToolCommandFlag == 2 ) && (g_ToolDatBuf[1] < g_LocalZCTotal64kPage) 
						&&(( g_LocalZCFlag == 2 ) ||( g_LocalZCFlag == 3 )))
					{			
						g_ToolCommandFlag = 2;
						g_LocalZCFlag = 3;
						g_ToolBaowenFlag = 2;
						/* 用于转储发送数据 */
						g_LocalZCStartFlashPage = 0;			
						g_LocalZCRec64KPageNo = g_ToolDatBuf[ 1 ];
						g_LocalZC64kSum = 0;	        
					}		
					
				}		
			}
			else if(g_ToolDatBuf[0] == 0xF1)
			{
				if(g_ToolCommandFlag == 0 || g_ToolCommandFlag == 2)
				{		
					g_ToolCommandFlag = 2;
					g_LocalZCFlag = 0;
					g_ToolBaowenFlag = 0;	
				}		
			}
			
			else if(( g_ToolCommandFlag == 2 ) && (g_ToolDatBuf[0] < g_LocalZCTotal64kPage) 
				&&(( g_LocalZCFlag == 2 ) ||( g_LocalZCFlag == 3 )))
			{			
				g_ToolCommandFlag = 2;
				g_LocalZCFlag = 3;
				g_ToolBaowenFlag = 2;
				/* 用于转储发送数据 */
				g_LocalZCStartFlashPage = 0;			
				g_LocalZCRec64KPageNo = g_ToolDatBuf[ 0 ];
				g_LocalZC64kSum = 0;	       
			}		
			
			else if(g_ToolDatBuf[0] == 0xEE && g_ToolDatBuf[1] == 0xEE && g_ToolDatBuf[15] == CheckSum(15,g_ToolDatBuf) )
			{
				memcpy((uint8_t*)&g_UpdataCodePageTotal,g_ToolDatBuf+8,3);
				
				g_UpdataCodePageTotal.programpage = Get16ValueFrom8Buf(g_ToolDatBuf,11);
				if(g_UpdataCodePageTotal.programtype == CODETYPE)
				{
					g_UpFlag = 1;
					g_ToolCodeTypeErrorFlag = 0;				
					g_LocalUpdateFlag = 1;					
					g_LocalUpdateQuestPageNo = 0;
					g_ToolCommandFlag = 1;
					g_ToolBaowenFlag = 2;
					g_UartUpOffset = 0;
					g_UartUpPageNo=0;
	//				g_DispUpdate = 1;                   //本地升级开始标志																
					MajorVersion = g_ToolDatBuf[9];  
					MinorVersion = g_ToolDatBuf[10];
					j = (g_UpdataCodePageTotal.programpage *496)/w25qxx.SectorSize+1;
					for(i=0;i<j;i++)
					{      
						W25qxx_EraseSector(FLASH_IAP_START1/w25qxx.SectorSize+i);
					}	
				}
				else if(g_UpdataCodePageTotal.programtype == VOICECODETYPE && g_ShouldUpdateVoiceFlag == 0)
				{
					g_UpFlag = 1;
					g_ToolCodeTypeErrorFlag = 0;				
					g_LocalUpdateFlag = 1;					
					g_LocalUpdateQuestPageNo = 0;
					g_ToolCommandFlag = 1;
					g_ToolBaowenFlag = 2;
					g_UartUpOffset = 0;
					g_UartUpPageNo=0;
	//				g_DispUpdate = 1;                   //本地升级开始标志																
					MajorVersion = g_ToolDatBuf[9];  
					MinorVersion = g_ToolDatBuf[10];
					j = (g_UpdataCodePageTotal.programpage *496)/w25qxx.SectorSize+1;
					g_UpdateVoiceTypeFlag = 1;
					for(i=0;i<j;i++)
					{      
						W25qxx_EraseSector(FLASH_VOICE_START/w25qxx.SectorSize+i);
					}	
				}	
				else
				{
					g_ToolCodeTypeErrorFlag = 1;
					g_LocalUpdateFlag = 1;
					g_ToolCommandFlag = 1;
					g_ToolBaowenFlag = 2;
				}			
			}
			else if(g_ToolDatBuf[0] == 0xDC  && g_ToolDatBuf[1] == 0xDC && g_ToolDatBuf[511] == CheckSum(511,g_ToolDatBuf) )
			{
				if((StrNcmpString(g_ToolDatBuf+7,(uint8_t*)&g_UpdataCodePageTotal,3,&pos) == 0)
					&& (g_ToolDatBuf[10] == (g_UpdataCodePageTotal.programpage&0xFF)) 
					&& g_ToolDatBuf[11] == (g_UpdataCodePageTotal.programpage>>8)
					&&( Get16ValueFrom8Buf( g_ToolDatBuf , 12 ) == g_LocalUpdateQuestPageNo))					
				{
					memcpy(g_UartUPBuf+g_UartUpOffset,g_ToolDatBuf + 14,496);									
					g_UartUpOffset += 496;
					if(g_UartUpOffset >= 512)
					{				
						
						if(g_UpdateVoiceTypeFlag == 1)
						{
							W25qxx_WritePages((uint8_t*)&g_UartUPBuf,FLASH_VOICE_START+g_UartUpPageNo*2*256,256);
							W25qxx_WritePages((uint8_t*)&g_UartUPBuf+256,FLASH_VOICE_START+g_UartUpPageNo*2*256+256,256);
							
						}						
						else
						{						
							W25qxx_WritePages((uint8_t*)&g_UartUPBuf,FLASH_IAP_START1+g_UartUpPageNo*2*256,256);
							W25qxx_WritePages((uint8_t*)&g_UartUPBuf+256,FLASH_IAP_START1+g_UartUpPageNo*2*256+256,256);	
						}							
						g_UartUpPageNo++;
						memcpy ( g_UartUPBuf , g_UartUPBuf + 512 , g_UartUpOffset - 512 );
						g_UartUpOffset = g_UartUpOffset-512;			
					}																	
					g_LocalUpdateQuestPageNo++;	
					g_LocalUpdateFlag = 2;
					g_ToolBaowenFlag = 2;	
					g_ToolCommandFlag = 1;
					g_ToolResendFlag = 0;
				}		
			}
			else if(g_ToolDatBuf[0] == RECEIVETAG_ALL || g_ToolDatBuf[0] == RYDWSENDTAG)
			{
				ProcessDataWithTool(g_ToolDatBuf,len);
			}
		}
	}
	//发送处理
	if(g_Uart5DirFlag == 1)
	{
	
//		if(g_ToolResetDtu == 2)
//		{
//			ReturnResetDtuResToTool(1);
//			g_ToolResetDtu = 0;
//		}
//		else if(g_ToolRetSys == 1)
//		{
//			ReturnResetSysResToTool(1);
//			g_ToolRetSys = 0;
//		}
		if(g_ToolBaowenFlag == 2)
		{
			if(g_ToolCommandFlag == 1)		//升级
			{
				if(g_LocalUpdateFlag == 1 || g_LocalUpdateFlag == 2)
				{
					g_LocalUpdateFlag = 0;
					Local_Update_Quest();
					delay_us(120);                   //agj add start
					g_ToolResendTimers = 20;
					g_ToolResendFlag = 1;
					g_ToolRecTimer = uwTick;	

				}
			}
			else if(g_ToolCommandFlag == 2)	//转储
			{
				if( g_LocalZCFlag == 1 )
				{
					g_LocalUart5SendBuf[ 0 ] = 0x01;
					g_LocalUart5SendBuf[ 1 ] = 0x00;		
					g_LocalUart5SendBuf[ 2 ] = 0x00;		
					g_LocalUart5SendBuf[ 3 ] = 0x00;		
					g_LocalUart5SendBuf[ 4 ] = g_LocalZCTotal64kPage;	
					Tool485_TX_EN();
					g_Uart5DirFlag = 2;	
					delay_us(2000);
					HAL_UART_Transmit(&hUART5,(uint8_t *)&g_LocalUart5SendBuf, 5, 1000);		//转发RTK差分数据给GPS模块
					g_Uart5TxTimer = uwTick;					
				}
				else if( g_LocalZCFlag == 3 )
				{					
                    W25qxx_ReadBytes((uint8_t*)&g_tmpBuf , g_LocalZCRec64KPageNo*64*2*512 , 512 );										
					g_LocalZC64kSum = CheckSumNoFan(512, g_tmpBuf, g_LocalZC64kSum );
					for(i=0;i<512;i++)
					{
					 g_FlashtempBuff[i] = g_tmpBuf[i] & 0x00ff;
					}
					
					Tool485_TX_EN();
					g_Uart5DirFlag = 2;	
					delay_us(2000);
					HAL_UART_Transmit(&hUART5,(uint8_t *)&g_FlashtempBuff, 512, 1000);		//转发RTK差分数据给GPS模块
					g_Uart5TxTimer = uwTick;							
					g_LocalZCStartFlashPage++;
				}			
			}
			g_ToolBaowenFlag = 0;
		}
		else if(g_ShouldJumpToStartFlag == 2)
		{
			g_ShouldJumpToStartFlag = 0;
			SoftReset();
		}			
		/* 发送 数据转储 */
		if(( g_ToolCommandFlag == 2 )&&( g_LocalZCFlag == 3))
		{
			W25qxx_ReadBytes((uint8_t*)g_tmpBuf , (g_LocalZCRec64KPageNo*64*2 + g_LocalZCStartFlashPage)*512 , 512 );													
			g_LocalZC64kSum = CheckSumNoFan(512,g_tmpBuf, g_LocalZC64kSum );						
			
			for(j=0;j<512;j++)
			{
				g_FlashtempBuff[j] = g_tmpBuf[j] & 0x00ff;
			}		
			if( g_LocalZCStartFlashPage == 127)
			{
				g_FlashtempBuff[512] = g_LocalZC64kSum;	
				Tool485_TX_EN();
				g_Uart5DirFlag = 2;	
				delay_us(2000);
				HAL_UART_Transmit(&hUART5,(uint8_t *)&g_FlashtempBuff, 513, 1000);		//转发RTK差分数据给GPS模块
				g_Uart5TxTimer = uwTick;					
				g_LocalZCFlag =2;
			}
			else
			{
				Tool485_TX_EN();
				g_Uart5DirFlag = 2;	
				delay_us(2000);
				HAL_UART_Transmit(&hUART5,(uint8_t *)&g_FlashtempBuff, 512, 1000);		//转发RTK差分数据给GPS模块
				g_Uart5TxTimer = uwTick;	
			}
			g_LocalZCStartFlashPage++;											
		}						
		//未收到升级包重新请求
		if(g_ToolCommandFlag == 1 && g_ToolResendFlag == 1 && g_ToolResendTimers!=0
			 &&g_LocalUpdateFlag != 0 && JudgeMilSecondDelayInPro(g_ToolRecTimer,1500) == true)
		{
			g_LocalUpdateFlag = 0;
			delay_us(120);         //Delay_10us(12); 
			Local_Update_Quest();
			g_ToolResendTimers--;
			g_ToolRecTimer = uwTick;		
		}
	}
	
}

