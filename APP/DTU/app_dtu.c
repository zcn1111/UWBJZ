#include "app_dtu.h"
#include "DTU_BulkData.h"
#include "RYDW_Init.h"
#include "main.h"
#include "Update.h"
#include "g_GlobalValDef.h"
#include "Drv_W25Qxx_QSPI.h"
#include "RYDW_Locate.h"
#include "ssd1306.h"
#include "iap.h"
#include "APP_STORE.h"
#include "RYDW_StationCard.h"
#include "RTK_Comm.h"
#include "RYDW_Postion.h"
#include "ssd1306_tests.h"

const char* UserAgent = "NTRIP M307Client/1.0";         // 鉴权密码
const char* host = "rtk.ntrip.qxwz.com";                    // 服务器地址rtk.ntrip.qxwz.com
const char* port = "8002";                              // 服务端口
const char* mountpoint = "AUTO";                        // NTRIP挂载点


uint8_t		g_CtrlMsgFlag =0;
uint8_t 	sendsocketid;
int 		g_SocketId;             //连接ID
uint8_t 	CelluarStep;            //流程步骤
uint8_t 	g_ATWaitFlag;           //AT命令等待标记，=0未发送，=1发送数据等待应答
uint32_t 	g_ATWaitDelay;          //AT命令等待时间
uint8_t     g_CelluarPowerOn;       //模块上电标记
uint32_t    g_CelluarPowerOnDelay;  //上电延时
uint8_t 	g_ATSendCnt;            //AT命令发送次数
uint8_t 	g_ChangeMobileNumStep;  //修改本机号标记
uint8_t 	g_OperatorType;			//1-移动 2-联通 3-电信
uint8_t 	g_ActType;				//1-2G 2-3G 3-4G
uint8_t		g_ReconnetcFlag;        //重连标记
char		g_ReturnATCmd[140];
uint8_t		g_ReturnATCmdLen;
uint8_t		g_AskATCmdFlag;         //发送AT命令标记
uint8_t		g_QisendStep;           //发送数据模块
uint8_t		g_CorsNtripStep;        //连接NTRIP步骤
uint8_t 	g_SendToSocketFailCount;//发送失败次数
uint8_t 	g_SendToDataNetFailCount;
uint8_t     g_SendToCORSNetFailCount;
//uint32_t    g_PowerOffML307CCmdFlag;//断电标记
//uint32_t    g_PowerOffML307CCmdDelay;
uint32_t    g_CelluarPowerOffDelay;
uint8_t 	g_CmdBuf[3072];
bool 		g_Send5ASuc;
bool 		g_SendD6Suc;
uint32_t 	g_SendD6BagNo;	//发送D6数据包序号
uint32_t 	g_RecD6BagNo;	//接收D6数据包序号
uint32_t 	g_SendD6Delay;
uint8_t 	g_UpdateCodeTypeErrorFlag;		//程序程序类型错误标记
uint8_t 	g_NetAskTime;
uint8_t 	g_NetAskTimeCnt;
uint8_t 	g_D6Phone[6];
uint8_t     g_AskAuthorityFlag;       /* 收到返回控制权限的数据包 */
uint8_t     g_ChangeIPFlag;                   //修改IP的标志
uint8_t 	g_NetAskIPAndPortFlag;
uint8_t		g_AskIDFlag;
uint32_t 	g_Send5ABagNo;	//发送正常数据包序号
uint8_t     g_ClearFlashFlag;			//清空FLASH  1-整片擦除，2-指定地址删除 
uint32_t    g_ClearFlashStartAddr;	    //清空FLASH起始位置
uint32_t    g_ClearFlashLen;			//清空FLASH长度
uint8_t     g_AskParFlag = 0;
bool 		g_UpdateSuc;
char		g_AskATCmd[80];
char        auth[128] = {0};
char        authBase64[200] = {0};
char        request[512] = {0};
uint8_t 	tmpsendbuf[3072];
uint16_t 	tmpsendDataNum;
uint8_t     g_RturnFaultMessageFlag;  /* 返回故障信息*/
uint8_t     g_ErrCode;
uint8_t     g_InitAskParFlag = 0;
uint8_t     g_NeedSendGGAFlag;
uint8_t     g_PunCoorSendFlag=0;          //打点数据发送标记
uint8_t     g_DtuSendOk=0;                //DTU发送成功
uint32_t    g_DTUSendDataDelay;
uint8_t     g_ChangeCORSACCFlag;
uint32_t    g_SendCorsDataDelay;

uint8_t     g_CorsNetErr;
uint32_t    g_CorsNetDelay;
char 		iccidbuff[32]={0};
/****************************************************************************
函数名称: void InitNet(void)
功能描述: 
参数输入: 
参数输出: 
返  回  值: 无
其      他: 无
编写者：
*****************************************************************************/
void InitNet(void)
{	
	g_CorsNetErr = 0;
	g_DataNetOk = 0;
	g_CorsNetOk = 0;	
	g_NetAskTime = 1;
	g_NetAskTimeCnt = 6;
	g_Signal = 0;
	CelluarStep = STA_PWR_ON;
	g_Send5ASuc = true;
	g_SendD6Suc = true;
	g_SendToDataNetFailCount = 0;
	g_SendToCORSNetFailCount = 0;
	g_ATSendCnt = 0;
	g_CorsNtripStep = 0;
	g_DTUSendDataDelay=uwTick;
	g_SendCorsDataDelay = uwTick;
	g_CorsNetDelay = uwTick;
}
/****************************************************************************
函数名称: void SendAT(char* s)
功能描述: 
参数输入: 
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
*****************************************************************************/
void SendAT(char* s)
{
	uint16_t len;
	
	memset(g_Uart4TxBuf,0,sizeof(g_Uart4TxBuf));
	len = strlen(s);
	memcpy(g_Uart4TxBuf,s,len);
	UartSend(&hUART4,g_Uart4TxBuf,len);		
}

// Base64编码函数
void base64_encode(const char *input, char *output, int size) 
{
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
	int i = 0;
    unsigned char char_array_3[3], char_array_4[4];
    
    while (size--) {
        char_array_3[i++] = *(input++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i <4 ; i++) 
                *output++ = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(int k = i; k < 3; k++) 
            char_array_3[k] = '\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (int k = 0; k < i + 1; k++)
            *output++ = base64_chars[char_array_4[k]];
        
        while(i++ < 3) 
            *output++ = '=';
    }
    *output = '\0';
}


void combineStrings(char* output, const char* str1, const char* str2,uint8_t flag) 
{
 
    if(!output || !str1 || !str2) return;  // 防止NULL指针
	int index = 0;
    // 复制第一个字符串
    while (*str1) {
        output[index++] = *str1++;
    }
    if(flag==1)
	{
		// 添加冒号
		output[index++] = ':';
	}
    // 复制第二个字符串
    while (*str2) {
        output[index++] = *str2++;
    }
    
    // 添加字符串结束符
    output[index] = '\0';
}

/*GET /添加1 HTTP/1.1\r\nHost: 添加2:添加3\r\nAuthorization: Basic 添加4\r\nUser-Agent: 添加5\r\nConnection: close\r\n\r\n*/
void create_http_request(char *output, size_t out_size,const char *add1, const char *add2, const char *add3, const char *add4, const char *add5) 
{
    const char *parts[] = {
        "GET /",
        add1,      // 挂载点
        " HTTP/1.1\r\nHost: ",
        add2,      // 主机名
        ":",
        add3,      // 端口号
        "\r\nAuthorization: Basic ",
        add4,      // 认证信息
        "\r\nUser-Agent: ",
        add5,      // 用户代理
        "\r\nConnection: close\r\n\r\n"
    };
    
    int part_count = sizeof(parts) / sizeof(parts[0]);
    char *pos = output;
    size_t remain = out_size;
    *pos = '\0';  // 确保以空字符结尾
    
    // 为NULL的字段设为空字符串
    const char *safe_values[5] = {
        add1 ? add1 : "",
        add2 ? add2 : "",
        add3 ? add3 : "",
        add4 ? add4 : "",
        add5 ? add5 : ""
    };
    
    // 合并所有部分
    for(int i = 0, j = 0; i < part_count && remain > 1; i++) {
        const char *current = (i % 2 == 0) ? parts[i] : safe_values[j++];
        if (!current) current = "";
        
        // 逐个字符复制
        while (*current != '\0' && remain > 1) 
		{
            *pos++ = *current++;
            remain--;
        }			
    }
    
    // 确保字符串正确终止
    *pos = '\0';	
}
/******************************************************************
函数名称: void SendRecSuc(void)
功能描述: 向服务器返回接收成功指令
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendRecSuc(void)
{
	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
	tmpsendbuf[0] = 0xDF;
	tmpsendbuf[1] = g_RecD6BagNo&0xFF;
	tmpsendbuf[2] = (g_RecD6BagNo>>8)&0xFF;
	tmpsendbuf[3] = (g_RecD6BagNo>>16)&0xFF;
	tmpsendbuf[4] = (g_RecD6BagNo>>24)&0xFF;
	tmpsendbuf[5] = CheckSum(5,tmpsendbuf);	
	tmpsendDataNum = 0x06;
//	HAL_UART_Transmit(&hUART4, tmpsendbuf, 6, 1000);		//
}
/******************************************************************
函数名称: void NetGetUpdateType(void)
功能描述: 向服务器请求升级类型
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void GetUpdateType(void)
{
	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
	
	tmpsendbuf[0] = 0x5A;
	tmpsendbuf[1] = 0xA5;
	tmpsendbuf[2] = 0x10;
	tmpsendbuf[3] = 0x00;
	tmpsendbuf[4] = 0xEA;	
	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
	memcpy(tmpsendbuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);	
	tmpsendbuf[14] = g_VersionHex[0];
	tmpsendbuf[15] = g_VersionHex[1];	
	tmpsendbuf[19] = CheckSum(15,tmpsendbuf+4);	
	tmpsendbuf[24] = CheckSum(24,tmpsendbuf);	
	tmpsendDataNum = 25;
	sendsocketid =0;
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,25, 1000);		//
}
/******************************************************************
函数名称: void IIIRain_ReturnFaultMessageToGsm(void)
功能描述: III_RAIN 主板给DTU返回故障信息
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：agj
******************************************************************/
void ReturnFaultMessageToGsm(void)
{
	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
	
	tmpsendbuf[0] = 0x5A;
	tmpsendbuf[1] = 0xA5;
	tmpsendbuf[2] = 0x10;
	tmpsendbuf[3] = 0;
	tmpsendbuf[4] = 0xe0;
	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
	memcpy(tmpsendbuf+11,(uint8_t *)&g_BCDSysTime,5);
	tmpsendbuf[16] = g_ErrCode;                      //故障信息编码
	tmpsendbuf[17] = 0;
	tmpsendbuf[18] = 0;
	tmpsendbuf[19] = CheckSum(15,tmpsendbuf+4);
	tmpsendbuf[20] = g_Send5ABagNo&0xFF;
	tmpsendbuf[21] = (g_Send5ABagNo>>8)&0xFF;
	tmpsendbuf[22] = (g_Send5ABagNo>>16)&0xFF;
	tmpsendbuf[23] = (g_Send5ABagNo>>24)&0xFF;
	tmpsendbuf[24] = CheckSum(24,tmpsendbuf);
	tmpsendDataNum = 25;
	sendsocketid =0;
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,25, 1000);		//
}
/******************************************************************
函数名称: void SendFullUpdateBag(void)
功能描述: 向服务器请求升级数据包(全量升级)
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendFullUpdateBag(void)
{
	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
	
	tmpsendbuf[0] = 0x5A;
	tmpsendbuf[1] = 0xA5;
	tmpsendbuf[2] = 0x10;
	tmpsendbuf[3] = 0x00;
	tmpsendbuf[4] = 0xEE;	
	
	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
	memcpy(tmpsendbuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);
	tmpsendbuf[14] = ( g_UpdateCodePageTotal.programpage & 0xff );		
	tmpsendbuf[15] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
	tmpsendbuf[16] = ( g_RemoteUpdatePackNo & 0xff );		
	tmpsendbuf[17] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
	tmpsendbuf[19] = CheckSum(15,tmpsendbuf+4);	
	tmpsendbuf[24] = CheckSum(24,tmpsendbuf);
	tmpsendDataNum = 25;
	sendsocketid = 0;
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,25, 1000);		//
}
/******************************************************************
函数名称: void SendUpdateBag(void)
功能描述: 向服务器请求升级数据包
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendUpdateBag(void)
{
	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
	
	tmpsendbuf[0] = 0x5A;
	tmpsendbuf[1] = 0xA5;
	tmpsendbuf[2] = 0x20;
	tmpsendbuf[3] = 0x00;
	tmpsendbuf[4] = 0xEF;	
	
	memcpy(tmpsendbuf+5,g_MobileInFlash.mobileno[4],6);
	memcpy(tmpsendbuf+11,(uint8_t*)&g_UpdateCodePageTotal,3);
	tmpsendbuf[14] = ( g_UpdateCodePageTotal.programpage & 0xff );		
	tmpsendbuf[15] = (( g_UpdateCodePageTotal.programpage >>8 ) & 0xff );	
	tmpsendbuf[16] = ( g_RemoteUpdatePackNo & 0xff );		
	tmpsendbuf[17] = (( g_RemoteUpdatePackNo >>8 ) & 0xff );	
	if(g_UpdateType == 0xDC)
		tmpsendbuf[18] = 0;	
	else
		tmpsendbuf[18] = g_UpdateType;
	if(g_UpdateType == 1)
	{
		tmpsendbuf[19] = g_VersionHex[0];
		tmpsendbuf[20] = g_VersionHex[1];
	}
	tmpsendbuf[35] = CheckSum(31,tmpsendbuf+4);	
	tmpsendbuf[40] = CheckSum(40,tmpsendbuf);		
	tmpsendDataNum = 41;
	sendsocketid = 0;
//	HAL_UART_Transmit(&hUART4, tmpsendbuf,41, 1000);		//
}
/******************************************************************
函数名称: Bool SendD6Data(void)
功能描述: 发送D6数据
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
void SendD6Data(uint8_t *data, uint8_t type)
{
    uint8_t i,cmpbuf[5],tmpmobile[6],gsm7bit[140],tmptime[6];

	memset(tmpsendbuf,0,sizeof(tmpsendbuf));
    memset(cmpbuf,0,sizeof(cmpbuf));
	
	tmpsendbuf[0] = 0xD6;
	memcpy(tmpsendbuf + 1,g_D6Phone,6);	//接收手机号
	tmpsendbuf[7] = g_SendD6BagNo;						//D6数据包
	if(g_MobileInFlash.Crc16 == CRC16((uint8_t*)&g_MobileInFlash,sizeof(g_MobileInFlash)-2))
    {
        if((CheckNumIsBCD((uint8_t*)g_MobileInFlash.mobileno[4],5) == true)&&
            (memcmp((uint8_t*)g_MobileInFlash.mobileno[4],cmpbuf,5)!=0))
		{
			for(i=0;i<6;i++)
			{
				tmpmobile[i]=CharSwap(g_MobileInFlash.mobileno[4][i]);	//手机号每个字节高低位交换
			}
			for( i = 0 ; i < 6  ; i++ )
			{
				Bcd2Asc(tmpmobile[i],tmpsendbuf+8+2*i);				//转换ASCII码
			}
			if(type == D67bit)
			{
				memset(tmpsendbuf+20,0x30,4);							//协议标识为0，编码方式为00-7bit					
				TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
				tmptime[0] = CharSwap(g_BCDTime.Year);
				tmptime[1] = CharSwap(g_BCDTime.Month);
				tmptime[2] = CharSwap(g_BCDTime.Day);
				tmptime[3] = CharSwap(g_BCDTime.Hour);
				tmptime[4] = CharSwap(g_BCDTime.Minute);
				tmptime[5] = CharSwap(g_BCDTime.Second);
				for( i = 0 ; i < 6  ; i++ )
				{
					Bcd2Asc(tmptime[i],tmpsendbuf+24+2*i);		//年月日时分秒
				}
				i=0;														//时区
				Bcd2Asc(i,tmpsendbuf+36);
				i=160;														//短信长度
				Bcd2Asc(i,tmpsendbuf+38);
				
				for( i = 0 ; i < 80  ; i++ )								//80字节转换ASCII码-160
				{
					Bcd2Asc(*data,g_tmpBuf+2*i);
					data++;
				}
				gsmEncode7bit((char*)g_tmpBuf,gsm7bit,160);					//转换7位编码-140
				for( i = 0 ; i < 140  ; i++ )								//140字节转换ASCII码-280
				{
					Bcd2Asc(gsm7bit[i],tmpsendbuf+40+2*i);
				}
				tmpsendbuf[320] = CheckSum(320,tmpsendbuf);
				tmpsendDataNum = 321; 
				sendsocketid = 0;
//				HAL_UART_Transmit(&hUART4, tmpsendbuf,321, 1000);
			}
			else
			{
				memset(tmpsendbuf+20,0x30,2);							//协议标识为0
				tmpsendbuf[22] = 0x30;									//编码方式为04-8bit
				tmpsendbuf[23] = 0x34;
				TimeHexToBcd(&g_BCDTime,(uint8_t*)&g_SysTime);
				tmptime[0] = CharSwap(g_BCDTime.Year);
				tmptime[1] = CharSwap(g_BCDTime.Month);
				tmptime[2] = CharSwap(g_BCDTime.Day);
				tmptime[3] = CharSwap(g_BCDTime.Hour);
				tmptime[4] = CharSwap(g_BCDTime.Minute);
				tmptime[5] = CharSwap(g_BCDTime.Second);
				for( i = 0 ; i < 6  ; i++ )
				{
					Bcd2Asc(tmptime[i],tmpsendbuf+24+2*i);		//年月日时分秒
				}
				i=0;														//时区
				Bcd2Asc(i,tmpsendbuf+36);
				i=140;														//短信长度
				Bcd2Asc(i,tmpsendbuf+38);
				
				for( i = 0 ; i < 140  ; i++ )								//140字节转换ASCII码-280
				{
					Bcd2Asc(data[i],tmpsendbuf+40+2*i);
				}
				tmpsendbuf[320] = CheckSum(320,tmpsendbuf);
				tmpsendDataNum = 321; 
				sendsocketid = 0;
//				HAL_UART_Transmit(&hUART4, tmpsendbuf,321, 1000);
			}
			g_SendD6Suc = false;
			g_SendD6Delay = uwTick;
		}
	}
}
/******************************************************************
函数名称: void ReturnAskATCmdtoNet(void)
功能描述: 向服务器返回ID版本号
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void ReturnAskATCmdtoNet(void)
{
	uint8_t data[140],cmdlen;

    memset(data,0,140);
    data[0] = 0xCF;
    data[1] = 0x10;
    data[2] = 0x01;
	cmdlen = strlen(g_AskATCmd);
	memcpy(data+3,g_AskATCmd,cmdlen);
	data[3+cmdlen] = 0x0D;
	data[4+cmdlen] = 0x0A;
	if(cmdlen+2+g_ReturnATCmdLen > 136)
		memcpy(data+5+cmdlen,g_ReturnATCmd,136-cmdlen-2);
	else
		memcpy(data+5+cmdlen,g_ReturnATCmd,g_ReturnATCmdLen);
    data[139] = CheckSum(139,data);
	SendD6Data(data,D68bit);
}
/******************************************************************
函数名称: void ReturnMobileNoToNet(void)
功能描述: 向服务器返回控制权限号
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ReturnMobileNoToNet(void)
{
    uint8_t i,tmpbuf[80];

    memset(tmpbuf,0,sizeof(tmpbuf));
	
    memset(tmpbuf,0xA8,6);
    tmpbuf[6] = 0x01;

    for(i=0;i<2;i++)
    {
        memcpy(tmpbuf+7+6*i,g_MobileInFlash.mobileno[i],6);    
    }
    memset(tmpbuf+19,0xff,12);                        //路局号和工务段号
    memset(tmpbuf+31,0,4);            
    memcpy(tmpbuf+35,g_MobileInFlash.mobileno[4],6);  //本机号
    memcpy(tmpbuf+41,g_MobileInFlash.mobileno[3],6);  //短信接收号
    memset(tmpbuf+47,0,33);
    SendD6Data(tmpbuf,D67bit);
}
/******************************************************************
函数名称: void ReturnIDtoNet(void)
功能描述: 向服务器返回ID版本号
参数输入: 无
参数输出: 
返  回  值: 无
其      他: 无
******************************************************************/
void ReturnIDtoNet(void)
{
	uint8_t tmpbuf[80];
	
    memset(tmpbuf,0,sizeof(tmpbuf));

    tmpbuf[0] = 0xCF;
	tmpbuf[1] = 0x80;	
	tmpbuf[2] = 0x0B;
	tmpbuf[3] = 0x01;
	memcpy(tmpbuf+36,g_VersionAscii,4);
	memcpy(tmpbuf+40,g_VersioninFlash.bootversion, 4); 
    memset(tmpbuf + 44,0,35);	
	tmpbuf[79] = CheckSum(79,tmpbuf);	
	SendD6Data(tmpbuf,D67bit);
}
void ReturnClearFlashToGsm(void)
{
    uint8_t data[80];
    
    memset(data,0,80);
    data[0] = 0xCF;
    data[1] = 0x80;
    data[2] = 0x12;
	data[3] = g_ClearFlashFlag;
	if(g_ClearFlashFlag == 2)
	{		
		data[4] = g_ClearFlashStartAddr&0xFF;
		data[5] = (g_ClearFlashStartAddr>>8)&0xFF;
		data[6] = (g_ClearFlashStartAddr>>16)&0xFF;
		data[7] = (g_ClearFlashStartAddr>>24)&0xFF;
		data[8] = g_ClearFlashLen&0xFF;
		data[9] = (g_ClearFlashLen>>8)&0xFF;
		data[10] = (g_ClearFlashLen>>16)&0xFF;
		data[11] = (g_ClearFlashLen>>24)&0xFF;
	}
    data[79] = CheckSum(79,data);
    SendD6Data(data,D67bit);
}
void ReturnPartoNet(void)
{
	uint8_t tmpbuf[140];
	
    memset(tmpbuf,0,sizeof(tmpbuf));

    tmpbuf[0] = 0xCF;
	tmpbuf[1] = 0x91;	
	tmpbuf[2] = 0x01;
	tmpbuf[3] = g_ModeInFlash.type;
	tmpbuf[4] = g_SendDiffMark;
	memcpy(tmpbuf+5,g_CorsAccFlash.user,32);
	memcpy(tmpbuf+37,g_CorsAccFlash.pass,16);	
	memcpy(tmpbuf+53,g_IDInFlash.IDnum,7);	
	tmpbuf[139] = CheckSum(139,tmpbuf);	
	SendD6Data(tmpbuf,D68bit);
}

void SendGGAFunc(void)
{
	if(g_GpsGGABufLen>0)
	{
		memset(tmpsendbuf,0,sizeof(tmpsendbuf));	
		memcpy(tmpsendbuf,g_GpsGGABuf,g_GpsGGABufLen);		
		tmpsendDataNum = g_GpsGGABufLen;
		sendsocketid =1;
	}
	g_GpsGGABufLen = 0;	
}
/******************************************************************
函数名称: void ControlMsg(void)
功能描述: 控制信息
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ControlMsg(void)
{
	if(g_UpdateCodeTypeErrorFlag == 1)
	{
		if(g_UpdateVoiceTypeFlag == 1)
		{					
			SendFullUpdateBag();
		}
		else
		{
			SendUpdateBag();
		}			
		g_UpdateCodeTypeErrorFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else if(g_AskAuthorityFlag == 1)
	{
		ReturnMobileNoToNet();
		g_AskAuthorityFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else if(g_AskIDFlag == 1)
	{
		ReturnIDtoNet();
		g_AskIDFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else if(g_ClearFlashFlag == 1 || g_ClearFlashFlag == 2)
	{
		ReturnClearFlashToGsm();
		g_ClearFlashFlag = 0;
		g_CtrlMsgFlag = 0;
		SoftReset();
	}		
    else if(g_RturnFaultMessageFlag == 1)   //返回故障信息
    {
        if(g_GetMyMobileOKFlag == true)
        {
            ReturnFaultMessageToGsm();
            g_RturnFaultMessageFlag = 0;
            g_CtrlMsgFlag = 0;
        }
    }
	else if(g_AskParFlag == 1)   //返回基站基础信息
    {
		ReturnPartoNet();
		g_AskParFlag = 0;
		g_CtrlMsgFlag = 0;
	}
	else if(g_AskATCmdFlag == 3)
	{
		ReturnAskATCmdtoNet();
		g_AskATCmdFlag = 0;
		g_CtrlMsgFlag = 0;
		memset(g_AskATCmd,0,sizeof(g_AskATCmd));
		memset(g_ReturnATCmd,0,sizeof(g_ReturnATCmd));
		g_ReturnATCmdLen = 0;
	}	
	else
		g_CtrlMsgFlag = 0;
	g_ControlMsgDelay = uwTick;
	if(g_InitAskParFlag == 1)
	{
		g_InitAskParFlag = 2;
		g_AskIDFlag = 1;
		g_CtrlMsgFlag = 1;	
	}
}

/******************************************************************
函数名称: void SocketRecDataAnalysis(uint8_t *data,int len)
功能描述: 服务器接收数据解析
参数输入: *data-数据 len-数据长度
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void SocketRecCORSAnalysis(uint8_t *data,int len)
{
	uint32_t	rtk_interval;
	
	memset(g_RtkDiffBuf, 0, RTK_DIFF_BUFSIZE);
	memcpy(g_RtkDiffBuf, data, len);
	g_RtkDiffLen = len;
	g_recvdiff = 0;
	g_RTKRecvIntervalTimer = uwTick;	
		
	if (g_LastIntervalTimer <= g_RTKRecvIntervalTimer)
	{
		rtk_interval = g_RTKRecvIntervalTimer - g_LastIntervalTimer; 
	}
	else
	{
		rtk_interval = 0xFFFFFFFF - g_LastIntervalTimer + g_RTKRecvIntervalTimer;
	}
	if (rtk_interval > 700)
	{
		/* 计算两次接收差分数据间隔超过700ms 认为是分开的两次 */
		g_RecvRTKCnt++;
		g_DispRtkCnt++;
		g_recvdiff = 1;
		g_LastIntervalTimer = g_RTKRecvIntervalTimer;
	}
	if (g_RecvRTKCnt >= RYDW_BOARDCAST_INTERVAL) g_RecvRTKCnt = RYDW_BOARDCAST_INTERVAL;
	g_StationStatus.CorsOK = 1;
	g_StationStatus.RecvDiffOK = 1;	
}
/******************************************************************
函数名称: void SocketRecDataAnalysis(uint8_t *data,int len)
功能描述: 服务器接收数据解析
参数输入: *data-数据 len-数据长度
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void SocketRecDataAnalysis(uint8_t *data,int len)
{
	uint32_t recd6bagno,rec5abagno;
	switch(g_CmdBuf[0])
	{
		case 0x5A:
			{
				uint8_t tmpmobile[6],typebuf[32],cmpbuf[8],tmp,type,equalflag;
				uint16_t i=0,j=0,k=0;
				uint8_t recbuf[300];
				uint8_t gsmcode7[140];
				uint8_t cmdbuf[140];
				uint8_t mobile_rxflag = 0;
				uint8_t tempmobile[5][6]; 
				
				if(g_CmdBuf[1] == 0xA5 && g_CmdBuf[2] == 0x5A && g_CmdBuf[3] == 0xA5 && g_QisendStep == 0)
				{
					if(g_CmdBuf[4] == 0xEE)
					{
						memset(cmpbuf,0xEE,8);
						if(memcmp(g_CmdBuf+4, cmpbuf, 8) == 0)
						{
							g_RecD6BagNo = g_CmdBuf[20]+(g_CmdBuf[21]<<8)+(g_CmdBuf[22]<<16)+(g_CmdBuf[23]<<24);
							SendRecSuc();
							memcpy((uint8_t*)&g_UpdateCodePageTotal,g_CmdBuf+12,3);
							g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(g_CmdBuf,15);
							if(g_UpdateCodePageTotal.programtype == CODETYPE)
							{
								g_UpdateCodeTypeErrorFlag = 0;
								g_RemoteUpdateGCFlag = 2;								
								g_RemoteUpdatePackNo = 0;			//重置升级包序号
								g_UpdateType = 0;					//重置升级类型（0-全量升级 1-增量升级），后续由AnalysisRemoteUpgradeTypeData设置
								g_ResendFlag = 0;
								g_ResendTimes = 5;								
								MajorVersion = g_CmdBuf[13];                        		//主版本号
								MinorVersion = g_CmdBuf[14];                        		//次版本号
								LastMajorVersion = MajorVersion;                        	//备份主版本号
								LastMinorVersion = MinorVersion;                        	//备份次版本号
								LastTotalBagNum = Get16ValueFrom8Buf(g_CmdBuf,15);  		//包总数
								NormalUpdateFileSize.FileSize = 0;
								WriteNormalFileSizeToFlash();
								DiffUpdateFileSize.FileSize = 0;
								WriteDiffFileSizeToFlash();
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
								MajorVersion = g_CmdBuf[13];                        		//主版本号
								MinorVersion = g_CmdBuf[14];                        		//次版本号
								LastMajorVersion = MajorVersion;                        	//备份主版本号
								LastMinorVersion = MinorVersion;                        	//备份次版本号
								LastTotalBagNum = Get16ValueFrom8Buf(g_CmdBuf,15);  		//包总数
								NormalUpdateFileSize.FileSize = 0;
								WriteNormalFileSizeToFlash();
								DiffUpdateFileSize.FileSize = 0;
								WriteDiffFileSizeToFlash();
								g_UpFlag =1;
								g_UpdateVoiceTypeFlag = 1;									
							}							
						}
					}
					else if(g_CmdBuf[4] == 0xDF)
					{
						memset(cmpbuf,0xDF,8);
						if(memcmp(g_CmdBuf+4, cmpbuf, 8) == 0)
						{
							g_NetAskTime = 1;
							g_NetAskTimeCnt = 6;
							g_DataNetOk = 1;							
						}
						CelluarStep = STA_STATE_IDLE;
					}
					else
					{
						j=0;
						for(i=5;i<37;i+=2)
						{
							tmp = Asc2Hex(g_CmdBuf[i]);
							tmp = (tmp<<4);
							tmp += Asc2Hex(g_CmdBuf[i+1]);									
							typebuf[j++] = tmp;
						}
						
						type = typebuf[7];
						if(g_CmdBuf[4] == 0xD6 && g_CmdBuf[317] == CheckSum(313,g_CmdBuf+4) && g_SendD6Suc)
						{
							g_RecD6BagNo = g_CmdBuf[318]+(g_CmdBuf[319]<<8)+(g_CmdBuf[320]<<16)+(g_CmdBuf[321]<<24);
							SendRecSuc();
//							g_LedGToggleFlag = 1;
//							g_LedGToggleDelay = TIM2->CNT;
							memset(cmpbuf,0,6);
							for(i=0;i<6;i++)
							{
								tmpmobile[i]=CharSwap(typebuf[i]);
							}
							if(type == 0x00)
							{
								gsmDecode7bit(gsmcode7,(char*)recbuf,140);		
							}							
							memcpy(g_D6Phone,tmpmobile,6);
							if(type == 0x00)//7bit
							{
								g_RecD6BagNo = g_CmdBuf[318]+(g_CmdBuf[319]<<8)+(g_CmdBuf[320]<<16)+(g_CmdBuf[321]<<24);
								j=0;
								for(i=37;i<317;i+=2)
								{
									tmp = Asc2Hex(g_CmdBuf[i]);
									tmp = (tmp<<4);
									tmp += Asc2Hex(g_CmdBuf[i+1]);									
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
											g_ChangeIPFlag = 1;
											CelluarStep = STA_CLOSE_TCP;	//重连服务器										
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
										g_SendD6Delay = uwTick;;
									}
								}
								if(cmdbuf[0] == 0xCF && cmdbuf[1] == 0x0F)		
								{//重启命令
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
											if( memcmp(tempmobile[k],cmpipbuf[0],sizeof(tempmobile[k])) == 0 || 
												memcmp(tempmobile[k],cmpipbuf[1],sizeof(tempmobile[k])) == 0 )
											{
											}
											else
											{
												j = 1;
												if( k == 0)                                                 //
												{
													memcpy(g_DataIPBack,g_ServerInfo.g_ServerIPAddr[2],4);  //
													g_DataPortBack = g_ServerInfo.g_ServerPortNo[2];
													memcpy(g_ServerInfo.g_ServerIPAddr[2],tempmobile[k],4);
													g_ServerInfo.g_ServerPortNo[2] = tempmobile[k][4]*100+tempmobile[k][5];
												}
												else if(k == 1 )                                            //
												{
													memcpy(g_ServerInfo.g_ServerIPAddr[0],tempmobile[k],4);
													g_ServerInfo.g_ServerPortNo[0] = tempmobile[k][4]*100+tempmobile[k][5];
												}
												else if( k == 2)                                            //
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
											{
												CmpIPAndPort();                                         //
												if((memcmp(g_DataIPBack,g_ServerInfo.g_ServerIPAddr[2],4) != 0) || (g_DataPortBack != g_ServerInfo.g_ServerPortNo[2]))  //
												{
													g_ChangeIPFlag = 1;
													CelluarStep = STA_CLOSE_TCP;
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
								else if(cmdbuf[0] == 0xCF && cmdbuf[1]== 0x10 && cmdbuf[2] == 0x00)
								{
									if(0== g_CtrlMsgFlag)
									{
										for(i=3;i<79;i++) 
										{
											if(cmdbuf[i] == 0x0D && cmdbuf[i+1] == 0x0A)
											{
												memcpy(g_AskATCmd,cmdbuf+3,i+1);
												g_AskATCmdFlag = 1;	
												g_CtrlMsgFlag = 1;														
												CelluarStep = STA_SENDEX_TO_SOCKET;
												g_SendD6Suc = false;
												g_SendD6Delay = uwTick;
												break;
											}
										}
										
									}												
								}								
							}
							else if(type == 0x08)
							{
								g_RecD6BagNo = g_CmdBuf[318]+(g_CmdBuf[319]<<8)+(g_CmdBuf[320]<<16)+(g_CmdBuf[321]<<24);
								j=0;									
								for(i=0;i<140;i++)
								{
									cmdbuf[i] = (Asc2Hex(g_CmdBuf[37+i*2])<<4) + Asc2Hex(g_CmdBuf[i*2+38]);
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
											{//cors模式																												
												g_MarkInfo.latitude = 0xFFFFFFFF;
												g_MarkInfo.longitude = 0xFFFFFFFF;
												g_MarkInfo.fAltitude = 0xFFFFFFFF;
												memset(g_MarkInfo.res,0x00,8);														
												WriteMarkToFlash();
												g_NeedInitRTK = 1;
												CelluarStep = STA_PWR_OFF;
											}
											else if(g_ModeInFlash.type == 0)
											{//自收敛模式
												memset((uint8_t*)&g_CorsAccFlash, 0xFF, sizeof(g_CorsAccFlash));
												WriteCorsAccToFlash();
												g_MarkWarnFlag = 2;
												CelluarStep = STA_PWR_OFF;									
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
												memcpy(g_Cors.user,cmdbuf+5,32);
												memcpy(g_Cors.pass,cmdbuf+37,16);															
												g_SelfContCORS = 0x01;	
												CelluarStep = STA_PWR_OFF;
												g_LocationProg	= 0;												
											}
											else
											{//CORS
												if(memcmp(cmdbuf+5,g_CorsAccFlash.user,32) == 0 && memcmp(cmdbuf+37,g_CorsAccFlash.pass,16) == 0 )
													;
												else
												{
													memcpy(g_CorsAccFlash.user,cmdbuf+5,32);
													memcpy(g_CorsAccFlash.pass,cmdbuf+37,16);								
													WriteCorsAccToFlash();													
													CelluarStep = STA_PWR_OFF;		//需重新连接			
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
											g_ChangeIPFlag = 1;
											CelluarStep = STA_CLOSE_TCP;
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
				else if(g_CmdBuf[1] == 0xA5 && g_CmdBuf[2] == 0x06 && g_CmdBuf[3] == 0x00)
				{
					if(g_CmdBuf[4] == 0xD8 && g_CmdBuf[10] == CheckSum(10,g_CmdBuf))
					{
						rec5abagno = g_CmdBuf[5]+(g_CmdBuf[6]<<8)+(g_CmdBuf[7]<<16)+(g_CmdBuf[8]<<24);
						if(rec5abagno == g_Send5ABagNo)//判断包序号、是否发送成功
						{
							g_Send5ASuc = true;
							g_Send5ABagNo++;
							g_SendToDataNetFailCount = 0;
						}
					}
					if(g_QisendStep == 0)
						CelluarStep = STA_STATE_IDLE;
				}
				else if(g_CmdBuf[1] == 0xA5 && g_CmdBuf[2] == 0x10 && g_CmdBuf[3] == 0x00 && g_QisendStep == 0)
				{
					if(g_CmdBuf[4] == 0xEB && g_CmdBuf[24] == CheckSum(24,g_CmdBuf))
					{
						memcpy((uint8_t*)&g_UpdateCodePageTotal,g_CmdBuf+11,3);							
						if(g_CmdBuf[14] == g_VersionHex[0] && g_CmdBuf[15] == g_VersionHex[1])
						{
							g_UpdateType = g_CmdBuf[16];
							g_UpdateCodePageTotal.programpage = Get16ValueFrom8Buf(g_CmdBuf,17);
							g_RemoteUpdateGCFlag = 4;
							g_ResendFlag = 0;
						}
					}
					CelluarStep = STA_STATE_IDLE;
				}
				else if(g_CmdBuf[1] == 0xA5 && g_CmdBuf[4] == 0x00)
				{//服务器应答FA00
					if((g_CmdBuf[5] == 0x02 || g_CmdBuf[5] == 0x05) && g_CmdBuf[16] == CheckSum(16,g_CmdBuf))
					{//应答基站定位数据
						g_DtuSendOk = 1;
					}
					else if(g_CmdBuf[5] == 0x03 && g_CmdBuf[16] == CheckSum(16,g_CmdBuf))
					{//应答基站差分数据						
					}
					g_FileSendDataToDtu = 0;
				}
			}
			break;
		case 0xD7:
			if(g_CmdBuf[8] == CheckSum(8,g_CmdBuf))
			{
				recd6bagno = g_CmdBuf[7];
				if(recd6bagno == g_SendD6BagNo)//判断包序号、是否发送成功
				{
					g_SendD6Suc = true;
					g_SendD6BagNo++;				
				}
			}
			CelluarStep = STA_STATE_IDLE;
			break;
		case 0xDC:
			{
				uint8_t tmpbuf1[3];
				
				memset(tmpbuf1,0xDC,3);

				if(memcmp(g_CmdBuf+1,tmpbuf1,3)==0)
				{
					if(g_CmdBuf[511] == CheckSum(511,g_CmdBuf))
					{
						if((g_UpdateCodePageTotal.programpage == Get16ValueFrom8Buf(g_CmdBuf,10))&&(g_RemoteUpdatePackNo ==Get16ValueFrom8Buf(g_CmdBuf,12)))
						{
							memcpy(g_UartUPBuf,g_CmdBuf+14,496);
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
								g_UpdateType = g_CmdBuf[4];
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
									g_RemoteUpdateGCFlag = 6;               
									g_UpdateSuc = true;

								}
								else
								{
									g_RemoteUpdateGCFlag = 4;	// 3													
								}								
							}
						}
					}
				}				
			}
			if(g_QisendStep == 0)
				CelluarStep = STA_STATE_IDLE;
			break;
		default:
			break;
	}
}
/******************************************************************
函数名称: void ProcessDtuNoAnswerFunc(void)
功能描述: 处理DTU无应答
参数输入: 无
参数输出: 
返  回  值 : 无
其       他: 
******************************************************************/
void ProcessDtuNoAnswerFunc(void)
{
    if(g_FileSendDataToDtu >= 180)
    {
		g_FileSendDataToDtu = 0;		
    }
}
/******************************************************************
函数名称: void SendC3Func(void)
功能描述: 发送打点数据的函数
参数输入: 无
参数输出: 
返  回  值 : 无
其       他: 无
******************************************************************/
bool SendPunCoorFunc(void)
{
    uint8_t tempdata;
    uint8_t cmpbuf[5];
	uint16_t i;
	uint32_t tmpoffset;
	
	memset(cmpbuf,0,sizeof(cmpbuf));
    
    if(g_PunCoorSendFlag == 0 || (g_PunCoorSendFlag == 1 && g_DtuSendOk == 1)) //数据未发送，或者有数据发送且已发送成功    
    {
        if(g_PunCoorSendFlag == 1 && g_DtuSendOk == 1)
        {
            for(i=0;i<g_PUNCOORAddnumb;i++)
			{
				tempdata = INDEX_SEND_SUC;            
				W25qxx_WriteByte(tempdata,g_ReadAddrPar[i].Readaddr);
				g_PUNCOORDatainFlash.needsendcnt--;								
			}
			g_PUNCOORAddnumb = 0;
			memset(g_ReadAddrPar,0xFF,sizeof(g_ReadAddrPar));
			g_PunCoorSendFlag = 0;			
            return true;        //20150120
        }
        else if(g_PunCoorSendFlag == 0 && g_PUNCOORDatainFlash.needsendcnt > 0)
        {
			if(RYDW_GeneratePostionRecord() == true)    //发送 c3
			{
				g_PunCoorSendFlag = 1;
				g_DtuSendOk = 2;
				g_SendPunCoorDelay = uwTick;
			}
			else
				return true;				
        }
        else if(g_PUNCOORDatainFlash.needsendcnt == 0)
            return true;				
    }
    else if(g_PunCoorSendFlag == 1 && (g_DtuSendOk == 0 || g_DtuSendOk == 2))
    {/*重发*/
        if(JudgeMilSecondDelayInPro(g_SendPunCoorDelay , 30*1000) == true)
        {
		
			if(RYDW_GeneratePostionRecord() == true)    //重新发送D5
			{
				g_DtuSendOk = 2;
				g_SendPunCoorDelay = uwTick;
				g_FileSendDataToDtu++;
				ProcessDtuNoAnswerFunc();					
			}
        }
    }
    return false;
}

/******************************************************************
函数名称: Bool IdleStatePro(void)
功能描述: 
参数输入: 无
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void IdleStatePro(void)
{
	tmpsendDataNum = 0;
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
				SoftReset();
			}							
		}
	}
	else if(g_DataNetOk == 1 || g_CorsNetOk == 1)
	{
		if(g_CtrlMsgFlag > 0 && g_DataNetOk == 1 && g_InitAskParFlag > 0)
		{			
			ControlMsg();
		}
		else if (g_NeedSendGGAFlag == 1 && g_CorsNetOk == 1)
		{//发送定位数据给千寻服务
			SendGGAFunc();
			g_NeedSendGGAFlag = 0;
		}		
		else if(g_PUNCOORDatainFlash.needsendcnt>0 && g_DataNetOk == 1)//判断是否发送定位数据
		{/*发送打点数据*/
			SendPunCoorFunc();
			if(g_InitAskParFlag == 0)
			{
				g_InitAskParFlag = 1;
				g_AskParFlag = 1;
				g_CtrlMsgFlag = 1;
			}
		}		
		else if((g_RecvRTKCnt >= RYDW_BOARDCAST_INTERVAL) && WorkMode == 0 && g_RtkDiffLen > 0 && g_recvdiff == 1 && g_DataNetOk == 1)
		{//差分数据|| JudgeMilSecondDelayInPro(g_SendCorsDataDelay, 30000) == true
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
				g_SendCorsDataDelay = uwTick;				
			}
			g_RecvRTKCnt =0;
		}			
		else if (((JudgeMilSecondDelayInPro(g_tmrUploadBasePosDelay, 30000) == true) || (g_BasePosUploaded == 1)) && g_DataNetOk == 1)
		{//发送定位数据
			g_BasePosUploaded = 0;
			g_tmrUploadBasePosDelay = uwTick;
			memset(tmpsendbuf, 0x00, sizeof(tmpsendbuf));
			RYDW_GenerateBaseStationPosRecord(tmpsendbuf, &g_UploadRecordLen,1);		//生成基站定位数据
			if(g_InitAskParFlag == 0)
			{
				g_InitAskParFlag = 1;
				g_AskParFlag = 1;
				g_CtrlMsgFlag = 1;
			}				
		}
		ProcessDtuNoAnswerFunc();		
	}
	if(!g_SendD6Suc && JudgeMilSecondDelayInPro(g_SendD6Delay,Delay10Sec) == true)
	{
		g_SendD6Suc = true;
		g_CtrlMsgFlag = 0;
	}
	if(tmpsendDataNum > 0)
	{
		CelluarStep = STA_SEND_TO_DATATCP;
		g_QisendStep = 1;			
	}				
}
/******************************************************************
函数名称: PushSocket(uint8_t id)
功能描述: socket缓存发送到服务器
参数输入: id-socket id
参数输出: 
返  回  值: 无
其      他: 无
编写者：hly
******************************************************************/
//void SendDatatoSocket_SEND(void)
//{
//	UartSend(&hUART4,(uint8_t*)&tmpsendbuf,tmpsendDataNum);
//	CelluarStep = STA_SEND_TO_DATATCP;
//	g_ATWaitFlag = 1;
//	g_ATWaitDelay = 2000;
//	g_SocketId = 0;
//	g_SendD6Suc = false;
//	g_SendD6Delay = TIM2->CNT;
//	g_SendToNetFailCount++;
//}

void ML307C_Process(void)
{	
	int datalen,recid;	
	char *p;
	uint16_t size=0,i=0;
	uint32_t recd6bagno,rec5abagno;
	uint16_t increase1,inphth,disincrease;	
	uint8_t cmpipbuf[2][32];												
	memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
	memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));		
	/*解析接收*/
	/* 检查接收数据长度 */
	size = GetRingUsedLen(&g_Uart4RxRing);
	if(size > 0)
	{
		if (JudgeMilSecondDelayInPro(g_Uart4TxTimer,Delay5mSec) == true)
		{
			memset(g_BulkRecvBuf, 0x00, sizeof(g_BulkRecvBuf));
			RingBufPop(&g_Uart4RxRing, g_BulkRecvBuf, size);
			//for循环一遍查找cors数据
			for(i=0;i<size;i++)
			{
				if(mem_mem(g_BulkRecvBuf+i,size-i,mipurc,strlen(mipurc)) != NULL)
				{			
					char recbuf[21]; 
					memcpy(recbuf, g_BulkRecvBuf+i, 20);
					if(mem_mem(recbuf,20,rtcp,strlen(rtcp)) != NULL)
					//if(strstr(recbuf,rtcp) != NULL)
					{
						char *result = strstr((const char*)g_BulkRecvBuf+i,rtcp);
						i += result - (const char*)(g_BulkRecvBuf+i);
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
						memset(g_CmdBuf,0,sizeof(g_CmdBuf));
						p = (char *)g_BulkRecvBuf+i;
						p = strstr(p, rtcp);
						p = strstr(p, ",");	
						recid = atol(p+1);
						p = strstr(p+1, ",");
						datalen = atol(p+1);	
						p = strstr(p+1, ",");	
//						char *data_start; 
//						data_start = p+2;
						memcpy(g_CmdBuf, p+1, datalen);				
						if (recid == 0)
						{
							SocketRecDataAnalysis(g_CmdBuf,datalen);
							g_SendToDataNetFailCount = 0;
						}
						else if (recid == 1)
						{
							SocketRecCORSAnalysis(g_CmdBuf,datalen);
							g_SendToCORSNetFailCount = 0;
						}
						i+=datalen;
					}
					else if(mem_mem(g_BulkRecvBuf+i,size-i,disconn,strlen(disconn)) != NULL)
					{
						char *result = strstr((const char*)g_BulkRecvBuf+i,disconn);
						i += result - (const char*)(g_BulkRecvBuf+i);					
						p = (char *)g_BulkRecvBuf+i;
						p = strstr(p, disconn);
						recid = atol(p+1);
						CelluarStep = STA_CLOSE_TCP;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
						g_SocketId = recid;
						g_ReconnetcFlag = 1;
						if(recid == datasocketid)
						{	
							g_DataNetOk = 0;
							g_CorsNtripStep = 0;
						}
						else if(recid == corssocketid)
							g_CorsNetOk = 0;
						
					}				
				}
			}
			switch(CelluarStep)
			{
				case STA_PWR_ON:                 
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL)
					{
						CelluarStep = STA_CLOSE_ECHO;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;					
				case STA_CLOSE_ECHO:           
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL)
					{
						CelluarStep = STA_CHK_SIM_SWITCH;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;
				case STA_CHK_SIM_SWITCH:      
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL)
					{
						CelluarStep = STA_CHK_DATA_TIMEOUT;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;
				case STA_CHK_DATA_TIMEOUT:      
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL)
					{
						CelluarStep = STA_CHK_CORS_TIMEOUT;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;					
				case STA_CHK_CORS_TIMEOUT:     
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL)
					{
						CelluarStep = STA_CHK_SIM_STA;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;					
				case STA_CHK_SIM_STA:       
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL && mem_mem(g_BulkRecvBuf,size,_cpin,strlen(_cpin)) != NULL && mem_mem(g_BulkRecvBuf,size,_ready,strlen(_ready)) != NULL)
					{
						CelluarStep = STA_CHK_SIM_CGREG;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;								
				case STA_CHK_SIM_CGREG:      //查询注册网络
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL && mem_mem(g_BulkRecvBuf,size,_chkcgreg,strlen(_chkcgreg)) != NULL)
					{
						int state;						
						p = (char*)g_BulkRecvBuf;
						p = strstr(p,","); 						
						state = atoi(p+1);						
						if(state == 1 || state == 5)
						{
							CelluarStep = STA_CHK_ICCID;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
						}
						
					}
					break;
				case STA_CHK_ICCID:   //读取ICCID
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL && mem_mem(g_BulkRecvBuf,size,_iccid,strlen(_iccid)) != NULL)
					{
						p = strstr((char*)g_BulkRecvBuf, _iccid);
						if (p) 
						{
							char id[32];
							memset(iccidbuff, 0, sizeof(iccidbuff));
							sscanf(p + strlen("+MCCID: "), "%s", iccidbuff);				
							CelluarStep = STA_CHK_SIGNAL;
						}
						else
						{
							CelluarStep = STA_CHK_ICCID;
						}						
					}
					break;
				case STA_CHK_SIGNAL:     //查询信号强度
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL && mem_mem(g_BulkRecvBuf,size,_csq,strlen(_csq)) != NULL)
					{
						int signal;
						p = (char*)g_BulkRecvBuf;					
						p = strstr(p,_csq);						
						signal = atoi(p+strlen(_csq));
						g_Signal = signal;
						if(signal != 0 && signal != 99 && signal != 100 && signal != 199 && (g_DataNetOk == 0 || g_ChangeMobileNumStep == 9))
						{
							CelluarStep = STA_CHK_NET_TYPE;
							if(g_ChangeMobileNumStep == 9)
								g_ChangeMobileNumStep = 0;
							g_ATWaitFlag = 0;
						}
						else if(g_DataNetOk == 1)
						{
							CelluarStep = STA_STATE_IDLE;
							g_ATWaitFlag = 0;
						}
						g_ATSendCnt = 0;
					}
					break;
				case STA_CHK_NET_TYPE:  //查询连接网络
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL && mem_mem(g_BulkRecvBuf,size,_cops,strlen(_cops)) != NULL)
					{
						int act;						

						p = (char*)g_BulkRecvBuf;
						p = strstr(p,"\",");
						act = atoi(p+2);
						
						if(act == 0 || act == 1 || act == 3)
							g_ActType = 1;
						else if(act == 2 || act == 4 || act == 5 || act == 6)
							g_ActType = 2;
						else if(act == 7)
							g_ActType = 3;	
						if(g_ModeInFlash.type == 0) 
						{//自收敛
							if(g_SelfContCORS ==0x01 && g_CorsNetErr == 0)
								CelluarStep = STA_CREAT_CORSTCP;
							else
								CelluarStep = STA_CREAT_DATATCP;
						}
						else if(g_ModeInFlash.type == 1)
						{//CORS模式
							if( memcmp(g_CorsAccFlash.user,cmpipbuf[0],32) == 0 || memcmp(g_CorsAccFlash.user,cmpipbuf[1],32) == 0 
								||  memcmp(g_CorsAccFlash.pass,cmpipbuf[0],16) == 0 || memcmp(g_CorsAccFlash.pass,cmpipbuf[1],16) == 0)
							{
								g_MarkWarnFlag = 1;
								CelluarStep = STA_CREAT_DATATCP;
							}
							else
							{
								g_MarkWarnFlag = 0;							
								if(g_CorsNetOk == 0 && g_CorsNetErr == 0)							
									CelluarStep = STA_CREAT_CORSTCP;//STA_CFG_PDP_CORS;
								else if(g_DataNetOk == 0)
									CelluarStep = STA_CREAT_DATATCP;
							}
						}
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;	
				case STA_CHK_CCLK:	   //读模块时间				
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL && mem_mem(g_BulkRecvBuf,size,_chkcclk,strlen(_chkcclk)) != NULL)
					{
						p = (char*)g_BulkRecvBuf;
						p = strstr(p,_chkcclk);
						g_SysTime.Year = StrToTime((uint8_t *)&p);
						g_SysTime.Minute = StrToTime((uint8_t *)&p+3);
						g_SysTime.Day = StrToTime((uint8_t *)&p+6);
						g_SysTime.Hour = StrToTime((uint8_t *)&p+9);
						g_SysTime.Minute = StrToTime((uint8_t *)&p+12);
						g_SysTime.Second = StrToTime((uint8_t *)&p+15);
						convertToUTC8(&g_SysTime);																	
						CelluarStep = STA_STATE_IDLE;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;

					}				
					break;
//				case STA_CFG_PDP:      //设置PDP上下文--数据服务
//					if(strstr((const char*)g_BulkRecvBuf,_ok) != NULL)
//					{
//						CelluarStep = STA_ACTIVE_PDP;
//						g_ATSendCnt = 0;
//						g_ATWaitFlag = 0;
//					}
//					break;
//				case STA_CFG_PDP_CORS:   //设置PDP上下文--cors服务
//					if(strstr((const char*)g_BulkRecvBuf,_ok) != NULL)
//					{
//						CelluarStep = STA_ACTIVE_PDP_CORS;
//						g_ATSendCnt = 0;
//						g_ATWaitFlag = 0;
//					}
//					break;				
//				case STA_ACTIVE_PDP:     //激活PDP上下文--数据服务
//					if(strstr((const char*)g_BulkRecvBuf,_ok) != NULL)
//					{
//						CelluarStep = STA_CREAT_DATATCP;
//						g_ATSendCnt = 0;
//						g_ATWaitFlag = 0;
//					}
//					break;		
//				case STA_ACTIVE_PDP_CORS://激活PDP上下文--cors服务
//					if(strstr((const char*)g_BulkRecvBuf,_ok) != NULL)
//					{
//						CelluarStep = STA_CREAT_CORSTCP;
//						g_ATSendCnt = 0;
//						g_ATWaitFlag = 0;
//					}
//					break;	
				case STA_CREAT_DATATCP:   //连接数据服务
					if(mem_mem(g_BulkRecvBuf,size,_err,strlen(_err)) != NULL )
					{
						CelluarStep = STA_CLOSE_TCP;
						g_ReconnetcFlag = 1;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					else if(mem_mem(g_BulkRecvBuf,size,_mipopen,strlen(_mipopen)) != NULL)
					{
						int id;
						p = (char*)g_BulkRecvBuf;
						p = strstr(p,_mipopen);
						id = atoi(p+strlen(_mipopen));
						if(id == datasocketid)
						{													
							if (g_ModeInFlash.type == 0)
								CelluarStep = STA_STATE_IDLE;
							else if (g_ModeInFlash.type == 1)
							{
								if (g_MarkWarnFlag == 1)
									CelluarStep = STA_STATE_IDLE;
								else if(g_CorsNetOk == 1)
									CelluarStep = STA_STATE_IDLE;
								else if(g_CorsNetErr == 0)
									CelluarStep = STA_CREAT_CORSTCP;//STA_CFG_PDP_CORS;	
							}								
							g_DataNetOk = 1;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
						}
					}
					break;				
				case STA_CREAT_CORSTCP:
					if(mem_mem(g_BulkRecvBuf,size,_err,strlen(_err)))
					{
						CelluarStep = STA_CLOSE_TCP;
						g_ReconnetcFlag = 1;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}					
					else if(mem_mem(g_BulkRecvBuf,size,_mipopen,strlen(_mipopen)) != NULL)
					{
						int id;
						
						p = (char*)g_BulkRecvBuf;
						p = strstr(p,_mipopen);
						id = atoi(p+strlen(_mipopen));
						if(id == corssocketid)
						{						
							CelluarStep = STA_CREAT_CORSNtrip;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
						}
					}				
					break;
				case STA_CREAT_CORSNtrip:
					{
						if(g_CorsNtripStep == 2 && mem_mem(g_BulkRecvBuf,size,_mipsend,strlen(_mipsend)) != NULL)
						{
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
							g_CorsNtripStep	= 3;						
						}
						else if(g_CorsNtripStep == 4 && mem_mem(g_BulkRecvBuf,size,http,strlen(http)) != NULL)
						{
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;							
							g_CorsNtripStep	= 0;
							g_CorsNetOk = 1;
							if(g_DataNetOk == 1)
								CelluarStep = STA_STATE_IDLE;
							else
								CelluarStep = STA_CREAT_DATATCP;
						}
						break;
					}
				case STA_CLOSE_TCP:
					if(mem_mem(g_BulkRecvBuf,size,distcp_ok,strlen(distcp_ok)) != NULL)
					{
						if(g_ChangeIPFlag == 1)
						{
							g_ChangeIPFlag = false;
							CelluarStep = STA_CHK_SIM_CGREG;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
							g_DataNetOk = 0;
						}
						else if(g_ReconnetcFlag == 2)
						{
							CelluarStep = STA_CREAT_DATATCP;
							g_ReconnetcFlag = 0;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
							g_ATWaitDelay = 5000;
						}						
					}
					else if (mem_mem(g_BulkRecvBuf,size,distcp_cors_ok,strlen(distcp_cors_ok)) != NULL)
					{
						if (g_ChangeCORSACCFlag == 1)
						{
							g_ChangeCORSACCFlag = 0;
							if(g_ModeInFlash.type == 1 || g_SelfContCORS == 1)
								CelluarStep = STA_CHK_SIM_CGREG;
							else
								CelluarStep = STA_STATE_IDLE;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;								
						}
						else
						{
							if(g_ModeInFlash.type == 1)
							{	
								if (g_MarkWarnFlag == 1)
									CelluarStep = STA_STATE_IDLE;
								else if (g_CorsNetErr == 0)								
									CelluarStep = STA_CREAT_CORSTCP;//STA_CFG_PDP_CORS;
							}
							else						
								CelluarStep = STA_STATE_IDLE;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
							g_ReconnetcFlag = 0;							
						}								
					}
					else if(mem_mem(g_BulkRecvBuf,size,_err,strlen(_err)) != NULL )
					{
						CelluarStep = STA_CHK_SIM_CGREG;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
						g_ChangeIPFlag = false;
						g_ChangeCORSACCFlag = 0;
						g_ReconnetcFlag = 0;
					}
					break;				
				case STA_SEND_TO_DATATCP:

					if(mem_mem(g_BulkRecvBuf,size,_mipsend,strlen(_mipsend)) != NULL)
					{
						if(g_QisendStep == 2)
						{
							g_QisendStep = 3;
							g_ATWaitFlag = 0;
						}
					}
					else if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL )
					{
						g_QisendStep = 0;
						CelluarStep = STA_STATE_IDLE;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}					
					else if(mem_mem(g_BulkRecvBuf,size,_err,strlen(_err)) != NULL)
					{
						g_QisendStep = 0;
						CelluarStep = STA_STATE_IDLE;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
						if (sendsocketid == datasocketid)
							g_SendToDataNetFailCount++;	
						else if(sendsocketid == corssocketid)
							g_SendToCORSNetFailCount++;						
					}
					break;	
				case STA_SENDEX_TO_SOCKET:
					if(mem_mem(g_BulkRecvBuf,size,_ok,strlen(_ok)) != NULL )
					{
						if(g_AskATCmdFlag == 1)
						{
							g_AskATCmdFlag = 2;
							CelluarStep = STA_NET_CHK_AT;
						}
						else
							CelluarStep = STA_STATE_IDLE;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
						g_SendToSocketFailCount = 0;
						g_SendToDataNetFailCount = 0;
					}
					else if(mem_mem(g_BulkRecvBuf,size,_err,strlen(_err)) != NULL)
					{
						if(g_SendToSocketFailCount++ >= 10)
						{						
							HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);
							g_CelluarPowerOn = 0;
							g_CelluarPowerOffDelay = uwTick;
							CelluarStep = STA_STATE_IDLE;
							g_DataNetOk	= 0;
							g_CorsNetOk = 0;							
						}
						else
							CelluarStep = STA_STATE_IDLE;
						g_ATSendCnt = 0;
						g_ATWaitFlag = 0;
					}
					break;					
				case STA_NET_CHK_AT:
					memset(g_ReturnATCmd,0,sizeof(g_ReturnATCmd));
					if(size > 136)
						g_ReturnATCmdLen = 136;
					else
						g_ReturnATCmdLen = size;
					memcpy(g_ReturnATCmd,g_BulkRecvBuf,g_ReturnATCmdLen);
					if(g_AskATCmdFlag == 2)
						g_AskATCmdFlag = 3;
					CelluarStep = STA_STATE_IDLE;
					g_CtrlMsgFlag = 1;
					g_ATWaitFlag = 0;
					g_ATSendCnt = 0;
					break;				
				default:
					break;
			}
		}
		
	}
	
	if((JudgeMilSecondDelayInPro(g_DTUSendDataDelay,g_ATWaitDelay*Delay1mSec) == true && g_ATWaitFlag == 1) || 
		(JudgeMilSecondDelayInPro(g_DTUSendDataDelay,Delay1mSec) == true && g_ATWaitFlag == 0))
	{
		if(g_CelluarPowerOn == 1 && JudgeMilSecondDelayInPro(g_CelluarPowerOnDelay,Delay3Sec) == true)
		{
			switch(CelluarStep)
			{
				case STA_PWR_ON:
					if(g_ATSendCnt < 20)
					{
						SendAT(at);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;						
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
						g_InitAskParFlag = 0;
					}
					else
					{//异常处理等待150s硬件复位模块
						CelluarStep = STA_PWR_OFF;
					}
					break;
				case STA_CLOSE_ECHO:
					if(g_ATSendCnt < 20)
					{
						SendAT(ate0);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{//异常处理等待150s硬件复位模块
						CelluarStep = STA_PWR_OFF;
					}										
					break;
				case STA_CHK_SIM_SWITCH:					
					if(g_ATSendCnt < 20)
					{					
						SendAT(singlesim_1);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}
					break;
				case STA_CHK_DATA_TIMEOUT:					
					if(g_ATSendCnt < 20)
					{					
						SendAT(sendtimeoutdata);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}
					break;
				case STA_CHK_CORS_TIMEOUT:					
					if(g_ATSendCnt < 20)
					{					
						SendAT(sendtimeoutcors);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}
					break;					
				case STA_CHK_SIM_STA:
					if(g_ATSendCnt < 20)
					{
						SendAT(cpin);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 5000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{//20s内无法识别SIM卡状态重启
						CelluarStep = STA_PWR_OFF;
					}					
					break;				
				case STA_CHK_SIM_CGREG:
					if(g_ATSendCnt < 60)
					{
						SendAT(chkcgreg);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{//60s内无法注册PS跳过
						CelluarStep = STA_PWR_OFF;
					}					
					break;
				case STA_CHK_ICCID:
					if(g_ATSendCnt < 60)
					{					
						SendAT(iccid);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}														
					break;
				case STA_CHK_SIGNAL:
					if(g_ATSendCnt < 20)
					{					
						SendAT(csq);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}										
					break;
				case STA_CHK_NET_TYPE:
					if(g_ATSendCnt < 20)
					{
						SendAT(cops);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 5000;	
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}					
					break;
				case STA_CHK_CCLK:
					if(g_ATSendCnt < 20)
					{					
						SendAT(chkclk);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}					
					break;
//				case STA_CFG_PDP:
//					if(g_ATSendCnt < 20)    					//CelluarStep = STA_CREAT_DATATCP;break;
//					{					
//						SendAT(cfgapn);
//						g_ATWaitFlag = 1;
//						g_ATWaitDelay = 3000;
//						g_ATSendCnt++;
//					}
//					else
//					{
//						CelluarStep = STA_CHK_NET_TYPE;
//					}					
//					break;
//				case STA_CFG_PDP_CORS:
//					SendAT(cfgapn_cors);
//					g_ATWaitFlag = 1;
//					g_ATWaitDelay = 3000;
//					break;				
//				case STA_ACTIVE_PDP:
//					if(g_ATSendCnt < 20)
//					{
//						SendAT(actcgact);
//						g_ATWaitFlag = 1;
//						g_ATWaitDelay = 150000;
//						g_ATSendCnt++;
//					}
//					else
//					{//等待150s复位模块
//						CelluarStep = STA_PWR_OFF;
//					}
//					break;	

//                case STA_ACTIVE_PDP_CORS:
//					if(g_ATSendCnt < 20)
//					{
//						SendAT(actcgact_cors);
//						g_ATWaitFlag = 1;
//						g_ATWaitDelay = 150000;
//						g_ATSendCnt++;
//					}
//					else
//					{//等待150s复位模块
//						CelluarStep = STA_PWR_OFF;
//					}
//					break;						
					
				case STA_CREAT_DATATCP:
					if(g_ATSendCnt < 20)
					{
						char str[40];						
						memset(str,0,sizeof(str));
						sprintf(str,"%s%d.%d.%d.%d\",%d\r",mipopen,
							g_ServerInfo.g_ServerIPAddr[2][0],g_ServerInfo.g_ServerIPAddr[2][1],g_ServerInfo.g_ServerIPAddr[2][2],g_ServerInfo.g_ServerIPAddr[2][3],g_ServerInfo.g_ServerPortNo[2]);
						SendAT(str);
						g_InitAskParFlag = 0;
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 15000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}			
					else
					{
						CelluarStep = STA_PWR_OFF;
					}
					
					break;
				
				case STA_CREAT_CORSTCP:
					if(g_ATSendCnt < 20)
					{
						SendAT(mipopen_cors);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 15000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						CelluarStep = STA_PWR_OFF;
					}					
					break;
				case STA_CREAT_CORSNtrip:  //需考虑重连和一直连不上的情况
					{						
						if(g_ATSendCnt < 20)
						{
							if(g_CorsNtripStep == 0)
							{
								char str[40];
								memset(str,0,sizeof(str));	
								if(g_ModeInFlash.type == 0)
									combineStrings(auth,g_Cors.user,g_Cors.pass,1);
								else
									combineStrings(auth,g_CorsAccFlash.user,g_CorsAccFlash.pass,1);
								base64_encode(auth,authBase64,strlen(auth));
								create_http_request(request,sizeof(request),mountpoint,host,port,authBase64,UserAgent);						
								sprintf(str,"%s%d,%d\r",mipsend,corssocketid,strlen(request));
								SendAT(str);
								g_ATWaitFlag = 1;
								g_ATWaitDelay = 5000;
								g_CorsNtripStep	= 2;
								g_DTUSendDataDelay = uwTick;
							}
							else if(g_CorsNtripStep == 3)
							{							
								UartSend(&hUART4,(uint8_t *)request,strlen(request));
								g_CorsNtripStep	= 4;
								g_ATWaitDelay = 5000;
								g_ATWaitFlag = 1;
								g_DTUSendDataDelay = uwTick;
							}
							else if (g_CorsNtripStep == 4 || g_CorsNtripStep == 2)
								g_CorsNtripStep = 0;
						
						}
						else
						{
							CelluarStep = STA_PWR_OFF;
						}						
					}
					break;				
				case STA_CLOSE_TCP:
					{
						char str[40];
						memset(str,0,sizeof(str));
						
						if(g_ATSendCnt < 20)
						{
							if(g_ChangeIPFlag == 1)
							{
								sprintf(str,"%s%d\r",distcp,datasocketid);
								SendAT(str);
								g_ATWaitFlag = 1;
								g_ATWaitDelay = 3000;
								g_DataNetOk = 0;
								g_DTUSendDataDelay = uwTick;
								g_InitAskParFlag = 0;
							}
							else if (g_ChangeCORSACCFlag == 1)
							{
								sprintf(str,"%s%d\r",distcp,corssocketid);
								SendAT(str);
								g_ATWaitFlag = 1;
								g_ATWaitDelay = 3000;
								g_CorsNetOk = 0;
								g_DTUSendDataDelay = uwTick;
							}
							else if(g_ReconnetcFlag == 1)
							{
								sprintf(str,"%s%d\r",distcp,g_SocketId);
								SendAT(str);
								g_ATWaitFlag = 1;
								g_ATWaitDelay = 3000;
								g_ReconnetcFlag = 2;
								g_DTUSendDataDelay = uwTick;
								if(g_SocketId == 0)
									g_InitAskParFlag = 0;
							}
							else if(g_ReconnetcFlag == 2)
							{
								HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET); //关闭电源
								g_CelluarPowerOn = 0;
								g_CelluarPowerOffDelay = uwTick;
								CelluarStep = STA_STATE_IDLE;
								g_DataNetOk	= 0;
								g_CorsNetOk = 0;
								g_InitAskParFlag = 0;
							}
						}
						else
						{
							CelluarStep = STA_PWR_OFF;
							g_InitAskParFlag = 0;
						}																	
					}					
					break;
				case STA_SEND_TO_DATATCP:
					if((g_SendDiffMark == 2 && g_SendToDataNetFailCount > 6) || (g_SendDiffMark == 1 && g_SendToDataNetFailCount > 30) || g_SendToCORSNetFailCount > 60)
					{
						CelluarStep = STA_PWR_OFF;
					}
					else
					{
						if(g_QisendStep == 1)
						{
							char str[40];
							sprintf(str,"%s%d,%d\r",mipsend,sendsocketid,tmpsendDataNum);
							SendAT(str);
							g_ATWaitFlag = 1;
							g_ATWaitDelay = 2000;
							g_QisendStep = 2;
							g_DTUSendDataDelay = uwTick;
						}
						else if(g_QisendStep == 3)
						{							
							UartSend(&hUART4,(uint8_t*)&tmpsendbuf,tmpsendDataNum);		
							g_QisendStep = 4;
							g_ATWaitFlag = 1;
							g_ATWaitDelay = 2000;
							g_DTUSendDataDelay = uwTick;
						}
						else
						{
							if (sendsocketid == datasocketid)
								g_SendToDataNetFailCount++;	
							else if(sendsocketid == corssocketid)
								g_SendToCORSNetFailCount++;							
							g_QisendStep = 0;
							CelluarStep = STA_STATE_IDLE;
							if(!g_SendD6Suc)
								g_SendD6Suc = true;
						}
					}
					break;
				case STA_SENDEX_TO_SOCKET:
					if((g_SendDiffMark == 2 && g_SendToDataNetFailCount > 4) || (g_SendDiffMark == 1 && g_SendToDataNetFailCount > 30) || g_SendToCORSNetFailCount > 60)
					{
						CelluarStep = STA_PWR_OFF;
					}
					else
					{
						if(g_AskATCmdFlag == 1)
						{
							g_AskATCmdFlag = 2;
							CelluarStep = STA_NET_CHK_AT;
							g_ATSendCnt = 0;
							g_ATWaitFlag = 0;
						}
						else	
							CelluarStep = STA_STATE_IDLE;
					}
					break;					
				case STA_PWR_OFF:
					if(g_CelluarPowerOn == 1)
					{
						HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);			//关闭DTU电源
						g_CelluarPowerOn = 0;
						g_CelluarPowerOffDelay = uwTick;
						g_DataNetOk	= 0;
						g_CorsNetOk = 0;
						g_InitAskParFlag = 0;
					}
					CelluarStep = STA_STATE_IDLE;
					break;
					
				case STA_NET_CHK_AT:
					if(g_ATSendCnt < 20)
					{
						SendAT(g_AskATCmd);
						g_ATWaitFlag = 1;
						g_ATWaitDelay = 3000;
						g_ATSendCnt++;
						g_DTUSendDataDelay = uwTick;
					}
					else
					{
						g_CtrlMsgFlag = 0;
						g_AskATCmdFlag = 0;
						CelluarStep = STA_STATE_IDLE;
					}
					break;
				
				case STA_STATE_IDLE:
					IdleStatePro();					
					break;
				default:
					break;
			}
		
		}
		else if(g_CelluarPowerOn == 0 && JudgeMilSecondDelayInPro(g_CelluarPowerOffDelay,Delay30Sec) == true)
		{
			HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_SET);			//开启DTU电源
			InitCelluarGpio();
			CelluarStep = STA_PWR_ON;
			g_CelluarPowerOn = 1;
			g_CelluarPowerOnDelay = uwTick;
			InitNet();
			BSP_UartSetupPara(&hUART4, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
			BSP_UartBindInstance(&hUART4, UART4);
			HAL_NVIC_SetPriority(UART4_IRQn, 6, 0);
			__HAL_UART_ENABLE_IT(&hUART4, UART_IT_IDLE);	//开启空闲中断
			HAL_NVIC_EnableIRQ(UART4_IRQn);				//启用串口4中断  /* DMA串口接收初始化 */	
			BSP_DMAInitRxPara(&hDMA_Uart4Rx, DMA_REQUEST_2, DMA_NORMAL, DMA_PRIORITY_MEDIUM);
			BSP_DMAUartBindStream(&hDMA_Uart4Rx, DMA2_Channel5, &hUART4, DMA_CHANNEL_RX);
			/* 环形缓存初始化 */
			InitRingBuf(&g_Uart4RxRing, UART4_RX_BUFSIZE, g_Uart4RingBuf);
			HAL_UART_Receive_DMA(&hUART4, g_Uart4RxBuf, UART4_RX_BUFSIZE);			
		}
		
		if (g_CorsNetOk == 0 && g_CorsNetErr == 0 && (g_ModeInFlash.type == 1 || (g_ModeInFlash.type == 0 && g_SelfContCORS ==0x01)))
		{//如果CORS三分钟都没连接成功
			if (JudgeMilSecondDelayInPro(g_CorsNetDelay,Delay3Min) == true)
			{
				g_CorsNetErr = 1;
				g_CorsNetDelay = uwTick;	
				if (g_ModeInFlash.type == 0)
				{				
					ssd1306_Fill(Black);			//清除屏幕显示
					ssd1306_UpdateScreen();			//刷新屏幕
					ssd1306_maininterface();
				}					

			}
		}
	}
}


