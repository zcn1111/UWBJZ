/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IAP_H
#define __IAP_H

#include <stdint.h>

typedef struct
{
	uint32_t 	oldsize;		//旧bin文件大小
	uint32_t 	diffsize;		//差分升级bin文件大小
	uint8_t 	newsum;			//新bin文件累加和
	uint8_t 	res[6];			//预留
	uint8_t 	sum;			//累加和
}DIFFINFO;

typedef  void (*iapfun)(void);				//定义一个函数类型的参数
   
#define FLASH_BOOT_ADDR		0x08000000  	//0X08000000~0X0800FFFF的空间为Bootloader使用(共64KB)	  
#define FLASH_APP1_ADDR		0x08010000  	//第一个应用程序起始地址(存放在FLASH)

extern uint64_t iapbuf[256]; 	//2K字节缓存  

//跳转到应用程序段
//appxaddr:用户代码起始地址.
void IAPLoadApp(uint32_t appxaddr);

uint8_t ProgramUserData(void);
void DiffProgramUserData(void);
void StopPeripherals(void);
void SoftReset(void);
#endif /* __IAP_H */