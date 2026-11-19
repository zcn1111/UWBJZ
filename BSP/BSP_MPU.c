#include "main.h"
#include "BSP.h"
#include "BSP_MPU.h"

/******** 句柄变量 ********/

/******************************************************************
函数名称: void Cache_Enable
功能描述: 配置Cache启用
参数输入: 无
参数输出: 无
返 回 值: 无
其    他:
******************************************************************/
void Cache_Enable(void)
{
	SCB_EnableICache();		//使能I-Cache
//	SCB_EnableDCache();		//使能D-Cache
	SCB_DisableDCache();	//禁用D-Cache
	SCB->CACR|=1<<2;		//强制D-Cache透写,如不开启,实际使用中可能遇到各种问题	
}

/******************************************************************
函数名称: void MPURegionConfig
功能描述: MPU区域配置
参数输入: 无
参数输出: 无
返 回 值: 无
其    他:
******************************************************************/
void MPURegionConfig(uint32_t BaseAddr, uint8_t RgnNbr, uint8_t RgnSize, uint8_t Permission, uint8_t ShareEn, uint8_t CacheEn, uint8_t BufEn)
{
	MPU_Region_InitTypeDef	MPU_InitStruct;
		
	/* 配置NAND保护 */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;					//保护区域		=使能
	MPU_InitStruct.Number = RgnNbr;								//保护区域编号	
	MPU_InitStruct.BaseAddress = BaseAddr;						//保护区域基地址	
	MPU_InitStruct.Size = RgnSize;								//保护区域大小	
	MPU_InitStruct.SubRegionDisable = 0X00;						//子区域			=无
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;				//保护级别		=LEVEL0
	MPU_InitStruct.AccessPermission = Permission;				//访问权限		
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;	//可执行指令		=是
	MPU_InitStruct.IsShareable = ShareEn;						//可共享			
	MPU_InitStruct.IsCacheable = CacheEn;						//可用Cache		
	MPU_InitStruct.IsBufferable = BufEn;						//可用缓冲		
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
}

/******************************************************************
函数名称: void MPUConfig
功能描述: MPU参数配置
参数输入: 无
参数输出: 无
返 回 值: 无
其    他:
******************************************************************/
void MPUConfig(void)
{
	
	/* 配置MPU之前先关闭MPU */
	HAL_MPU_Disable();
	
//	/*  配置全地址 
//	保护区域基地址	=0x00000000
//	保护区域编号		=Region0
//	保护区域大小		=4GB 全部地址
//	共享			=是
//	Cache			=否
//	缓冲			=否
//	*/
//	MPURegionConfig(0x00000000, MPU_REGION_NUMBER0, MPU_REGION_SIZE_4GB, MPU_REGION_FULL_ACCESS, MPU_ACCESS_SHAREABLE, MPU_ACCESS_NOT_CACHEABLE, MPU_ACCESS_NOT_BUFFERABLE);
	
	
	/*  配置NAND保护 
	保护区域基地址	=0x80000000 NAND使用0x80000000
	保护区域编号		=Region3
	保护区域大小		=256MB NAND使用256MB
	共享			=是
	Cache			=否
	缓冲			=否
	*/
	MPURegionConfig(0x80000000, MPU_REGION_NUMBER3, MPU_REGION_SIZE_256MB, MPU_REGION_FULL_ACCESS, MPU_ACCESS_SHAREABLE, MPU_ACCESS_NOT_CACHEABLE, MPU_ACCESS_NOT_BUFFERABLE);
	
//	/*  配置AXI SRAM保护 
//	保护区域基地址	=0x24000000
//	保护区域编号		=Region1
//	保护区域大小		=512KB
//	共享			=是
//	Cache			=否
//	缓冲			=否
//	*/
//	MPURegionConfig(0x24000000, MPU_REGION_NUMBER1, MPU_REGION_SIZE_512KB, MPU_REGION_FULL_ACCESS, MPU_ACCESS_SHAREABLE, MPU_ACCESS_NOT_CACHEABLE, MPU_ACCESS_NOT_BUFFERABLE);

	/*  配置DTCM保护 
	保护区域基地址	=0x20000000
	保护区域编号		=Region0
	保护区域大小		=512KB
	共享			=是
	Cache			=否
	缓冲			=否
	*/
	MPURegionConfig(0x20000000, MPU_REGION_NUMBER0, MPU_REGION_SIZE_512KB, MPU_REGION_FULL_ACCESS, MPU_ACCESS_SHAREABLE, MPU_ACCESS_NOT_CACHEABLE, MPU_ACCESS_NOT_BUFFERABLE);

//	/*  配置EthDMA保护 
//	保护区域基地址	=0x30040000
//	保护区域编号		=Region5
//	保护区域大小		=256B
//	共享			=是
//	Cache			=否
//	缓冲			=否
//	*/
//	MPURegionConfig(0x30040000, MPU_REGION_NUMBER5, MPU_REGION_SIZE_256B, MPU_REGION_FULL_ACCESS, MPU_ACCESS_SHAREABLE, MPU_ACCESS_NOT_CACHEABLE, MPU_ACCESS_NOT_BUFFERABLE);

	/* 启用MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
	
}

