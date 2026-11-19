#include "BSP.h"
#include "string.h"
#include "BSP.h"
#include "Drv_W25Qxx_QSPI.h"
#include "g_GlobalValDef.h"
#include "RYDW_Postion.h"
uint8_t g_SaveSendEmptyFlag;	//包发送完成标记 =1表示该包已全部写完，并且发送到最后一包
uint8_t g_SaveSendEmptyOffset;	

/***************************************************************************************
函数名称: void FindNeedSendCnt(DATAINFLASH *datainflash)
功能描述: 找内存中未发送的数据个数
参数输入: *datainflash-flash存储读取相关结构体
参数输出: 无
返 回 值: 无
其    他: 
***************************************************************************************/
void FindNeedSendCnt(DATAINFLASH *datainflash)
{
	uint16_t i,j;
	uint8_t tmpbuf[128];

	for(i=0;i<datainflash->flashlen/datainflash->baglen;i++)
	{	//读标记
		W25qxx_ReadBytes(tmpbuf,datainflash->flashstartaddr+datainflash->baglen*i+datainflash->datacnt*datainflash->datalen,datainflash->datacnt*2+1);
		if(tmpbuf[datainflash->datacnt*2] != BAG_SEND_EMPTY)								//先判断是否已全部发送完毕
		{
			for(j=0;j<datainflash->datacnt;j++)					
			{
				if(tmpbuf[j] == INDEX_FLASH_NOT_EMPTY && tmpbuf[j+datainflash->datacnt] != INDEX_SEND_SUC)		//再判断是否有未发送的数据
				{
					datainflash->needsendcnt++;
				}
			}
		}
	}
}

/***************************************************************************************
函数名称: void FindWriteDataAddr(DATAINFLASH *datainflash)
功能描述: 找写记录的起始地址
参数输入: *datainflash-flash存储读取相关结构体
参数输出: 无
返 回 值: 无
其    他: 
***************************************************************************************/
void FindWriteDataAddr(DATAINFLASH *datainflash)
{
	uint16_t i,j;
	uint8_t tmpbuf[128];
	
	for(i=0;i<datainflash->flashlen/datainflash->baglen;i++)
	{	//读标记
		W25qxx_ReadBytes(tmpbuf,datainflash->flashstartaddr+datainflash->baglen*i+datainflash->datacnt*datainflash->datalen,datainflash->datacnt*2+1);
		if(tmpbuf[datainflash->datacnt*2] != BAG_SEND_EMPTY)	//先判断是否已全部发送完毕
		{
			for(j=0;j<datainflash->datacnt;j++)					
			{
				if(tmpbuf[j] == INDEX_FLASH_IS_EMPTY)			//再判断写标记是否可写入
				{
					datainflash->writedataaddr = datainflash->flashstartaddr+datainflash->baglen*i+datainflash->datalen*j;
					datainflash->writeflagaddr = datainflash->flashstartaddr+datainflash->baglen*i+datainflash->datacnt*datainflash->datalen+j;
					return;
				}
			}
		}
		if(i == datainflash->flashlen/datainflash->baglen-1)
		{
			datainflash->writedataaddr = datainflash->flashstartaddr;
			datainflash->writeflagaddr = datainflash->flashstartaddr+datainflash->datacnt*datainflash->datalen;
		}
	}
	
}


/**********************************************************************************
函数名称: void SaveOneRecordToFlash(DATAINFLASH *datainflash,uint8_t* buf)
功能描述: 给某一页某偏移处写入数据,且写存储标记
参数输入: *datainflash-flash存储读取相关结构体,包含读写指针及参数 *wbuffer-数据包
参数输出: 无
返 回 值: 无
其    他: 
************************************************************************************/
void SaveOneRecordToFlash(DATAINFLASH *datainflash,uint8_t* buf)
{
	uint8_t tmpbuf;
	
	tmpbuf = INDEX_FLASH_NOT_EMPTY;
		
	FlashWriteData(buf,datainflash->writedataaddr,datainflash->datalen);	
	W25qxx_WriteByte(tmpbuf,datainflash->writeflagaddr);
	
}
/*************************************************************************************
函数名称: void SaveRecordToFlash(DATAINFLASH *datainflash,uint8_t *wbuffer)
功能描述: 保存数据记录到Flash中
参数输入: *datainflash-flash存储读取相关结构体,包含读写指针及参数 *wbuffer-数据包
参数输出: 无
返 回 值: 无
其    他: 
**************************************************************************************/
void SaveRecordToFlash(DATAINFLASH *datainflash,uint8_t *wbuffer)
{
    uint32_t dataaddr_temp,flagaddr_temp,datacnt_temp;
    
    dataaddr_temp = datainflash->writedataaddr;
	flagaddr_temp = datainflash->writeflagaddr;
	datacnt_temp = datainflash->needsendcnt;
    if(dataaddr_temp < datainflash->flashstartaddr || dataaddr_temp >= (datainflash->flashstartaddr + datainflash->flashlen))
        return;
    if(datainflash->datalen == 0)
        return;
    
    if((dataaddr_temp-datainflash->flashstartaddr)%(w25qxx.SectorSize*2) == 0)  						//按照最大字节数(128字节一组数据占用2个sector)计算偏移
    {
		W25qxx_EraseSector(dataaddr_temp/w25qxx.SectorSize);
		W25qxx_EraseSector(dataaddr_temp/w25qxx.SectorSize+1);
		SaveOneRecordToFlash(datainflash,wbuffer);
        dataaddr_temp += datainflash->datalen;
		flagaddr_temp++;
		datacnt_temp++;
    }
    else if(((dataaddr_temp-datainflash->flashstartaddr)%datainflash->baglen)/datainflash->datalen == datainflash->datacnt-1) //若为一包中的最后一条
    {
		SaveOneRecordToFlash(datainflash,wbuffer);
		if((dataaddr_temp-datainflash->flashstartaddr)/datainflash->baglen == datainflash->flashlen/datainflash->baglen-1)//若为最后一包,写数据指针循环到第一包开始位置,不是最后一包,写数据指针移动到下一包开始位置
		{
			dataaddr_temp = datainflash->flashstartaddr;
		}
		else 
		{
			dataaddr_temp += datainflash->datalen*2;
		}
		flagaddr_temp = dataaddr_temp + datainflash->datalen*datainflash->datacnt;
        datacnt_temp++;
    }
    else																//不是特殊情况写数据指针移动一条数据长度
    {
		SaveOneRecordToFlash(datainflash,wbuffer);
		dataaddr_temp += datainflash->datalen;
		flagaddr_temp++;
		datacnt_temp++;
    }
    datainflash->writedataaddr = dataaddr_temp;
    datainflash->writeflagaddr = flagaddr_temp;
	datainflash->needsendcnt = datacnt_temp;
}


/********************************************************************************************
函数名称: uint32_t FindUpOrDownBagAddr(DATAINFLASH *datainflash,uint32_t inaddr,uint8_t flag)
功能描述: 找上一页或下一页
参数输入: inaddr是当前页地址，flag是1表示找上一页，为2表示找下一页
参数输出: 找到的页地址
返 回 值: 无
其    他: 
********************************************************************************************/
uint32_t FindUpOrDownBagAddr(DATAINFLASH *datainflash,uint32_t inaddr,uint8_t flag)
{
    uint32_t tempaddr;
    
	if(flag == 1)       //找上一页
	{
		if(inaddr == datainflash->flashstartaddr)//判断是否为flash起始位置
			tempaddr = datainflash->flashstartaddr+datainflash->flashlen-datainflash->baglen;
		else
			tempaddr = inaddr-datainflash->baglen;
	}
	else if(flag == 2)  //找下一页
	{
		if(inaddr == datainflash->flashstartaddr+datainflash->flashlen-datainflash->baglen)//判断是否为flash中最后一包
			tempaddr = datainflash->flashstartaddr;
		else
			tempaddr = inaddr+datainflash->baglen;
	}
    return tempaddr;
}


/***************************************************************************************
函数名称: uint8_t ReadRecordFromFlash(DATAINFLASH *datainflash,uint8_t *wbuffer)
功能描述: 从Flash中读要发送的数据记录
参数输入: *datainflash-flash存储读取相关结构体,包含读写指针及参数 *wbuffer-数据包
参数输出: 
返 回 值: 1：读成功，0，没有读卡记录
其    他: 
****************************************************************************************/
uint8_t ReadRecordFromFlash(DATAINFLASH *datainflash,uint8_t *wbuffer,uint8_t num)
{
    uint32_t addr_rstart,addr_rend,index_rstart,bagtotal,bagtemp;   				//读起始地址和结束地址,起始序号
    int16_t i,j;
	uint8_t tmpbuf[128];
    uint8_t cntl;
	cntl = 0;    
    if(datainflash->needsendcnt == 0)
        return 0;
	
	memset(tmpbuf,0,128);
    if((datainflash->writedataaddr-datainflash->flashstartaddr)%datainflash->baglen == 0)                 		//若写指针为第0条
    {
		addr_rstart = FindUpOrDownBagAddr(datainflash,datainflash->writedataaddr,1); //读起始地址
        addr_rend = FindUpOrDownBagAddr(datainflash,datainflash->writedataaddr,2);   //读结束地址
		index_rstart = 0;
		if(addr_rstart < addr_rend)													 //计算要读多少包
        {
            bagtotal = (addr_rstart+datainflash->flashlen-addr_rend)/datainflash->baglen+1;
        }
        else if(addr_rstart > addr_rend)
        {
            bagtotal = (addr_rstart-addr_rend)/datainflash->baglen+1;
        }
        else 
            return 0;
        bagtemp = addr_rstart;
        for(i=0;i<bagtotal;i++)                                        
        {
			W25qxx_ReadBytes(tmpbuf,bagtemp+datainflash->datalen*datainflash->datacnt,datainflash->datacnt*2+1);
			if(tmpbuf[datainflash->datacnt*2] != BAG_SEND_EMPTY)			//先判断该包是否已全部发送完
			{
				for(j = datainflash->datacnt-1;j >= 0;j--)
				{
					if(tmpbuf[j] == INDEX_FLASH_NOT_EMPTY)						//有写数据标记
					{
						if(tmpbuf[j+datainflash->datacnt] != INDEX_SEND_SUC)	//无发送标记
						{
							cntl++;
							if(num == cntl)
							{							
								W25qxx_ReadBytes(wbuffer,bagtemp+datainflash->datalen*j,datainflash->datalen);
								datainflash->readflagaddr = bagtemp+(datainflash->datalen+1)*datainflash->datacnt+j;
								g_ReadDataAddr = bagtemp+datainflash->datalen*j;
//								if(datainflash->needsendcnt == 1)
//								{
//									g_SaveSendEmptyFlag = 1;
//									g_SaveSendEmptyOffset = datainflash->datacnt-j;
//								}
								return 1;
							}
						}
					}
					else
					{
						break;
					}
					
//					if(j == 0)
//					{
//						W25qxx_WriteByte(BAG_SEND_EMPTY,bagtemp+(datainflash->datalen+2)*datainflash->datacnt);
//					}
				}				
			}
			bagtemp = FindUpOrDownBagAddr(datainflash,bagtemp,1);
        }
//        datainflash->needsendcnt = 0;
        return 0;
    }
	else
	{
		addr_rstart = datainflash->writedataaddr;
		addr_rend = FindUpOrDownBagAddr(datainflash,datainflash->writedataaddr,2);
        addr_rend = FindUpOrDownBagAddr(datainflash,addr_rend,2);
		index_rstart = (datainflash->writeflagaddr-datainflash->writedataaddr)%datainflash->datacnt;
		W25qxx_ReadBytes(tmpbuf,addr_rstart+datainflash->datalen*index_rstart,datainflash->datacnt*2+1);
		if(tmpbuf[datainflash->datacnt*2] != BAG_SEND_EMPTY)
		{
			for(i = datainflash->datacnt-index_rstart;i >= 0;i--)
			{
				if(tmpbuf[i] == INDEX_FLASH_NOT_EMPTY && tmpbuf[i+datainflash->datacnt] != INDEX_SEND_SUC)
				{					
					cntl++;
					if(num == cntl)
					{							
						W25qxx_ReadBytes(wbuffer,addr_rstart-(datainflash->datacnt-index_rstart)*datainflash->datalen+i*datainflash->datalen,datainflash->datalen);
						datainflash->readflagaddr = addr_rstart+datainflash->datalen*index_rstart+datainflash->datacnt+i;
						g_ReadDataAddr = addr_rstart-(datainflash->datacnt-index_rstart)*datainflash->datalen+i*datainflash->datalen;
						return 1;
					}										
				}
			}
		}	
		addr_rstart = FindUpOrDownBagAddr(datainflash,addr_rstart-(datainflash->datacnt-index_rstart)*datainflash->datalen,1);
		if(addr_rstart < addr_rend)
        {
			bagtotal = (addr_rstart+datainflash->flashlen-addr_rend)/datainflash->baglen+1;
        }
        else if(addr_rstart > addr_rend)
        {
			bagtotal = (addr_rstart-addr_rend)/datainflash->baglen+1;
        }
        else 
            return 0;		
		bagtemp = addr_rstart;
        for(i=0;i<bagtotal;i++)                                        
        {
			W25qxx_ReadBytes(tmpbuf,bagtemp+datainflash->datalen*datainflash->datacnt,datainflash->datacnt*2+1);
			if(tmpbuf[datainflash->datacnt*2] != BAG_SEND_EMPTY)
			{
				for(j = datainflash->datacnt-1;j >= 0;j--)
				{
					if(tmpbuf[j] == INDEX_FLASH_NOT_EMPTY)
					{
						if(tmpbuf[j+datainflash->datacnt] != INDEX_SEND_SUC)
						{
							cntl++;
							if(num == cntl)
							{							
								W25qxx_ReadBytes(wbuffer,bagtemp+datainflash->datalen*j,datainflash->datalen);
								datainflash->readflagaddr = bagtemp+(datainflash->datalen+1)*datainflash->datacnt+j;
								g_ReadDataAddr = bagtemp+datainflash->datalen*j;
//								if(datainflash->needsendcnt == 1)
//								{
//									g_SaveSendEmptyFlag = 1;
//									g_SaveSendEmptyOffset = datainflash->datacnt-j;
//								}
								return 1;
							}
						}
					}
					else
					{
						break;
					}
					
//					if(j == 0)
//					{
//						W25qxx_WriteByte(BAG_SEND_EMPTY,bagtemp+(datainflash->datalen+2)*datainflash->datacnt);
//					}
				}			
			}
			bagtemp = FindUpOrDownBagAddr(datainflash,bagtemp,1);
        }
//        datainflash->needsendcnt = 0;
        return 0;
	}
}





