#include "IMU_Acc.h"
#include "madgwick_uwb_fusion.h"
#include "uwb_gps_transform.h"
#include "instance.h"
//#include "app_glovar.h"
#include "app_cmd.h"
#include "bsp_dw1000.h"
#include "g_GlobalValDef.h"
#include "Type.h"
uint8_t switch8 = 1;                           //拨码开关键值0b00000001
uint8_t instance_mode = TAG;                         //设备运行角色
uint8_t dev_id;                                         //设备ID
uint8_t group_id;                                       //组ID
uint8_t anc_id;                                         //如当前角色是基站，则表示当前基站ID
uint8_t tag_id;                                         //如当前角色是标签，则表示当前标签ID
uint8_t state = STA_IDLE;                               //状态机状态控制
int32_t distance_report[8];                             //基站测距值数组，用于打包输出
int32_t group_report[8];                                //基站组ID数组，用于打包输出
uint32_t range_time;                                    //测距产生时间，串口打包发送
uint8_t frame_seq_nb = 0;                               //每帧数据增加1
uint8_t range_nb = 0;                                   //每次range增加1(poll resp1~4 fianl维护一套range_nb)
uint8_t recv_tag_id;                                    //如当前角色是基站，则表示当前基站收到标签发送过来数据的标签ID
uint8_t recv_anc_id;                                    //如当前角色是标签，则表示当前标签收到基站发送过来数据的基站ID
uint8_t range_status = RANGE_NULL;                      //测距成功标志位，用于打包输出
float rx_power;                                         //接收RSSI
float rssi[8];

uint16_t inst_slot_number;                              //系统内最大标签容量
uint8_t inst_dataRate;                                  //通信速率，用于根据当前110K还是6.8M确定数据超时等通信过程相关参数
uint8_t inst_ch;                                        //信道号Channel number
uint8_t inst_prf;                                       //PRF
uint8_t inst_one_slot_time;                             //一个slot的时间，根据通信速率不同而不同，单位ms
uint32_t inst_final_rx_timeout;                         //基站final接收超时时间，根据通信速率不同而不同，单位us
uint32_t inst_resp_rx_timeout;                          //标签发送poll后接收resp超时时间，根据通信速率不同而不同，单位us
uint64_t inst_poll2final_time;                          //单TWR周期poll起始到final结束的总时间
uint32_t inst_data_interval;                            //相邻两条数据的间隔，如poll和第一个resp的间隔，resp1和resp2的间隔，根据通信速率不同而不同，单位us
uint16 ant_dly = ANT_DLY;                               //天线延时
uint32 tx_power;                                        //发射增益代码
uint8_t UART_RX_BUF[200];                               //串口接收BUF
uint32_t uart_rx_len;                                   //串口接收数据长度
//vec3d anchorArray[8];                                   //基站坐标，用于标签解算自身位置
double distance_now_m;                                  //基站计算本周期测距结果，单位米
int32 distance_offset_cm;                               //距离校准，单位cm
uint8_t sos = 0;
uint8_t alarm = 0;
//uint8_t battery = 0;
int user_data[10];
uint8_t USE_IMU;
uint8_t ancrange_flag = 0;
uint8_t temp_dev_id;
uint8_t ancrange_count = 0;
uint8_t target_ancid = 0;
double g_tag_h=0.1;		//定位卡高度
double g_anchor_h=1.9;		//基站高度

DevConfig_t g_DevConfig;	//设备参数
/* 添加的变量 */
uint32_t flushcycle=0;		//计算数据更新

typedef struct
{
	uint32_t stat_timer;
	uint16_t stat_timeout_ms;
	uint32_t total_cnt;
	uint32_t ok_cnt;
	float okrate;
} stat_info;

stat_info twr_stat;

OutputFlag_t g_OutputFlag;
TagInfo_t PrevTag;

uint8_t twr_cnt=0;
uint8_t dis_cnt=0;
uint8_t ResultMode=0; 	//0=测距;1=1D测距;2=2D测距
uint8_t result_str[256];
uint8_t imu_str[128];
double length=0.0;		//1维测距基站安装距离
uint32_t imu_timer=0;
uint8_t ShowRanging=1;	//显示测距信息
uint16_t ShowIMUInterval=0;	//显示IMU信息间隔

typedef struct
{
	double x;
	double y;
	double lon;
	double lat;
	double dist;	//测距值
} ancdist_t;

ancdist_t anc_dist[MAX_ANCHORS];	//基站测距结果

coord_t anchor_cood[MAX_ANCHORS];	//基站坐标 4个
coord_t tag_pos;					//标签坐标

// 基站坐标（对应我们实测的布局）
static const anchor_t anchors[] = {
    {4.0f,  0.0f, 1},   // 基站A
    {4.0f, 20.0f, 2},   // 基站B
    {-1.2f, 10.0f, 3},  // 基站C
};

// 角度-偏置标定表（需通过标定实验获得，此处为示例值）
// 格式：{角度上限(度), 偏置补偿量(m)}
static const bias_entry_t bias_table[] = {
    { 30.0f, 0.15f},   // 0~30°：正向，偏小
    { 60.0f, 0.25f},   // 30~60°
    {120.0f, 0.35f},   // 60~120°
    {180.0f, 0.50f},   // 120~180°：背向，偏大
};

uint8_t UART_COMMAND_BUF[128];

/*************** UWB坐标转换 ***************/
uint8_t TransGPS=0;

/*******************函数声明***************************/
void parse_uart(uint8_t* data);
//void read_anc_coord(void);
//void print_config(void);


void set_instance(void)
{
//	led_off(LED_ALL);

	/* 根据基站和标签配置 */
    if(instance_mode == ANCHOR)
    {
		dwt_setcallbacks(&anc_tx_conf_cb, &anc_rx_ok_cb, &anc_rx_to_cb, &anc_rx_err_cb);
    }
    else
    {   //设置标签的中断回调函数
        dwt_setcallbacks(&tag_tx_conf_cb, &tag_rx_ok_cb, &tag_rx_to_cb, &tag_rx_err_cb);
    }

    //按角色初始化设备短地址，设置状态机初始状态
    if(instance_mode == ANCHOR)
    {
        /* 设备短地址为2个字节，为了区分A0和T0短地址，基站的最高位为1
        * 如A1短地址=0x8001，T1短地址=0x0001
        */
        uint16_t anc_short_add = 0x8000 | dev_id;
        dwt_setaddress16(anc_short_add);
        anc_id = dev_id;

        if(anc_id == 0)  //A0的group ID最高bit设置为1，为时序校准基站
        {
            group_id = group_id | 0x80;
        }
        dwt_forcetrxoff();
        state = STA_INIT_POLL_SYNC;

    }
    else //tag
    {
        dwt_setaddress16(dev_id);
        tag_id = dev_id;

        dwt_forcetrxoff();
        state = STA_IDLE;
    }
}

/******************************************************************
函数名称: bool SaveDevConfig
功能描述: 保存设备参数
参数输入: DevConfig_t	Cfg		设备参数
		  uint16_t		CfgLen	数据长度
返 回 值: bool		true=成功;false=失败
其    他: 无
******************************************************************/
bool SaveDevConfig(DevConfig_t Cfg, uint16_t CfgLen)
{
	uint16_t wb=0;
	uint16_t crc16=0;
	
	wb = WriteDataToFlashWithCRC16((uint8_t*)&Cfg, CfgLen, FLASH_UWB_PARAMETER_START, FLASH_UWB_PARAMETER_LEN);	//参数写flash
	if (wb == CfgLen)
	{
		debug_printf("Parameter has been saved.\r\n");
		return true;
	}
	else
	{
		debug_printf("Save failure.\r\n");
		return false;
	}
}

/******************************************************************
函数名称: bool ReadDevConfig
功能描述: 读取设备参数
参数输入: DevConfig_t	*Cfg	设备参数
		  uint16_t		CfgLen	数据长度
返 回 值: bool		true=成功;false=失败
其    他: 无
******************************************************************/
bool ReadDevConfig(DevConfig_t *Cfg, uint16_t CfgLen)
{
	uint16_t rb=0;
	uint16_t crc16=0;
	DevConfig_t devcfg;
	
	rb = ReadDataFromFlashWithCRC16((uint8_t*)&devcfg, CfgLen, FLASH_UWB_PARAMETER_START, FLASH_UWB_PARAMETER_LEN);	//参数写flash
 	if (rb == CfgLen)
	{
		*Cfg = devcfg;
		debug_printf("Parameter has been read.\r\n");
		return true;
	}
	else
	{
		debug_printf("Save failure.\r\n");
		return false;
	}
}

/******************************************************************
函数名称: void ShowAnchorCoordinates
功能描述: 显示基站坐标
参数输入: 
返 回 值: 
其    他: 无
******************************************************************/
void ShowAnchorCoordinates(void)
{
	uint8_t i;
	uint16_t len;
	
	len = 0;
	for (i=0; i<MAX_AHCHOR_NUMBER; i++)
	{
		if ((anchor_cood[i].lon > 0) && (anchor_cood[i].lat > 0))
		{
			sprintf(g_DebugInfoBuf + len, "coord #%d X=%.3f Y=%.3f lon=%.7f lat=%.7f\r\n", i, 
					anchor_cood[i].x, anchor_cood[i].y, 
					anchor_cood[i].lon, anchor_cood[i].lat);
			len = strlen(g_DebugInfoBuf);
		}
	}
	HAL_UART_Transmit(&hUSART3, (uint8_t*)&g_DebugInfoBuf, len, 500);
}


/******************************************************************
函数名称: void ShowDevConfig
功能描述: 显示配置参数
参数输入: 
返 回 值: 
其    他: 无
******************************************************************/
void ShowDevConfig(void)
{
	uint16_t len;
	char tmpstr[64]={0};
	
	memset(g_DebugInfoBuf, 0x00, 1024);
	sprintf(g_DebugInfoBuf, "========== ConfigInfo ==========\r\n");
	len = strlen(g_DebugInfoBuf);
	if (g_DevConfig.DeviceMode == TAG)
	{
		sprintf(g_DebugInfoBuf + len, "DevMode = TAG\r\n");
	}
	else if (g_DevConfig.DeviceMode == ANCHOR)
	{
		sprintf(g_DebugInfoBuf + len, "DevMode = ANCHOR\r\n");
	}
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "DevID = %d\r\n", g_DevConfig.DeviceID);	//设备ID
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "PanID = 0x%04x\r\n", g_DevConfig.PanID);	//网络ID
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "AntDly = %d\r\n", g_DevConfig.AntDelay);	//天线延迟
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "SolutionMode = %d\r\n", g_DevConfig.SolutionMode);	//解算模式
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "StatisticsMode = %d\r\n", g_DevConfig.StatisticsMode);//统计模式
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "ShowSolution = %d\r\n", g_DevConfig.ShowSolution);	//显示解算结果
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "Anchor Height = %.3f\r\n", g_DevConfig.AnchorHeight);//基站高度
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "Tag Height = %.3f\r\n", g_DevConfig.TagHeight);		//标签高度
	len = strlen(g_DebugInfoBuf);
	sprintf(g_DebugInfoBuf + len, "==== DW Config ====\r\n");
	len = strlen(g_DebugInfoBuf);
	
	/* Dw参数配置 */
	/* chan */
	sprintf(g_DebugInfoBuf + len, "Chan=%d\r\n", g_DevConfig.DwConfig.chan);	//通道
	len = strlen(g_DebugInfoBuf);
	
	/* prf */
	memset(tmpstr, 0x00, 64);
	switch (g_DevConfig.DwConfig.prf)
	{
		case DWT_PRF_16M:
			strncpy(tmpstr, "DWT_PRF_16M", 11);
			break;
		case DWT_PRF_64M:
			strncpy(tmpstr, "DWT_PRF_64M", 11);
			break;
		default:
			break;
	}
	sprintf(g_DebugInfoBuf + len, "prf=%s\r\n", tmpstr);	
	len = strlen(g_DebugInfoBuf);
	
	/* txPreambLength */
	memset(tmpstr, 0x00, 64);
	switch (g_DevConfig.DwConfig.txPreambLength)
	{
		case DWT_PLEN_4096:
			strncpy(tmpstr, "DWT_PLEN_4096", 13);
			break;
		case DWT_PLEN_2048:
			strncpy(tmpstr, "DWT_PLEN_2048", 13);
			break;
		case DWT_PLEN_1536:
			strncpy(tmpstr, "DWT_PLEN_1536", 13);
			break;
		case DWT_PLEN_1024:
			strncpy(tmpstr, "DWT_PLEN_1024", 13);
			break;
		case DWT_PLEN_512:
			strncpy(tmpstr, "DWT_PLEN_512", 12);
			break;
		case DWT_PLEN_256:
			strncpy(tmpstr, "DWT_PLEN_256", 12);
			break;
		case DWT_PLEN_128:
			strncpy(tmpstr, "DWT_PLEN_128", 12);
			break;
		case DWT_PLEN_64:
			strncpy(tmpstr, "DWT_PLEN_64", 11);
			break;
		default:
			break;
	}
	sprintf(g_DebugInfoBuf + len, "txPreambLength=%s\r\n", tmpstr);
	len = strlen(g_DebugInfoBuf);
	
	/* rxPAC */
	memset(tmpstr, 0x00, 64);
	switch (g_DevConfig.DwConfig.rxPAC)
	{
		case DWT_PAC8:
			strncpy(tmpstr, "DWT_PAC8", 8);
			break;
		case DWT_PAC16:
			strncpy(tmpstr, "DWT_PAC16", 9);
			break;
		case DWT_PAC32:
			strncpy(tmpstr, "DWT_PAC32", 9);
			break;
		case DWT_PAC64:
			strncpy(tmpstr, "DWT_PAC64", 9);
			break;
		default:
			break;
	}
	sprintf(g_DebugInfoBuf + len, "rxPAC=%s\r\n", tmpstr);
	len = strlen(g_DebugInfoBuf);
	
	/* txCode */
	sprintf(g_DebugInfoBuf + len, "txCode=%d\r\n", g_DevConfig.DwConfig.txCode);
	len = strlen(g_DebugInfoBuf);
	
	/* rxCode */
	sprintf(g_DebugInfoBuf + len, "rxCode=%d\r\n", g_DevConfig.DwConfig.rxCode);
	len = strlen(g_DebugInfoBuf);
	
	/* nsSFD */
	sprintf(g_DebugInfoBuf + len, "nsSFD=%d\r\n", g_DevConfig.DwConfig.nsSFD);
	len = strlen(g_DebugInfoBuf);
	
	/* dataRate */
	memset(tmpstr, 0x00, 64);
	switch (g_DevConfig.DwConfig.dataRate)
	{
		case DWT_BR_110K:
			strncpy(tmpstr, "DWT_BR_110K", 11);
			break;
		case DWT_BR_850K:
			strncpy(tmpstr, "DWT_BR_850K", 11);
			break;
		case DWT_BR_6M8:
			strncpy(tmpstr, "DWT_BR_6M8", 10);
			break;
		default:
			break;
	}
	sprintf(g_DebugInfoBuf + len, "dataRate=%s\r\n", tmpstr);
	len = strlen(g_DebugInfoBuf);
	
	/* phrMode */
	memset(tmpstr, 0x00, 64);
	switch (g_DevConfig.DwConfig.phrMode)
	{
		case DWT_PHRMODE_STD:
			strncpy(tmpstr, "DWT_PHRMODE_STD", 15);
			break;
		case DWT_PHRMODE_EXT:
			strncpy(tmpstr, "DWT_PHRMODE_EXT", 15);
			break;
		default:
			break;
	}
	sprintf(g_DebugInfoBuf + len, "phrMode=%s\r\n", tmpstr);
	len = strlen(g_DebugInfoBuf);
	
	/* sfdTO */
	sprintf(g_DebugInfoBuf + len, "sfdTO=%d\r\n", g_DevConfig.DwConfig.sfdTO);
	len = strlen(g_DebugInfoBuf);
	
	HAL_UART_Transmit(&hUSART3, (uint8_t*)&g_DebugInfoBuf, len, 500);		//输出
}

/******************************************************************
函数名称: void ShowUserData
功能描述: 显示用户字节
参数输入: uint8_t	recvid	接收设备id
返 回 值: 
其    他: 无
******************************************************************/
void ShowUserData(uint8_t recvid)
{
	uint8_t i;
	uint8_t user=0;
	uint16_t len;
	
	/* 判断用户字节是否存在 */
	for (i=0; i<10; i++)
	{
		if (user_data[i] != 0x00) 
		{
			user = 1;
			break;
		}
	}
	/* 输出用户字节 */
	if (user)
	{
		memset(g_DebugInfoBuf, 0x00, 1024);
		len = 0;
		sprintf(g_DebugInfoBuf + len, "##RecvId=%d ", recvid);
		len = strlen(g_DebugInfoBuf);
		for (i=0; i<10; i++)
		{
			sprintf(g_DebugInfoBuf + len, "%02X ", user_data[i]);
			len = strlen(g_DebugInfoBuf);
			if (i == 9)		//最后一个字节
			{
				sprintf(g_DebugInfoBuf + len, "\r\n");
				len = strlen(g_DebugInfoBuf);
			}
		}
		HAL_UART_Transmit(&hUSART3, (uint8_t*)&g_DebugInfoBuf, len, 500);		//输出
	}
}

#if defined (ULM1) || defined (LD150) || defined (LD_PA)
/* DW1000初始化 */
void DW1000_init(void)
{
//    uint8_t e2prom_data_read[EEP_UNIT_SIZE] = {0};
//    uint8_t e2prom_data_write[EEP_UNIT_SIZE] = {0};

    static dwt_config_t config1 = {             //6.8M
        .chan = 2,                              /* Channel number. */
        .prf = DWT_PRF_64M,                     /* Pulse repetition frequency. */
        .txPreambLength = DWT_PLEN_64,          /* Preamble length. Used in TX only. */
        .rxPAC = DWT_PAC8,                      /* Preamble acquisition chunk size. Used in RX only. */
        .txCode = 10,                           /* TX preamble code. Used in TX only. */
        .rxCode = 10,                           /* RX preamble code. Used in RX only. */
        .nsSFD = 1,                             /* 0 to use standard SFD, 1 to use non-standard SFD. */
        .dataRate = DWT_BR_6M8,                 /* Data rate. */
        .phrMode = DWT_PHRMODE_STD,             /* PHY header mode. */
        .sfdTO = (65 + DW_NS_SFD_LEN_6M8 - 8)   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    };

    static dwt_config_t config2 = {                 //110K
        .chan = 2,                                  /* Channel number. */
        .prf = DWT_PRF_64M,                         /* Pulse repetition frequency. */
        .txPreambLength = DWT_PLEN_1024,            /* Preamble length. Used in TX only. */
        .rxPAC = DWT_PAC32,                         /* Preamble acquisition chunk size. Used in RX only. */
        .txCode = 10,                               /* TX preamble code. Used in TX only. */
        .rxCode = 10,                               /* RX preamble code. Used in RX only. */
        .nsSFD = 1,                                 /* 0 to use standard SFD, 1 to use non-standard SFD. */
        .dataRate = DWT_BR_110K,                    /* Data rate. */
        .phrMode = DWT_PHRMODE_STD,                 /* PHY header mode. */
        .sfdTO = (1025 + DW_NS_SFD_LEN_110K - 32)   /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    };



//     static dwt_config_t config2 = {                 //850K
//         .chan = 2,                                  /* Channel number. */
//         .prf = DWT_PRF_64M,                         /* Pulse repetition frequency. */
//         .txPreambLength = DWT_PLEN_256,             /* Preamble length. Used in TX only. */
//         .rxPAC = DWT_PAC16,                         /* Preamble acquisition chunk size. Used in RX only. */
//         .txCode = 9,                                /* TX preamble code. Used in TX only. */
//         .rxCode = 9,                                /* RX preamble code. Used in RX only. */
//         .nsSFD = 1,                                 /* 0 to use standard SFD, 1 to use non-standard SFD. */
//         .dataRate = DWT_BR_850K,                    /* Data rate. */
//         .phrMode = DWT_PHRMODE_STD,                 /* PHY header mode. */
//         .sfdTO = (257 + DW_NS_SFD_LEN_850K - 16)    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
//     };

    static dwt_txconfig_t txconfig_options = {
        .PGdly = 0xc2,            /* PG delay */
        .power = TX_POWER         /* TX power */
    };

	/* 复位DW1000 初始化DW1000前SPI配置为低速 */
//    reset_DW1000(); /* Target specific drive of RSTn line into DW1000 low for a period. */
//    port_set_dw1000_slowrate();

    if(DWT_DEVICE_ID != dwt_readdevid()) //如读取失败，先执行唤醒
    {
    	port_wakeup_IC(); //使用SPI-NS管脚唤醒DW1000
        dwt_softreset();  //软件复位
    }
    reset_DW1000();       //复位

	/* 初始化失败 */
    if (dwt_initialise(DWT_LOADUCODE) == DWT_ERROR)
    {
		debug_printf("DWT Init failure.\r\n");
		//DW1000初始化错误
		
        while (1);
    }
    port_set_dw1000_fastrate();


    /* 1号：IMU标签 on=输出IMU数据， off=输出正常mc数据
     * 2号：通信特征  ULM1_2_Off=4tags|110K|100ms|CH2   ULM1_2_On=1tag|6.8M|10ms|CH5
     * 3号：外部耗电开关，用于加大设备功耗，避免充电宝自动关闭，on=开启，off=关闭
     * 4号：角色控制，on=基站，off=标签
     * 5，6，7号：设备ID，000=0，001=1，010=2 ....
     * 8号：硬件测距卡尔曼滤波开关，on=开启，off=关闭（默认on）
     */
	
	/* 使用6.8MHz配置 */
	inst_slot_number = MAX_TAG_NUMBER_SW2_ON;
	dwt_configure(&config1);//设置射频通信参数
	inst_dataRate = config1.dataRate;
	inst_ch = config1.chan;
	inst_prf = config1.prf;

//	/* 使用110KHz配置 */
//	inst_slot_number = MAX_TAG_NUMBER_SW2_OFF;
//	dwt_configure(&config2);//设置射频通信参数
//	inst_dataRate = config2.dataRate;
//	inst_ch = config2.chan;
//	inst_prf = config2.prf;
	
	/* 配置通信时间 */
    if(inst_dataRate == DWT_BR_6M8)
    {
        inst_one_slot_time = ONE_SLOT_TIME_MS_6P8M;
        inst_final_rx_timeout = FINAL_RX_TIMEOUT_6P8M;
        inst_resp_rx_timeout = RESP_RX_TIMEOUT_6P8M;
        inst_data_interval = DATA_INTERVAL_TIME_6P8M;
        inst_poll2final_time = ((FIRST_RESP_SEND_6P8M + MAX_AHCHOR_NUMBER * inst_data_interval) * UUS_TO_DWT_TIME);
    }
    else if(inst_dataRate == DWT_BR_110K)
    {
        inst_one_slot_time = ONE_SLOT_TIME_MS_110K;
        inst_final_rx_timeout = FINAL_RX_TIMEOUT_110K;
        inst_resp_rx_timeout = RESP_RX_TIMEOUT_110K;
        inst_data_interval = DATA_INTERVAL_TIME_110K;
        inst_poll2final_time = ((FIRST_RESP_SEND_110K + MAX_AHCHOR_NUMBER * inst_data_interval) * UUS_TO_DWT_TIME);
    }
	
	/* 配置发送功率参数 按默认值 */
	txconfig_options.power = TX_POWER;
    tx_power = txconfig_options.power;
    dwt_configuretxrf(&txconfig_options);//设置发射功率和pg值

	/* 读设备参数配置 */
	if (ReadDevConfig(&g_DevConfig, sizeof(g_DevConfig)) == false)
	{
		/* 读参数配置失败 使用默认配置 */
		g_DevConfig.DeviceID = 0;
		g_DevConfig.DeviceMode = TAG;
		g_DevConfig.PanID = PAN_ID;
		g_DevConfig.AntDelay = ANT_DLY;
		g_DevConfig.SolutionMode = 0;	//测距模式
		g_DevConfig.DwConfig = config2;
		g_DevConfig.AnchorHeight = 1.9;	//默认基站高度1.9m 标签高度0.1m(10cm)
		g_DevConfig.TagHeight = 0.1;
	}
	if (g_DevConfig.DeviceMode == ANCHOR)	//当前设备是基站 默认不显示距离
	{
		g_DevConfig.ShowSolution = 0;
	}

	/* 配置基站和标签模式 */
	instance_mode = g_DevConfig.DeviceMode;
//	instance_mode = ANCHOR;		//配置为基站
//	instance_mode = TAG;		//配置为标签

	/* 配置ID和组ID */
	dev_id = g_DevConfig.DeviceID;
	group_id = 0;

	/* 配置天线延迟 按默认值 */
	/* 标签天线延迟 使用默认16500 */
	ant_dly = g_DevConfig.AntDelay;
//	if (instance_mode == TAG)
//	{
//		ant_dly = 16500;
//	}
//	else if (instance_mode == ANCHOR)
//	{
//		if (dev_id == 0) ant_dly = 16592;
//		if (dev_id == 1) ant_dly = 16564;
//		if (dev_id == 2) ant_dly = 16627;
//		if (dev_id == 3) ant_dly = 16629;
//	}
//	ant_dly = ANT_DLY;
    dwt_setrxantennadelay(ant_dly);//设置天线延时
    dwt_settxantennadelay(ant_dly);

	/* 设置网络参数 */
	dwt_setpanid(g_DevConfig.PanID);//设置PAN ID 组号
    dwt_enableframefilter(DWT_FF_DATA_EN | DWT_FF_ACK_EN);  //设置帧过滤模式开启

    dwt_setfinegraintxseq(0); // 启用外置 PA 前关闭细粒度 TX 时序
    dwt_setlnapamode(1, 1);//设置外置PA和LNA控制开启
    dwt_setleds(DWT_LEDS_ENABLE | DWT_LEDS_INIT_BLINK);//设置DW3000控制的收发指示灯开启，低功耗时可注释掉


	if (instance_mode == ANCHOR)
	{
		debug_printf("[DW1000] Set ANCHOR mode\r\n");
	}
	else
	{
		debug_printf("[DW1000] Set TAG mode\r\n");
	}
	
	
    //设置中断标志
    dwt_setinterrupt(DWT_INT_TFRS | DWT_INT_RFCG | (DWT_INT_ARFE | DWT_INT_RFSL | DWT_INT_SFDT | DWT_INT_RPHE | DWT_INT_RFCE | DWT_INT_RFTO | DWT_INT_RXPTO), 1);
    set_instance();
    
	debug_printf("[DW1000] Configure Successfully.\r\n");
}
#endif

/******************************************************************
函数名称: uint8_t dw_statistics
功能描述: 数据统计
参数输入: stat_info	*StatInfo	统计信息
参数输出: coord_t  *tag        迭代后坐标
返 回 值: 1=正常;0=错误
******************************************************************/
uint8_t dw_statistics(stat_info *StatInfo)
{
	uint8_t ret=0;
	char stat_str[128]={0};
	uint16_t slen=0;
	
	if (range_status == RANGE_TWR_OK)
	{
		StatInfo->total_cnt++;
		StatInfo->ok_cnt++;
	}
	else if (range_status == RANGE_ERROR)
	{
		StatInfo->total_cnt++;
	}
	
	/* 统计结果 */
	if (JudgeTick(StatInfo->stat_timer, StatInfo->stat_timeout_ms) == true)
	{
		StatInfo->stat_timer = uwTick;
		StatInfo->okrate = 0;
		if (StatInfo->total_cnt > 0)
		{
			StatInfo->okrate = (float)StatInfo->ok_cnt / (float)StatInfo->total_cnt * 100;
		}
		ret = 1;
		memset(stat_str, 0x00, 128);
		sprintf((char*)stat_str, "OK/Total=%d/%d, rate=%2.2f\r\n", StatInfo->ok_cnt, StatInfo->total_cnt, StatInfo->okrate);
		slen = strlen(stat_str);
		HAL_UART_Transmit(&hUSART3, (uint8_t*)&stat_str, slen, 500);		//输出统计信息
		StatInfo->ok_cnt = 0;
		StatInfo->total_cnt = 0;
	}
		
	return ret;
}

/******************************************************************
函数名称: void gn_refine
功能描述: 高斯牛顿迭代精化，在 LLS 初值基础上，迭代最小化非线性残差
参数输入: coord_t  *tag        初始LLS坐标
          coord_t  *anchor     基站坐标数组（长度 num_anchor）
          int      num_anchor  参与定位的基站数量
          int      max_iter    迭代次数一般为5
参数输出: coord_t  *tag        迭代后坐标
******************************************************************/
void gn_refine(coord_t *tag, ancdist_t *anchor, int num_anchor, int max_iter)
{
	double x = tag->x, y = tag->y;
	double H[MAX_ANCHORS][2], r[MAX_ANCHORS];   // N行2列Jacobian + 残差
	double HTH[2][2], HTr[2], det;
	double dd, dx, dy, delta_x, delta_y;
	int i, iter;

	for (iter = 0; iter < max_iter; iter++)
	{
		/* 构建 Jacobian H 和残差 r（对 N 个基站） */
		for (i = 0; i < num_anchor; i++)
		{
			dx = x - anchor[i].x;
			dy = y - anchor[i].y;
			dd = sqrt(dx * dx + dy * dy);
			if (dd < 0.001) dd = 0.001;     // 防除零
			H[i][0] = dx / dd;
			H[i][1] = dy / dd;
			r[i] = anchor[i].dist - dd;
		}

		/* H^T*H (2x2) 与 H^T*r (2x1)：对 N 个基站累加 */
		HTH[0][0] = 0.0; HTH[0][1] = 0.0; HTH[1][0] = 0.0; HTH[1][1] = 0.0;
		HTr[0] = 0.0; HTr[1] = 0.0;
		for (i = 0; i < num_anchor; i++)
		{
			HTH[0][0] += H[i][0] * H[i][0];
			HTH[0][1] += H[i][0] * H[i][1];
			HTH[1][1] += H[i][1] * H[i][1];
			HTr[0] += H[i][0] * r[i];
			HTr[1] += H[i][1] * r[i];
		}
		HTH[1][0] = HTH[0][1];

		/* 解 HTH*delta = HTr（克莱姆法则） */
		det = HTH[0][0]*HTH[1][1] - HTH[0][1]*HTH[1][0];
		if (fabs(det) < 1e-10) break;
		delta_x = (HTH[1][1]*HTr[0] - HTH[0][1]*HTr[1]) / det;
		delta_y = (HTH[0][0]*HTr[1] - HTH[1][0]*HTr[0]) / det;

		/* 更新 + 收敛判断 */
		x += delta_x;
		y += delta_y;
		if (fabs(delta_x) < 0.0001 && fabs(delta_y) < 0.0001) break;
	}

	tag->x = x;
	tag->y = y;
}

/******************************************************************
函数名称: int lls_solve
功能描述: 最小二乘法计算定位坐标 (支持 3~8 个基站)
参数输入: const ancdist_t	*anchors    基站坐标数组(有效基站连续排列, >=3)
          int           	num_anchor  有效基站个数 (3 ~ MAX_ANCHORS)
          double        	*out_x      输出标签 X 坐标指针
          double        	*out_y      输出标签 Y 坐标指针
返 回 值: 1 = 定位成功
          0 = 失败(参数非法 / 基站数不足 / 几何退化 det 过小)
其    他: 无
******************************************************************/
int lls_solve(const ancdist_t *anchors, int num_anchor, double *out_x, double *out_y)
{
    double ATA[2][2] = {0};     /* 正规方程系数矩阵 A^T A */
    double ATb[2]    = {0};     /* 正规方程右端向量 A^T b */
    double det;
    int    i;

    /* 参数校验 */
    if (anchors == NULL || out_x == NULL || out_y == NULL)
        return 0;
    if (num_anchor < 3 || num_anchor > MAX_ANCHORS)
        return 0;

    /* 参考站取第 0 号有效基站 */
    double x0 = anchors[0].x;
    double y0 = anchors[0].y;
    double d0 = anchors[0].dist;

    /* 逐站构造差分方程并累加进正规方程 */
    for (i = 1; i < num_anchor; i++)
    {
        double xi  = anchors[i].x;
        double yi  = anchors[i].y;
        double di  = anchors[i].dist;
        double a0  = 2.0 * (x0 - xi);                    /* 方程 x 列系数 */
        double a1  = 2.0 * (y0 - yi);                    /* 方程 y 列系数 */
        double b   = di*di - d0*d0 + (x0*x0 + y0*y0) - (xi*xi + yi*yi);

        ATA[0][0] += a0 * a0;
        ATA[0][1] += a0 * a1;
        ATA[1][0] += a0 * a1;   /* 对称矩阵 */
        ATA[1][1] += a1 * a1;
        ATb[0]    += a0 * b;
        ATb[1]    += a1 * b;
    }

    /* 克莱姆法则求解 2x2 */
    det = ATA[0][0] * ATA[1][1] - ATA[0][1] * ATA[1][0];
    if (fabs(det) < 1e-10)
        return 0;   /* 几何退化(基站共线/重叠/站数不足), 无法定位 */

    *out_x = (ATA[1][1] * ATb[0] - ATA[0][1] * ATb[1]) / det;
    *out_y = (ATA[0][0] * ATb[1] - ATA[1][0] * ATb[0]) / det;

    return 1;
}

/******************************************************************
函数名称: int pack_active_anchors
功能描述: 绑定有效基站测距数据
参数输入: const anchor_t *anc      基站数组(带 range, 最多 MAX_ANCHORS 个)
          int            max_n     数组实际长度(通常 MAX_ANCHORS)
          double        *dist_lls  输出: 压缩后连续距离数组
          ancdist_t		*local_anchor 输出: 压缩后连续坐标数组(与dist对齐)
返 回 值: 有效基站个数 n (n<3 时调用方应放弃本次定位)
其    他: 无
******************************************************************/
int pack_active_anchors(const coord_t *anc, const double *dist, int max_n, ancdist_t *local_anchor)
{
    int n = 0, i;

    for (i = 0; i < max_n; i++)
    {
        if (dist[i] > 0)                //只接收有效距离 剔除无效距离
        {
			local_anchor[n].dist = dist[i];
            local_anchor[n].x = anc[i].x;    
            local_anchor[n].y = anc[i].y;
			local_anchor[n].lon = anc[i].lon;
			local_anchor[n].lat = anc[i].lat;
            n++;
        }
    }
    return n;
}

/******************************************************************
函数名称: int dist_projection
功能描述: 高度投影
参数输入: const anchor_t *anc		基站数组(带 range, 最多 MAX_ANCHORS 个)
          int			max_n		数组实际长度(通常 MAX_ANCHORS)
          double        anchor_h	基站高度
          double		tag_h		标签高度
返 回 值: 有效基站个数 n (n<3 时调用方应放弃本次定位)
其    他: 无
******************************************************************/
int dist_projection(ancdist_t *anchor, int max_n, double anchor_h, double tag_h)
{
	double dh=0;
	double dd=0;
	int i;
	
	/* 变量检查 */
	if ((anchor_h > 0) && (tag_h > 0) && (anchor_h > tag_h))
	{
		dh = anchor_h - tag_h;
	}
	else
		return 0;
	
	for (i=0; i<max_n; i++)
	{
		dd = anchor[i].dist;
		anchor[i].dist = sqrt(dd*dd - dh*dh);	//计算平面投影
	}
	
	return 1;
}

/******************************************************************
函数名称: uint16_t dw_FormatOutputStr
功能描述: 格式化显示结果
参数输入: char			*OutStr	输出字符串
		  coord_t		tag		标签位置
		  double		Dist[]	距离数组
		  float			Rssi[]	信号强度数组
		  OutputFlat_t	Flag	定位输出标记
返 回 值: uint16		字符串长度
其    他: 无
******************************************************************/
uint16_t dw_FormatOutputStr(char *OutStr, coord_t tag, double Dist[], float Rssi[], OutputFlag_t Flag)
{
	uint8_t i;
	uint16_t len;
	
	len = 0;
	
	/* 输出异常点信息 */
	if (Flag.TagAbnormal == 1)
	{
		sprintf(OutStr + len, "[X]");
		len = strlen(OutStr);
	}
	
	/* 输出标签坐标 */
	if (Flag.ShowPos == 1)
	{
		sprintf(OutStr + len, "PosX=%.3f PosY=%.3f ", tag.x, tag.y);
		len = strlen(OutStr);
	}
	
	/* 输出RSSI */
	if (Flag.ShowRssi == 1)
	{
		/* 遍历所有基站数据 */
		for (i=0; i<MAX_AHCHOR_NUMBER; i++)
		{
			if (Dist[i] > 0)	//有效数据
			{
				sprintf(OutStr + len, "rssi%d=%.2f ", i, Rssi[i]);
				len = strlen(OutStr);
			}
		}
	}
	
	/* 输出距离 */
	if (Flag.ShowDist == 1)
	{
		for (i=0; i<MAX_AHCHOR_NUMBER; i++)
		{
			if (Dist[i] > 0)	//有效数据
			{
				sprintf(OutStr + len, "d%d=%.3f ", i, Dist[i]);
				len = strlen(OutStr);
			}
		}
	}
	
	/* 输出经纬度 */
	if (Flag.ShowGPS == 1)
	{
		sprintf(OutStr + len, "lon=%.7f lat=%.7f", tag.lon, tag.lat);
		len = strlen(OutStr);
	}
	
	/* 换行 */
	sprintf(OutStr + len, "\r\n");
	len = strlen(OutStr);
	return len;
}

/******************************************************************
函数名称: uint8_t dw_AbnormalDetection
功能描述: 异常数据检测
参数输入: uint8_t	Mode	计算类型
返 回 值: 
其    他: 无
******************************************************************/
uint8_t dw_AbnormalDetection(TagInfo_t *Prev, double Tagx, double Tagy, ancdist_t *anchor, uint8_t n, uint32_t NewTick)
{
	uint8_t i;
	double dt=0;
	double dx, dy, spd;
	double dsr_max=0, dsr=0;
	
	uint8_t ret=0x00;
	
	/* 第一帧, 无法判断 */
	if (Prev->Tick == 0) 
	{           
        Prev->TagCoord.x = Tagx; 
		Prev->TagCoord.y = Tagy;
        Prev->Tick = NewTick;
        for (i = 0; i < n; i++)
		{
			Prev->Distance[i] = anchor[i].dist;
		}
        return 0;
    }
	
	/* 计算时间 */
	dt = (double)(NewTick - Prev->Tick) / 1000.0;		//计算时间
	if (dt <= 0)
		return 0;
	
	/* 计算速度 */
	dx = Tagx - Prev->TagCoord.x;
	dy = Tagy - Prev->TagCoord.y;
	spd = sqrt(dx*dx + dy*dy) / dt;
	
	/* 计算测距变化率 取最大值 */
	for (i=0; i<n; i++)
	{
		dsr = fabs(anchor[i].dist - Prev->Distance[i]) / dt;
		if (dsr > dsr_max) dsr_max = dsr;
	}
	
	/* 更新上次值 */
	Prev->TagCoord.x = Tagx;
	Prev->TagCoord.y = Tagy;
	Prev->Tick = NewTick;
	for (i=0; i<n; i++)
	{
		Prev->Distance[i] = anchor[i].dist;
	}
	
	/* 判断异常 */
	ret = 0;
	if (spd > V_THRE) ret |= 0x01;		//速度超门限
	if (dsr_max > R_THRE) ret |= 0x02;	//测距超门限
	return ret;
}

/******************************************************************
函数名称: uint8_t dw_CalcDistanceOrCoordinates
功能描述: 计算距离或坐标
参数输入: uint8_t	Mode	计算类型
返 回 值: 
其    他: 无
******************************************************************/
uint8_t dw_CalcDistanceOrCoordinates(uint8_t Mode)
{
	uint8_t i;
	double distance_dd[8]={0.0};		//计算距离
//	double rssi[8]={0.0};				//计算信号强度
	uint16_t strlen;
	uint8_t ret=0;
	
	int n;
	int rets=0;
	
	/* 初始化 */
	for (i=0; i<8; i++)
	{
		distance_dd[i] = 0.0;
		/* 处理距离转换成米 */
		if (distance_report[i] > 0)
		{
			distance_dd[i] = (double)distance_report[i] / 1000.0;
		}
		else
		{
			distance_dd[i] = -1;
		}
	}
	memset(result_str, 0x00, 256);
	strlen = 0;

	/* 测距模式 */
	if (Mode == 0)
	{
		g_OutputFlag.ShowPos = 0;	//输出控制 只显示信号强度和距离
		g_OutputFlag.ShowRssi = 1;
		g_OutputFlag.ShowDist = 1;
		g_OutputFlag.ShowGPS = 0;
		g_OutputFlag.TagAbnormal = 0;
		strlen = dw_FormatOutputStr((char*)&result_str, tag_pos, distance_dd, rssi, g_OutputFlag);	//格式化输出字符串
		ret = 1;
	} /* Mode=0 测距模式 */
		
	/* 2维测距模式 */
	if (Mode == 2)
	{
		ret = 0;
		/* 使用最小二乘法+高斯牛顿迭代精化 计算坐标 */
		n = pack_active_anchors(anchor_cood, distance_dd, MAX_ANCHORS, anc_dist);	//剔除无效基站数据
		if (n < 3)
		{
			ret = 0;
		}
		else
		{
			ret = 0;
			rets = dist_projection(anc_dist, n, g_DevConfig.AnchorHeight, g_DevConfig.TagHeight);		//高度投影 换算平面距离
			if (rets == 1)
			{
				rets = lls_solve(anc_dist, n, &tag_pos.x, &tag_pos.y);		//LLS计算标签位置
				if (rets == 1)
				{
					gn_refine(&tag_pos, anc_dist, n, 5);	//高斯牛顿迭代
					g_OutputFlag.TagAbnormal = 0;
					rets = dw_AbnormalDetection(&PrevTag, tag_pos.x, tag_pos.y, anc_dist, n, uwTick);	//检测异常点
					if (rets != 0)
					{
						//当前点异常 沿用上一个坐标点
						tag_pos.x = PrevTag.TagCoord.x;
						tag_pos.y = PrevTag.TagCoord.y;
						g_OutputFlag.TagAbnormal = 1;	//异常点标记
					}
					enu_to_gps(tag_pos.x, tag_pos.y, anchor_cood[0].lat, anchor_cood[0].lon, &tag_pos.lat, &tag_pos.lon);	//ENU坐标转换GPS坐标
					g_OutputFlag.ShowPos = 1;	//全部数据都输出
					g_OutputFlag.ShowRssi = 1;
					g_OutputFlag.ShowDist = 1;
					g_OutputFlag.ShowGPS = 1;
					strlen = dw_FormatOutputStr((char*)&result_str, tag_pos, distance_dd, rssi, g_OutputFlag);
					ret = 1;
				}
			}
		}
	} /* 2维测距模式 */
	
	if (ret)
	{
		HAL_UART_Transmit(&hUSART3, result_str, strlen, 500);		//输出测距结果
	}
	
	return ret;
}

/* IMU融合初始化 */
void dw_imu_fusion_init(void)
{
	/* 初始化Madgwick算法 目前ODR=12.5Hz */
	madgwick_init(&madgwick, 0.05f, 12.5f);
	
	/* 初始化天线补偿算法 */
	antenna_comp_init(&fusion.comp, anchors, 3, bias_table, 4);
	
	/* 初始化运动检测器 */
	motion_detect_init(&fusion.motion);
	
	/* 状态初始化 */
	fusion.pos_x = 0.0f;
    fusion.pos_y = 0.0f;
    fusion.yaw = 0.0f;
    fusion.hist_idx = 0;
    fusion.hist_count = 0;
    fusion.madgwick = madgwick;
}

void dw_main(void)
{
//    uint8_t UART_TX_DATA[512], UART_TX_DEBUG[128];//串口数据输出
    int len=0,len2=0;
    static uint16_t fn = 0;         //串口数据流水号，每帧数据+1
	uint8_t ret;
	uint16_t slen=0;
	uint16_t rb=0;
	uint8_t i, gcnt;
	int transret=0;
	
	IMUResult imuret;

	/* DW1000 初始化 */
    DW1000_init();
	set_instance();
	
	twr_stat.stat_timer = uwTick;
	twr_stat.ok_cnt = 0;
	twr_stat.total_cnt = 0;
	twr_stat.stat_timeout_ms = 10000;
	
	imu_timer = uwTick;
	
	flushcycle = uwTick;
	g_DevConfig.StatisticsMode = 1;
	dw_imu_fusion_init();	//初始化融合算法
	uwb_gps_init(&xgps);	//初始化gps转换算法
	memset(anchor_cood, 0x00, sizeof(anchor_cood));
	rb = ReadUwbCoordinatesFromFlash((uint8_t*)&anchor_cood, sizeof(anchor_cood));	//读基站坐标信息
	if (rb == sizeof(anchor_cood))		//调用初始化算法
	{
		gcnt = 0;
		for (i=0; i<4; i++)
		{
			if ((anchor_cood[i].lat != 0) && (anchor_cood[i].lon != 0))		//基站坐标有经纬度
			{
				xpoint[i].uwb_x = anchor_cood[i].x;
				xpoint[i].uwb_y = anchor_cood[i].y;
				xpoint[i].lat = anchor_cood[i].lat;
				xpoint[i].lon = anchor_cood[i].lon;
				gcnt++;
			}
		}
		transret = uwb_gps_calibrate(&xgps, xpoint, gcnt);		//初始化经纬度坐标
		if (!transret)
		{
			ShowAnchorCoordinates();
		}
	}
	
	/* 测距功能实现，按角色执行基站状态机或标签状态机 */
    while(1)
    {
		if (instance_mode == TAG)
			tag_app();
		else if (instance_mode == ANCHOR)
			anchor_app();
		
		/* 每100ms检测中断状态 */
		if (JudgeTick(flushcycle, 100) == true)
		{
			flushcycle = uwTick;
			g_recvirq++;
			if (g_recvirq >= 10)
			{
				g_recvirq = 0;
//				if (instance_mode == ANCHOR)
//					HAL_NVIC_SystemReset();		//基站自动重启
			}
		}
		
		/* 基站模式 */
		if (instance_mode == ANCHOR)
		{
			/* 输出测距信息 */
			if (g_DevConfig.ShowSolution == 1)
			{
				if (range_status == RANGE_TWR_OK)
				{
					ret = dw_CalcDistanceOrCoordinates(0);	//基站模式仅测距
					/* 初始化标志位 */
				}
				else if (range_status == RANGE_ERROR)
				{
				}
			}
			
			/* 输出用户数据 */
			ShowUserData(recv_tag_id);
		}
		
		/* 标签模式 */
		if (instance_mode == TAG)
		{
			/* 测距成功完成 */
			if (range_status == RANGE_TWR_OK)
			{
				ret = dw_CalcDistanceOrCoordinates(g_DevConfig.SolutionMode);	//数据解算
				/* 计算统计结果 */
				if (g_DevConfig.StatisticsMode == 1)
				{
					dw_statistics(&twr_stat);
				}
			}
			/* 测距错误 */
			else if (range_status == RANGE_ERROR)
			{
				if (g_DevConfig.StatisticsMode == 1)
				{
					dw_statistics(&twr_stat);
				}
			}			
		}
		
		/* 清除测距标志位 */
		if ((range_status == RANGE_TWR_OK) || (range_status == RANGE_ERROR))
		{
			/* 初始化标志位 */
			range_status = RANGE_NULL;//清空标志位
			for(i = 0; i < 8; i++)  //清空distance_report数组，设置无效值
			{
			   distance_report[i] = -1; 
			   group_report[i] = -1; 
			}
		}
		int16_t dbgret = DebugRecvProcess();
		if (dbgret == DBGRET_OK)
		{
			CmdRecvProcess((char*)&g_DebugDatBuf+1, '$', 1);	//数据解析 '$'数据头 需要\r\n结尾
		
		
		}		
//		/* 接收串口数据 */
//		if(g_Uart3.usRxCount > 0)//收到串口数据
//		{
//			CmdRecvProcess((char*)&g_RxBuf3, '$', 1);	//数据解析 '$'数据头 需要\r\n结尾
//			g_Uart3.usRxCount = 0;
//			memset(g_RxBuf3, 0, sizeof(g_RxBuf3));
//		}
    }
}

/*
    串口指令集，注意发送指令以$开头，以\r\n结尾
    $rboot              重启
    $rantdly            查询天线延时参数
    $reset              恢复默认参数
    $santdly,16375      设置天线延时参数（10进制）
    $stxpwr,1f1f1f1f    设置发射增益参数（16进制）
    $sanccd,0,0,2,0,3.1,2,3.1,0,2,3.1,3.1,2  设置基站坐标A0.X,A0.Y,A0.Z,A1.X,A1,Y,A1,Z,A2.X,A2,Y,A2,Z,A3.X,A3,Y,A3,Z
    $cali,-30,-169      校准距离和PDOA，第一个参数为距离，单位cm，第二个参数为PDOA单位角度
    $saddr,1            设置标签ID
    $sgrop,1            设置组ID
    $ssw,00010001       设置拨码开关值，设置后实体拨码开关将不起作用，16进制 0001 0001
    $s2onslot,1         设置2号拨码on时的最大标签容量
    $s2offslot,1        设置2号拨码off时的最大标签容量
    $synctime,8640000   设置系统同步时间
    $ancrangestart      基站间测距开始
    $ancrangestop       基站间测距停止
    $sdata,12,34,56,78,90,AA,BB,00,00,00   标签发送自定义数据，长度固定，不足补0
	
	$fusion,1			启用融合算法1=启用,0=关闭
	$showimuinfo,100	显示陀螺仪信息 100=显示间隔100ms，0=不显示
	$setranging,1		启用测距 1=启用;0=关闭
*/

#if 1
/* 串口指令解析 */
void parse_uart(uint8_t* data)
{

    uint8_t len = 0;
    uint8_t e2prom_data_write[EEP_UNIT_SIZE] = {0};

	memset(UART_COMMAND_BUF, 0x00, 128);
    if((strcmp((char*)data, "$sdata\r\n") == 0) || (strlen((char*)data) == 38))  //标签发送自定义数据
    {
        char *ptr, *retptr;
        ptr = (char*)data;
        retptr = strtok(ptr, ",");//解析数据头
        if((strcmp(retptr, "$sdata") == 0) && (instance_mode == TAG))
        {
            for(uint8_t i = 0; i < 10; i++)
            {
                ptr = NULL;
                retptr = strtok(ptr, ",");
                sscanf(retptr, "%x", &user_data[i]);
                //printf("user_data[%d] = %x\r\n", i, user_data[i]);
            }
        }
    }


    else if(strchr((char*)data, ',') > 0)//带参数指令 如 $santdly,11223
    {
        char *ptr, *retptr;
        ptr = (char*)data;
        retptr = strtok(ptr, ",");//解析数据头

        if(strcmp(retptr, "$synctime") == 0)    //$synctime,8640000 设置系统同步时间
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            sys_time_diff = HAL_GetTick() - atoi(retptr);
        }



        else if(strcmp(retptr, "$santdly") == 0)//设置天线延时参数  $santdly,16375
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            ant_dly = atoi(retptr);
            e2prom_data_write[0] = 0xAA;
            e2prom_data_write[1] = ant_dly >> 8;
            e2prom_data_write[2] = (uint8)ant_dly;
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
            HAL_NVIC_SystemReset();//重启
        }
        else if(strcmp(retptr, "$stxpwr") == 0)//设置发射增益参数  $stxpwr,1f1f1f1f 
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            sscanf(retptr, "%08lx", &tx_power);
            e2prom_data_write[0] = 0xAA;
            e2prom_data_write[1] = tx_power >> 24;
            e2prom_data_write[2] = tx_power >> 16;
            e2prom_data_write[3] = tx_power >> 8;
            e2prom_data_write[4] = tx_power;
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
            HAL_NVIC_SystemReset();//重启 
        }

        else if(strcmp(retptr, "$sanccd") == 0) //给标签设置基站坐标，用于标签自己三边定位输出定位结果
        {
            retptr[7] = ',';
//            uint8_t zero[COORD_LENGTH] = {0};
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
            HAL_NVIC_SystemReset();//重启
        }
        else if(strcmp(retptr, "$saddr") == 0) //设置标签ID
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            e2prom_data_write[0] = 0xAA;
            e2prom_data_write[1] = atoi(retptr);
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);            
            HAL_NVIC_SystemReset();//重启
        }
        else if(strcmp(retptr, "$sgrop") == 0) //设置组ID
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            e2prom_data_write[0] = 0xAA;
            e2prom_data_write[1] = atoi(retptr);
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);            
            HAL_NVIC_SystemReset();//重启
        }
        else if(strcmp(retptr, "$ssw") == 0) //设置拨码开关值
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            e2prom_data_write[0] = 0xAA;
			
			switch8 = atoi(retptr);
			
			
//            int sw8 = strtol(retptr, NULL, 2);
            e2prom_data_write[1] = switch8;
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            //len = sprintf((char*)UART_COMMAND_BUF, "e2prom_data_write=0x%x\r\n", e2prom_data_write[1]);
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);            
            HAL_NVIC_SystemReset();//重启
        }
        else if(strcmp(retptr, "$s2onslot") == 0) //设置2号拨码on最大标签容量
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            e2prom_data_write[0] = 0xAA;
            e2prom_data_write[1] = atoi(retptr);
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            //len = sprintf((char*)UART_COMMAND_BUF, "e2prom_data_write=0x%x\r\n", e2prom_data_write[1]);
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);            
            HAL_NVIC_SystemReset();//重启
        }
        else if(strcmp(retptr, "$s2offslot") == 0) //设置2号拨码off最大标签容量
        {
            ptr = NULL;
            retptr = strtok(ptr, ",");
            e2prom_data_write[0] = 0xAA;
            e2prom_data_write[1] = atoi(retptr);
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            //len = sprintf((char*)UART_COMMAND_BUF, "e2prom_data_write=0x%x\r\n", e2prom_data_write[1]);
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);            
            HAL_NVIC_SystemReset();//重启
        }
		/* 显示IMU信息 */
		else if (strcmp(retptr, "$showimuinfo") == 0)
		{
			ptr = NULL;
			retptr = strtok(ptr, ",");
			ShowIMUInterval = atol(retptr);
			if (ShowIMUInterval <= 0)
			{
				ShowIMUInterval = 0;
				len = sprintf((char*)UART_COMMAND_BUF, "$IMU Hide\r\n");
				HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
			}
			else
			{
				len = sprintf((char*)UART_COMMAND_BUF, "$IMU Show %d\r\n", ShowIMUInterval);
				HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
			}
		}
		/* 启用测距 */
		else if (strcmp(retptr, "$setranging") == 0)
		{
			ptr = NULL;
			retptr = strtok(ptr, ",");
			ShowRanging = atol(retptr);
			if (ShowRanging > 0) ShowRanging = 1; else ShowRanging = 0;
		}
    }
    else //无参数指令 如$rboot
    {
        if(strcmp((char*)data, "$rboot\r\n") == 0)//重启
        {
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
            HAL_NVIC_SystemReset();//重启 
        }
        else if(strcmp((char*)data, "$rantdly\r\n") == 0)//查询天线延时参数
        {
            len = sprintf((char*)UART_COMMAND_BUF, "ant_dly = %d\r\n", ant_dly);
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
        }
        else if(strcmp((char*)data, "$reset\r\n") == 0)//恢复默认参数
        {
            len = sprintf((char*)UART_COMMAND_BUF, "$setok\r\n");
            HAL_UART_Transmit(&hUSART3, &UART_COMMAND_BUF[0], len, 1000);
            uint8_t write_zero[256]={0};
            HAL_NVIC_SystemReset();//重启 
        }
    }
}
#endif

/* 串口空闲中断回调 */
//void HAL_UART_IdleCpltCallback(UART_HandleTypeDef *huart)
//{
//    //HAL_UART_Transmit(&UART_PORT, &UART_RX_BUF[0], strlen((char*)UART_RX_BUF), 1000);
//    uart_rx_len = strlen((char*)UART_RX_BUF);
//}

