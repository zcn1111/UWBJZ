#include "BSP.h"

#define ADDR_24C02 0xA0

uint8_t USE_EXT_EEPROM = 0;           //外接EEPROM,当检测到EEPROM时赋值为1
uint8_t EquipmentNum[EEP_UNIT_SIZE] = {0};
uint8_t HardwareVer[64] = {0};


uint8_t e2prom_Init(void)
{
	if (HAL_I2C_IsDeviceReady(&hI2C3, ADDR_24C02, 1, 2000) != HAL_OK) 
	{
		return 0;
	}
	return 1;
}

uint8_t e2prom_Write_Bytes(uint16_t addr, uint8_t *data, uint16_t length)
{
    if( HAL_I2C_Mem_Write( &hI2C3, ADDR_24C02, addr, I2C_MEMADD_SIZE_8BIT, data, length, 0xFFFF ) == HAL_OK )
        return 1;
    else
        return 0;
}


uint8_t e2prom_Read_Bytes(uint16_t addr, uint8_t *data, uint16_t length)
{
    if( HAL_I2C_Mem_Read( &hI2C3, ADDR_24C02, addr, I2C_MEMADD_SIZE_8BIT, data, length, 1000 ) == HAL_OK )
        return 1;
    else
        return 0;
}



int E2prom_Write(uint8_t addr, uint8_t* data_write, int len)
{
    if((addr%8)!=0)
    {
        return 0;
    }
    int ret=0;
    int i=0,j=0;
    i=len/8;
    j=len%8;
    uint8_t *data_write_temp = data_write; 

    while(i>0)
    {
        ret = e2prom_Write_Bytes(addr,data_write,8);
        data_write+=8;
        addr+=8;
        i--;
        HAL_Delay(5);
    }

    if(j>0)
    {
        ret = e2prom_Write_Bytes(addr,data_write,j);  
        j=0;
        HAL_Delay(5);
    }
    data_write=data_write_temp;
    return ret;
}


int E2prom_Read(uint8_t addr, uint8_t* data_read, int len)
{
    if((addr%8)!=0)
    {
        return 0;
    }
    int ret=0;
    int i=0,j=0;
    i=len/8;
    j=len%8; 
    uint8_t *data_read_temp = data_read; 

    while(i>0)
    {
        ret = e2prom_Read_Bytes(addr, data_read, 8);
        i--;
        addr+=8;
        data_read+=8;
        HAL_Delay(5);
    }

	if(j>0)
	{
		ret = e2prom_Read_Bytes(addr, data_read, j);
        HAL_Delay(5);
	}

    data_read = data_read_temp;
    return ret;
}


void Read_EQUIPNUM_EEPROM(void)
{
	memset(EquipmentNum,0x00,sizeof(EquipmentNum));
	E2prom_Read(EEPROM_EQUIPNUM_ADDR,EquipmentNum,EEP_UNIT_SIZE);
}

void Write_EQUIPNUM_EEPROM(void)
{
	E2prom_Write(EEPROM_EQUIPNUM_ADDR, EquipmentNum, EEP_UNIT_SIZE);
}


void Read_HARDWAREVER_EEPROM(void)
{
	memset(HardwareVer,0x00,sizeof(HardwareVer));
	E2prom_Read(EEPROM_HARDWAREVER_ADDR,HardwareVer,64);
}

void Write_HARDWAREVER_EEPROM(void)
{
	E2prom_Write(EEPROM_HARDWAREVER_ADDR, HardwareVer, 64);
}







