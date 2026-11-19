#ifndef __BSP_MPU_H
#define __BSP_MPU_H


/* 引用头文件 */
#include "BSP.h"

/******* 宏定义定义 *******/
//SRAM保护区域
#define SRAM_REGION_NUMBER		MPU_REGION_NUMBER0		//SRAM使用region0
#define SRAM_ADDRESS_START		0x24000000				//SRAM首地址
#define SRAM_REGION_SIZE		MPU_REGION_SIZE_512KB	//SRAM区大小

//NAND保护区域
#define NAND_REGION_NUMBER      MPU_REGION_NUMBER3	    //NANDFLASH使用region3
#define NAND_ADDRESS_START      0X80000000              //NANDFLASH区的首地址
#define NAND_REGION_SIZE        MPU_REGION_SIZE_256MB   //NANDFLASH区大小

/******* GPIO管脚名称定义 *******/

/******* 数据结构体定义 *******/


/******* 外部变量声明 *******/
/*  */


/******* 外部函数声明 *******/
void MPUConfig(void);

void Cache_Enable(void);

#endif

