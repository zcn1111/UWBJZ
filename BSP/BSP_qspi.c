#include "main.h"
#include "BSP.h"

/******** QSPI句柄变量 ********/
QSPI_HandleTypeDef 	hQSPI;		//QSPI句柄
QSPI_CommandTypeDef	hQSPICmd;	//QSPI命令控制句柄

/******************************************************************
函数名称: void BSP_SetupQspiPara(void)
功能描述: 配置QSPI参数
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_SetupQspiPara(QSPI_HandleTypeDef *QspiHandle, uint32_t ClkPre, uint32_t ClkMode, uint32_t FlashSize, uint32_t FlashID)
{
	QspiHandle->Init.ClockPrescaler = ClkPre;			//QSPI分频 	=ClkPre QSPI时钟= AHB / (ClkPre + 1) = 200/2 = 100MHz
	QspiHandle->Init.ClockMode = ClkMode;				//时钟模式 	=ClkMode
	QspiHandle->Init.FlashSize = FlashSize;				//FLASH大小	=FlashSize
	QspiHandle->Init.FifoThreshold = 4;					//FIFO阈值 	=4字节
	
	QspiHandle->Init.SampleShifting = QSPI_SAMPLE_SHIFTING_HALFCYCLE;	//采样位移	=半个周期
	QspiHandle->Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_5_CYCLE;	//片选高电平	=5个时钟
}

/******************************************************************
函数名称: void BSP_QspiBindInstance
功能描述: 绑定QSPI实例
参数输入: 无
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void BSP_QspiBindInstance(QSPI_HandleTypeDef *QspiHandle, QUADSPI_TypeDef *Inst)
{
	QspiHandle->Instance = Inst;
	if (HAL_QSPI_Init(QspiHandle) != HAL_OK)
	{
		Error_Handler();
	}
}


/******************************************************************
函数名称: void HAL_QSPI_MspInit
功能描述: QSPI底层管脚初始化
参数输入: QSPI_HandleTypeDef	*huart	串口句柄
参数输出: 无
返 回 值: 
其    他: HAL库中初始化串口时需要调用该函数，原函数为弱函数，函数名不能变，不在.h中声明
******************************************************************/
void HAL_QSPI_MspInit(QSPI_HandleTypeDef *hqspi)
{
	GPIO_InitTypeDef GPIO_InitStruct={0};
	
	__HAL_RCC_QSPI_CLK_ENABLE();		//使能QSPI时钟
	
	GPIO_SetupPin(GPIOE, GPIO_PIN_11, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF10_QUADSPI);	//QSPI_BK1_NCS
	
	GPIO_SetupPin(GPIOE, GPIO_PIN_10, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF10_QUADSPI);	//QSPI_BK1_CLK
	GPIO_SetupPin(GPIOE, GPIO_PIN_12, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF10_QUADSPI);	//QSPI_BK1_IO0
	GPIO_SetupPin(GPIOE, GPIO_PIN_13, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF10_QUADSPI);	//QSPI_BK1_IO1
	GPIO_SetupPin(GPIOE, GPIO_PIN_14, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF10_QUADSPI);	//QSPI_BK1_IO2
	GPIO_SetupPin(GPIOE, GPIO_PIN_15, GPIO_MODE_AF_PP, GPIO_NOPULL, GPIO_SPEED_FREQ_VERY_HIGH, GPIO_AF10_QUADSPI);	//QSPI_BK1_IO3
}

/******************************************************************
函数名称: void QSPI_Receive
功能描述: QSPI接收数据
参数输入: uint8_t	*Buf	接收缓存
		  uint32_t	Len		接收数据长度
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void QSPI_Receive(uint8_t *Buf, uint32_t Len)
{
	hQSPI.Instance->DLR = Len - 1;			//配置数据长度
	HAL_QSPI_Receive(&hQSPI, Buf, 5000);	//接收数据
}


/******************************************************************
函数名称: void QSPI_Receive
功能描述: QSPI发送数据
参数输入: uint8_t	*Buf	发送缓存
		  uint32_t	Len		发送数据长度
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void QSPI_Transmit(uint8_t *Buf, uint32_t Len)
{
	hQSPI.Instance->DLR = Len - 1;			//配置数据长度
	HAL_QSPI_Transmit(&hQSPI, Buf, 5000);	//发送数据
}

/******************************************************************
函数名称: void QSPI_SetupCmdHandle
功能描述: QSPI配置命令句柄
参数输入: uint32_t	InstMode	指令模式
		  uint32_t	AddrMode	地址模式
		  uint32_t	AddrSize	地址长度
		  uint32_t	DataMode	数据模式
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void QSPI_SetupCmdHandle(uint32_t InstMode, uint32_t AddrMode, uint32_t AddrSize, uint32_t DataMode)
{
	hQSPICmd.InstructionMode = InstMode;	//指令模式
	hQSPICmd.AddressMode = AddrMode;		//地址模式
	hQSPICmd.AddressSize = AddrSize;		//地址长度
	hQSPICmd.DataMode = DataMode;			//数据模式
	
	hQSPICmd.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;			//每次都发送指令
	hQSPICmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;	//交替字节	=无
	hQSPICmd.DdrMode = QSPI_DDR_MODE_DISABLE;				//DDR模式	=禁用
	hQSPICmd.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;	//
}

/******************************************************************
函数名称: void QSPI_SendCmd
功能描述: QSPI发送命令
参数输入: uint32_t	Inst		指令
		  uint32_t	Addr		发送地址
		  uint32_t	DummyCyc	哑元字节
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void QSPI_SendCmd(uint32_t Inst, uint32_t Addr, uint32_t DummyCyc)
{
	hQSPICmd.Instruction = Inst;
	hQSPICmd.Address = Addr;				//地址
	hQSPICmd.DummyCycles = DummyCyc;		//空指令周期数
	
	HAL_QSPI_Command(&hQSPI, &hQSPICmd, 5000);
}

/******************************************************************
函数名称: void QSPI_DirectCmd
功能描述: QSPI直接发送命令
参数输入: uint32_t	Inst		指令
		  uint32_t	Addr		发送地址
		  uint32_t	DummyCyc	哑元字节
		  uint32_t	InstMode	指令模式
		  uint32_t	AddrMode	地址模式
		  uint32_t	AddrSize	地址长度
		  uint32_t	DataMode	数据模式
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void QSPI_DirectCmd(uint32_t Instruction, uint32_t Address, uint32_t DummyCycles, uint32_t InstructionMode, uint32_t AddressMode, uint32_t AddressSize, uint32_t DataMode)
{
    QSPI_CommandTypeDef Cmdhandler;

    Cmdhandler.Instruction = Instruction;                 		//指令
    Cmdhandler.Address = Address;                            	//地址
    Cmdhandler.DummyCycles = DummyCycles;                   	//设置空指令周期数
    Cmdhandler.InstructionMode = InstructionMode;				//指令模式
    Cmdhandler.AddressMode = AddressMode;   					//地址模式
    Cmdhandler.AddressSize = AddressSize;   					//地址长度
    Cmdhandler.DataMode = DataMode;             				//数据模式
    Cmdhandler.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;       		//每次都发送指令
    Cmdhandler.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 	//无交替字节
    Cmdhandler.DdrMode = QSPI_DDR_MODE_DISABLE;					//关闭DDR模式
    Cmdhandler.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	
    HAL_QSPI_Command(&hQSPI, &Cmdhandler, 5000);
}

void QSPI_Send_CMD(uint32_t Instruction, uint32_t Address, uint32_t DummyCycles, uint32_t InstructionMode, uint32_t AddressMode, uint32_t AddressSize, uint32_t DataMode)
{
    QSPI_CommandTypeDef Cmdhandler;

    Cmdhandler.Instruction = Instruction;                 		//指令
    Cmdhandler.Address = Address;                            	//地址
    Cmdhandler.DummyCycles = DummyCycles;                   	//设置空指令周期数
    Cmdhandler.InstructionMode = InstructionMode;				//指令模式
    Cmdhandler.AddressMode = AddressMode;   					//地址模式
    Cmdhandler.AddressSize = AddressSize;   					//地址长度
    Cmdhandler.DataMode = DataMode;             				//数据模式
    Cmdhandler.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;       		//每次都发送指令
    Cmdhandler.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE; 	//无交替字节
    Cmdhandler.DdrMode = QSPI_DDR_MODE_DISABLE;					//关闭DDR模式
    Cmdhandler.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	
    HAL_QSPI_Command(&hQSPI, &Cmdhandler, 5000);
}

/******************************************************************
函数名称: void QSPI_CSEnable
功能描述: QSPI片选使能
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void QSPI_CSEnable(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
}

/******************************************************************
函数名称: void QSPI_CSDisable
功能描述: QSPI片选禁能
参数输入: 
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void QSPI_CSDisable(void)
{
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
}


