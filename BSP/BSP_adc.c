#include "Main.h"
#include "BSP.h"

/******** 外部变量定义 ********/
ADC_HandleTypeDef	hADC1;				//ADC1句柄
ADC_HandleTypeDef   hADC2;              //ADC2句柄

ADC_ChannelConfTypeDef hADC_Ch3;		//ADC通道句柄
ADC_ChannelConfTypeDef hADC_Ch13;		//ADC通道句柄



float g_fBatVal;
float g_MainVal;
uint32_t g_tmrAdcDelay;
uint32_t g_BatVal;
uint32_t g_MainsPower;
/******** 外部函数定义 ********/

/****************************************  初始化及管脚配置函数 ***********************************************************/

/******************************************************************
函数名称: void BSP_AdcInitPara_ADC1
功能描述: 初始化ADC1参数配置
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_AdcInitPara_ADC1(ADC_HandleTypeDef *AdcHandler)
{
	/* ADC基础参数 */
	AdcHandler->Init.ClockPrescaler	= ADC_CLOCKPRESCALER_PCLK_DIV4;			//ADC时钟分频		=4分频 ADCCLK = SYSCLK / Div = 8Mhz
	AdcHandler->Init.Resolution = ADC_RESOLUTION_12B;						//ADC分辨率			=12位
	AdcHandler->Init.DataAlign = ADC_DATAALIGN_RIGHT;						//ADC数据对齐		=右对齐

	/* 扫描转换模式 */
	AdcHandler->Init.ScanConvMode = DISABLE;								//ADC扫描模式		=关闭
	AdcHandler->Init.EOCSelection = DISABLE;								//ADCEOC模式		=关闭
	
	/* 连续转换模式 */
	AdcHandler->Init.ContinuousConvMode = DISABLE;							//ADC连续转换模式	=关闭
	AdcHandler->Init.NbrOfConversion = 1;									//ADC转换序列		=1
	
	/* 间歇转换模式 */
	AdcHandler->Init.DiscontinuousConvMode = DISABLE;						//ADC间歇采样		=关闭
	AdcHandler->Init.NbrOfDiscConversion = 0;								//ADC间歇采样通道	=0
	
	AdcHandler->Init.ExternalTrigConv = ADC_SOFTWARE_START;					//ADC转换触发		=软件触发
	AdcHandler->Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;	//ADC转换触发边沿	=无
	AdcHandler->Init.LowPowerAutoWait = DISABLE;							//
	AdcHandler->Init.DMAContinuousRequests = DISABLE;
	AdcHandler->Init.Overrun = ADC_OVR_DATA_PRESERVED;
	AdcHandler->Init.OversamplingMode = DISABLE;
    AdcHandler->Init.DMAContinuousRequests = DISABLE;                       //关闭DMA请求	
}

/******************************************************************
函数名称: void BSP_AdcSetupChannelPara
功能描述: 配置ADC通道参数
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_AdcSetupChannelPara(ADC_ChannelConfTypeDef *AdcChConf, uint32_t Chnl, uint32_t Rank, uint32_t SmplTime, uint32_t InputMode, uint32_t OffsetNbr, uint32_t Offset)
{
	AdcChConf->Channel = Chnl;				//配置通道		=Chnl
	AdcChConf->Rank = Rank;					//配置转换序列	=Rank
	AdcChConf->SamplingTime = SmplTime;		//配置采样时间	=SmplTime
	AdcChConf->SingleDiff = InputMode;		//配置单端/差分	=InputMode
	AdcChConf->OffsetNumber = OffsetNbr;	//配置偏移号		=OffsetNbr
	AdcChConf->Offset = Offset;				//配置偏移		=Offset
}

/******************************************************************
函数名称: void BSP_AdcBindInstance
功能描述: 绑定ADC实例
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_AdcBindInstance(ADC_HandleTypeDef *AdcHandler, ADC_TypeDef *AdcInst)
{
	AdcHandler->Instance = AdcInst;
	if (HAL_ADC_Init(AdcHandler) != HAL_OK)
	{
		Error_Handler();
	}
	ADC_Enable(AdcHandler);
}

/******************************************************************
函数名称: void BSP_AdcLinkChannel
功能描述: 初始化ADC通道
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_AdcInitChannel(ADC_HandleTypeDef *hadc, ADC_ChannelConfTypeDef *AdcChConf)
{
	HAL_ADC_ConfigChannel(hadc, AdcChConf);						//配置ADC通道
	HAL_ADCEx_Calibration_Start(hadc, AdcChConf->SingleDiff);	//校准ADC通道
}

/******************************************************************
函数名称: void HAL_ADC_MspInit
功能描述: ADC底层管脚初始化
参数输入: ADC_HandleTypeDef	*hadc	ADC句柄
参数输出: 无
返 回 值: 
其    他: HAL库中初始化ADC时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
	__HAL_RCC_ADC_CLK_ENABLE();		//使能ADC时钟
	
	GPIO_SetupPin(BAT_ADC_PORT, BAT_ADC_PIN, GPIO_MODE_ANALOG_ADC_CONTROL, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);	
	GPIO_SetupPin(AC12_ADC_PORT, AC12_ADC_PIN, GPIO_MODE_ANALOG_ADC_CONTROL, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
}

/****************************************  采样函数 ***********************************************************/
/******************************************************************
函数名称: void ADC_GetVal
功能描述: ADC采样值
参数输入: ADC_HandleTypeDef	*hadc	ADC句柄
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
uint16_t ADC_GetVal(ADC_HandleTypeDef *hadc)
{
	uint16_t val=0;
	
	/* 启动转换 */
	HAL_ADC_Start(hadc);										//开启ADC
	HAL_ADC_PollForConversion(hadc, 10);						//轮询转换
	if (HAL_IS_BIT_SET(HAL_ADC_GetState(hadc), HAL_ADC_STATE_REG_EOC))
	{
		val = HAL_ADC_GetValue(hadc);							//返回采样结果
	}
	return val;
}

/******************************************************************
函数名称: void ADC_GetVal_Average
功能描述: ADC采样值
参数输入: ADC_HandleTypeDef	*hadc	ADC句柄
		  uint16_t			BufLen	数据队列长度
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
uint32_t ADC_GetVal_Average(ADC_HandleTypeDef *hadc,uint16_t BufLen)
{
	uint32_t val=0;
	uint16_t i;
	uint32_t sum=0;
	uint32_t avg=0;
	uint32_t vaildnum=0;
	uint32_t pos_max, pos_min;	//最大最小值位置
	uint32_t val_max, val_min;	//最大最小值		
	uint32_t ValBuf[10];
	
	/* 启动转换 */
	memset(ValBuf, 0x00, 10);
	
	for(i= 0; i<BufLen; i++ )
	{
		HAL_ADC_Start(hadc);										//开启ADC
		HAL_ADC_PollForConversion(hadc, 10);						//轮询转换
		if (HAL_IS_BIT_SET(HAL_ADC_GetState(hadc), HAL_ADC_STATE_REG_EOC))
		{
			val = HAL_ADC_GetValue(hadc);							//返回采样结果
			ValBuf[i] = val;
		}	
	}
	/* 计算平均值 */
	pos_max = 0;
	pos_min = 0;
	val_max = 0;
	val_min = 0xFFFFFFFF;
	for (i=0; i<BufLen; i++)
	{
		if (ValBuf[i] != 0)
		{
			if (ValBuf[i] > val_max)
			{
				val_max = ValBuf[i];
				pos_max = i;
			}
			else if (ValBuf[i] <= val_min)
			{
				val_min = ValBuf[i];
				pos_min = i;
			}
			vaildnum++;
			sum += ValBuf[i];
		}
	}
	
	if (vaildnum > 2)
	{
		sum = sum - ValBuf[pos_max] - ValBuf[pos_min];
		vaildnum -= 2;
		avg = sum / vaildnum;
	}	
	else
		avg = sum / vaildnum;
	/* 计算平均值 */
	return avg;
}


/******************************************************************
函数名称: void ADC_Init
功能描述: ADC初始化
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ADC_Init(void)
{
	g_tmrAdcDelay = uwTick;
}

/******************************************************************
函数名称: void ADC_Acq
功能描述: ADC采集
参数输入: 
参数输出: 
返 回 值: 无
其    他: 无
******************************************************************/
void ADC_Acq(void)
{
	g_BatVal = ADC_GetVal_Average(&hADC1,10);
	g_fBatVal = (float)g_BatVal /4096 * 3.3 * 4.117;

	g_MainsPower = ADC_GetVal_Average(&hADC2,10);
	g_MainVal = (float)g_MainsPower /4096 * 3.3 * 5.70588;			
}



/****************************************  中断回调函数 ***********************************************************/




