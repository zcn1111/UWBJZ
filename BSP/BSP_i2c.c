#include "main.h"
#include "BSP_i2c.h"

/******** 句柄变量 ********/
I2C_HandleTypeDef	hI2C1;
I2C_HandleTypeDef	hI2C2;
I2C_HandleTypeDef	hI2C3;


/******************************************************************
函数名称: void I2c1Init
功能描述: I2C传输配置
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void I2c1Init(void)
{
	hI2C1.Instance = I2C1;									//I2C1
	hI2C1.Init.Timing = 0x10D0EAEF;							//时序配置
	hI2C1.Init.OwnAddress1 = 0;								//本机地址	=0x00
	hI2C1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;	//地址模式	=7位
	hI2C1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;	//双地址		=关闭
	hI2C1.Init.OwnAddress2 = 0;								//本机地址2	=0x00
	hI2C1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;	//广播		=关闭
	hI2C1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;		//时钟拉伸	=关闭
	if (HAL_I2C_Init(&hI2C1) != HAL_OK)
	{
		Error_Handler();
	}
}
void I2C2_Init(void)
{
//  hI2C2.Instance = I2C2;
//  hI2C2.Init.Timing = 0x00707CBB;  // 标准模式100kHz的时序值
//  hI2C2.Init.OwnAddress1 = 0;
//  hI2C2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
//  hI2C2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
//  hI2C2.Init.OwnAddress2 = 0;
//  hI2C2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
//  hI2C2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
//  hI2C2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
//  
//  if (HAL_I2C_Init(&hI2C2) != HAL_OK)
//  {
//    Error_Handler();
//  }

//  /* 配置I2C2的时序 */
//  if (HAL_I2CEx_ConfigAnalogFilter(&hI2C2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
//  {
//    Error_Handler();
//  }
//  
//  if (HAL_I2CEx_ConfigDigitalFilter(&hI2C2, 0) != HAL_OK)
//  {
//    Error_Handler();
//  }
}
void I2C3_Init(void)
{
  hI2C3.Instance = I2C3;
  hI2C3.Init.Timing = 0x00707CBB;  // 标准模式100kHz的时序值
  hI2C3.Init.OwnAddress1 = 0;
  hI2C3.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hI2C3.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hI2C3.Init.OwnAddress2 = 0;
  hI2C3.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hI2C3.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hI2C3.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  
  if (HAL_I2C_Init(&hI2C3) != HAL_OK)
  {
    Error_Handler();
  }
  /* 配置I2C3的时序 */
  if (HAL_I2CEx_ConfigAnalogFilter(&hI2C3, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_I2CEx_ConfigDigitalFilter(&hI2C3, 0) != HAL_OK)
  {
    Error_Handler();
  }
  
}
void MX_GPIO_Init(void)
{
//  GPIO_InitTypeDef GPIO_InitStruct = {0};

//  // 使能GPIOB时钟
//  __HAL_RCC_GPIOB_CLK_ENABLE();

//  // 配置I2C2引脚: PB10 (SCL), PB11 (SDA)
//  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_14;
//  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;        // 开漏输出
//  GPIO_InitStruct.Pull = GPIO_PULLUP;            // 上拉
//  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
//  GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;     // I2C2复用功能
//  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}
/******************************************************************
函数名称: void BSP_I2cSetupTiming
功能描述: 配置I2C时序参数
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
//void BSP_I2cSetupTiming(I2C_HandleTypeDef *I2cHandler, uint32_t SrcClkFreq, uint32_t I2cClkFreq,  

/******************************************************************
函数名称: void BSP_I2cSetupPara
功能描述: 配置I2C参数
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_I2cSetupPara(I2C_HandleTypeDef *I2cHandler, uint8_t Addr, uint32_t AddrMode, uint32_t GenCallMode, uint32_t NoStretchMode)
{
	I2cHandler->Init.OwnAddress1 = (uint32_t)Addr;		//本机地址		=Addr
	I2cHandler->Init.AddressingMode = AddrMode;			//地址模式		=AddrMode
	I2cHandler->Init.GeneralCallMode = GenCallMode;		//广播模式		=GenCallMode
	I2cHandler->Init.NoStretchMode = NoStretchMode;		//禁止时钟拉伸	=NoStretch
	
	/* 双地址模式 */
	I2cHandler->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;	//关闭双地址
	I2cHandler->Init.OwnAddress2 = 0;
}

/******************************************************************
函数名称: void BSP_I2cBindInstance
功能描述: 绑定I2C实例
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 无
******************************************************************/
void BSP_I2cBindInstance(I2C_HandleTypeDef *I2cHandler, I2C_TypeDef *Inst)
{
	I2cHandler->Instance = Inst;	//绑定I2C实例
	I2cHandler->Init.Timing = 0x30420F13;		//100K @ 16Mhz
	if (HAL_I2C_Init(I2cHandler) != HAL_OK)
	{
		Error_Handler();
	}
}

/******************************************************************
函数名称: void HAL_I2C_MspInit
功能描述: 串口底层管脚初始化
参数输入: I2C_HandleTypeDef	*hi2c	串口句柄
参数输出: 无
返 回 值: 
其    他: HAL库中初始化I2C时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{
	if(hi2c->Instance==I2C1)
	{
		__HAL_RCC_I2C1_CLK_ENABLE();
		/**I2C1 GPIO Configuration
		PB6     ------> I2C1_SCL
		PB7     ------> I2C1_SDA
		*/
		
		GPIO_SetupPin(GPIOB, GPIO_PIN_9, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C1);
		GPIO_SetupPin(GPIOB, GPIO_PIN_8, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C1);
	}
	else if (hi2c->Instance == I2C2)
	{
		__HAL_RCC_I2C2_CLK_ENABLE();
		/**I2C2 GPIO Configuration
		PB13     ------> I2C2_SCL
		PB14     ------> I2C2_SDA
		*/
//	GPIO_SetupPin(GPIOB, GPIO_PIN_13, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C2);
//	GPIO_SetupPin(GPIOB, GPIO_PIN_14, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C2);
	}
	else if (hi2c->Instance == I2C3)
	{
		__HAL_RCC_I2C3_CLK_ENABLE();
		/**I2C3 GPIO Configuration
		PA8     ------> I2C2_SCL
		PC9     ------> I2C2_SDA
		*/
		GPIO_SetupPin(GPIOC, GPIO_PIN_1, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C3);
		GPIO_SetupPin(GPIOC, GPIO_PIN_0, GPIO_MODE_AF_OD, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF4_I2C3);
	}

}

/**
* @brief I2C MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param hi2c: I2C handle pointer
* @retval None
*/
void HAL_I2C_MspDeInit(I2C_HandleTypeDef* hi2c)
{
  if(hi2c->Instance==I2C1)
  {
  /* USER CODE BEGIN I2C1_MspDeInit 0 */

  /* USER CODE END I2C1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_I2C1_CLK_DISABLE();

    /**I2C1 GPIO Configuration
    PB8     ------> I2C1_SCL
    PB9     ------> I2C1_SDA
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_9);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_8);

  /* USER CODE BEGIN I2C1_MspDeInit 1 */

  /* USER CODE END I2C1_MspDeInit 1 */
  }

}

