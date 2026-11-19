/******************************************************************
版权所有:Copyright (C), 2014, 山西世恒技术有限公司
项目名称: III型雨量
文件名称: BSP_softi2c.c
作    者: GX
版    本: // 版本
完成日期: // 完成日期
功能描述: 软件模拟I2C总线驱动
其他说明: 
功能列表: 
SCL时钟信号流程：
	1.先拉低半个周期
	2.SDA数据在时钟低（时间中间）的时候修改（开始和停止除外）
	3.拉高半个周期
	4.SDA数据在时钟高（时间中间）的时候读取
	5.起始或停止在时钟高的时候发送上升沿或下降沿
历史记录: // 修改历史记录列表，每条修改记录应包括修改日期、修改
          // 作者及修改内容简述

1. 日期:
作者:
修改内容:
2. ..
******************************************************************/

#include "BSP.h"
//#include "delay.h"
/* 函数声明 */


SoftI2cPar_t Softi2c1;
/******************************************************************
函数名称: void SI2C_Init
功能描述: 软件模拟I2C初始化
参数输入: 无
参数输出: SoftI2cPar_t	*Par		模拟I2C参数
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_Init(SoftI2cPar_t *Par)
{
	
	GPIO_InitTypeDef	GPIO_InitStruct ={0};
	SoftI2cPar_t		init;
	
	/* 配置SDA SCL管脚 */
	__HAL_RCC_GPIOH_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;			//配置管脚开漏模式
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;	//极高
	HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
	
	/* 配置管脚 */
	init.SCLPort = GPIOH;
	init.SCLPin = GPIO_PIN_4;
	init.SDAPort = GPIOH;
	init.SDAPin = GPIO_PIN_5;
	
	/* 配置时间参数 */
	init.SCLHighHoldDelayUs = 12;	//SCL高电平保持时间
	init.SCLLowHoldDelayUs = 12;	//SCL低电平保持时间
	init.SDASampleDelayUs = 6;		//SDA采样时间
	init.SDAChangeDelayUs = 6;		//SDA切换时间
	
	/* 配置延迟函数 */
	/* 延迟1us函数，需要自己实现 */
	init.cbDelayUs = Delay1us;
	
	*Par = init;
}


/******************************************************************
函数名称: void SI2C_Start
功能描述: 模拟I2C发送起始
参数输入: SoftI2cPar_t	*Par		模拟I2C参数
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_Start(SoftI2cPar_t *Par)
{
	uint16_t rhigh;
	
	rhigh = Par->SCLHighHoldDelayUs - Par->SDASampleDelayUs;
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_RESET);	//SCL低半个周期
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);	//SDA拉高
	Par->cbDelayUs(Par->SCLLowHoldDelayUs);
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_SET);	//SCL高半个周期
	Par->cbDelayUs(Par->SDASampleDelayUs);
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_RESET);	//SDA下降沿
	Par->cbDelayUs(rhigh);
}

/******************************************************************
函数名称: void SI2C_Stop
功能描述: 模拟I2C发送停止
参数输入: SoftI2cPar_t	*Par		模拟I2C参数
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_Stop(SoftI2cPar_t *Par)
{
	uint16_t rhigh;
	
	rhigh = Par->SCLHighHoldDelayUs - Par->SDASampleDelayUs;
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_RESET);	//SCL低半个周期
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_RESET);	//SDA拉低
	Par->cbDelayUs(Par->SCLLowHoldDelayUs);
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_SET);	//SCL高半个周期
	Par->cbDelayUs(Par->SDASampleDelayUs);
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);	//SDA上升沿
	Par->cbDelayUs(rhigh);
}

/******************************************************************
函数名称: void SI2C_WriteBit
功能描述: 模拟I2C写入一位
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
		  uint8			SBit	发送数据
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_WriteBit(SoftI2cPar_t *Par, uint8_t SBit)
{
	uint16_t rlow;
	
	rlow = Par->SCLLowHoldDelayUs - Par->SDAChangeDelayUs;
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_RESET);	//SCL低半个周期
	Par->cbDelayUs(Par->SDAChangeDelayUs);
	if (SBit == 0)
		HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_RESET);//发送低位
	else
		HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);	//发送高位
	Par->cbDelayUs(rlow);											//时钟低延时
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_SET);		//SCL高半个周期
	Par->cbDelayUs(Par->SCLHighHoldDelayUs);
}

/******************************************************************
函数名称: void SI2C_ReadBit
功能描述: 模拟I2C读取一位
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
		  uint8			*RBit	读取数据
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_ReadBit(SoftI2cPar_t *Par, uint8_t *RBit)
{
	uint16_t rhigh;
	uint8_t read;
	
	rhigh = Par->SCLHighHoldDelayUs - Par->SDASampleDelayUs;
	read = 1;
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_RESET);	//SCL低半个周期
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);		//复位SDA
	Par->cbDelayUs(Par->SCLLowHoldDelayUs);
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_SET);		//SCL高半个周期
	Par->cbDelayUs(Par->SDASampleDelayUs);
	read = HAL_GPIO_ReadPin(Par->SDAPort, Par->SDAPin) & 0x01;		//读SDA管脚
	Par->cbDelayUs(rhigh);
	
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);		//SDA复位
	*RBit = read;
}

/******************************************************************
函数名称: void SI2C_SendAck
功能描述: 模拟I2C发送应答
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_SendAck(SoftI2cPar_t *Par)
{
	uint16_t rlow;
	
	rlow = Par->SCLLowHoldDelayUs - Par->SDAChangeDelayUs;
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_RESET);	//SCL低半个周期
	Par->cbDelayUs(Par->SDAChangeDelayUs);
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_RESET);	//发送应答
	Par->cbDelayUs(rlow);											//时钟低延时
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_SET);		//SCL高半个周期
	Par->cbDelayUs(Par->SCLHighHoldDelayUs);
}

/******************************************************************
函数名称: void SI2C_SendNAck
功能描述: 模拟I2C发送否应答
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_SendNAck(SoftI2cPar_t *Par)
{
	uint16_t rlow;
	
	rlow = Par->SCLLowHoldDelayUs - Par->SDAChangeDelayUs;
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_RESET);	//SCL低半个周期
	Par->cbDelayUs(Par->SDAChangeDelayUs);
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);		//发送否应答
	Par->cbDelayUs(rlow);											//时钟低延时
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_SET);		//SCL高半个周期
	Par->cbDelayUs(Par->SCLHighHoldDelayUs);
}

/******************************************************************
函数名称: uint8 SI2C_RecvAck
功能描述: 模拟I2C接收应答
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
参数输出: 无
返 回 值: uint8_t 0=应答;1=否应答
其    他: 无
******************************************************************/
uint8_t SI2C_RecvAck(SoftI2cPar_t *Par)
{
	uint16_t rhigh;
	uint8_t read;
	
	rhigh = Par->SCLHighHoldDelayUs - Par->SDASampleDelayUs;
	read = 1;
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_RESET);	//SCL低半个周期
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);		//SDA释放
	Par->cbDelayUs(Par->SCLLowHoldDelayUs);
	
	HAL_GPIO_WritePin(Par->SCLPort, Par->SCLPin, GPIO_PIN_SET);		//SCL高半个周期
	Par->cbDelayUs(Par->SDASampleDelayUs);							//SDA采样延时
	read = HAL_GPIO_ReadPin(Par->SDAPort, Par->SDAPin) & 0x01;		//读SDA管脚
	Par->cbDelayUs(rhigh);											//时钟高延时
	
	HAL_GPIO_WritePin(Par->SDAPort, Par->SDAPin, GPIO_PIN_SET);		//复位释放
	return read;
}


/******************************************************************
	模拟I2C字节读写操作
******************************************************************/

/******************************************************************
函数名称: void SI2C_WriteByte
功能描述: 模拟I2C写字节
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
		  uint8_t		Byte	发送字节
		  uint8_t		*Ack	从机应答 0=ACK;1=NACK
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_WriteByte(SoftI2cPar_t *Par, uint8_t Byte, uint8_t *Ack)
{
	uint8_t i;
	uint8_t sbit;
	uint8_t slvack;
	
	/* 发送字节 */
	/* I2C发送先发高位，再发低位 */
	for (i=0; i<8; i++)
	{
		if ((Byte & 0x80) == 0x00)
			sbit = 0;
		else
			sbit = 1;
		SI2C_WriteBit(Par, sbit);
		Byte = Byte << 1;			//发送高位后数据左移
	}
	
	/* 接收应答 */
	slvack = SI2C_RecvAck(Par);
	*Ack = slvack;
}

/******************************************************************
函数名称: void SI2C_ReadByte
功能描述: 模拟I2C读字节
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
		  uint8_t		*Byte	接收字节
		  uint8_t		Ack		主机应答 0=ACK;1=NACK
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void SI2C_ReadByte(SoftI2cPar_t *Par, uint8_t *Byte, uint8_t Ack)
{
	uint8_t i;
	uint8_t sbit;
	uint8_t read;
	
	/* 读取字节 */
	/* I2C读取先读高位，再读低位 */
	read = 0;
	for (i=0; i<8; i++)
	{
		read = read << 1;
		SI2C_ReadBit(Par, &sbit);
		if (sbit == 0)
			read = read & 0xFE;
		else
			read = read | 0x01;
	}
	*Byte = read;
	
	/* 发送应答 */
	if (Ack == 0)
		SI2C_SendAck(Par);
	else
		SI2C_SendNAck(Par);
}

/******************************************************************
函数名称: int16_t SI2C_WriteDataByReg
功能描述: 模拟I2C给指定寄存器写数据
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
		  uint8_t		DevAddr 设备地址
		  uint8_t		RegAddr	寄存器地址
		  uint8_t		*wData	写入数据
		  uint16_t		Len		数据长度
参数输出: 无
返 回 值: 0=写入正常;<0=写入失败
其    他: 无
******************************************************************/
int16_t SI2C_WriteDataByReg(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t RegAddr, uint8_t *wData, uint16_t Len)
{
	uint8_t addr;
	uint8_t ack;
	uint16_t i;
	uint16_t m;
	
	int16_t ret;
	
	m = 0;
	ack = 1;	//0=应答;1=否应答
	ret = SI2C_RET_TIMEOUT;
	do
	{
		/* 发送设备地址 */
		SI2C_Start(Par);
		addr = ((DevAddr & 0x7F) << 1) | SI2C_DIR_WRITE;
		SI2C_WriteByte(Par, addr, &ack);
		if (ack == 1)
		{
			m++;
			SI2C_Stop(Par);
			ret = SI2C_RET_NODEVICE;
			continue;
		}
		
		/* 发送寄存器地址 */
		SI2C_WriteByte(Par, RegAddr, &ack);
		if (ack == 1)
		{
			m++;
			SI2C_Stop(Par);
			ret = SI2C_RET_NODEVICE;
			continue;
		}
		
		/* 发送数据 */
		for (i=0; i<Len; i++)
		{
			ret = SI2C_RET_OK;
			SI2C_WriteByte(Par, wData[i], &ack);
			if (ack == 1)
			{
				m++;
				SI2C_Stop(Par);
				ret = SI2C_RET_NOACK;
				break;
			}
		}
		if (ret == SI2C_RET_OK)
		{
			SI2C_Stop(Par);
		}
	} while ((ret != SI2C_RET_OK) && (m < 10));
	
	return ret;
}

/******************************************************************
函数名称: int16_t SI2C_ReadDataByReg
功能描述: 模拟I2C给指定寄存器读数据
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
		  uint8_t		DevAddr 设备地址
		  uint8_t		RegAddr	寄存器地址
		  uint8_t		*rData	读出数据
		  uint16_t		Len		数据长度
参数输出: 无
返 回 值: 0=读出正常;<0=读出失败
其    他: 无
******************************************************************/
int16_t SI2C_ReadDataByReg(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t RegAddr, uint8_t *rData, uint16_t Len)
{
	uint8_t addr;
	uint8_t ack;
	uint16_t i;
	uint16_t m;
	
	int16_t ret;
	
	m = 0;
	ack = 1;	//0=应答;1=否应答
	ret = SI2C_RET_TIMEOUT;
	do
	{
		/* 发送设备地址 */
		SI2C_Start(Par);
		addr = ((DevAddr & 0x7F) << 1) | SI2C_DIR_WRITE;
		SI2C_WriteByte(Par, addr, &ack);
		if (ack == 1)
		{
			m++;
			SI2C_Stop(Par);
			ret = SI2C_RET_NODEVICE;
			continue;
		}
		
		/* 发送寄存器地址 */
		SI2C_WriteByte(Par, RegAddr, &ack);
		if (ack == 1)
		{
			m++;
			SI2C_Stop(Par);
			ret = SI2C_RET_NODEVICE;
			continue;
		}
		
		/* 发送设备读地址 */
//		SI2C_Start(Par);
		addr = ((DevAddr & 0x7F) << 1) | SI2C_DIR_READ;
		SI2C_WriteByte(Par, addr, &ack);
		if (ack == 1)
		{
			m++;
			SI2C_Stop(Par);
			ret = SI2C_RET_NODEVICE;
			continue;
		}
		
		/* 读取数据 */
		for (i=0; i<Len; i++)
		{
			ret = SI2C_RET_OK;
			if (i == (Len - 1))
			{
				SI2C_ReadByte(Par, rData + i, 1);	//最后一个字节读取后发送否应答停止通信
				SI2C_Stop(Par);
			}
			else
			{
				SI2C_ReadByte(Par, rData + i, 0);	//非最后一个字节读取后发送应答
			}
		}
	} while ((ret != SI2C_RET_OK) && (m < 10));
	
	return ret;
}

/******************************************************************
函数名称: int16_t SI2C_WriteDataDirect
功能描述: 模拟I2C直接写数据
参数输入: SoftI2cPar_t	*Par	模拟I2C参数
		  uint8_t		DevAddr 设备地址
		  uint8_t		*wData	写入数据
		  uint16_t		Len		数据长度
参数输出: 无
返 回 值: 0=写入正常;<0=写入失败
其    他: 不需要写寄存器时使用
******************************************************************/
int16_t SI2C_WriteDataDirect(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t *wData, uint16_t Len)
{
	uint8_t addr;
	uint8_t ack;
	uint16_t i;
	uint16_t m;
	
	int16_t ret;
	
	m = 0;
	ack = 1;	//0=应答;1=否应答
	ret = SI2C_RET_TIMEOUT;
	do
	{
		/* 发送设备地址 */
		SI2C_Start(Par);
		addr = ((DevAddr & 0x7F) << 1) | SI2C_DIR_WRITE;
		SI2C_WriteByte(Par, addr, &ack);
		if (ack == 1)
		{
			m++;
			SI2C_Stop(Par);
			ret = SI2C_RET_NODEVICE;
			continue;
		}
				
		/* 发送数据 */
		for (i=0; i<Len; i++)
		{
			ret = SI2C_RET_OK;
			SI2C_WriteByte(Par, wData[i], &ack);
			if (ack == 1)
			{
				m++;
				SI2C_Stop(Par);
				ret = SI2C_RET_NOACK;
				break;
			}
		}
		if (ret == SI2C_RET_OK)
		{
			SI2C_Stop(Par);
		}
	} while ((ret != SI2C_RET_OK) && (m < 10));
	
	return ret;
}

/******************************************************************
函数名称: int16_t SI2C_ReadDataDirect
功能描述: 模拟I2C直接读数据
参数输入: SoftI2cPar_t	*Par		模拟I2C参数
		  uint8_t		DevAddr 设备地址
		  uint8_t		*rData	读出数据
		  uint16_t		Len		数据长度
参数输出: 无
返 回 值: 0=读出正常;<0=读出失败
其    他: 不需要写寄存器时使用
******************************************************************/
int16_t SI2C_ReadDataDirect(SoftI2cPar_t *Par, uint8_t DevAddr, uint8_t *rData, uint16_t Len)
{
	uint8_t addr;
	uint8_t ack;
	uint16_t i;
	uint16_t m;
	
	int16_t ret;
	
	m = 0;
	ack = 1;	//0=应答;1=否应答
	ret = SI2C_RET_TIMEOUT;
	do
	{
		/* 发送设备地址读 */
		SI2C_Start(Par);
		addr = ((DevAddr & 0x7F) << 1) | SI2C_DIR_READ;
		SI2C_WriteByte(Par, addr, &ack);
		if (ack == 1)
		{
			m++;
			SI2C_Stop(Par);
			ret = SI2C_RET_NODEVICE;
			continue;
		}
						
		/* 读取数据 */
		for (i=0; i<Len; i++)
		{
			ret = SI2C_RET_OK;
			if (i == (Len - 1))
			{
				SI2C_ReadByte(Par, rData + i, 1);	//最后一个字节读取后发送否应答停止通信
				SI2C_Stop(Par);
			}
			else
			{
				SI2C_ReadByte(Par, rData + i, 0);	//非最后一个字节读取后发送应答
			}
		}
	} while ((ret != SI2C_RET_OK) && (m < 10));
	
	return ret;
}


