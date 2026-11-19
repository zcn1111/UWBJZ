#ifndef __G_FUNC_H
#define __G_FUNC_H

/* 引用头文件 */
#include "stm32l4xx.h"

#include "BSP_timer.h"

/******* 宏定义 *******/
/* 大小端 */
#define M_LITTLE_ENDIAN		0
#define M_BIG_ENDIAN		1

/******* 外部变量声明 *******/

/******* 外部函数声明 *******/
/* 计算校验和类 */
uint8_t CalcChecksum(uint8_t *Buf, uint32_t Len);
uint8_t CalcSum(uint8_t *Buf, uint32_t Len);
uint8_t CalcSumByInitialVal(uint8_t *Buf, uint32_t Len, uint8_t InitVal);
uint8_t CalcCheckBCC(uint8_t *Buf, uint32_t Len);
uint8_t CalcCRC8(uint8_t *Buf, uint16_t Len);
uint16_t CRC16(uint8_t* pchMsg, uint16_t wDataLen);
uint16_t CalcCRC16(uint8_t *Buf, uint16_t Len);
uint16_t CalcCRC16ByInitialVal(uint8_t *Buf, uint16_t Len, uint16_t InitVal);
uint32_t CalcCRC32(uint8_t *Buf, uint16_t Len);
uint32_t CalcCRC32ByTable(uint8_t *Buf, uint16_t Len);
void GenerateCRC32Table(void);

uint8_t CheckSum(uint16_t Len, uint8_t *Buf);


/* 数据转换类 */
uint8_t Hex2Asc(uint8_t Hex);
uint8_t Asc2Hex(uint8_t Asc);
void Hex2AscInString(uint8_t Hex, uint8_t *strAsc);
uint8_t Asc2HexInString(uint8_t *strAsc);
void Bcd2Asc(uint8_t Bcd, uint8_t *strAsc);
uint8_t Asc2Bcd(char AscH, char AscL);
uint8_t Bcd2Hex(uint8_t Bcd);
uint8_t Hex2Bcd(uint8_t Hex);
uint16_t Hex2BcdInString(uint8_t *HexStr, uint16_t HexStrLen, uint8_t *BcdStr, uint16_t *BcdStrLen);
uint16_t Arr2AscStr(uint8_t *Arr, uint16_t ArrLen, char SpiltChr, char *AscStr);
uint16_t Bcd2AscStr(uint8_t *BcdArr, uint16_t ArrLen, char SpiltChr, char *AscStr);
uint32_t HexStr2Int32(char *strHex);
void TimeHexToBcd(HexDateTime_t *dest ,uint8_t *src);
uint8_t HexToBCD(uint8_t val);

/* 时间格式转换 */
HexDateTime_t BcdTime2HexTime(BcdDateTime_t BcdTime);
BcdDateTime_t HexTime2BcdTime(HexDateTime_t HexTime);
HexDateTime_t Stamp2HexTime(uint32_t Stamp);
uint32_t HexTime2Stamp(HexDateTime_t HexTime);
uint32_t HexTime2ElapsedSecond(HexDateTime_t HexTime);
bool IsHexTimeSame(HexDateTime_t OldTime, HexDateTime_t NewTime);


/* 数据格式转换 */
uint8_t CharSwap(uint8_t Chr);
uint32_t ConvArrayToUint32(uint8_t *Buf, uint16_t Pos, uint8_t Mode);
uint32_t Conv3ByteToUint32(uint8_t *Buf, uint16_t Pos, uint8_t Mode);
uint16_t ConvArrayToUint16(uint8_t *Buf, uint16_t Pos, uint8_t Mode);
void ConvUint32ToArray(uint32_t Val, uint8_t *OutBuf, uint8_t Mode);
void ConvUint16ToArray(uint32_t Val, uint8_t *OutBuf, uint8_t Mode);
void ConvIPStrToArray(char *IPStr, uint8_t *OutBuf);
void ConvStrToPhonenum(char *Str, uint8_t *Pnum);
uint16_t ConvPortArrayToUint16(uint8_t *PortBuf);
void ConvUint16ToPortBuf(uint16_t Port, uint8_t *PortBuf);


/* 数据检查类 */
uint8_t CheckNumIsBCD(uint8_t *Buf, uint32_t Len);
uint8_t CheckStringIsSame(uint8_t *Buf, uint32_t Len, uint8_t ChkStr);
uint8_t CheckStringContaniedChar(uint8_t *Buf, uint32_t Len, uint8_t Char);

/* 文件名字符串转换 */
uint16_t MergePathAndFilename(const char *Path, const char *Filename, char *FullName);
uint16_t SpiltPathAndFilename(char *FullName, char *Path, char *Filename);
void GetExtname(const char *sFilename, char *Filename, uint16_t *FilenameLen, char *Extname);

uint16_t Get16ValueFrom8Buf(uint8_t *buf,uint8_t no);
uint8_t StrNcmpString(uint8_t *string1,uint8_t *string2,uint16_t len,uint16_t *pos);
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength);
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength);
void SystimeToBCDTime(HexDateTime_t *dest,HexDateTime_t src );
uint8_t CheckSumNoC(uint8_t *Array,uint16_t len_Array);

uint8_t StrToTime(uint8_t* pStr);
uint8_t CheckSumNoFan(uint16_t len_Array,uint8_t *Array,uint8_t BeforeSum);
void* mem_mem(const void* haystack, size_t haystack_len,
              const void* needle, size_t needle_len);
#endif

