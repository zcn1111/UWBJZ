#include "main.h"
#include "Update.h"
#include "Drv_W25Qxx_QSPI.h"
#include "DTU_Comm.h"
#include "g_GlobalValDef.h"
//#include "tool.h"

//UPDATAPROGRAM UpdateData;                 //客服工具升级的数据
UPDATESTATES UpdateStates;                //客服工具升级的状态
UpdateFile NormalUpdateFileSize;          //正常文件大小
UpdateFile FixUpdateFileSize;     //固件文件大小
UpdateFile BootFileSize;          //agj
UpdateFile VoiceFileSize;         
UpdateFile 	DiffUpdateFileSize;			//增量升级主程序文件大小
uint8_t UpdateBuf[1488];                  //客服工具升级包数据储存 
TickTimerPara_t UpdateWaitDelay;          //客服工具重新索要数据包等待时间

UpdatePara_t g_UpdatePara;

/*升级参数*/
uint8_t       g_UpdateVoiceTypeFlag;  //升级语音程序标志
uint8_t       g_UpFlag;
uint8_t       g_ShouldUpdateVoiceFlag;
/******************************************************************
函数名称: void Update_CalcStartPktOrder
功能描述: 计算接收开始包序号
参数输入: UpdatePara_t	*Para	升级参数
		  uint8_t		*Buf	接受缓存
参数输出: 无
返 回 值: 
其    他: 
******************************************************************/
void Update_CalcStartPktOrder(UpdatePara_t *Para, uint8_t *Buf)
{
	uint16_t totalpkt;
	
	/* 版本号不一致 重新开始升级 */
	if ((Para->MajorVersion != Buf[15]) || (Para->MinorVersion != Buf[16]))
	{
		Para->RequestPktOrder = 0;
		Para->RecvdPktOrder = 0;
		
	}
	else
	{
		/* 包总数不一样 重新开始升级 */
		totalpkt = ConvArrayToUint16(Buf, 17, M_LITTLE_ENDIAN);
		if (totalpkt != Para->TotalPktNbr)
		{
			Para->RequestPktOrder = 0;
			Para->RecvdPktOrder = 0;
		}
	}
	
}

/******************************************************************
函数名称: int16_t Update_InitUpdatePara
功能描述: 升级参数初始化
参数输入: UpdatePara_t	*Para	升级参数
参数输出: 无
返 回 值: int16_t	升级状态
其    他: 
******************************************************************/
int16_t Update_InitUpdatePara(UpdatePara_t *Para)
{
//	Para->RecvdPktOrder = 0;
//	Para->RequestPktOrder = 0;
//	Para->TotalPktNbr = 0;
//	
//	Para->MajorVersion = 0;
//	Para->MinorVersion = 0;
//	Para->RetryCnt = 0;
//	Para->RetryWaiting = 20;
//	
//	Para->ProgramType = PROG_CODETYPE;
//	Para->UpdateProgress = UPD_PROG_NONE;
//	
//	return UPD_STA_OK;
}

/******************************************************************
函数名称: int16_t Update_RecvData
功能描述: 接收升级数据
参数输入: UpdatePara_t	*Para	升级参数
		  uint8_t		*DatBuf	数据缓存
参数输出: 无
返 回 值: int16_t	升级状态
其    他: 
******************************************************************/
int16_t Update_RecvData(UpdatePara_t *Para, uint8_t *DatBuf)
{
//	uint16_t len;
//	uint16_t i;
//	uint8_t type=0;
//	uint8_t chksum=0;
//	uint16_t flashpage;
//	uint32_t pageaddr;
//	
//	int16_t ret;
//	
//	/* 接收数据帧头 */
//	type = 0;
//	ret = UPD_ERR_PKTHEAD;
//	for (i=0; i<8; i++)
//	{
//		if (DatBuf[i] == 0xEE)
//			type = 1;
//		else
//		{
//			type = 0;
//			break;
//		}
//	}
//	if (type == 0)
//	{
//		for (i=0; i<7; i++)
//		{
//			if (DatBuf[i] == 0xDC)
//				type = 2;
//			else
//			{
//				type = 0;
//				break;
//			}
//		}
//	}
//	if (type == 0)
//		return UPD_ERR_PKTHEAD;
//	
//	/* 处理数据 */
//	/* 升级下发命令 */
//	if (type == 1)
//	{
//		chksum = CalcChecksum(DatBuf, 15);
//		if (chksum == DatBuf[15])
//		{
//			/* 校验程序类型 */
//			if (DatBuf[8] != PROG_CODETYPE)
//				return UPD_ERR_PROGTYPE;
//			
//			Para->ProgramType = DatBuf[8];
//			Para->MajorVersion = DatBuf[9];
//			Para->MinorVersion = DatBuf[10];
//			Para->TotalPktNbr = ConvArrayToUint16(DatBuf, 11, M_LITTLE_ENDIAN);
//			Update_CalcStartPktOrder(Para, DatBuf);
//			
//			Para->FlashSaveBuf = UpdateBuf;
//			Para->RecvdPktOrder = 0;
//			Para->UnSavedDataLen = 0;
//			Para->UpdateProgress = UPD_PROG_UPDATING;
//			Para->RetryCnt = 0;
//			ret = UPD_STA_RECVCMD;
//		}
//		else
//			ret = UPD_ERR_CHKSUM;
//	}
//	/* 升级数据包 */
//	else if (type == 2)
//	{
//		chksum = CalcChecksum(DatBuf, 511);
//		if (chksum == DatBuf[511])
//		{
//			if (Para->ProgramType != DatBuf[7])
//				return UPD_ERR_PROGTYPE;
//			if (Para->TotalPktNbr != ConvArrayToUint16(DatBuf, 10, M_LITTLE_ENDIAN))
//				return UPD_ERR_PKTORDER;
//			if (Para->RequestPktOrder != ConvArrayToUint16(DatBuf, 12, M_LITTLE_ENDIAN))
//				return UPD_ERR_PKTORDER;
//			
//			/* 第一包数据开始接收 擦除所需Flash */
//			if (Para->RecvdPktOrder == 0)
//			{
//				Para->CurFlashPageNo = 0;
//				Para->UnSavedDataLen = 0;
//				flashpage = (Para->TotalPktNbr * 496) / w25qxx.SectorSize + 1;		//计算需要擦除的Flash扇区数
//				for (i=0; i<flashpage; i++)
//				{
//					W25qxx_EraseSector(FLASH_IAP_START1 / w25qxx.SectorSize + i);	//按扇区擦除
//				}
//			}
//			/* 写入Flash */
//			memcpy(Para->FlashSaveBuf + Para->UnSavedDataLen, DatBuf + 14, 496);			//复制数据到缓存
//			Para->UnSavedDataLen += 496;
//			if (Para->UnSavedDataLen >= 512)
//			{
//				pageaddr = FLASH_IAP_START1 / 256 + Para->CurFlashPageNo * 2;
//				/* 写入Flash */
//				W25qxx_WritePage(Para->FlashSaveBuf, pageaddr, 0, 256);
//				W25qxx_WritePage(Para->FlashSaveBuf + 256, pageaddr + 1, 0, 256);
//				Para->CurFlashPageNo += 1;
//				memcpy(Para->FlashSaveBuf, Para->FlashSaveBuf + 512, Para->UnSavedDataLen - 512);		//复制剩余字节
//				Para->UnSavedDataLen -= 512;
//			}
//			Para->RecvdPktOrder += 1;		//已接收的包序号
//			Para->RetryCnt = 0;
//			Para->UpdateProgress = UPD_PROG_UPDATING;
//			ret = UPD_STA_RECVDAT;
//			/* 最后一包数据 写入剩余部分 */
//			if (Para->RecvdPktOrder == Para->TotalPktNbr)
//			{
//				if (Para->UnSavedDataLen != 0)
//				{
//					pageaddr = FLASH_IAP_START1 / 256 + Para->CurFlashPageNo * 2;
//					memset(Para->FlashSaveBuf + Para->UnSavedDataLen, 0xFF, (1488 - Para->UnSavedDataLen));	//剩余部分清0xFF
//					W25qxx_WritePage(Para->FlashSaveBuf, pageaddr, 0, 256);
//					W25qxx_WritePage(Para->FlashSaveBuf + 256, pageaddr + 1, 0, 256);
//					Para->RecvdPktOrder += 1;
//				}
//				/* 写入文件大小 */
//				g_NormalUpdateFilePara.FileSize = Para->RecvdPktOrder;
////				WriteNormalFileSizeToFlash();
//				Para->UpdateProgress = UPD_PROG_FINISH;
//				Para->RetryCnt = 0;
//				ret = UPD_STA_FINISH;
//			}
//		}
//		else
//			ret = UPD_ERR_CHKSUM;
//	}
//	
//	return ret;
}

///******************************************************************
//函数名称: int16_t Update_GenerateRequest
//功能描述: 生成升级索要数据
//参数输入: UpdatePara_t	*Para	升级参数
//		  uint8_t		*DatBuf	数据缓存
//参数输出: 无
//返 回 值: int16_t	升级状态
//其    他: 
//******************************************************************/
//int16_t Update_GenerateRequest(UpdatePara_t *Para, uint8_t *DatBuf)
//{
//	uint8_t chksum;
//	
//	DatBuf[0] = 0xEE;
//	memcpy(DatBuf + 1, g_MobileInFlash.BJnum.MobNum, 6);
//	DatBuf[7] = Para->ProgramType;
//	DatBuf[8] = Para->MajorVersion;
//	DatBuf[9] = Para->MinorVersion;
//	ConvUint16ToArray(Para->TotalPktNbr, DatBuf + 10, M_LITTLE_ENDIAN);
//	/* 索要包 */
//	Para->RequestPktOrder = Para->RecvdPktOrder;
//	ConvUint16ToArray(Para->RequestPktOrder, DatBuf + 12, M_LITTLE_ENDIAN);
//	DatBuf[14] = 0x00;
//	chksum = CalcChecksum(DatBuf, 15);
//	DatBuf[15] = chksum;
//	
//	return UPD_STA_OK;
//}

///******************************************************************
//函数名称: int16_t Update_GenerateFinish
//功能描述: 生成升级结束数据
//参数输入: UpdatePara_t	*Para	升级参数
//		  uint8_t		*DatBuf	数据缓存
//参数输出: 无
//返 回 值: int16_t	升级状态
//其    他: 
//******************************************************************/
//int16_t Update_GenerateFinish(UpdatePara_t *Para, uint8_t *DatBuf)
//{
//	uint8_t chksum;
//	
//	DatBuf[0] = 0xEE;
//	memcpy(DatBuf + 1, g_MobileInFlash.BJnum.MobNum, 6);
//	DatBuf[7] = Para->ProgramType;
//	DatBuf[8] = Para->MajorVersion;
//	DatBuf[9] = Para->MinorVersion;
//	ConvUint16ToArray(Para->TotalPktNbr, DatBuf + 10, M_LITTLE_ENDIAN);
//	/* 升级已结束 */
//	DatBuf[12] = 0xEE;
//	DatBuf[13] = 0xEE;
//	DatBuf[14] = 0x00;
//	chksum = CalcChecksum(DatBuf, 15);
//	DatBuf[15] = chksum;
//	
//	return UPD_STA_OK;
//}

///******************************************************************
//函数名称: void Update_Quest_Tool(void)
//功能描述: 通过客服工具升级程序的应答包
//参数输入: 无
//返    回: 无
//******************************************************************/
//void Update_Quest_Tool(void)
//{
//	memset(TxBuf3,0,sizeof(TxBuf3));
//	
//	TxBuf3[0] = 0xEE;
//	TxBuf3[1] = 0;
//	TxBuf3[2] = 0;
//	TxBuf3[3] = 0;
//	TxBuf3[4] = 0;
//	TxBuf3[5] = 0;
//	TxBuf3[6] = 0;
//	TxBuf3[7] = UpdateData.programtype;                       //程序类型
//	TxBuf3[8] = UpdateData.mainverno;                         //主版本号
//	TxBuf3[9] = UpdateData.secverno;                          //副版本号
//    TxBuf3[10] = (UpdateData.programpage & 0xFF );            //总包数
//	TxBuf3[11] = (UpdateData.programpage>>8);
//	
//	if(UpdateStates.bit.UpdateNormal == 1)
//	{
//		if(UpdateStates.bit.UpdateOver == 1)                  //接收完成
//		{
//			TxBuf3[12] = 0xEE;
//			TxBuf3[13] = 0xEE;	
//			if(UpdateData.dataLenToSave != 0)
//			{
//				memset(UpdateBuf + UpdateData.dataLenToSave,0xff,1488 - UpdateData.dataLenToSave);
//				W25qxx_WritePage(UpdateBuf,FLASH_IAP_START1/256+UpdateData.SavePageNo*2,0,256);
//				W25qxx_WritePage(UpdateBuf+256,FLASH_IAP_START1/256+UpdateData.SavePageNo*2+1,0,256);
//				//UpdataData.SavePageNo++;
//			}
//			NormalUpdateFileSize.FileSize = UpdateData.SavePageNo + 1;    //注意 由于boot程序与本程序循环计算不同，顾加1调整
//			WriteNormalFileSizeToFlash();                         //将升级文件页数写入w25中
//			UpdateStates.bit.UpgradeInProgress = 0;
//			SetupTickTimerSta_1s(TTM_MODE_SINGLE, TTM_STOP, &UpdateWaitDelay);
//		}
//		else
//		{
//			TxBuf3[12] = UpdateData.DownPacketNo & 0xff;
//			TxBuf3[13] = ( UpdateData.DownPacketNo>>8) & 0xff;		
//		}
//	}
//	else if(UpdateStates.bit.UpdateMistake == 1)
//	{
//		TxBuf3[12] = 0xEE;
//		TxBuf3[13] = 0xEE;
//		UpdateStates.bit.UpgradeInProgress = 0;
//	}
//	TxBuf3[14] = 0;	
//	TxBuf3[15] = CalcChecksum((uint8_t*)&TxBuf3, 15*2);
//	UartSend(&hUART7,(uint8_t*)&TxBuf3,16*2);
//}

///******************************************************************
//函数名称: void Update_Quest_DTU(MobileNum_t PhoneNum)
//功能描述: 通过DTU升级程序的应答包
//参数输入: PhoneNum  手机号BCD格式
//返    回: 无
//******************************************************************/
//void Update_Quest_DTU(MobileNum_t PhoneNum)
//{
//	MobileNum_t sendphone;
//	DataSendMode_t sendmode;
//	DevSelfSta_t sta;
//	
//	sta.all = 0x00;
//	
//	memset(g_DataDomainBuf, 0, sizeof(g_DataDomainBuf));
//	
//	sendphone = PhoneNum;
//	sendmode.all = 0x0F;
//	DTU_AddSendModeInPhoneNum(&sendphone, sendmode);	//处理最后一个字节低4位	
//	
//	g_DataDomainBuf[0] = 0xEE;
//	memcpy(g_DataDomainBuf+1, sendphone.MobNum, 6);		//复制手机号
//	memset(TxBuf3,0,sizeof(TxBuf3));
//	
//	g_DataDomainBuf[7] = UpdateData.programtype;                       //程序类型
//	g_DataDomainBuf[8] = UpdateData.mainverno;                         //主版本号
//	g_DataDomainBuf[9] = UpdateData.secverno;                          //次版本号
//    g_DataDomainBuf[10] = (UpdateData.programpage & 0xFF );            //总包数
//	g_DataDomainBuf[11] = (UpdateData.programpage>>8);
//	
//	if(UpdateStates.bit.UpdateNormal == 1)
//	{
//		if(UpdateStates.bit.UpdateOver == 1)                  //接收完成
//		{
//			g_DataDomainBuf[12] = 0xEE;
//			g_DataDomainBuf[13] = 0xEE;	
//			if(UpdateData.dataLenToSave != 0)
//			{
//				memset(UpdateBuf + UpdateData.dataLenToSave,0xff,1488 - UpdateData.dataLenToSave);
//				W25qxx_WritePage(UpdateBuf,FLASH_IAP_START1/256+UpdateData.SavePageNo*2,0,256);
//				W25qxx_WritePage(UpdateBuf+256,FLASH_IAP_START1/256+UpdateData.SavePageNo*2+1,0,256);
//				//UpdataData.SavePageNo++;
//			}
//			NormalUpdateFileSize.FileSize = UpdateData.SavePageNo + 1;    //注意 由于boot程序与本程序循环计算不同，顾加1调整
//			WriteNormalFileSizeToFlash();                         //将升级文件页数写入w25中
//			UpdateStates.bit.UpgradeInProgress = 0;
//			SetupTickTimerSta_1s(TTM_MODE_SINGLE, TTM_STOP, &UpdateWaitDelay);
//		}
//		else
//		{
//			g_DataDomainBuf[12] = UpdateData.DownPacketNo & 0xff;
//			g_DataDomainBuf[13] = (UpdateData.DownPacketNo>>8) & 0xff;		
//		}
//	}
//	else if(UpdateStates.bit.UpdateMistake == 1)
//	{
//		g_DataDomainBuf[12] = 0xEE;
//		g_DataDomainBuf[13] = 0xEE;
//		UpdateStates.bit.UpgradeInProgress = 0;
//	}
//	g_DataDomainBuf[14] = 0;	
//	g_DataDomainBuf[15] = CalcChecksum(g_DataDomainBuf, 15);
//	 DTU_SendRequestCmd(sta, TCODE_DEV_SENDDATA, 16);
//}
