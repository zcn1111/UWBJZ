#include "IP2365.h"
#include "main.h"

IP2365_HandleTypeDef hip2365;
uint32_t g_DetectChaDelay;
uint8_t  g_IP2365Init = 0;
uint8_t  g_IP2365ErrNum;
/**
  * @brief  初始化IP2365
  * @param  hip: IP2365处理结构体指针
  * @param  hi2c: I2C处理结构体指针
  * @retval 错误代码
  */
int8_t IP2365_Init(IP2365_HandleTypeDef *hip, I2C_HandleTypeDef *hi2c)
{
  if(hip == NULL || hi2c == NULL)
    return IP2365_ERROR;
  
  hip->hi2c = hi2c;
  hip->address = IP2365_I2C_ADDR;
  
  // 检查设备是否响应
  if(HAL_I2C_IsDeviceReady(hip->hi2c, hip->address << 1, 3, 1000) != HAL_OK)
  {
    return IP2365_ERROR;
  }
  
  return IP2365_OK;
}

/**
  * @brief  读取IP2365寄存器
  * @param  hip: IP2365处理结构体指针
  * @param  reg: 寄存器地址
  * @param  data: 读取的数据指针
  * @retval 错误代码
  */
int8_t IP2365_ReadRegister(IP2365_HandleTypeDef *hip, uint8_t reg, uint8_t *data)
{
  if(hip == NULL || data == NULL)
    return IP2365_ERROR;
  
  HAL_StatusTypeDef status = HAL_I2C_Mem_Read(hip->hi2c, hip->address << 1, reg, 
                                             I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
  
  if(status != HAL_OK)
    return IP2365_ERROR;
  
  return IP2365_OK;
}

/**
  * @brief  写入IP2365寄存器
  * @param  hip: IP2365处理结构体指针
  * @param  reg: 寄存器地址
  * @param  data: 要写入的数据
  * @retval 错误代码
  */
int8_t IP2365_WriteRegister(IP2365_HandleTypeDef *hip, uint8_t reg, uint8_t data)
{
  if(hip == NULL)
    return IP2365_ERROR;
  
  HAL_StatusTypeDef status = HAL_I2C_Mem_Write(hip->hi2c, hip->address << 1, reg, 
                                              I2C_MEMADD_SIZE_8BIT, &data, 1, 0xFFFF);
  
  if(status != HAL_OK)
    return IP2365_ERROR;
  
  return IP2365_OK;
}

/**
  * @brief  启用或禁用充电
  * @param  hip: IP2365处理结构体指针
  * @param  enable: true启用充电，false禁用
  * @retval 错误代码
  */
int8_t IP2365_EnableCharging(IP2365_HandleTypeDef *hip, bool enable)
{
  uint8_t reg_data;
  
  // 读取当前寄存器值
  if(IP2365_ReadRegister(hip, IP2365_REG_SYS_CTL1, &reg_data) != IP2365_OK)
    return IP2365_ERROR;
  
  // 设置或清除充电使能位
  if(enable)
    reg_data |= 0x01;  // 设置充电使能位
  else
    reg_data &= ~0x01; // 清除充电使能位
  
  // 写回寄存器
  return IP2365_WriteRegister(hip, IP2365_REG_SYS_CTL1, reg_data);
}

/**
  * @brief  设置输入端充电电流
  * @param  hip: IP2365处理结构体指针
  * @param  current: 充电电流(单位: A)
  * @retval 错误代码
  */
int8_t IP2365_SetInputCurrent(IP2365_HandleTypeDef *hip, float current)
{
  // 计算寄存器值: ISET = (current - 0.05) / 0.1
  // 限制在有效范围内(0-63)
  uint8_t reg_value = (uint8_t)((current - 0.05f) / 0.1f);
  if(reg_value > 63) reg_value = 63;
  
  // 写入寄存器
  return IP2365_WriteRegister(hip, IP2365_REG_VIN_ISET, reg_value);
}

/**
  * @brief  设置电池端充电电流
  * @param  hip: IP2365处理结构体指针
  * @param  current: 充电电流(单位: A)
  * @retval 错误代码
  */
int8_t IP2365_SetBatteryCurrent(IP2365_HandleTypeDef *hip, float current)
{
  // 计算寄存器值: ISET = (current - 0.05) / 0.1
  // 限制在有效范围内(0-63)
  uint8_t reg_value = (uint8_t)((current - 0.05f) / 0.1f);
  if(reg_value > 63) reg_value = 63;
  
  // 写入寄存器
  return IP2365_WriteRegister(hip, IP2365_REG_BAT_ISET, reg_value);
}

/**
  * @brief  启用或禁用电阻设置电流功能
  * @param  hip: IP2365处理结构体指针
  * @param  bat_enable: 是否启用电池端电阻设置
  * @param  vin_enable: 是否启用输入端电阻设置
  * @retval 错误代码
  */
int8_t IP2365_EnableResistorCurrentSetting(IP2365_HandleTypeDef *hip, bool bat_enable, bool vin_enable)
{
  uint8_t reg_data = 0;
  
  // 设置电池端电阻使能位
  if(bat_enable)
    reg_data |= (1 << 3);
  
  // 设置输入端电阻使能位
  if(vin_enable)
    reg_data |= (1 << 2);
  
  // 写入寄存器
  return IP2365_WriteRegister(hip, IP2365_REG_RJSET, reg_data);
}

/**
  * @brief  启用或禁用NTC功能
  * @param  hip: IP2365处理结构体指针
  * @param  enable: true启用，false禁用
  * @retval 错误代码
  */
int8_t IP2365_EnableNTC(IP2365_HandleTypeDef *hip, bool enable)
{
  uint8_t reg_data;
  
  // 读取当前寄存器值
  if(IP2365_ReadRegister(hip, IP2365_REG_NTC_CTL, &reg_data) != IP2365_OK)
    return IP2365_ERROR;
  
  // 设置或清除NTC使能位
  if(enable)
    reg_data |= 0x01;  // 设置NTC使能位
  else
    reg_data &= ~0x01; // 清除NTC使能位
  
  // 写回寄存器
  return IP2365_WriteRegister(hip, IP2365_REG_NTC_CTL, reg_data);
}

/**
  * @brief  设置NTC温度阈值
  * @param  hip: IP2365处理结构体指针
  * @param  high_temp: 高温阈值设置 (0-3)
  * @param  mid_temp: 中温阈值设置 (0-3)
  * @param  low_temp: 低温阈值设置 (0-3)
  * @retval 错误代码
  */
int8_t IP2365_SetNTCTresholds(IP2365_HandleTypeDef *hip, uint8_t high_temp, uint8_t mid_temp, uint8_t low_temp)
{
  uint8_t reg_data = 0;
  
  // 设置高温阈值 (位6-5)
  reg_data |= ((high_temp & 0x03) << 5);
  
  // 设置中温阈值 (位4-3)
  reg_data |= ((mid_temp & 0x03) << 3);
  
  // 设置低温阈值 (位2-1)
  reg_data |= ((low_temp & 0x03) << 1);
  
  // 保持NTC使能位不变
  uint8_t current_reg;
  if(IP2365_ReadRegister(hip, IP2365_REG_NTC_CTL, &current_reg) == IP2365_OK)
  {
    reg_data |= (current_reg & 0x01); // 保留NTC使能位
  }
  
  // 写入寄存器
  return IP2365_WriteRegister(hip, IP2365_REG_NTC_CTL, reg_data);
}

/**
  * @brief  获取NTC状态
  * @param  hip: IP2365处理结构体指针
  * @param  status: NTC状态指针
  * @retval 错误代码
  */
int8_t IP2365_GetNTCStatus(IP2365_HandleTypeDef *hip, IP2365_NTCStatus *status)
{
  uint8_t reg_data;
  
  if(IP2365_ReadRegister(hip, IP2365_REG_NTC_STAT, &reg_data) != IP2365_OK)
    return IP2365_ERROR;
  
  // 提取NTC状态位 (位6-4)
  *status = (IP2365_NTCStatus)((reg_data >> 4) & 0x07);
  
  return IP2365_OK;
}

/**
  * @brief  设置LED模式
  * @param  hip: IP2365处理结构体指针
  * @param  charging_mode: 充电中LED模式 (0:灭, 1:灭, 2:闪, 3:亮)
  * @param  full_mode: 充满后LED模式 (0:灭, 1:亮)
  * @retval 错误代码
  */
int8_t IP2365_SetLEDMode(IP2365_HandleTypeDef *hip, uint8_t charging_mode, uint8_t full_mode)
{
  uint8_t reg_data = 0;
  
  // 设置充电中LED模式 (位5-4)
  reg_data |= ((charging_mode & 0x03) << 4);
  
  // 设置充满后LED模式 (位1)
  reg_data |= ((full_mode & 0x01) << 1);
  
  // 写入寄存器
  return IP2365_WriteRegister(hip, IP2365_REG_LED_REG, reg_data);
}

/**
  * @brief  获取充电状态
  * @param  hip: IP2365处理结构体指针
  * @param  status: 充电状态指针
  * @retval 错误代码
  */
int8_t IP2365_GetChargingStatus(IP2365_HandleTypeDef *hip, uint8_t *status)
{
  uint8_t reg_data;
  
  if(IP2365_ReadRegister(hip, IP2365_REG_CHG_FULL, &reg_data) != IP2365_OK)
    return IP2365_ERROR;
  
  // 提取充电状态位 (位2-0)
  *status = reg_data;//(IP2365_ChargingStatus)(reg_data & 0x07);
  
  return IP2365_OK;
}

/**
  * @brief  检查是否充满
  * @param  hip: IP2365处理结构体指针
  * @param  full: 充满状态指针
  * @retval 错误代码
  */
int8_t IP2365_IsChargingFull(IP2365_HandleTypeDef *hip, bool *full)
{
  uint8_t reg_data;
  
  if(IP2365_ReadRegister(hip, IP2365_REG_CHG_FULL, &reg_data) != IP2365_OK)
    return IP2365_ERROR;
  
  // 检查充满标志位 (位5)
  *full = (reg_data & 0x20) != 0;
  
  return IP2365_OK;
}

/**
  * @brief  检查输入是否过压
  * @param  hip: IP2365处理结构体指针
  * @param  ovp: 过压状态指针
  * @retval 错误代码
  */
int8_t IP2365_IsInputOvervoltage(IP2365_HandleTypeDef *hip, bool *ovp)
{
  uint8_t reg_data;
  
  if(IP2365_ReadRegister(hip, IP2365_REG_NTC_STAT, &reg_data) != IP2365_OK)
    return IP2365_ERROR;
  
  // 检查输入过压状态位 (位0)
  *ovp = (reg_data & 0x01) != 0;
  
  return IP2365_OK;
}
void IP2365_Detect(void)
{
	uint8_t chg_status;
	if (JudgeMilSecondDelayInPro(g_DetectChaDelay, 60000) == true)
	{
		if(g_IP2365Init == 0x00 && IP2365_Init(&hip2365, &hI2C2) == IP2365_OK)
		{			  	
			g_IP2365Init = 0x01;
			g_IP2365ErrNum = 0;			
		}	
		else if(g_IP2365Init == 0x01)
		{
			if(IP2365_GetChargingStatus(&hip2365, &chg_status) == IP2365_OK)
			{
				if((chg_status & 0x20) == 0)
				{//正在充电
				
				}
				else
				{//充满
				
				}
			}
			else
			{
				if(g_IP2365ErrNum >1)
				{
					g_IP2365ErrNum=0;
					g_IP2365Init = 0;
				}
				else
					g_IP2365ErrNum++;
			}			
		}	
		g_DetectChaDelay = uwTick;	
	}
}







