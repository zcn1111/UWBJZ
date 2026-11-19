/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __W25QXX_QSPI1_H
#define __W25QXX_QSPI1_H


#include "stdint.h"
#include "stdbool.h"

#define INVALIDADDR 0xFFFF
#define Flash_CheckBadBlockPage( PageAddr ) 	(g_FlashBadBlock[ PageAddr / 32 ] & ( 1 << ( PageAddr % 31 )))
#define Flash_SetBadBlockPage( PageAddr ) 	(g_FlashBadBlock[ PageAddr / 32 ] |= ( 1 << ( PageAddr % 31 )))
#define Flash_ClearBadBlockPage( PageAddr ) 	(g_FlashBadBlock[ PageAddr / 32 ]  &= (~ ( 1 << ( PageAddr % 31 ))))

//Ö¸Áî±í
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg1		0x05     //¶Á×´Ì¬¼Ä´æÆ÷1Ö¸Áî
#define W25X_ReadStatusReg2		0x35     //¶Á×´Ì¬¼Ä´æÆ÷2Ö¸Áî
#define W25X_ReadStatusReg3		0x15     //¶Á×´Ì¬¼Ä´æÆ÷3Ö¸Áî
#define W25X_WriteStatusReg1    0x01     //Ð´×´Ì¬¼Ä´æÆ÷1Ö¸Áî
#define W25X_WriteStatusReg2    0x31     //Ð´×´Ì¬¼Ä´æÆ÷2Ö¸Áî
#define W25X_WriteStatusReg3    0x11     //Ð´×´Ì¬¼Ä´æÆ÷3Ö¸Áî
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20     //²Á³ýÒ»¸öÉÈÇø
#define W25X_BlockErase         0xD8     //²Á³ý64k
#define W25X_ChipErase			0xC7     //²Á³ýÕû¸öÐ¾Æ¬
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_ReadUniqID         0x4B
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 
#define W25X_Enable4ByteAddr    0xB7
#define W25X_Exit4ByteAddr      0xE9
#define W25X_SetReadParam		0xC0 
#define W25X_EnterQPIMode       0x38
#define W25X_ExitQPIMode        0xFF

extern uint8_t g_tmpBuf[4096];
extern uint8_t g_FlashtempBuff[1024];
extern uint8_t g_BoardType;
/*
  Author:     Nima Askari
  WebSite:    http://www.github.com/NimaLTD
  Instagram:  http://instagram.com/github.NimaLTD
  Youtube:    https://www.youtube.com/channel/UCUhY7qY1klJm1d2kulr9ckw
  
  Version:    1.1.1
  
  
  Reversion History:
  
  (1.1.1)
  Fix some errors.
  
  (1.1.0)
  Fix some errors.
  
  (1.0.0)
  First release.
*/

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>

#include "stm32l4xx.h"

typedef enum
{
	W25Q10=1,
	W25Q20,
	W25Q40,
	W25Q80,
	W25Q16,
	W25Q32,
	W25Q64,
	W25Q128,
	W25Q256,
	W25Q512,
	
}W25QXX_ID_t;

typedef struct
{
	W25QXX_ID_t	ID;
	uint8_t		UniqID[8];
	uint16_t	PageSize;
	uint32_t	PageCount;
	uint32_t	SectorSize;
	uint32_t	SectorCount;
	uint32_t	BlockSize;
	uint32_t	BlockCount;
	uint32_t	CapacityInKiloByte;
	uint8_t		StatusRegister1;
	uint8_t		StatusRegister2;
	uint8_t		StatusRegister3;	
	uint8_t		Lock;	
}w25qxx_t;

extern w25qxx_t	w25qxx;
//############################################################################
// in Page,Sector and block read/write functions, can put 0 to read maximum bytes 
//############################################################################
bool		W25qxx_Init(void);

uint32_t	W25qxx_ReadID(void);

void		W25qxx_EraseChip(void);
void 		W25qxx_EraseSector(uint32_t SectorAddr);
void 		W25qxx_EraseBlock(uint32_t BlockAddr);

uint32_t	W25qxx_PageToSector(uint32_t PageAddress);
uint32_t	W25qxx_PageToBlock(uint32_t PageAddress);
uint32_t	W25qxx_SectorToBlock(uint32_t SectorAddress);
uint32_t	W25qxx_SectorToPage(uint32_t SectorAddress);
uint32_t	W25qxx_BlockToPage(uint32_t BlockAddress);

bool 		W25qxx_IsEmptyPage(uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_PageSize);
bool 		W25qxx_IsEmptySector(uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_SectorSize);
bool 		W25qxx_IsEmptyBlock(uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToCheck_up_to_BlockSize);

void 		W25qxx_WriteByte(uint8_t pBuffer, uint32_t Bytes_Address);
void 		W25qxx_WriteBytes(uint8_t *pBuffer, uint32_t WriteAddr_inBytes, uint32_t NumByteToWrite);
void 		W25qxx_WritePage(uint8_t *pBuffer, uint32_t Page_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_PageSize);
void        W25qxx_WritePages(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void 		W25qxx_WriteSector(uint8_t *pBuffer, uint32_t Sector_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_SectorSize);
void 		W25qxx_WriteBlock(uint8_t* pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToWrite_up_to_BlockSize);


void 		W25qxx_ReadByte(uint8_t *pBuffer, uint32_t Bytes_Address);
void 		W25qxx_ReadBytes(uint8_t *pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead);
void 		W25qxx_ReadPage(uint8_t *pBuffer, uint32_t Page_Address,uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_PageSize);
void 		W25qxx_ReadSector(uint8_t *pBuffer, uint32_t Sector_Address,uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_SectorSize);
void 		W25qxx_ReadBlock(uint8_t* pBuffer, uint32_t Block_Address, uint32_t OffsetInByte, uint32_t NumByteToRead_up_to_BlockSize);
bool        FindFlashIsBlank(uint8_t *buf, uint16_t len);

bool        FlashWriteData(uint8_t *pBuffer, uint32_t WriteAddr_inBytes, uint32_t NumByteToWrite);
bool        FlashReadData(uint8_t* pBuffer, uint32_t ReadAddr, uint32_t NumByteToRead,uint8_t ClrFlag);
void        W25q_Write_N(uint8_t *pBuffer, uint32_t WriteAddr_inBytes, uint32_t NumByteToWrite);
void ReadBoardType(void);

//############################################################################
#ifdef __cplusplus
}
#endif


#endif /* __W25QXX_H */
