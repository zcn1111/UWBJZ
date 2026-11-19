#include "BSP.h"
#include "g_GlobalValDef.h"
#include "Update.h"
#include "Drv_W25Qxx_QSPI.h"
#include "iap.h"
#include "IAP_bsdiff.h"
iapfun jump2app;  
uint16_t UPDATE_COUNT;


__asm void MSR_MSP(uint32_t addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
//跳转到应用程序段
//appxaddr:用户代码起始地址.
void IAPLoadApp(uint32_t appxaddr)
{ 
	if(((*(__IO uint32_t*)appxaddr)&0x2FF00000)==0x20000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(__IO uint32_t*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(__IO uint32_t*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		jump2app();									//跳转到APP.
	}
}


void SoftReset(void)
{
	uint8_t i;
	void (*SysMemBootJump)(void);        /* 声明一个函数指针 */
	__IO uint32_t BootAddr = FLASH_BASE; 
	/* 关闭全局中断 */

	/* 关闭滴答定时器，复位到默认值 */
	SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
	
	/* 设置所有时钟到默认状态，使用HSI时钟 */
	HAL_RCC_DeInit();
	
	
	/* 关闭所有中断，清除所有中断挂起标志 */
	for (i = 0; i < 8; i++)
	{
		NVIC->ICER[i]=0xFFFFFFFF;
		NVIC->ICPR[i]=0xFFFFFFFF;
	}	

	SCB->VTOR = FLASH_BASE;
	HAL_NVIC_SystemReset();

}


unsigned char testbuf[1024*2];
#define DIFF_INFORM_SIZE	16 // 4字节旧BIN文件大小 + 1字节累加和 + 4字节差分文件大小 + 6字节预留 + 1字节累加和

/*********************************************************************************************************
**函数名称：DiffProgramUserData
**函数功能：增量升级解压文件
**入口参数：无
**出口参数：
********************************************************************************************************/
void DiffProgramUserData(void)
{
	int i,j,n;
	int old_addr,old_size,decompress_addr,decompress_size,new_addr,offset;
	int64_t new_size;
	uint8_t sum=0;
	DIFFINFO diffinfo;
	W25qxx_ReadBytes((uint8_t*)&diffinfo, FLASH_IAP_DIFFUSER_START1, DIFF_INFORM_SIZE); 
	
	if(diffinfo.sum != CheckSumNoC((uint8_t*)&diffinfo,sizeof(diffinfo)-1))
	{
		// 校验失败，不能更新
		DiffUpdateFileSize.FileSize = 0;
		WriteDiffFileSizeToFlash();
		return;
	}
	// 注意：
	// FLASH_IAP_DIFFUSER_START1 差分文件
	// FLASH_IAP_DIFFUSER_START2 解压后的文件
	// FLASH_IAP_START1  还原后的文件，这是最终要拷贝至内部FLASH的新BIN文件
	// 新BIN放置在 FLASH_IAP_START1 FLASH_IAP_START1 的大小）
	
	// 1. 从 FLASH_IAP_DIFFUSER_START1 解压到 FLASH_IAP_DIFFUSER_START2
	decompress_file((FLASH_IAP_DIFFUSER_START1+DIFF_INFORM_SIZE), diffinfo.diffsize , FLASH_IAP_DIFFUSER_START2, &decompress_size);
	
	// 以下为必须的参数
	old_addr 				= FLASH_BASE | 0x10000;				// 旧BIN地址
	old_size 				= diffinfo.oldsize;							// 旧BIN长度
	decompress_addr 		= FLASH_IAP_DIFFUSER_START2;		// 解压后的BIN地址
	new_addr				= FLASH_IAP_START1;			// 新BIN地址	
	
	// 2. 根据 内部FLASH 的旧BIN文件，比对 FLASH_IAP_DIFFUSER_START2 内的解压文件，最终生成新BIN到 FLASH_IAP_START1
	recover_bspatch_file(old_addr, old_size, decompress_addr, decompress_size, new_addr, &new_size);//
	
	// 验证 校验和OK
	j = new_size;
	for(i=0;i<960;i++)
	{
		//IWDG_Feed()();
		if(j)
		{
			if(j > 1024)
				{n = 1024;}
			else
				{n=j;}
			
			memset(testbuf,0,2048);
			W25qxx_ReadBytes(testbuf, FLASH_IAP_START1+i*1024, n);
			sum += CheckSumNoC((uint8_t*)&testbuf,n);
			j -= n;
		}
	}
	
	if(sum != diffinfo.newsum)
	{
		// 校验失败，不能更新
		DiffUpdateFileSize.FileSize = 0;
		WriteDiffFileSizeToFlash();
		
		NormalUpdateFileSize.FileSize = 0;
		WriteNormalFileSizeToFlash();
	}
	else
	{
		// 3. 写标记，后续从 FLASH_IAP_START1 拷贝程序至内部FLASH
		DiffUpdateFileSize.FileSize = 0;
		WriteDiffFileSizeToFlash();
		
		if(new_size%496 == 0)
		{
			NormalUpdateFileSize.FileSize = new_size/496;
		}
		else
		{
			NormalUpdateFileSize.FileSize = new_size/496+1;
			offset = 496-(new_size%496);
			memset(testbuf,0xFF,offset);
			W25q_Write_N(testbuf,FLASH_IAP_START1+new_size,offset);//  数据写在 备份区域 FLASH_IAP_START2
		}
		WriteNormalFileSizeToFlash();
	}
}



