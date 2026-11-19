#ifndef  __E2PROM_H_
#define  __E2PROM_H_

#include "stdint.h"
#include "stdbool.h"


#define EEP_UNIT_SIZE    48
#define EEPROM_EQUIPNUM_ADDR     (0*EEP_UNIT_SIZE)
#define EEPROM_HARDWAREVER_ADDR  (64)
extern uint8_t EquipmentNum[EEP_UNIT_SIZE];
extern uint8_t HardwareVer[64];
uint8_t e2prom_Init(void);
int E2prom_Write(uint8_t addr,uint8_t*data_write,int len);
int E2prom_Read(uint8_t addr,uint8_t*data_read,int len);

uint8_t e2prom_Write_Bytes(uint16_t addr, uint8_t *data, uint16_t length);
uint8_t e2prom_Read_Bytes(uint16_t addr, uint8_t *data, uint16_t length);
void Read_EQUIPNUM_EEPROM(void);
void Write_EQUIPNUM_EEPROM(void);
void Read_HARDWAREVER_EEPROM(void);
void Write_HARDWAREVER_EEPROM(void);
#endif
