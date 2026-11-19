

#include "BSP.h"
#include "APP_STORE.h"
#include "Drv_W25Qxx_QSPI.h"
#include "g_GlobalValDef.h"
#include "RYDW_Postion.h"

DATAINFLASH g_PUNCOORDatainFlash;		//存储打点数据

void InitFlashPar(void)
{
	g_PUNCOORDatainFlash.flashstartaddr = FLASH_RYDW_CONPOINT_START;
	g_PUNCOORDatainFlash.flashlen = FLASH_RYDW_CONPOINT_LEN;
	g_PUNCOORDatainFlash.datalen = DATALEN_32BYTES;
	g_PUNCOORDatainFlash.datacnt = DATACNT_32BYTES;
	g_PUNCOORDatainFlash.baglen = BAGLEN_32BYTES;
	FindWriteDataAddr(&g_PUNCOORDatainFlash);
	FindNeedSendCnt(&g_PUNCOORDatainFlash);	
	
	g_SaveSendEmptyFlag	=0;
	g_SaveSendEmptyOffset = 0;
	g_ReadDataAddr = 0xFFFFFFFF;
	memset(g_ReadAddrPar,0xFF,sizeof(g_ReadAddrPar));	
}


