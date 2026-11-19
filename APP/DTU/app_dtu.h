#ifndef __APP_DTU_H
#define __APP_DTU_H
#include "bsp.h"

#define datasocketid	0                                        //业务服务器socket
#define corssocketid	1                                        //CORS服务器socket
#define _ok          		        "OK"
#define _err                        "ERROR"
#define at				            "AT\x0d"		
#define ate0                        "ATE0\x0d"                   //关闭回显
#define singlesim_0                 "AT+SINGLESIM=0\x0d"         //指定手机卡0--插拔卡
#define singlesim_1                 "AT+SINGLESIM=1\x0d"         //指定手机卡1--贴片卡
#define cpin			            "AT+CPIN?\x0d"													//查询SIM卡状态
#define _cpin		                "+CPIN:"
#define	_ready			            "READY"
#define csq                         "AT+CSQ\x0d"                 //查询信号质量
#define cops                        "AT+COPS?\x0d"               //查询连接网络类型 
#define _csq                        "+CSQ: "			         //返回网络信号强度
#define _cops                       "+COPS: "    			     //查询连接网络类型
#define chkcreg                     "AT+CREG?\x0d"               //查询注册网络状态
#define _chkcreg                    "+CREG: "				     //返回注册网络信息
#define chkcgreg                    "AT+CEREG?\x0d"              //查询网络注册情况
#define _chkcgreg                   "+CEREG: "				     //返回网络注册情况
#define setclk                      "AT+CCLK=\x22"               //设置模块的时间、日期
#define chkclk                      "AT+CCLK?\x0d"               //查询模块的时间、日期
#define _chkcclk                    "+CCLK: \""                  //读时间模块的返回码
#define mipopen				        "AT+MIPOPEN=0,\"TCP\",\""
#define mipopen_cors				"AT+MIPOPEN=1,\"TCP\",\"rtk.ntrip.qxwz.com\",8002\x0d"    //CORS账号服务
#define _mipopen			        "+MIPOPEN: "
//#define _mipopen_cors			    "+MIPOPEN: 1,0"
#define disconn                     "\"disconn\""
#define	mipsend 				    "AT+MIPSEND="
#define	mipsend_cors			    "AT+MIPSEND=1\x0d"
#define	http				        "200"
#define _mipsend			        "\x0d\x0a>"
#define distcp				        "AT+MIPCLOSE="
#define distcp_ok				    "+MIPCLOSE: 0"
#define distcp_cors_ok				"+MIPCLOSE: 1"
#define mipurc      				"+MIPURC:"
#define rtcp                        "\"rtcp\""
#define cfgapn                      "AT+MIPCALL?\x0d"//"AT+CGDCONT=0,\"IPV4V6\",\"CMNET\"\x0d"  //定义PDP上下文
#define actcgact                    "AT+CGACT=0,1\x0d"                   //激活PDP 
#define cfgapn_cors                 "AT+CGDCONT=1,\"IP\",\"UNINET\"\x0d"  //定义PDP上下文
#define actcgact_cors               "AT+CGACT=1,1\x0d"                   //激活PDP 
#define sendtimeoutdata			    "AT+MIPCFG=\"timeout\",0,2\x0d"
#define sendtimeoutcors				"AT+MIPCFG=\"timeout\",1,2\x0d"

#define iccid                       "AT+MCCID\x0d"          //读取ICCID客服工具读取
#define _iccid                      "+MCCID: "        	    //读取ICCID的返回码

enum
{
	STA_PWR_ON 			= 1,		//上电
	STA_CLOSE_ECHO 		= 2,		//关闭数据回显
	STA_CHK_SIM_SWITCH  = 3,        //切换手机卡1/手机卡2
	STA_CHK_DATA_TIMEOUT = 4,
	STA_CHK_CORS_TIMEOUT = 5,
	STA_CHK_SIM_STA		= 6,		//查询SIM卡状态 	
	STA_CHK_SIM_CGREG 	= 7,    	//查询SIM卡PS业务状态
	STA_CHK_ICCID       = 8,        //查询手机卡ICCID    
	STA_CHK_NET_TYPE 	= 9,	    //查询运营商
	STA_CHK_SIGNAL		= 10,	    //查询信号强度
	STA_CHK_CCLK		= 11,	    //查询模块时间
	STA_SET_CCLK		= 12,	    //设置模块时间	
	STA_CFG_PDP			= 13,       //配置SOCKET1的PDP场景、参数
    STA_CFG_PDP_CORS	= 14,       //配置SOCKET2的PDP场景、参数
	STA_ACTIVE_PDP 		= 15,      	//激活SOCKET1的PDP上下文
	STA_ACTIVE_PDP_CORS	= 16,      	//激活SOCKET2的PDP上下文	
	STA_CREAT_DATATCP 	= 17,      	//创建TCP数据发送连接
	STA_CREAT_CORSTCP   = 18,      	//创建TCP的CORS连接
	STA_SENDCREAT_CORSNtrip =19,
	STA_CREAT_CORSNtrip     =20,
	STA_CLOSE_TCP 	    = 21,      	//断开服务	
	STA_SEND_TO_DATATCP = 22,      	//发送数据到数据服务器
	STA_SEND_TO_CORSTCP = 23,      	//发送数据到CORS服务器      
	STA_SENDEX_TO_SOCKET =24,      	//发送缓存数据到远程主机
	STA_NET_CHK_AT		= 25,       //发送AT命令
	STA_REC_SYMBOL      = 26,      	//收到'>' 	
	STA_PWR_OFF			= 27,       //断电 
	STA_STATE_IDLE 		= 99,  	    //空闲
	STA_REBOOT 			= 100   	//断电中	
};


extern uint8_t		g_CtrlMsgFlag;
extern uint8_t 	    CelluarStep;   //流程步骤
extern uint8_t      g_CelluarPowerOn;       //模块上电标记
extern uint32_t     g_CelluarPowerOnDelay;  //上电延时
extern uint8_t      g_ChangeCORSACCFlag;
extern uint16_t     tmpsendDataNum;
extern uint8_t 		g_D6Phone[6];
extern bool 		g_UpdateSuc;
extern bool 	    g_SendD6Suc;
extern bool 		g_Send5ASuc;
extern uint32_t 	g_Send5ABagNo;	//发送正常数据包序号
extern uint32_t 	g_SendD6BagNo;	//发送D6数据包序号
extern uint32_t 	g_RecD6BagNo;	//接收D6数据包序号
extern uint8_t 	    tmpsendbuf[3072];
extern uint32_t     g_SendD6Delay;
extern uint8_t      g_SendToDataNetFailCount;
extern uint8_t      g_SendToCORSNetFailCount;
extern uint8_t 		g_UpdateCodeTypeErrorFlag;		//程序程序类型错误标记
extern uint8_t 		g_NetAskTime;
extern uint8_t 		g_NetAskTimeCnt;
extern uint8_t      g_AskAuthorityFlag;       /* 收到返回控制权限的数据包 */
extern uint8_t      g_ChangeIPFlag;                   //修改IP的标志
extern uint8_t 		g_NetAskIPAndPortFlag;
extern uint8_t		g_AskIDFlag;
extern uint8_t      g_ClearFlashFlag;			//清空FLASH  1-整片擦除，2-指定地址删除
extern uint32_t     g_ClearFlashStartAddr;	    //清空FLASH起始位置
extern uint32_t     g_ClearFlashLen;			//清空FLASH长度
extern uint8_t      g_AskParFlag;
extern uint8_t      g_RturnFaultMessageFlag;  /* 返回故障信息*/
extern uint8_t      g_ErrCode;
extern uint8_t      g_InitAskParFlag;
extern uint8_t      g_PunCoorSendFlag;          //打点数据发送标记
extern uint8_t      g_DtuSendOk;                //DTU发送成功
extern uint32_t     g_DTUSendDataDelay;
extern uint8_t 	 	sendsocketid;
extern uint8_t      g_NeedSendGGAFlag;
extern int 			g_SocketId;             //连接ID
extern uint8_t      g_CorsNetErr;
extern uint32_t     g_CorsNetDelay;
extern char 		iccidbuff[32];
void SendRecSuc(void);
void ControlMsg(void);
void GetUpdateType(void);
void ProcessDtuNoAnswerFunc(void);
void ReturnClearFlashToGsm(void);
void ReturnFaultMessageToGsm(void);
void ReturnIDtoNet(void);
void ReturnMobileNoToNet(void);
void SendD6Data(uint8_t *data, uint8_t type);
bool SendPunCoorFunc(void);
void SendFullUpdateBag(void);
void SendUpdateBag(void);
void InitNet(void);
void ML307C_Process(void);
void SendGGAFunc(void);
#endif