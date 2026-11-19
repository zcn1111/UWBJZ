#include "main.h"
#include "BSP_fmc_fsmc.h"
#include "BSP_MPU.h"

/******** 句柄变量 ********/
//SRAM_HandleTypeDef hSRAM1;
NAND_HandleTypeDef hNAND1;


/******************************************************************
函数名称: void Nand1Init(void)
功能描述: Nand参数初始化
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void Nand1Init(void)
{
	FMC_NAND_PCC_TimingTypeDef ComSpaceTiming = {0};
	FMC_NAND_PCC_TimingTypeDef AttSpaceTiming = {0};

	/* 初始化NAND设备类型 */
	hNAND1.Instance = FMC_Bank3_R;
	/* NAND参数初始化 */
	hNAND1.Init.NandBank = FMC_NAND_BANK3;							//片选		=BANK3
	hNAND1.Init.Waitfeature = FMC_NAND_PCC_WAIT_FEATURE_DISABLE;	//等待特性	=禁用
	hNAND1.Init.MemoryDataWidth = FMC_NAND_PCC_MEM_BUS_WIDTH_8;		//内存位宽	=8位
	hNAND1.Init.EccComputation = FMC_NAND_ECC_DISABLE;				//ECC校验	=禁用
	hNAND1.Init.ECCPageSize = FMC_NAND_ECC_PAGE_SIZE_512BYTE;		//ECC位宽	=512字节
//	hNAND1.Init.TCLRSetupTime = 10;
//	hNAND1.Init.TARSetupTime = 10;
	hNAND1.Init.TCLRSetupTime = 3;
	hNAND1.Init.TARSetupTime = 3;
	
	/* NAND配置 */
	hNAND1.Config.PageSize = 4096;		//页大小 = 4096B + 128B(Spare Area)
	hNAND1.Config.SpareAreaSize = 128;
	hNAND1.Config.BlockSize = 64;		//块大小 = 1 Page * 64 = 128KB + 4KB(Spare Area)
	hNAND1.Config.BlockNbr = 2048;		//块总数
	hNAND1.Config.PlaneNbr = 1;
	hNAND1.Config.PlaneSize = 2048;
//	hNAND1.Config.PlaneNbr = 1;
//	hNAND1.Config.PlaneSize = 4096;
	hNAND1.Config.ExtraCommandEnable = DISABLE;
	
	/* 原时序配置 */
//	/* ComSpaceTiming */
//	ComSpaceTiming.SetupTime = 10;
//	ComSpaceTiming.WaitSetupTime = 10;
//	ComSpaceTiming.HoldSetupTime = 10;
//	ComSpaceTiming.HiZSetupTime = 10;
//	
//	/* AttSpaceTiming */
//	AttSpaceTiming.SetupTime = 10;
//	AttSpaceTiming.WaitSetupTime = 10;
//	AttSpaceTiming.HoldSetupTime = 10;
//	AttSpaceTiming.HiZSetupTime = 10;

	/* 时序配置 */
	/* ComSpaceTiming */
	ComSpaceTiming.SetupTime = 3;
	ComSpaceTiming.WaitSetupTime = 5;
	ComSpaceTiming.HoldSetupTime = 3;
	ComSpaceTiming.HiZSetupTime = 3;
	
	/* AttSpaceTiming */
	AttSpaceTiming.SetupTime = 3;
	AttSpaceTiming.WaitSetupTime = 5;
	AttSpaceTiming.HoldSetupTime = 3;
	AttSpaceTiming.HiZSetupTime = 3;

	if (HAL_NAND_Init(&hNAND1, &ComSpaceTiming, &AttSpaceTiming) != HAL_OK)
	{
		Error_Handler( );
	}
	
}

/******************************************************************
函数名称: void HAL_NAND_MspInit
功能描述: NAND底层管脚初始化
参数输入: NAND_HandleTypeDef	*hnand	NAND句柄
参数输出: 无
返 回 值: 
其    他: HAL库中初始化NAND时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_NAND_MspInit(NAND_HandleTypeDef *hnand)
{
	GPIO_InitTypeDef GPIO_InitStruct={0};
    
	/* 配置FMC通用管脚 */
	__HAL_RCC_FMC_CLK_ENABLE();             //使能FMC时钟
	__HAL_RCC_GPIOC_CLK_ENABLE();			//使能GPIOC时钟
	__HAL_RCC_GPIOD_CLK_ENABLE();           //使能GPIOD时钟
	__HAL_RCC_GPIOE_CLK_ENABLE();           //使能GPIOE时钟
	__HAL_RCC_GPIOG_CLK_ENABLE();           //使能GPIOG时钟
	
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;          	//管脚模式	=复用
	GPIO_InitStruct.Pull = GPIO_NOPULL;					//管脚上拉	=不上拉
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	//管脚速度	=极高
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;			//管脚复用	=FMC
	
	//初始化PD4 NOE引脚
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	//初始化PD5 NWE引脚
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	//初始化PD11 A16_CLE引脚
	GPIO_InitStruct.Pin = GPIO_PIN_11;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	//初始化PD12 A17_ALE引脚
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
	//初始化PD14,PD15,PD0,PD1,PE7,PE8,PE9,PE10  D0~D7数据引脚
	GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
	
	if (hnand->Instance == FMC_Bank3_R)
	{
		//初始化PC6 R/B引脚
		GPIO_InitStruct.Pin = GPIO_PIN_6;		
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;				//管脚模式	=输入
		GPIO_InitStruct.Pull = GPIO_PULLUP;					//管脚上拉	=上拉          
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	//管脚速度	=极高
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
				
		//初始化PC8 NCE3引脚
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;          	//管脚模式	=复用
		GPIO_InitStruct.Pull = GPIO_NOPULL;					//管脚上拉	=不上拉
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	//管脚速度	=极高
		GPIO_InitStruct.Alternate = GPIO_AF9_FMC;			//管脚复用	=FMC
		GPIO_InitStruct.Pin = GPIO_PIN_8;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);  
	}
	
}

/******************************************************************
函数名称: void HAL_NAND_MspDeInit
功能描述: NAND底层管脚反初始化
参数输入: NAND_HandleTypeDef	*hnand	NAND句柄
参数输出: 无
返 回 值: 
其    他: HAL库中反初始化NAND时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_NAND_MspDeInit(NAND_HandleTypeDef *hnand)
{
	/* FMC关闭时钟 */
	__HAL_RCC_FMC_CLK_DISABLE();
	
	HAL_GPIO_DeInit(GPIOE, GPIO_PIN_9);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15 |\
							GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5);
	HAL_GPIO_DeInit(GPIOD, GPIO_PIN_6);
	
	/* 关闭中断 */
	HAL_NVIC_DisableIRQ(FMC_IRQn);
}

