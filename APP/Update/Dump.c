#include "Dump.h"
#include "Drv_W25Qxx_QSPI.h"
#include "DTU_Comm.h"
//#include "tool.h"

uint8_t 	*pDumpBuf;	//转储缓存
DumpPara_t	g_DumpPara;	//转储参数

uint8_t		g_DumpSendBuf[DUMP_SEND_BUFSIZE];
/******************************************************************
函数名称: DPResult Dump_InitDumpPara
功能描述: 转储参数初始化
参数输入: DumpPara_t		*DmpPara	转储参数
参数输出: 
返 回 值: DPResult
其    他:
******************************************************************/
DPResult Dump_InitDumpPara(DumpPara_t *DmpPara)
{
	DmpPara->DumpProgress = DUMP_PROG_NONE;
	DmpPara->DumpType = 0;
	
	DmpPara->PageSize = 65535;		//默认一页64KB
	DmpPara->TotalPageNbr = 0;
	DmpPara->RequestPageOrd = 0;	//索要页清零
	DmpPara->SendPageSize = 16384;	//默认发送一页16KB
	DmpPara->SendPageOrd = 0;
	DmpPara->SendPageOffset = 0;
	DmpPara->ChkSum = 0;
	
	memset(DmpPara->FileName, 0x00, 64);
	DmpPara->FileSizeInByte = 0;
	DmpPara->FileOffsetInByte = 0;
	
	DmpPara->RetryCnt = 0;
	DmpPara->RetryWaiting = 20;		//默认重试等待20s
	
	pDumpBuf = NULL;				//缓存初始化

	
	return DPRET_OK;
}

/******************************************************************
函数名称: DPResult Dump_SetupPageSize
功能描述: 配置转储大小参数
参数输入: DumpPara_t		*DmpPara	转储参数
参数输出: 
返 回 值: DPResult
其    他:
******************************************************************/
DPResult Dump_SetupPageSize(DumpPara_t *DmpPara, uint32_t PageSize, uint32_t SendPageSize, uint8_t *pBuf)
{
	DmpPara->PageSize = PageSize;			//转储页大小
	DmpPara->RequestPageOrd = 0;
	DmpPara->SendPageSize = SendPageSize;	//发送转储页大小
	DmpPara->SendPageOrd = 0;
	DmpPara->SendPageOffset = 0;
	
	/* 转储页大小表示客服工具或DTU一次性接收的数据大小
	   发送转储页大小表示主板一次发送数据的大小
	   因为主板内存的限制无法一次发送整页的数据，因此需要分次发送
	*/
//	if (pBuf == NULL)
//	{
//		pDumpBuf = malloc(DmpPara->SendPageSize);
//		if (pDumpBuf == NULL)
//		{
//			return DPRET_MEM_FAIL;							//申请内存失败
//		}
//	}
//	else
//	{
//		pDumpBuf = pBuf;
//	}
	
	return DPRET_OK;
}


/******************************************************************
函数名称: DPResult Dump_GenLocateDumpFilename
功能描述: 生成本地转储文件名
参数输入: DumpPara_t		*DmpPara	转储参数
参数输出: 
返 回 值: DPResult
其    他:
******************************************************************/
DPResult Dump_GenLocateDumpFilename(DumpPara_t *DmpPara, uint16_t TrainCode, HexDateTime_t NowTime, uint8_t MemSize)
{
	DmpPara->FileName[0] = (uint8_t)(TrainCode & 0xFF);
	DmpPara->FileName[1] = (uint8_t)((TrainCode >> 8) & 0xFF);
	DmpPara->FileName[2] = NowTime.Month;
	DmpPara->FileName[3] = NowTime.Day;
	DmpPara->FileName[4] = MemSize;
	
	return DPRET_OK;
}

/******************************************************************
函数名称: DPResult Dump_GenLocateDumpFilename
功能描述: 生成转储数据
参数输入: DumpPara_t		*DmpPara	转储参数
参数输出: 
返 回 值: DPResult
其    他:
******************************************************************/
DPResult Dump_GenDumpData(DumpPara_t *DmpPara, uint8_t *Buf)
{
	uint32_t staddr;
	uint32_t offset;
	
	uint32_t i;
	
	if (pDumpBuf == NULL)
	{
		return DPRET_MEM_FAIL;			//申请缓存失败
	}
	
	memset(pDumpBuf, 0x00, DmpPara->SendPageSize);											//清缓存
	staddr = (DmpPara->RequestPageOrd * DmpPara->PageSize) + DmpPara->SendPageOffset;		//计算页地址
	FlashReadData(pDumpBuf, staddr, DmpPara->SendPageSize, false);							//读数据
	DmpPara->SendPageOffset += DmpPara->SendPageSize;										//计算已发送偏移
	DmpPara->ChkSum = CalcSumByInitialVal(pDumpBuf, DmpPara->SendPageSize, DmpPara->ChkSum);//计算累加和
	if (DmpPara->SendPageOffset >= DmpPara->PageSize)
	{
		/* 已经满一页长度 */
//		DmpPara->SendPageOffset = 0;
//		DmpPara->SendPageOrd++;
		return DPRET_SEND_PAGE_FIN;
	}
	
	return DPRET_SEND_PACKAGE_FIN;
}

/******************************************************************
函数名称: DPResult Dump_RecvDataFromTool
功能描述: 从客服工具接收数据
参数输入: DumpPara_t		*DmpPara	转储参数
		  uint8_t		*InBuf		输入数据
参数输出: 
返 回 值: DPResult
其    他:
******************************************************************/
DPResult Dump_RecvDataFromTool(DumpPara_t *DmpPara, uint8_t *InBuf)
{
	uint8_t dumpsta=0;
	DPResult ret;
	
	/* 数据接收处理 */
	ret = DPRET_NODATA_ERR;
	/* 转储状态 = 无转储 */
	if (DmpPara->DumpProgress == DUMP_PROG_NONE)
	{
		/* 接收到客服工具转储命令 */
		if (InBuf[0] == 0xE0)
		{
			Dump_GenLocateDumpFilename(DmpPara, 0, g_SysTime, 8);
			ret = DPRET_TOOL_FILENAME;
		}
		/* 接收到开始转储命令 */
		else if (InBuf[0] == 0xE2)
		{
			DmpPara->DumpProgress = DUMP_PROG_DUMPING;
			Dump_SetupPageSize(DmpPara, 65535, DUMP_SEND_BUFSIZE, g_DumpSendBuf);	//配置大小
			ret = DPRET_TOOL_START;
		}
	}
	/* 转储状态 = 正在转储 */
	else if (DmpPara->DumpProgress == DUMP_PROG_DUMPING)
	{
		if (InBuf[0] == 0xF1)
		{
			DmpPara->DumpProgress = DUMP_PROG_FINISH;
			ret = DPRET_TOOL_FINISH;
		}
		else
		{
			/* 接收页号 */
			DmpPara->RequestPageOrd = InBuf[0];
			DmpPara->SendPageOrd = DmpPara->RequestPageOrd;
			DmpPara->SendPageOffset = 0;
			ret = DPRET_TOOL_NEWPAGE;
		}
	}
	/* 转储状态 = 转储结束 */
	else if (DmpPara->DumpProgress == DUMP_PROG_FINISH)
	{
		Dump_InitDumpPara(DmpPara);
	}
	
	return ret;
}



