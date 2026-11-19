#include "w25qxxConf.h"
#include "Drv_W25Qxx_QSPI.h"
#include "bsp.h"
#include "main.h"

#define W25QXX_DUMMY_BYTE         0xA5

uint8_t g_tmpBuf[4096];
uint8_t g_FlashWriteBuffer[4096];  
uint8_t g_BoardType;
uint8_t g_FlashtempBuff[1024];
w25qxx_t	w25qxx;

#define	W25qxx_Delay(delay)		HAL_Delay(delay)



uint32_t W25qxx_ReadID(void)
{
	uint32_t deviceid=0;
	uint8_t temp[4]={0};
	uint8_t reg2;
	
	QSPI_SetupCmdHandle(QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);
	QSPI_SendCmd(W25X_ManufactDeviceID, 0, 0);
	QSPI_Receive(temp, 2);
	deviceid = (temp[0] << 8) | temp[1];
	
	return deviceid;
}

void W25qxx_ReadUniqID(void)
{
	uint8_t temp[8];
	for(uint8_t	i=0;i<4;i++)
	   temp[i] = W25QXX_DUMMY_BYTE;		
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	QSPI_Send_CMD(W25X_ReadUniqID, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);	
	QSPI_Transmit(temp,4);
	QSPI_Receive(temp,8);	
	for(uint8_t	i=0;i<8;i++)
		w25qxx.UniqID[i] = temp[i];
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
}


/****写使能**********************/
void W25qxx_WriteEnable(void)
{
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	QSPI_Send_CMD(W25X_WriteEnable, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_NONE);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_Delay(1);
}

void W25qxx_WriteDisable(void)
{
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
    QSPI_Send_CMD(W25X_WriteDisable, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_NONE);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_Delay(1);
}
/****读取W25QXX的状态寄存器*****************/
uint8_t W25qxx_ReadStatusRegister(uint8_t	SelectStatusRegister_1_2_3)
{
	uint8_t	status =0;

	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	if(SelectStatusRegister_1_2_3==1)
	{
		QSPI_Send_CMD(W25X_ReadStatusReg1, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);
		QSPI_Receive(&status,1);
		w25qxx.StatusRegister1 = status;
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
		QSPI_Send_CMD(W25X_ReadStatusReg2, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);		
		QSPI_Receive(&status,1);	
		w25qxx.StatusRegister2 = status;
	}
	else
	{
		QSPI_Send_CMD(W25X_ReadStatusReg3, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);				
		QSPI_Receive(&status,1);	
		w25qxx.StatusRegister3 = status;
	}	
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	return status;
}

void W25qxx_WriteStatusRegister(uint8_t	SelectStatusRegister_1_2_3,uint8_t Data)
{
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	if(SelectStatusRegister_1_2_3==1)
	{
        QSPI_Send_CMD(W25X_WriteStatusReg1, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);		
		w25qxx.StatusRegister1 = Data;
	}
	else if(SelectStatusRegister_1_2_3==2)
	{
        QSPI_Send_CMD(W25X_WriteStatusReg2, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);		
		w25qxx.StatusRegister2 = Data;
	}
	else
	{
        QSPI_Send_CMD(W25X_WriteStatusReg3, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);		
		w25qxx.StatusRegister3 = Data;
	}
	QSPI_Transmit(&Data,1);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
}


//W25QXX进入QSPI模式 
void W25QXX_Qspi_Enable(void)
{
	uint8_t stareg2;
	stareg2=W25qxx_ReadStatusRegister(2);		//先读出状态寄存器2的原始值
	if((stareg2&0X02)==0)						//QE位未使能
	{
		W25qxx_WriteEnable();					//写使能 
		stareg2|=1<<1;							//使能QE位		
		W25qxx_WriteStatusRegister(2,stareg2);	//写状态寄存器2
	}
	QSPI_Send_CMD(W25X_EnterQPIMode,0,0,QSPI_INSTRUCTION_1_LINE,QSPI_ADDRESS_NONE,QSPI_ADDRESS_8_BITS,QSPI_DATA_NONE);//写command指令,地址为0,无数据_8位地址_无地址_单线传输指令,无空周期,0个字节数据
//	W25QXX_QPI_MODE=1;				//标记QSPI模式
}

void W25qxx_WaitForWriteEnd(void)
{
	uint8_t status = 0;
	W25qxx_Delay(1);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	QSPI_Send_CMD(W25X_ReadStatusReg1, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_1_LINE);
	do
	{
		QSPI_Receive(&status,1);	
		w25qxx.StatusRegister1 = status;
		W25qxx_Delay(1);
	}
	while((w25qxx.StatusRegister1 & 0x01) == 0x01);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
}

bool	W25qxx_Init(void)
{
	w25qxx.Lock=1;	
	while(HAL_GetTick()<100)
		W25qxx_Delay(1);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_Delay(100);
	uint32_t	id;
	
	/* 退出QSPI模式 */
	QSPI_SetupCmdHandle(QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_NONE);
	QSPI_SendCmd(W25X_ExitQPIMode, 0, 0);
	id=W25qxx_ReadID();
	
	switch(id&0x0000FFFF)
	{
		case 0XC218:	// 	MX25Q256
			w25qxx.ID=W25Q256;
			w25qxx.BlockCount=512;
		break;
		case 0XEF18:	// 	w25q256
			w25qxx.ID=W25Q256;
			w25qxx.BlockCount=512;
		break;
		case 0XEF17:	// 	w25q128
			w25qxx.ID=W25Q128;
			w25qxx.BlockCount=256;
		break;
		case 0XEF16:	//	w25q64
			w25qxx.ID=W25Q64;
			w25qxx.BlockCount=128;
		break;
		case 0XEF15:	//	w25q32
			w25qxx.ID=W25Q32;
			w25qxx.BlockCount=64;
		break;
		case 0XEF14:	//	w25q16
			w25qxx.ID=W25Q16;
			w25qxx.BlockCount=32;
		break;
		case 0XEF13:	//	w25q80
			w25qxx.ID=W25Q80;
			w25qxx.BlockCount=16;
		break;
		default:
			w25qxx.Lock=0;	
			return false;
				
	}		
	w25qxx.PageSize=256;
	w25qxx.SectorSize=0x1000;
	w25qxx.SectorCount=w25qxx.BlockCount*16;
	w25qxx.PageCount=(w25qxx.SectorCount*w25qxx.SectorSize)/w25qxx.PageSize;
	w25qxx.BlockSize=w25qxx.SectorSize*16;
	w25qxx.CapacityInKiloByte=(w25qxx.SectorCount*w25qxx.SectorSize)/1024;
	W25qxx_ReadUniqID();
	W25qxx_ReadStatusRegister(1);
	W25qxx_ReadStatusRegister(2);
	W25qxx_ReadStatusRegister(3);

	w25qxx.Lock=0;	
	return true;	
}	

/****擦除整个芯片*************/
void	W25qxx_EraseChip(void)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;	

	W25qxx_WriteEnable();
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	QSPI_Send_CMD(W25X_ChipErase, 0, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_NONE, QSPI_ADDRESS_8_BITS, QSPI_DATA_NONE);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd();

	W25qxx_Delay(10);
	w25qxx.Lock=0;	
}

/****擦除一个扇区*************/
void W25qxx_EraseSector(uint32_t SectorAddr)
{
//	uint8_t Addr[4] = {0x00,0x00,0x00,0x00};
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;		

	
	W25qxx_WaitForWriteEnd();
	SectorAddr = SectorAddr * w25qxx.SectorSize;
	W25qxx_WriteEnable();
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
    QSPI_Send_CMD(W25X_SectorErase, SectorAddr, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_NONE);
	
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd();

	W25qxx_Delay(1);
	w25qxx.Lock=0;
}

/****64K擦除*************************/
void W25qxx_EraseBlock(uint32_t BlockAddr)
{
//	uint8_t Addr[4] = {0x00,0x00,0x00,0x00};
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;	
	
	W25qxx_WaitForWriteEnd();
	BlockAddr = BlockAddr * w25qxx.SectorSize*16;
	W25qxx_WriteEnable();
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
    QSPI_Send_CMD(W25X_BlockErase, BlockAddr, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_NONE);

	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd();

	W25qxx_Delay(1);
	w25qxx.Lock=0;
}

/***算页地址属于那一块/扇区等*********/
uint32_t	W25qxx_PageToSector(uint32_t	PageAddress)
{
	return ((PageAddress*w25qxx.PageSize)/w25qxx.SectorSize);
}


uint32_t	W25qxx_PageToBlock(uint32_t	PageAddress)
{
	return ((PageAddress*w25qxx.PageSize)/w25qxx.BlockSize);
}


uint32_t	W25qxx_SectorToBlock(uint32_t	SectorAddress)
{
	return ((SectorAddress*w25qxx.SectorSize)/w25qxx.BlockSize);
}


uint32_t	W25qxx_SectorToPage(uint32_t	SectorAddress)
{
	return (SectorAddress*w25qxx.SectorSize)/w25qxx.PageSize;
}


uint32_t	W25qxx_BlockToPage(uint32_t	BlockAddress)
{
	return (BlockAddress*w25qxx.BlockSize)/w25qxx.PageSize;
}


bool 	W25qxx_IsEmptyPage(uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_PageSize)
{
	while(w25qxx.Lock==1)
	W25qxx_Delay(1);
	w25qxx.Lock=1;	
	if(((NumByteToCheck_up_to_PageSize+OffsetInByte)>w25qxx.PageSize)||(NumByteToCheck_up_to_PageSize==0))
		NumByteToCheck_up_to_PageSize=w25qxx.PageSize-OffsetInByte;
	
	uint8_t	pBuffer[32];
	uint32_t	WorkAddress;
	uint32_t	i;
	for(i=OffsetInByte; i<w25qxx.PageSize; i+=sizeof(pBuffer))
	{
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		WorkAddress=(i+Page_Address*w25qxx.PageSize);
        QSPI_Send_CMD(W25X_FastReadData, WorkAddress, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

		QSPI_Receive(pBuffer,sizeof(pBuffer));		
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	
		for(uint8_t x=0;x<sizeof(pBuffer);x++)
		{
			if(pBuffer[x]!=0xFF)
				goto NOT_EMPTY;		
		}			
	}	
	if((w25qxx.PageSize+OffsetInByte)%sizeof(pBuffer)!=0)
	{
		i-=sizeof(pBuffer);
		for( ; i<w25qxx.PageSize; i++)
		{
			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
			WorkAddress=(i+Page_Address*w25qxx.PageSize);
            QSPI_Send_CMD(W25X_FastReadData, WorkAddress, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

		    QSPI_Receive(pBuffer,1);	
			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	
			if(pBuffer[0]!=0xFF)
				goto NOT_EMPTY;
		}
	}	

	w25qxx.Lock=0;
	return true;	
	NOT_EMPTY:

	w25qxx.Lock=0;
	return false;
}


bool 	W25qxx_IsEmptySector(uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_SectorSize)
{
	while(w25qxx.Lock==1)
	W25qxx_Delay(1);
	w25qxx.Lock=1;	
	if((NumByteToCheck_up_to_SectorSize>w25qxx.SectorSize)||(NumByteToCheck_up_to_SectorSize==0))
		NumByteToCheck_up_to_SectorSize=w25qxx.SectorSize;
	uint8_t	pBuffer[32];
	uint32_t	WorkAddress;
	uint32_t	i;
	for(i=OffsetInByte; i<w25qxx.SectorSize; i+=sizeof(pBuffer))
	{
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		WorkAddress=(i+Sector_Address*w25qxx.SectorSize);      
		QSPI_Send_CMD(W25X_FastReadData, WorkAddress, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

		QSPI_Receive(pBuffer,sizeof(pBuffer));
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	
		for(uint8_t x=0;x<sizeof(pBuffer);x++)
		{
			if(pBuffer[x]!=0xFF)
				goto NOT_EMPTY;		
		}			
	}	
	if((w25qxx.SectorSize+OffsetInByte)%sizeof(pBuffer)!=0)
	{
		i-=sizeof(pBuffer);
		for( ; i<w25qxx.SectorSize; i++)
		{
			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
			WorkAddress=(i+Sector_Address*w25qxx.SectorSize);     
			QSPI_Send_CMD(W25X_FastReadData, WorkAddress, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

		    QSPI_Receive(pBuffer,1);
			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	
			if(pBuffer[0]!=0xFF)
				goto NOT_EMPTY;
		}
	}	

	w25qxx.Lock=0;
	return true;	
	NOT_EMPTY:

	w25qxx.Lock=0;
	return false;
}

bool 	W25qxx_IsEmptyBlock(uint32_t Block_Address,uint32_t OffsetInByte,uint32_t NumByteToCheck_up_to_BlockSize)
{
	while(w25qxx.Lock==1)
	W25qxx_Delay(1);
	w25qxx.Lock=1;	
	if((NumByteToCheck_up_to_BlockSize>w25qxx.BlockSize)||(NumByteToCheck_up_to_BlockSize==0))
		NumByteToCheck_up_to_BlockSize=w25qxx.BlockSize;
	uint8_t	pBuffer[32];
	uint32_t	WorkAddress;
	uint32_t	i;
	for(i=OffsetInByte; i<w25qxx.BlockSize; i+=sizeof(pBuffer))
	{
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		WorkAddress=(i+Block_Address*w25qxx.BlockSize);   
		QSPI_Send_CMD(W25X_FastReadData, WorkAddress, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);
		QSPI_Receive(pBuffer,sizeof(pBuffer));
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	
		for(uint8_t x=0;x<sizeof(pBuffer);x++)
		{
			if(pBuffer[x]!=0xFF)
				goto NOT_EMPTY;		
		}			
	}	
	if((w25qxx.BlockSize+OffsetInByte)%sizeof(pBuffer)!=0)
	{
		i-=sizeof(pBuffer);
		for( ; i<w25qxx.BlockSize; i++)
		{
			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
			WorkAddress=(i+Block_Address*w25qxx.BlockSize);     
			QSPI_Send_CMD(W25X_FastReadData, WorkAddress, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

		    QSPI_Receive(pBuffer,1);
			HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	
			if(pBuffer[0]!=0xFF)
				goto NOT_EMPTY;
		}
	}	

	w25qxx.Lock=0;
	return true;	
	NOT_EMPTY:

	w25qxx.Lock=0;
	return false;
}

void W25qxx_WriteByte(uint8_t pBuffer, uint32_t WriteAddr_inBytes)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;
	
	W25qxx_WaitForWriteEnd();
	W25qxx_WriteEnable();
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
    QSPI_Send_CMD(W25X_PageProgram, WriteAddr_inBytes, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

	QSPI_Transmit(&pBuffer,1);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd();
	W25qxx_Delay(1);
	w25qxx.Lock=0;
}



/**************增加读出来和写进去的做比较***********************************************/
void W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t WriteAddr_inBytes, uint32_t NumByteToWrite)
{
	if(NumByteToWrite > 0)
	{
		while(w25qxx.Lock==1)
			W25qxx_Delay(1);
		w25qxx.Lock=1;
	//	uint8_t Addr[4] = {0x00,0x00,0x00,0x00};
		
		W25qxx_WaitForWriteEnd();
		W25qxx_WriteEnable();
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		QSPI_Send_CMD(W25X_PageProgram, WriteAddr_inBytes, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);
		
		QSPI_Transmit(pBuffer,NumByteToWrite);
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
		W25qxx_WaitForWriteEnd();
		W25qxx_Delay(1);
		w25qxx.Lock=0;	
	}
}

void 	W25qxx_WritePage(uint8_t *pBuffer	,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToWrite_up_to_PageSize)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;
	if(((NumByteToWrite_up_to_PageSize+OffsetInByte)>w25qxx.PageSize)||(NumByteToWrite_up_to_PageSize==0))
		NumByteToWrite_up_to_PageSize=w25qxx.PageSize-OffsetInByte;
	if((OffsetInByte+NumByteToWrite_up_to_PageSize) > w25qxx.PageSize)
		NumByteToWrite_up_to_PageSize = w25qxx.PageSize-OffsetInByte;
	
	W25qxx_WaitForWriteEnd();
	W25qxx_WriteEnable();
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
    Page_Address = (Page_Address*w25qxx.PageSize)+OffsetInByte;	
	QSPI_Send_CMD(W25X_PageProgram, Page_Address, 0, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

	QSPI_Transmit(pBuffer,NumByteToWrite_up_to_PageSize);	
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);
	W25qxx_WaitForWriteEnd();

	W25qxx_Delay(1);
	w25qxx.Lock=0;
}

void 	W25qxx_WriteSector(uint8_t *pBuffer	,uint32_t Sector_Address,uint32_t OffsetInByte	,uint32_t NumByteToWrite_up_to_SectorSize)
{
	uint32_t	StartPage;
	int32_t		BytesToWrite;
	uint32_t	LocalOffset;
	
	if((NumByteToWrite_up_to_SectorSize>w25qxx.SectorSize)||(NumByteToWrite_up_to_SectorSize==0))
		NumByteToWrite_up_to_SectorSize=w25qxx.SectorSize;

	if(OffsetInByte>=w25qxx.SectorSize)
	{
		return;
	}	
		
	if((OffsetInByte+NumByteToWrite_up_to_SectorSize) > w25qxx.SectorSize)
		BytesToWrite = w25qxx.SectorSize-OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_SectorSize;	
	StartPage = W25qxx_SectorToPage(Sector_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_WritePage(pBuffer,StartPage,LocalOffset,BytesToWrite);
		StartPage++;
		BytesToWrite-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToWrite>0);		

}

void 	W25qxx_WriteBlock	(uint8_t* pBuffer ,uint32_t Block_Address	,uint32_t OffsetInByte	,uint32_t	NumByteToWrite_up_to_BlockSize)
{
	if((NumByteToWrite_up_to_BlockSize>w25qxx.BlockSize)||(NumByteToWrite_up_to_BlockSize==0))
		NumByteToWrite_up_to_BlockSize=w25qxx.BlockSize;

	if(OffsetInByte>=w25qxx.BlockSize)
	{

		return;
	}	
	uint32_t	StartPage;
	int32_t		BytesToWrite;
	uint32_t	LocalOffset;	
	if((OffsetInByte+NumByteToWrite_up_to_BlockSize) > w25qxx.BlockSize)
		BytesToWrite = w25qxx.BlockSize-OffsetInByte;
	else
		BytesToWrite = NumByteToWrite_up_to_BlockSize;	
	StartPage = W25qxx_BlockToPage(Block_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_WritePage(pBuffer,StartPage,LocalOffset,BytesToWrite);
		StartPage++;
		BytesToWrite-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToWrite>0);		

}

void 	W25qxx_ReadByte(uint8_t *pBuffer,uint32_t Bytes_Address)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
//	uint8_t Addr[4] = {0x00,0x00,0x00,0x00};
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
    QSPI_Send_CMD(W25X_FastReadData, Bytes_Address, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

	QSPI_Receive(pBuffer,1);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);	

	w25qxx.Lock=0;
}

void W25qxx_ReadBytes(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead)
{	
	if(NumByteToRead > 0)
	{
		while(w25qxx.Lock==1)
			W25qxx_Delay(1);
		w25qxx.Lock=1;
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
		QSPI_Send_CMD(W25X_FastReadData, ReadAddr, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);
		QSPI_Receive(pBuffer,NumByteToRead);
		   
		HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);

		W25qxx_Delay(1);
		w25qxx.Lock=0;		
	}
}


void 	W25qxx_ReadPage(uint8_t *pBuffer,uint32_t Page_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_PageSize)
{
	while(w25qxx.Lock==1)
		W25qxx_Delay(1);
	w25qxx.Lock=1;
	if((NumByteToRead_up_to_PageSize>w25qxx.PageSize)||(NumByteToRead_up_to_PageSize==0))
		NumByteToRead_up_to_PageSize=w25qxx.PageSize;
	if((OffsetInByte+NumByteToRead_up_to_PageSize) > w25qxx.PageSize)
		NumByteToRead_up_to_PageSize = w25qxx.PageSize-OffsetInByte;

	Page_Address = Page_Address*w25qxx.PageSize+OffsetInByte;
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_RESET);
	QSPI_Send_CMD(W25X_FastReadData, Page_Address, 8, QSPI_INSTRUCTION_1_LINE, QSPI_ADDRESS_1_LINE, QSPI_ADDRESS_24_BITS, QSPI_DATA_1_LINE);

	QSPI_Receive(pBuffer,NumByteToRead_up_to_PageSize);
	HAL_GPIO_WritePin(_W25QXX_CS_GPIO,_W25QXX_CS_PIN,GPIO_PIN_SET);

	W25qxx_Delay(1);
	w25qxx.Lock=0;
}

void 	W25qxx_ReadSector(uint8_t *pBuffer,uint32_t Sector_Address,uint32_t OffsetInByte,uint32_t NumByteToRead_up_to_SectorSize)
{	
	if((NumByteToRead_up_to_SectorSize>w25qxx.SectorSize)||(NumByteToRead_up_to_SectorSize==0))
		NumByteToRead_up_to_SectorSize=w25qxx.SectorSize;

	if(OffsetInByte>=w25qxx.SectorSize)
	{

		return;
	}	
	uint32_t	StartPage;
	int32_t		BytesToRead;
	uint32_t	LocalOffset;	
	if((OffsetInByte+NumByteToRead_up_to_SectorSize) > w25qxx.SectorSize)
		BytesToRead = w25qxx.SectorSize-OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_SectorSize;	
	StartPage = W25qxx_SectorToPage(Sector_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_ReadPage(pBuffer,StartPage,LocalOffset,BytesToRead);
		StartPage++;
		BytesToRead-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToRead>0);		

}

void 	W25qxx_ReadBlock(uint8_t* pBuffer,uint32_t Block_Address,uint32_t OffsetInByte,uint32_t	NumByteToRead_up_to_BlockSize)
{
	if((NumByteToRead_up_to_BlockSize>w25qxx.BlockSize)||(NumByteToRead_up_to_BlockSize==0))
		NumByteToRead_up_to_BlockSize=w25qxx.BlockSize;
	

	if(OffsetInByte>=w25qxx.BlockSize)
	{
		
		return;
	}	
	uint32_t	StartPage;
	int32_t		BytesToRead;
	uint32_t	LocalOffset;	
	if((OffsetInByte+NumByteToRead_up_to_BlockSize) > w25qxx.BlockSize)
		BytesToRead = w25qxx.BlockSize-OffsetInByte;
	else
		BytesToRead = NumByteToRead_up_to_BlockSize;	
	StartPage = W25qxx_BlockToPage(Block_Address)+(OffsetInByte/w25qxx.PageSize);
	LocalOffset = OffsetInByte%w25qxx.PageSize;	
	do
	{		
		W25qxx_ReadPage(pBuffer,StartPage,LocalOffset,BytesToRead);
		StartPage++;
		BytesToRead-=w25qxx.PageSize-LocalOffset;
		pBuffer += w25qxx.PageSize - LocalOffset;
		LocalOffset=0;
	}while(BytesToRead>0);		
	
}



/******************************************************************
函数名称: void Soft_SPIFlashWrite(u8* Pbuffer,u32 Writeaddr,u16 NumberByteToWrite)
功能描述: 写入函数，具有自动换页功能
参数输入:    pBuffer			数据存储区
             WriteAddr		开始写入的地址(最大32bit)
             NumByteToWrite	要写入的字节数(最大65535)
参数输出: 
返回值 : 无
其他: 无
******************************************************************/
void W25qxx_WritePages(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint16_t pageremain;
    pageremain = 256 - WriteAddr % 256; //单页剩余的字节数

    if(NumByteToWrite > 0)
	{
		if(NumByteToWrite <= pageremain)pageremain = NumByteToWrite; //不大于256个字节

		while(1)
		{
			W25qxx_WriteBytes(pBuffer, WriteAddr, pageremain);

			if(NumByteToWrite == pageremain)break; //写入结束了

			else //NumByteToWrite>pageremain
			{
				pBuffer += pageremain;
				WriteAddr += pageremain;

				NumByteToWrite -= pageremain;			 //减去已经写入了的字节数

				if(NumByteToWrite > 256)pageremain = 256; //一次可以写入256个字节

				else pageremain = NumByteToWrite; 	 //不够256个字节了
			}
		}       		
	}
}

bool FlashWriteData(uint8_t *pBuffer, uint32_t WriteAddr_inBytes, uint32_t NumByteToWrite)
{
	uint8_t tmp;	
	tmp = 3;

	uint32_t offset_in_sector;
	uint32_t write_in_sector;	//扇区内写字节数
	uint32_t sector;
	uint16_t i;
	
	uint8_t tmpbuf;
	uint8_t blank;
	uint8_t oversector;
	
	sector = WriteAddr_inBytes / w25qxx.SectorSize;				//计算开始字节偏移的扇区
	offset_in_sector = (WriteAddr_inBytes) % w25qxx.SectorSize;	//计算开始扇区偏移
	
	/* 判断跨扇区 */
	oversector = 0;
	if (((WriteAddr_inBytes + NumByteToWrite) / w25qxx.SectorSize) != sector)	//结束字节扇区号和开始扇区号不一致
		oversector = 1;
	
	/* 判断当前Flash位置是否为空 */
	blank = 1;
	for (i=0; i<NumByteToWrite; i++)
	{
		W25qxx_ReadBytes(&tmpbuf, WriteAddr_inBytes + i, 1);
		if (tmpbuf != 0xFF)
		{
			blank = 0;
			break;
		}
	}
	
	/* 当前Flash位置为空 直接写 */
	if (blank == 1)
	{
		W25qxx_WritePages(pBuffer, WriteAddr_inBytes, NumByteToWrite);
	}
	
	/* 当前Flash位置不为空 采用 读-改-写 方式 */
	if (blank == 0)
	{
		/* 不跨扇区 直接写 */
		if (oversector == 0)
		{
			memset(g_FlashWriteBuffer, 0xFF, sizeof(g_FlashWriteBuffer));
			W25qxx_ReadSector(g_FlashWriteBuffer, sector, 0, w25qxx.SectorSize);	//读整扇区
			for (i=0; i<NumByteToWrite; i++)
			{
				g_FlashWriteBuffer[offset_in_sector + i] = pBuffer[i];				//修改
			}
			W25qxx_EraseSector(sector);												//擦除扇区
			W25qxx_WritePages(g_FlashWriteBuffer, sector * w25qxx.SectorSize, w25qxx.SectorSize);		//写当前扇区
		}
		/* 跨扇区 分两次写 */
		else if (oversector == 1)
		{
			/* 写前扇区 */
			write_in_sector = (w25qxx.SectorSize - offset_in_sector);
			memset(g_FlashWriteBuffer, 0xFF, sizeof(g_FlashWriteBuffer));
			W25qxx_ReadSector(g_FlashWriteBuffer, sector, 0, w25qxx.SectorSize);	//读整扇区
			for (i=0; i<write_in_sector; i++)
			{
				g_FlashWriteBuffer[offset_in_sector + i] = pBuffer[i];
			}
			W25qxx_EraseSector(sector);
			W25qxx_WritePages(g_FlashWriteBuffer, sector * w25qxx.SectorSize, w25qxx.SectorSize);
			/* 写后扇区 */
			write_in_sector = (NumByteToWrite - (w25qxx.SectorSize - offset_in_sector));
			memset(g_FlashWriteBuffer, 0xFF, sizeof(g_FlashWriteBuffer));
			W25qxx_ReadSector(g_FlashWriteBuffer, (sector + 1), 0, w25qxx.SectorSize);
			for (i=0; i<write_in_sector; i++)
			{
				g_FlashWriteBuffer[i] = pBuffer[i + (w25qxx.SectorSize - offset_in_sector)];
			}
			W25qxx_EraseSector(sector + 1);
			W25qxx_WritePages(g_FlashWriteBuffer, (sector + 1) * w25qxx.SectorSize, w25qxx.SectorSize);
		}
	}
	
	/* 校验写入数据 */
	memset(g_FlashWriteBuffer, 0x00, sizeof(g_FlashWriteBuffer));
	W25qxx_ReadBytes(g_FlashWriteBuffer, WriteAddr_inBytes, NumByteToWrite);	//读数据
	if (memcmp(g_FlashWriteBuffer, pBuffer, NumByteToWrite) == 0)				//数据一致
	{
		return true;
	}
	else
	{
		return false;
	}
	
//	if(NumByteToWrite > 0)
//	{
//		while(tmp--)    
//		{
//			W25qxx_WritePages(pBuffer, WriteAddr_inBytes, NumByteToWrite);
//			memset(g_FlashWriteBuffer,0,NumByteToWrite);        //sizeof(g_FlashWriteBuffer)
//			W25qxx_ReadBytes(g_FlashWriteBuffer,WriteAddr_inBytes, NumByteToWrite);            
//			
//			if(memcmp(g_FlashWriteBuffer,pBuffer,NumByteToWrite) == 0)   //agj
//			{
//				return true;
//			}
//			else
//			{
//				return false;
//			}
//		}
//		return false;	
//	}
	
	
}

/******************************************************************
函数名称: Bool FlashReadData
功能描述: 从Flash中指定地址位置读取指定长度的数据
参数输入: uint8_t*	pBuffer			读缓存
		 uint32_t	ReadAddr		读Flash地址
		 uint32_t	NumByteToRead	读数据长度
		 uint8_t	ClrFlag			清缓存标记
参数输出: uint8_t*	pBuffer			读出数据
返 回 值: true=读出成功;
其    他: 当 ClrFlag=true 时，读出数据全为空将缓存清0，否则按原样读出数据
******************************************************************/
bool FlashReadData(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead,uint8_t ClrFlag)
{	
	uint16_t i;
	
	if(NumByteToRead > 0)
	{
        W25qxx_ReadBytes(pBuffer, ReadAddr, NumByteToRead); 
		if (FindFlashIsBlank(pBuffer, NumByteToRead)  == true)
		{
			if (ClrFlag == true)
			{
				for(i=0; i<NumByteToRead; i++)
				{
					*(pBuffer + i) = 0;
				}
			}
		}
		return true;	
	}
}

/******************************************************************
函数名称: bool FindFlashIsBlank(uint8_t *buf, uint16_t len)
功能描述:判断BUF 寄存器是不是空的
参数输入: 
参数输出: 
返 回 值:  TRUE = BLANK ,FALSE = NO BLANK
其    他: 无
******************************************************************/

bool FindFlashIsBlank(uint8_t *buf, uint16_t len)
{
	uint16_t i;
	for(i=0;i<len;i++)
	{
		if(buf[i] != 0xff)
		{
			break;
		}
	}
	if( i == len )
		return true;// IS BLANK
	else
		return false; // NO BLANK
}


/*
*********************************************************************************************************
*	函 数 名:  W25q_Write_N
*	功能说明:  写任意长度数据，只适合确定下一扇区为空或者可擦除时写入，不适合中间插入数据
*	形    参:  pBuffer : 数据源缓冲区；
*			   WriteAddr_inBytes ：目标区域首地址
*			   NumByteToWrite ：数据个数，任意大小，但不能超过芯片容量。
*	返 回 值:  无
*********************************************************************************************************
*/
void W25q_Write_N(uint8_t *pBuffer, uint32_t WriteAddr_inBytes, uint32_t NumByteToWrite)
{
	int i=0,j=0;
	uint8_t *pbuf;
	j = NumByteToWrite;
	pbuf = pBuffer;
	uint32_t writeAddr = WriteAddr_inBytes;
	int last_page_size = 0;
	
	// 首次一定是从页的开始页写，但是后续连续写时，每次不能保证都从页的第一字节开始

	last_page_size = 256-(writeAddr % 256);	// 不为0，则当前页没写完，需要写当前页，当前页只能写256-addr%256的字节数
	
	// 这次要写的数据会跨扇区，则擦除后续的扇区
	for(i=0;i<j;i++)
	{
		if((writeAddr+i)% 4096 == 0)
		{
			W25qxx_EraseSector((writeAddr+i)/4096);
		}
	}
		
	do
	{
		// 扇区头，则擦扇区
		if(writeAddr % 4096 == 0)
		{
			W25qxx_EraseSector(writeAddr/4096);
		}

		// 当前页没写完
		if(last_page_size)// 余256也会进，不影响
		{
			if(j >= last_page_size)// 当前页不够写，先写完当前页
			{
				W25qxx_WriteBytes(pbuf,writeAddr ,last_page_size);
				pbuf += last_page_size;
				writeAddr += last_page_size;
				j-=last_page_size;
			}
			else// 当前页足够写
			{
				W25qxx_WriteBytes(pbuf,writeAddr ,j);
				pbuf += j;
				writeAddr += j;
				j-=j;
			}
			last_page_size = 0;
		}
		else if((j >=256)&&(last_page_size == 0))// 数据够整页整页写
		{
			W25qxx_WriteBytes(pbuf,writeAddr ,256);
			pbuf += 256;
			writeAddr += 256;
			j-=256;
		}
		else if(j < 256)// 写到了末尾，数量已经不够最后一页
		{
			W25qxx_WriteBytes(pbuf,writeAddr,j);
			pbuf += j;
			writeAddr += j;
			j-=j;
		}
	}
	while(j);
}

