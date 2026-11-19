#ifndef __BSP_STORE_H
#define __BSP_STORE_H

#include <stdint.h>
#include <stdbool.h>

#define INDEX_FLASH_NOT_EMPTY	0x11   
#define INDEX_SEND_SUC			0x22
#define INDEX_FLASH_IS_EMPTY	0xFF 

#define BAG_NOT_FULL			0xFF		//空标记,数据为填满或者未发送完成
#define BAG_SEND_EMPTY			0x33		//发送完毕标记

//1条数据8字节,一组数据中有25条,256字节为一组;存储方式为 数据(8*13)+写标记(25)+读标记(25)+空标记(1)+0(5)
#define DATALEN_8BYTES			8		
#define DATACNT_8BYTES			3		
#define BAGLEN_8BYTES			32
//1条数据16字节,一组数据中有14条,256字节为一组;存储方式为 数据(16*14)+写标记(14)+读标记(14)+空标记(1)+0(3)
#define DATALEN_16BYTES			16		
#define DATACNT_16BYTES			14		
#define BAGLEN_16BYTES			256		
//1条数据32字节,一组数据中有15条,512字节为一组,每页有8条数据,一组最后一页有7条数据,;存储方式为 数据(32*15)+写标记(15)+读标记(15)+空标记(1)+0(1)
#define DATALEN_32BYTES			32		
#define DATACNT_32BYTES			15		
#define BAGLEN_32BYTES			512		//
//1条数据64字节,一组数据中有31条,2048字节为一组;存储方式为 数据(64*31)+写标记(31)+读标记(31)+空标记(1)+0(1)
#define DATALEN_64BYTES			64		
#define DATACNT_64BYTES			31		
#define BAGLEN_64BYTES			2048		
//1条数据128字节,一组数据中有63条,8192字节为一组;存储方式为 数据(128*63)+写标记(63)+读标记(63)+空标记(1)+0(1)
#define DATALEN_128BYTES		128		
#define DATACNT_128BYTES		63		
#define BAGLEN_128BYTES			8192

typedef struct{
	uint32_t writedataaddr;		//写数据地址
	uint32_t writeflagaddr;		//写标记地址
	uint32_t readflagaddr;		//读标记地址
	uint32_t flashstartaddr;	//FLASH存储起始地址
	uint32_t flashlen;			//FLASH存储长度
	uint16_t datalen;			//数据长度
	uint16_t datacnt;			//一组数据存储条数
	uint16_t baglen;			//一组数据长度
	uint16_t needsendcnt;		//未发送条数
}DATAINFLASH;

extern uint32_t readdataaddr;
extern uint8_t g_SaveSendEmptyFlag;	//包发送完成标记 =1表示该包已全部写完，并且发送到最后一包
extern uint8_t g_SaveSendEmptyOffset;
void CountReaddataaddr(DATAINFLASH *datainflash);
void FindNeedSendCnt(DATAINFLASH *datainflash);
void FindWriteDataAddr(DATAINFLASH *datainflash);	//查找写数据地址
void SaveOneRecordToFlash(DATAINFLASH *datainflash,uint8_t* buf);
void SaveRecordToFlash(DATAINFLASH *datainflash,uint8_t *wbuffer);
uint32_t FindUpOrDownBagAddr(DATAINFLASH *datainflash,uint32_t inaddr,uint8_t flag);
uint8_t ReadRecordFromFlash(DATAINFLASH *datainflash,uint8_t *wbuffer,uint8_t num);
#endif /* __BSP_STORE_H */


