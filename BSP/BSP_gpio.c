#include "main.h"
#include "BSP.h"

/******** 外部变量定义 ********/
//EXTI_HandleTypeDef hEXTI1;
uint8_t g_CheckTool=1;	//1-检测到客服工具
/******************************************************************
函数名称: void GPIO_SetupPin
功能描述: 配置管脚状态
参数输入: GPIO_TypeDef	*GPIOx	Port
		  uint32_t		Pin		Pin
		  uint32_t		Mode	管脚模式
		  uint32_t		Pull	上下拉状态
		  uint32_t		Speed	管脚速度
		  uint32_t		Alter	管脚复用功能
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void GPIO_SetupPin(GPIO_TypeDef *GPIOx, uint32_t Pin, uint32_t Mode, uint32_t Pull, uint32_t Speed, uint32_t Alter)
{
	GPIO_InitTypeDef GPIO_InitStruct={0};
	
	/* 启用时钟 */
	if (GPIOx == GPIOA)
		__HAL_RCC_GPIOA_CLK_ENABLE();
	#ifdef GPIOB
	else if (GPIOx == GPIOB)
		__HAL_RCC_GPIOB_CLK_ENABLE();
	#endif
	#ifdef GPIOC
	else if (GPIOx == GPIOC)
		__HAL_RCC_GPIOC_CLK_ENABLE();
	#endif
	#ifdef GPIOD
	else if (GPIOx == GPIOD)
		__HAL_RCC_GPIOD_CLK_ENABLE();
	#endif
	#ifdef GPIOE
	else if (GPIOx == GPIOE)
		__HAL_RCC_GPIOE_CLK_ENABLE();
	#endif
	#ifdef GPIOF
	else if (GPIOx == GPIOF)
		__HAL_RCC_GPIOF_CLK_ENABLE();
	#endif
	#ifdef GPIOG
	else if (GPIOx == GPIOG)
		__HAL_RCC_GPIOG_CLK_ENABLE();
	#endif
	#ifdef GPIOH
	else if (GPIOx == GPIOH)
		__HAL_RCC_GPIOH_CLK_ENABLE();
	#endif
	#ifdef GPIOI
	else if (GPIOx == GPIOI)
		__HAL_RCC_GPIOI_CLK_ENABLE();
	#endif
	#ifdef GPIOJ
	else if (GPIOx == GPIOJ)
		__HAL_RCC_GPIOJ_CLK_ENABLE();
	#endif
	#ifdef GPIOK
	else if (GPIOx == GPIOK)
		__HAL_RCC_GPIOK_CLK_ENABLE();
	#endif
	
	/* 配置管脚 */
	GPIO_InitStruct.Pin = Pin;
	GPIO_InitStruct.Mode = Mode;
	GPIO_InitStruct.Pull = Pull;
	GPIO_InitStruct.Speed = Speed;
	GPIO_InitStruct.Alternate = Alter;
	
	HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/******************************************************************
函数名称: void GPIO_SetupExInterrupt(void)
功能描述: 配置GPIO外部中断
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void GPIO_SetupExInterrupt(EXTI_HandleTypeDef *hexti, GPIO_TypeDef *GPIOx, uint32_t Pin, uint32_t Mode, uint32_t Trigger)
{
	EXTI_ConfigTypeDef EXTI_InitStruct;
	uint32_t gpiosel;
	uint32_t line;
	
	/* 配置GPIO Port */
	#ifdef GPIOA
	if (GPIOx == GPIOA) gpiosel = EXTI_GPIOA;
	#endif
	#ifdef GPIOB
	else if (GPIOx == GPIOB) gpiosel = EXTI_GPIOB;
	#endif
	#ifdef GPIOC
	else if (GPIOx == GPIOC) gpiosel = EXTI_GPIOC;
	#endif
	#ifdef GPIOD
	else if (GPIOx == GPIOD) gpiosel = EXTI_GPIOD;
	#endif
	#ifdef GPIOE
	else if (GPIOx == GPIOE) gpiosel = EXTI_GPIOE;
	#endif
	#ifdef GPIOF
	else if (GPIOx == GPIOF) gpiosel = EXTI_GPIOF;
	#endif
	#ifdef GPIOG
	else if (GPIOx == GPIOG) gpiosel = EXTI_GPIOG;
	#endif
	#ifdef GPIOH
	else if (GPIOx == GPIOH) gpiosel = EXTI_GPIOH;
	#endif
	#ifdef GPIOI
	else if (GPIOx == GPIOI) gpiosel = EXTI_GPIOI;
	#endif
	#ifdef GPIOJ
	else if (GPIOx == GPIOJ) gpiosel = EXTI_GPIOJ;
	#endif
	#ifdef GPIOK
	else if (GPIOx == GPIOK) gpiosel = EXTI_GPIOK;
	#endif
	
	/* 配置Line */
	if (Pin == GPIO_PIN_0) line = EXTI_LINE_0;
	else if (Pin == GPIO_PIN_1) line = EXTI_LINE_1;
	else if (Pin == GPIO_PIN_2) line = EXTI_LINE_2;
	else if (Pin == GPIO_PIN_3) line = EXTI_LINE_3;
	else if (Pin == GPIO_PIN_4) line = EXTI_LINE_4;
	else if (Pin == GPIO_PIN_5) line = EXTI_LINE_5;
	else if (Pin == GPIO_PIN_6) line = EXTI_LINE_6;
	else if (Pin == GPIO_PIN_7) line = EXTI_LINE_7;
	else if (Pin == GPIO_PIN_8) line = EXTI_LINE_8;
	else if (Pin == GPIO_PIN_9) line = EXTI_LINE_9;
	else if (Pin == GPIO_PIN_10) line = EXTI_LINE_10;
	else if (Pin == GPIO_PIN_11) line = EXTI_LINE_11;
	else if (Pin == GPIO_PIN_12) line = EXTI_LINE_12;
	else if (Pin == GPIO_PIN_13) line = EXTI_LINE_13;
	else if (Pin == GPIO_PIN_14) line = EXTI_LINE_14;
	else if (Pin == GPIO_PIN_15) line = EXTI_LINE_15;
	
	/* 配置中断 */
	EXTI_InitStruct.Line = line;
	EXTI_InitStruct.GPIOSel = gpiosel;
	EXTI_InitStruct.Mode = Mode;
	EXTI_InitStruct.Trigger = Trigger;
	HAL_EXTI_SetConfigLine(hexti, &EXTI_InitStruct);		//配置外部中断
}

/******************************************************************
函数名称: void GpioInit(void)
功能描述: Gpio初始化
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_GpioInit(void)
{
	/* DTU电源控制 */
	GPIO_SetupPin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN, GPIO_SPEED_FREQ_LOW, 0);
	HAL_GPIO_WritePin(GSM_PWR_PORT, GSM_PWR_PIN, GPIO_PIN_RESET);	//初始化关闭GSM
	
	/* 系统3.3V电源控制 */
	GPIO_SetupPin(SYS_PWR_PORT, SYS_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	HAL_GPIO_WritePin(SYS_PWR_PORT, SYS_PWR_PIN, GPIO_PIN_SET);		//客服工具RS485和语音供电
//	GPIO_SetupPin(KILL_OFF_PORT, KILL_OFF_PIN, GPIO_MODE_OUTPUT_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM, 0);

	GPIO_SetupPin(Flash_PWR_PORT, Flash_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	HAL_GPIO_WritePin(Flash_PWR_PORT, Flash_PWR_PIN, GPIO_PIN_SET);		//EE(3mA)+FLASH(25mA)+DW1000(30mA)+RTC(1uA)
	
	GPIO_SetupPin(LY_PWR_PORT, LY_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	HAL_GPIO_WritePin(LY_PWR_PORT, LY_PWR_PIN, GPIO_PIN_SET);		//EE(3mA)+FLASH(25mA)+DW1000(30mA)+RTC(1uA)	
	/* RTK管脚控制 */
	/* RTK1电源控制管脚 高有效
	   RTK2电源控制管脚 低有效
	   RTK1备用电源控制管脚 高有效
	   RTK2备用电源控制管脚 低有效
	   复位管脚 高复位
	*/
	GPIO_SetupPin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);			//电源控制
	HAL_GPIO_WritePin(RTK_PWR_PORT, RTK_PWR_PIN, GPIO_PIN_RESET);												//初始化关闭RTK电源
	GPIO_SetupPin(RTK_RESET_PORT, RTK_RESET_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLDOWN, GPIO_SPEED_FREQ_LOW, 0);	//RTK复位
	HAL_GPIO_WritePin(RTK_RESET_PORT, RTK_RESET_PIN, GPIO_PIN_SET);
	GPIO_SetupPin(RTK_PPS_PORT, RTK_PPS_PIN, GPIO_MODE_IT_RISING, GPIO_NOPULL, GPIO_SPEED_FREQ_LOW, 0);			//RTK秒脉冲
	
	/* LoRa管脚控制 */
	GPIO_SetupPin(LORA_PWR_PORT, LORA_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);
	GPIO_SetupPin(LORA_RESET_PORT, LORA_RESET_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);
	GPIO_SetupPin(LORA_RXEN_PORT, LORA_RXEN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);
	GPIO_SetupPin(LORA_BUSY_PORT, LORA_BUSY_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);
	GPIO_SetupPin(LORA_TXEN_PORT, LORA_TXEN_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);
	
	HAL_GPIO_WritePin(LORA_PWR_PORT, LORA_PWR_PIN, GPIO_PIN_RESET);													//初始化关闭RoLa
	
	/* 蓝牙管脚配置 */
	GPIO_SetupPin(BT_MODESW_PORT, BT_MODESW_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);		//蓝牙工作模式配置
	GPIO_SetupPin(BT_HOLDWAKE_PORT, BT_HOLDWAKE_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);	//蓝牙唤醒
	GPIO_SetupPin(BT_LINK_PORT, BT_LINK_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);				//蓝牙连接状态
	
	/* LED配置 */
	GPIO_SetupPin(LORA_LED_PORT, LORA_LED_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, 0);
	GPIO_SetupPin(RTK_LED_PORT, RTK_LED_PIN, GPIO_MODE_OUTPUT_PP, GPIO_PULLUP, GPIO_SPEED_FREQ_HIGH, 0);
	HAL_GPIO_WritePin(RTK_LED_PORT, RTK_LED_PIN, GPIO_PIN_SET);
	/* IMU管脚配置 */
//	GPIO_SetupPin(IMUINT1_PORT, IMUINT1_PIN, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);			//IMU中断1 下降沿中断
//	GPIO_SetupPin(IMUINT2_PORT, IMUINT2_PIN, GPIO_MODE_IT_FALLING, GPIO_NOPULL, GPIO_SPEED_FREQ_HIGH, 0);			//IMU中断2 下降沿中断
	
	/* 语音管脚配置 */
	GPIO_SetupPin(VOICE_PWR_PORT, VOICE_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);	//语音电源控制 低有效
//	GPIO_SetupPin(VOICE_CLK_PORT, VOICE_CLK_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);	//语音CLK 双线串口 不能上下拉
//	GPIO_SetupPin(VOICE_SDA_PORT, VOICE_SDA_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);	//语音SDA 双线串口 不能上下拉
	GPIO_SetupPin(VOICE_MOSI_PORT, VOICE_MOSI_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);	
	GPIO_SetupPin(VOICE_MISO_PORT, VOICE_MISO_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	GPIO_SetupPin(VOICE_CLK_PORT, VOICE_CLK_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	GPIO_SetupPin(VOICE_NSS_PORT, VOICE_NSS_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	HAL_GPIO_WritePin(VOICE_NSS_PORT, VOICE_NSS_PIN,GPIO_PIN_SET);
	HAL_GPIO_WritePin(VOICE_MISO_PORT, VOICE_MISO_PIN,GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VOICE_MOSI_PORT, VOICE_MOSI_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(VOICE_CLK_PORT, VOICE_CLK_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(VOICE_PWR_PORT, VOICE_PWR_PIN, GPIO_PIN_RESET);	//开启语音电源 低有效
	/* OLED管脚配置 */
	GPIO_SetupPin(OLED_PWR_PORT, OLED_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);		//OLED电源控制 低有效
	GPIO_SetupPin(OLED_RES_PORT, OLED_RES_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);		//OLED复位
	HAL_GPIO_WritePin(OLED_RES_PORT, OLED_RES_PIN, GPIO_PIN_RESET);
	GPIO_SetupPin(OLED_DC_PORT, OLED_DC_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);		//OLED数据/命令切换 0=命令;1=数据
	
	/* 按键控制 */
	GPIO_SetupPin(KEY1_UP_PORT, KEY1_UP_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM, 0);
	GPIO_SetupPin(KEY2_DOWN_PORT, KEY2_DOWN_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM, 0);
	GPIO_SetupPin(KEY3_OK_PORT, KEY3_OK_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM, 0);
	/*客服工具电源控制*/
	GPIO_SetupPin(TOOL_PWR_PORT, TOOL_PWR_PIN, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, 0);
	GPIO_SetupPin(TOOL_INDET_PORT, TOOL_INDET_PIN, GPIO_MODE_INPUT, GPIO_NOPULL, GPIO_SPEED_FREQ_MEDIUM, 0);
	g_CheckTool = HAL_GPIO_ReadPin(TOOL_INDET_PORT,TOOL_INDET_PIN);      /*=1客服工具未插入    =0客服工具插入*/
}



/*
*********************************************************************************************************
*	函 数 名: InitCelluarGpio
*	功能说明: 
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitCelluarGpio(void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;	
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	/* 配置引脚为推挽输出 */
	GPIO_Initure.Pin = GSM_RESET_PIN;
	GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;			         
	GPIO_Initure.Pull = GPIO_NOPULL;				              
	GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;		        
	HAL_GPIO_Init(GSM_RESET_PORT, &GPIO_Initure);	
	HAL_GPIO_WritePin(GSM_RESET_PORT, GSM_RESET_PIN, GPIO_PIN_RESET);		
}


void DeInitCelluarGpio(void)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;	
	__HAL_RCC_GPIOC_CLK_ENABLE();
	
	/* 配置引脚为推挽输出 */
	GPIO_Initure.Pin = GSM_RESET_PIN;
	GPIO_Initure.Mode = GPIO_MODE_ANALOG;			
	GPIO_Initure.Pull      = GPIO_NOPULL;
	HAL_GPIO_Init(GSM_RESET_PORT, &GPIO_Initure);	
}



/****************************************  中断回调函数 ***********************************************************/

/******************************************************************
函数名称: void HAL_GPIO_EXTI_Callback
功能描述: GPIO外部中断回调
参数输入: uint16_t	GPIO_Pin	GPIO管脚
参数输出: 无
返 回 值: 
其    他: HAL库中产生GPIO外部中断时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
//void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
//{
//	HAL_StatusTypeDef hret;
//	
//	/* LORA中断处理 */
//	if (GPIO_Pin == LORA_DIO1_PIN)
//	{
//		
//	}
//}

