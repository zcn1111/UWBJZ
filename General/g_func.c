/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: g_func.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 通用处理函数
其他说明:
功能列表:
******************************************************************/

#include "g_func.h"
//#include "DTU_comm.h"
#include "BSP_timer.h"
#include "g_GlobalValDef.h"

#include "string.h"
#include "stdlib.h"

uint32_t g_CRC32_Table[256];

/******************************************************************
函数名称: uint8_t CheckNumIsBCD
功能描述: 检查数字是否为BCD码
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
参数输出: 无
返 回 值: uint8_t 0=不是BCD码;1=是BCD码
其    他: 无
******************************************************************/
uint8_t CheckNumIsBCD(uint8_t *Buf, uint32_t Len)
{
    uint32_t i;
    uint8_t beforeh,afterh;
    
    for(i=0; i<Len; i++)
    {
        beforeh = (Buf[i] & 0xf0) >> 4;
        afterh = Buf[i] & 0x0f;
        if(beforeh > 9 || afterh > 9 )
        {
            return 0;
        }
    }
    return 1;
}

/******************************************************************
函数名称: uint8_t CheckStringIsSame
功能描述: 检查数据全相同
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
		  uint8_t		ChkStr	检查数据
参数输出: 无
返 回 值: uint8_t 0=不全相同;1=全相同
其    他: 无
******************************************************************/
uint8_t CheckStringIsSame(uint8_t *Buf, uint32_t Len, uint8_t ChkStr)
{
	uint32_t i;
	uint8_t same;
	
	same = 1;
	for (i=0; i<Len; i++)
	{
		if (Buf[i] != ChkStr)
			same = 0;
	}
	return same;
}
/******************************************************************************
 * 函数名称: int8_t StrNcmpString(uint8_t *string1,uint8_t *string2,uint16_t len)
 * 功能描述: 比较两个字符串是否相等
 * 输　  入: 字符串及长度
 * 输　  出: 
 * 返    回: 1 大于 -1 小于 0 等于
********************************************************************************/

uint8_t StrNcmpString(uint8_t *string1,uint8_t *string2,uint16_t len,uint16_t *pos)
{
	uint16_t i;
	uint8_t ret;

	for(i=0;i<len;i++)
	{
		if(*string1 > *string2)
		{
			ret = 1;
			*pos = i;
			break;
		}
		else if(*string1 < *string2)
		{
			ret =  2;
			*pos = i;
			break;
		}
		string1++;
		string2++;
	}
	if( i == len )
		ret = 0;
	return ret;
}
/******************************************************************************
 * 函数名称: uint16_t Get16ValueFrom8Buf(uint8_t *buf,uint8_t no)
 * 功能描述: 从8 位数据中得到16 位数据
 * 输　     入: buf-- 8 位数据字符串 ，no 数据长度
 * 输　     出: 16 位数据的结果
 * 返           回:  得到的16 位的数据
********************************************************************************/
uint16_t Get16ValueFrom8Buf(uint8_t *buf,uint8_t no)
{
	uint16_t k;

	k=buf[no+1];
	k =( k<<8)+buf[no];
	return k;
}

/******************************************************************
函数名称: uint8_t CheckStringContainedChar
功能描述: 检查字符串包含指定字符
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
		  uint8_t		Char	包含字符
参数输出: 无
返 回 值: uint8_t 0=不包含;1=包含
其    他: 无
******************************************************************/
uint8_t CheckStringContaniedChar(uint8_t *Buf, uint32_t Len, uint8_t Char)
{
	uint32_t i;
	uint8_t contain=0;
	
	for (i=0; i<Len; i++)
	{
		if (Buf[i] == Char)
			contain = 1;
	}
	return contain;
}

/******************************************************************
函数名称: uint8_t CalcChecksum
功能描述: 计算校验和(数据累加和的补码)
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
参数输出: 无
返 回 值: uint8_t 校验和
其    他: 无
******************************************************************/
uint8_t CalcChecksum(uint8_t *Buf, uint32_t Len)
{
	uint32_t i;
	uint8_t sum=0;
	
	for(i=0; i<Len; i++)
	{
		sum += Buf[i];
	}
	sum = ~sum + 1;	//计算补码
	
	return sum;
}

/******************************************************************
函数名称: uint8_t CheckSum
功能描述: 计算校验和(数据累加和的补码)
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
参数输出: 无
返 回 值: uint8_t 校验和
其    他: 无
******************************************************************/
uint8_t CheckSum(uint16_t Len, uint8_t *Buf)
{
	uint16_t i;
	uint8_t sum=0;
	
	for(i=0; i<Len; i++)
	{
		sum += Buf[i];
	}
	sum = ~sum + 1;	//计算补码
	
	return sum;
}


/******************************************************************
函数名称: uint8_t CalcSum
功能描述: 计算累加和(数据累加和 不计算补码)
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
参数输出: 无
返 回 值: uint8_t 累加和
其    他: 无
******************************************************************/
uint8_t CalcSum(uint8_t *Buf, uint32_t Len)
{
	uint32_t i;
	uint8_t sum=0;
	
	for(i=0; i<Len; i++)
	{
		sum += Buf[i];
	}
	
	return sum;
}

/******************************************************************
函数名称: uint8_t CalcSumByInitialVal
功能描述: 带初值计算累加和(数据累加和 不计算补码)
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
		  uint8_t		InitVal	初值
参数输出: 无
返 回 值: uint8_t 累加和
其    他: 无
******************************************************************/
uint8_t CalcSumByInitialVal(uint8_t *Buf, uint32_t Len, uint8_t InitVal)
{
	uint32_t i;
	uint8_t sum=0;
	
	for(i=0; i<Len; i++)
	{
		sum += Buf[i];
	}
	sum = sum + InitVal;
	
	return sum;
}


/******************************************************************
函数名称: uint8_t CalcCheckBCC
功能描述: 计算异或校验(计算异或)
参数输入: uint8_t		*Buf	接收缓存
		  uint32_t		Len		数据长度
参数输出: 无
返 回 值: uint8_t 异或和
其    他: 无
******************************************************************/
uint8_t CalcCheckBCC(uint8_t *Buf, uint32_t Len)
{
    uint32_t i=0;
    uint8_t Check=0;
    
    for(i=0; i<Len; i++)
    {
        Check ^= Buf[i];
    }
    return Check;
}
/****************************************************************************
** 函数名称：CheckSum
** 函数功能：校验数据,返回校验和,累加和取反加1
** 入口参数：需要校验的字符串指针
** 出口参数：无
****************************************************************************/
uint8_t CheckSumNoFan(uint16_t len_Array,uint8_t *Array,uint8_t BeforeSum)
{
	uint16_t i;
	uint8_t Check = 0;

	//计算校验和
	for(i = 0;i< len_Array;i++)
	{
		Check += Array[i];
	}
	Check += BeforeSum;
	return Check;
}
/******************************************************************
函数名称: uint8_t CalcCRC8
功能描述: 计算CRC8 (CRC8-1Wire)
参数输入: uint8_t	*Buf	要校验的数据
		  uint16_t	Len		数据长度
参数输出: 无
返 回 值: uint16_t 计算CRC16结果
其    他: 无
******************************************************************/
uint8_t CalcCRC8(uint8_t *Buf, uint16_t Len)
{
 	uint8_t crc;
	uint16_t i;
	
	crc=0xFF;		//CRC计算赋初值

	while(Len--!=0)
	{
		crc=(crc ^ ((*Buf) & 0xff)) & 0xffff;		// 数据和CRC寄存器低8位异或 保存在CRC寄存器

		for(i=0;i<8;i++)
		{
			if((crc & 0x0001) != 0)		
			{
				crc=((crc) >> 1);		//判断最低位为1 先右移1位再与多项式异或
				crc=((crc) ^ 0x8C);		//CRC8-1Wire多项式 X8+X5+X4+1=0x31 倒序多项式=0x8C
			}
			else
			{
				crc=((crc) >> 1);		//判断最低位为0 右移1位
			}
		}
		Buf++;
	}
	return (crc & 0xFF);
}

/******************************************************************
函数名称: uint16_t CalcCRC16
功能描述: 计算CRC16 (CRC16-ModBus)
参数输入: uint8_t	*Buf	要校验的数据
		  uint16_t	Len		数据长度
参数输出: 无
返 回 值: uint16_t 计算CRC16结果
其    他: 无
******************************************************************/
uint16_t CalcCRC16(uint8_t *Buf, uint16_t Len)
{
 	uint16_t crc;
	uint16_t i;
	
	crc=0xFFFF;		//CRC计算赋初值

	while(Len--!=0)
	{
		crc=(crc ^ ((*Buf) & 0xff)) & 0xffff;		// 数据和CRC寄存器低8位异或 保存在CRC寄存器

		for(i=0;i<8;i++)
		{
			if((crc & 0x0001) != 0)		
			{
				crc=((crc) >> 1);		//判断最低位为1 先右移1位再与多项式异或
				crc=((crc) ^ 0xA001);	//CRC16-ModBus多项式 X16+X15+X2+1=0x8005 倒序多项式=0xA001
			}
			else
			{
				crc=((crc) >> 1);		//判断最低位为0 右移1位
			}
		}
		Buf++;
	}
	return (crc & 0xffff);
}

/******************************************************************
函数名称: uint16_t CalcCRC16ByInitialVal
功能描述: 带初值计算CRC16 (CRC16-ModBus)
参数输入: uint8_t	*Buf	要计算的数据
		  uint16_t	Len		数据长度
		  uint16_t	InitVal	初值
参数输出: 无
返 回 值: uint16 CRC16计算结果
其    他: 无
******************************************************************/
uint16_t CalcCRC16ByInitialVal(uint8_t *Buf, uint16_t Len, uint16_t InitVal)
{
 	uint16_t crc;
	uint16_t i;
	
	crc=InitVal;

	while(Len--!=0)
	{
		crc=(crc ^ ((*Buf) & 0xff)) & 0xffff;		// 数据和CRC寄存器低8位异或 保存在CRC寄存器

		for(i=0;i<8;i++)
		{
			if((crc & 0x0001) != 0)		
			{
				crc=((crc) >> 1);		//判断最低位为1 先右移1位再与多项式异或
				crc=((crc) ^ 0xA001);	//CRC16-ModBus多项式 X16+X15+X2+1=0x8005 倒序多项式=0xA001
			}
			else
			{
				crc=((crc) >> 1);		//判断最低位为0 右移1位
			}
		}
		Buf++;
	}
	return (crc & 0xffff);
}

/******************************************************************
函数名称: void GenerateCRC32Table
功能描述: 生成CRC32表
参数输入: 
参数输出: 无
返 回 值: 无
其    他: 无
******************************************************************/
void GenerateCRC32Table(void)
{
	uint16_t i, j;
	uint32_t crc;
	
	for (i=0; i<256; i++)
	{
		crc = (uint32_t)i;
		for (j=0; j<8; j++)
		{
			if (crc & 1)
				crc = 0xEDB88320 ^ (crc >> 1);
			else
				crc = crc >> 1;
		}
		g_CRC32_Table[i] = crc;
	}
}

/******************************************************************
函数名称: uint32_t CalcCRC32ByTable
功能描述: 计算CRC32(查表)
参数输入: uint8_t	*Buf	要校验的数据
		  uint16_t	Len		数据长度
参数输出: 无
返 回 值: uint32_t 计算CRC32结果
其    他: 无
******************************************************************/
uint32_t CalcCRC32ByTable(uint8_t *Buf, uint16_t Len)
{
	
//	uint32_t i, j;
//	uint32_t crc;
//	uint32_t crc_table[256];
//	//32.**初始化crc表,生成32位大小的crc表
//	//也可以直接定义出crc表,直接查表,
//	//但总共有256个,看着眼花,用生成的比较方便.
//	for(i=0; i<256; i++)
//	{
//		crc = (uint32_t)i;
//		for(j=0; j<8; j++)
//		{
//			if(crc & 1)
//				crc = 0xEDB88320UL ^ (crc >> 1);
//			else
//				crc = crc >> 1;
//		}
//		crc_table[i] = crc;
//	}
//	crc = 0;
//	for(i=0; i<Len; i++)
//	{
//		crc = crc_table[(crc ^ Buf[i]) & 0xff] ^ (crc >> 8);
//	}
//	return crc;
	uint32_t crc=0;
	uint16_t i;
	
	for (i=0; i<Len; i++)
	{
		crc = g_CRC32_Table[(crc ^ Buf[i]) & 0xFF] ^ (crc >> 8);
	}
	
	return crc;
}

/******************************************************************
函数名称: uint32_t CalcCRC32
功能描述: 计算CRC32
参数输入: uint8_t	*Buf	要校验的数据
		  uint16_t	Len		数据长度
参数输出: 无
返 回 值: uint32_t 计算CRC32结果
其    他: 无
******************************************************************/
uint32_t CalcCRC32(uint8_t *Buf, uint16_t Len)
{
 	uint32_t crc;
	uint16_t i;
	
//	crc=0xFFFFFFFF;		//CRC计算赋初值
	crc = 0x00000000;

	while(Len--!=0)
	{
		crc=(crc ^ ((*Buf) & 0xff)) & 0xffffffff;		// 数据和CRC寄存器低8位异或 保存在CRC寄存器

		for(i=0;i<8;i++)
		{
			if((crc & 0x00000001) != 0)		
			{
				crc=((crc) >> 1);				//判断最低位为1 先右移1位再与多项式异或
				crc=((crc) ^ 0xEDB88320);		//CRC32多项式 倒序多项式=0xEDB88320
			}
			else
			{
				crc=((crc) >> 1);				//判断最低位为0 右移1位
			}
		}
		Buf++;
	}
	return (crc & 0xffffffff);
}

/******************************************************************************
函数名称: uint8_t Hex2Asc
功能描述: 16进制转换为ASCII码
参数输入: uint8_t	Hex	16进制值 0x00~0x0F
参数输出: 无
返 回 值: uint8_t ASC码
其    他: 输入范围0x00~0x0F转换为对应ASC码，其他值转换为'0'
********************************************************************************/
uint8_t Hex2Asc(uint8_t Hex)
{
	uint8_t ret;
	
	if ((Hex >= 0x00) && (Hex <= 0x09))
	{
		ret = Hex + '0';
	}
	else if ((Hex >= 0x0A) && (Hex <= 0x0F))
	{
		ret = Hex - 10 + 'A';
	}
	else
	{
		ret = '0';
	}
	
	return ret;	
}


/******************************************************************************
函数名称: void Hex2AscInString
功能描述: 16进制转换为ASCII码字符数组
参数输入: uint8_t	*Hex	16进制数组
参数输出: uint8_t	*strAsc	ASC码字符数组
返 回 值: 
其    他: 无
********************************************************************************/
void Hex2AscInString(uint8_t Hex, uint8_t *strAsc)
{
	uint8_t hexh, hexl;
	
	hexh = (Hex >> 4) & 0x0F;
	hexl = Hex & 0x0F;
	
	/* 处理高半字节 */
	*strAsc = Hex2Asc(hexh);
	
	/* 处理低半字节 */
	*(strAsc + 1) = Hex2Asc(hexl);
}

/******************************************************************************
函数名称: uint16_t Arr2AscStr
功能描述: 数组转换为ASCII码字符串
参数输入: uint8_t	*Arr		16进制数组
		  uint16_t	ArrLen		数组长度
		  char		SpiltChr	分隔符 0x00=不分隔输出
参数输出: char		*AscStr	ASC码字符串
返 回 值: uint16_t	输出字符串长度
其    他: 无
********************************************************************************/
uint16_t Arr2AscStr(uint8_t *Arr, uint16_t ArrLen, char SpiltChr, char *AscStr)
{
	uint16_t len=0;
	uint16_t i;
	
	for (i=0; i<ArrLen; i++)
	{
		Hex2AscInString(Arr[i], (uint8_t*)(AscStr + len));
		len += 2;
		if (SpiltChr != 0x00)
		{
			AscStr[len] = SpiltChr;
			len += 1;
		}
	}
	return len;
}


/******************************************************************************
函数名称: uint8_t Asc2Hex
功能描述: ASCII码转换为16进制
参数输入: uint8_t	Asc	ASCII码
参数输出: 无
返 回 值: uint8_t 16进制值
其    他: 输入范围'0'~'9' 'A'~'F'或'a'~'f' 转换为对应16进制值，其他值转换为0x00
********************************************************************************/
uint8_t Asc2Hex(uint8_t Asc)
{
	uint8_t ret;

	if ((Asc >= '0') && (Asc <= '9'))	//asc:0--9
	{
		ret = Asc - '0';
	}
	else if ((Asc >= 'A') && (Asc <= 'F'))	//asc:A--F
	{
		ret = Asc - 'A' + 10;
	}
	else if ((Asc >= 'a') && (Asc <= 'f'))	//asc:a--f
	{
		ret = Asc - 'a' + 10;
	}
	
	return ret;	
}

/******************************************************************************
函数名称: uint8_t Asc2HexInString
功能描述: ASCII码数组转换为Hex值
参数输入: uint8_t	*strAsc	ASC码字符数组
参数输出: 无
返 回 值: uint8_t Hex值
其    他: 无
********************************************************************************/
uint8_t Asc2HexInString(uint8_t *strAsc)
{
	uint8_t hexh, hexl;
	uint8_t asch, ascl;
	uint8_t ret;
	
	asch = *strAsc;
	ascl = *(strAsc + 1);
	ret = 0x00;
	
	/* 处理高半字节 */
	hexh = Asc2Hex(asch);
	ret |= (hexh << 4) & 0xF0;
	
	/* 处理低半字节 */
	hexl = Asc2Hex(ascl);
	ret |= (hexl & 0x0F);
	
	return ret;
}


/******************************************************************************
 * 函数名称: Bcd2Asc
 * 功能描述: bcd码转换为两个ascii码字符 例:0x2F转换为"2F"
 * 输　  入: bcd:十六进制
 * 输　  出: retstr:2个ascii码字符
********************************************************************************/
void Bcd2Asc(uint8_t Bcd, uint8_t *strAsc)
{
	uint8_t tmp;

	/* 取出字节的高4位 */
	tmp = Bcd >> 4;
	strAsc[0] = Hex2Asc(tmp);

	/* 取出字节的低4位 */
	tmp = Bcd & 0x0F;
	strAsc[1] = Hex2Asc(tmp);
}

/******************************************************************************
函数名称: uint16_t Bcd2AscStr
功能描述: BCD数组转换为ASCII码字符串
参数输入: uint8_t	*BcdArr		BCD数组
		  uint16_t	ArrLen		数组长度
		  char		SpiltChr	分隔符 0x00=不分隔输出
参数输出: char		*AscStr	ASC码字符串
返 回 值: uint16_t	输出字符串长度
其    他: 无
********************************************************************************/
uint16_t Bcd2AscStr(uint8_t *BcdArr, uint16_t ArrLen, char SpiltChr, char *AscStr)
{
	uint16_t len=0;
	uint16_t i;
	
	for (i=0; i<ArrLen; i++)
	{
		Bcd2Asc(BcdArr[i], (uint8_t*)(AscStr + len));
		len += 2;
		if (SpiltChr != 0x00)
		{
			AscStr[len] = SpiltChr;
			len += 1;
		}
	}
	return len;
}

/******************************************************************************
 * 函数名称: uint8 Asc2Bcd(char AscH, char AscL)
 * 功能描述: 两个ascii码字符转换为bcd码 例:"2F"转换为0x2F
 * 输　  入: 2个ascii码字符 ascH:高字符,ascL:低字符
 * 输　  出: 十六进制
********************************************************************************/
uint8_t Asc2Bcd(char AscH, char AscL)
{
	return ((Asc2Hex(AscH) << 4) | Asc2Hex(AscL));
}

/******************************************************************************
函数名称: uint8_t Bcd2Hex
功能描述: BCD码转换为十六进制
参数输入: uint8_t	Bcd	BCD码字符
参数输出: 无
返 回 值: uint8_t Hex值
其    他: 无
********************************************************************************/
uint8_t Bcd2Hex(uint8_t Bcd)
{
	uint8_t hexh, hexl;
	uint8_t ret;
	
	hexh = (Bcd >> 4) & 0x0F;
	hexl = (Bcd & 0x0F);
	ret = hexh * 10 + hexl;
	
	return ret;
}

/******************************************************************************
函数名称: uint8_t Hex2Bcd
功能描述: 十六进制转换为BCD码
参数输入: uint8_t	Hex	十六进制字符
参数输出: 无
返 回 值: uint8_t BCD码
其    他: 无
********************************************************************************/
uint8_t Hex2Bcd(uint8_t Hex)
{
	uint8_t bcdh, bcdl;
	uint8_t ret;
	
	if (Hex > 99)
	{
		ret = 0;
	}
	else
	{
		bcdh = Hex / 10;
		bcdl = Hex % 10;
		ret = (bcdh << 4) + bcdl;
	}
	
	return ret;
}

/******************************************************************************
函数名称: uint16_t Hex2BcdInString
功能描述: 十六进制转换为BCD码字符串
参数输入: uint8_t	*HexStr		16进制字符串
		  uint16_t	HexStrLen	16进制字符串长度
参数输出: uint8_t	*BcdStr		BCD码字符串
		  uint16_t	*BcdStrLen	BCD码字符串长度
返 回 值: uint16_t 不可转换字符个数
其    他: 无
********************************************************************************/
uint16_t Hex2BcdInString(uint8_t *HexStr, uint16_t HexStrLen, uint8_t *BcdStr, uint16_t *BcdStrLen)
{
	uint16_t num=0;
	uint16_t len, outlen;
	uint16_t i;
	
	len = HexStrLen;
	outlen = 0;
	for (i=0; i<len; i++)
	{
		if (HexStr[i] > 99)
		{
			num++;
		}
		else
		{
			BcdStr[outlen] = Hex2Bcd(HexStr[i]);
			outlen++;
		}
	}
	
	*BcdStrLen = outlen;
	return num;
}

/******************************************************************************
函数名称: uint32 HexStr2Int32
功能描述: 十六进制字符串转换为32位整数
参数输入: uint8_t	Hex	十六进制字符
参数输出: 无
返 回 值: uint8_t BCD码
其    他: 无
********************************************************************************/
uint32_t HexStr2Int32(char *strHex)
{
	uint8_t slen;
	uint8_t i;
	uint32_t mask=0x00000001;
	
	uint32_t ret;
	
	slen = strlen(strHex);
	ret = 0;
	if ((strHex[0] == '0') && (strHex[1] == 'x'))	//检查是否是16进制格式字符串
	{
		for (i=(slen-1); i>=2; i--)					//去除字符串末尾的'\0'
		{
			ret += (Asc2Hex(strHex[i])) * mask;
			if (i > 2) mask = mask << 4;
		}
	}
	
	return ret;
}

/******************************************************************************
函数名称: HexDateTime_t BcdTime2HexTime
功能描述: BCD格式时间转换为十六进制时间
参数输入: BcdDateTime_t	*BcdTime	BCD格式时间
参数输出: 
返 回 值: HexDateTime_t	*HexTime	十六进制时间
其    他: 无
********************************************************************************/
HexDateTime_t BcdTime2HexTime(BcdDateTime_t BcdTime)
{
	HexDateTime_t tmpd;
	
	tmpd.Year = Bcd2Hex(BcdTime.Year);
	tmpd.Month = Bcd2Hex(BcdTime.Month);
	tmpd.Day = Bcd2Hex(BcdTime.Day);
	tmpd.Hour = Bcd2Hex(BcdTime.Hour);
	tmpd.Minute = Bcd2Hex(BcdTime.Minute);
	tmpd.Second= Bcd2Hex(BcdTime.Second);
	
	return tmpd;
}

/******************************************************************************
函数名称: BcdDateTime_t HexTime2BcdTime
功能描述: 十六进制时间转换为BCD格式时间
参数输入: HexDateTime_t	*HexTime	十六进制时间
参数输出: 
返 回 值: BcdDateTime_t	*BcdTime	BCD格式时间
其    他: 无
********************************************************************************/
BcdDateTime_t HexTime2BcdTime(HexDateTime_t HexTime)
{
	BcdDateTime_t tmpd;
	
	tmpd.Year = Hex2Bcd(HexTime.Year);
	tmpd.Month = Hex2Bcd(HexTime.Month);
	tmpd.Day = Hex2Bcd(HexTime.Day);
	tmpd.Hour = Hex2Bcd(HexTime.Hour);
	tmpd.Minute = Hex2Bcd(HexTime.Minute);
	tmpd.Second = Hex2Bcd(HexTime.Second);
	
	return tmpd;
}

/******************************************************************************
函数名称: HexDateTime Stamp2HexTime
功能描述: 时间戳格式转换为Hex时间格式
参数输入: uint32_t	Stamp	时间戳格式
参数输出: 
返 回 值: HexDateTime	Hex时间格式
其    他: 无
********************************************************************************/
HexDateTime_t Stamp2HexTime(uint32_t Stamp)
{
	HexDateTime_t tm;
	
	tm.Year = (uint8_t)((Stamp >> 26) & 0x0000003F);	//b26-b31:年 6位
	tm.Month = (uint8_t)((Stamp >> 22) & 0x0000000F);	//b22-b25:月 4位
	tm.Day = (uint8_t)((Stamp >> 17) & 0x0000001F);		//b17-b21:日 5位
	tm.Hour = (uint8_t)((Stamp >> 12) & 0x0000001F);	//b12-b16:时 5位
	tm.Minute = (uint8_t)((Stamp >> 6) & 0x0000003F);	//b6-b11:分 6位
	tm.Second = (uint8_t)(Stamp & 0x0000003F);			//b0-b5:秒 6位
	
	return tm;
}

/******************************************************************************
函数名称: uint32_t Stamp2HexTime
功能描述: 时间戳格式转换为Hex时间格式
参数输入: HexDateTime
参数输出: 
返 回 值: HexDateTime	Hex时间格式
其    他: 无
********************************************************************************/
uint32_t HexTime2Stamp(HexDateTime_t HexTime)
{
	uint32_t stm=0;

	stm |= HexTime.Year;
	stm <<= 4;
	stm |= (HexTime.Month & 0x0F);
	stm <<= 5;
	stm |= (HexTime.Day & 0x1F);
	stm <<= 5;
	stm |= (HexTime.Hour & 0x1F);
	stm <<= 6;
	stm |= (HexTime.Minute & 0x3F);
	stm <<= 6;
	stm |= (HexTime.Second & 0x3F);
	
	return stm;
}

/******************************************************************************
函数名称: uint32_t HexTime2ElapsedSecond
功能描述: 十六进制时间转换为2000/1/1 0:0:0经过的秒数
参数输入: HexDateTime_t	*HexTime	十六进制时间
参数输出: 
返 回 值: uint32_t
其    他: 无
********************************************************************************/
uint32_t HexTime2ElapsedSecond(HexDateTime_t HexTime)
{
	uint32_t sec=0;
	uint32_t day=0;
	uint16_t leap=0;
	uint16_t i;
	uint8_t monthday[12]={31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	
	leap = HexTime.Year / 4;
	if (IsLeapYear(HexTime.Year + 2000))
		monthday[1] = 29;
	day += HexTime.Year * 365 + leap;
	for (i=0; i<(HexTime.Month - 1); i++)
	{
		day += monthday[i];
	}
	day += HexTime.Day - 1;
	
	sec += (uint32_t)HexTime.Second;
	sec += (uint32_t)HexTime.Minute * 60;
	sec += (uint32_t)HexTime.Hour * 3600;
	sec += (uint32_t)day * 86400;
	
	return sec;
}

/******************************************************************************
函数名称: bool IsHexTimeSame
功能描述: 检查时间是否一致
参数输入: HexDateTime_t	NewTime	十六进制时间
		 HexDataTime_t	OldTime
参数输出: 
返 回 值: false=时间不一致;true=时间一致
其    他: 无
********************************************************************************/
bool IsHexTimeSame(HexDateTime_t OldTime, HexDateTime_t NewTime)
{
	uint8_t same;
	
	same = 0x00;
	if (NewTime.Year == OldTime.Year)		same |= 0x01;
	if (NewTime.Month == OldTime.Month)		same |= 0x02;
	if (NewTime.Day == OldTime.Day)			same |= 0x04;
	if (NewTime.Hour == OldTime.Hour)		same |= 0x08;
	if (NewTime.Minute == OldTime.Minute)	same |= 0x10;
	if (NewTime.Second == OldTime.Second)	same |= 0x20;
	
	if (same == 0x3F)
		return true;
	
	return false;
}

/******************************************************************************
 * 函数名称: CharSwap
 * 功能描述: 一个字节的半字节交换
 * 输　  入: 要转换的字节
 * 输　  出: 转换完的字节
********************************************************************************/
uint8_t CharSwap(uint8_t Chr)
{
	return  (((Chr << 4) & 0xF0) | ((Chr >> 4) & 0x0F));
}
/******************************************************************************
 * 函数名称: CRC16
 * 功能描述: 计算校验值
 * 输　  入: pchMsg，要校验的数据指针，wDataLen，数据长度
 * 输　  出: 无
 * 返    回：返回校验值
********************************************************************************/
uint16_t CRC16(uint8_t* pchMsg, uint16_t wDataLen)
{
 	uint16_t crc;
	uint16_t i;
	
	crc=0xFFFF;

	while(wDataLen--!=0)
	{

		crc=(crc ^ ((*pchMsg)&0xff))&0xffff;
		for(i=0;i<8;i++)
		{
			if((crc & 0x0001)!=0)
			{
				crc=((crc)>>1);
				crc=((crc) ^ 0xA001);
			}
			else
			{
				crc=((crc)>>1);
			}
		}
		pchMsg++;
	}
	return(crc&0xffff);
}
/******************************************************************
函数名称: uint8_t CheckCRC16
功能描述: 校验CRC16是否正确
参数输入: uint8_t	*Buf	要校验的数据
		  uint16_t	Len		数据长度
参数输出: 无
返 回 值: uint8_t 错误=0;正确=1
其    他: 无
******************************************************************/
uint8_t CheckCRC16(uint8_t *Buf, uint16_t Len)
{
	uint16_t crc,tmp;
	
	crc = CalcCRC16(Buf, Len-2);
	tmp = *(Buf + (Len-1));
	tmp = (tmp << 8) | *(Buf + (Len-2));
	if(crc  == tmp)
		return 1;
	else
		return 0;
}

/******************************************************************
函数名称: uint32_t ConvArrayToUint32
功能描述: 从数组指定位置取Len字节转换为uint32
参数输入: uint8_t	*Buf	要转换的字符串
		  uint16_t	Pos		取字节位置
		  uint8_t	Mode	大小端模式
参数输出: 无
返 回 值: uint32_t 转换后的32位值
其    他: 无
******************************************************************/
uint32_t ConvArrayToUint32(uint8_t *Buf, uint16_t Pos, uint8_t Mode)
{
	uint32_t val=0;
	
	if (Mode == M_LITTLE_ENDIAN)
		val = (Buf[Pos+3] << 24) | (Buf[Pos+2] << 16) | (Buf[Pos+1] << 8) | (Buf[Pos]);
	else if (Mode == M_BIG_ENDIAN)
		val = (Buf[Pos] << 24) | (Buf[Pos+1] << 16) | (Buf[Pos+2] << 8) | (Buf[Pos+3]);
	
	return val;
}

/******************************************************************
函数名称: uint32_t Conv3ByteToUint32
功能描述: 从数组指定位置取3字节转换为uint32
参数输入: uint8_t	*Buf	要转换的字符串
		  uint16_t	Pos		取字节位置
		  uint8_t	Mode	大小端模式
参数输出: 无
返 回 值: uint32_t 转换后的32位值
其    他: 无
******************************************************************/
uint32_t Conv3ByteToUint32(uint8_t *Buf, uint16_t Pos, uint8_t Mode)
{
	uint32_t val;
	
	if (Mode == M_LITTLE_ENDIAN)
		val = (0x00 << 24) | (Buf[Pos+2] << 16) | (Buf[Pos+1] << 8) | (Buf[Pos]);
	else if (Mode == M_BIG_ENDIAN)
		val = (Buf[Pos] << 24) | (Buf[Pos+1] << 16) | (Buf[Pos+2] << 8) | (0x00);
	
	return val;
}

/******************************************************************
函数名称: uint16_t ConvArrayToUint16
功能描述: 从数组指定位置取2字节转换为uint16
参数输入: uint8_t	*Buf	要转换的字符串
		  uint16_t	Pos		取字节位置
		  uint8_t	Mode	大小端模式
参数输出: 无
返 回 值: uint16_t 转换后的16位值
其    他: 无
******************************************************************/
uint16_t ConvArrayToUint16(uint8_t *Buf, uint16_t Pos, uint8_t Mode)
{
	uint16_t val=0;
	
	if (Mode == M_LITTLE_ENDIAN)
		val = (Buf[Pos+1] << 8) | (Buf[Pos]);
	else if (Mode == M_BIG_ENDIAN)
		val = (Buf[Pos] << 8) | (Buf[Pos+1]);
	
	return val;
}

/******************************************************************
函数名称: void ConvUint32ToArray
功能描述: 从uint32转换为数组
参数输入: uint32_t	Val		要转换的32位值
		  uint8_t	Mode	大小端模式
参数输出: uint8_t	*OutBuf	输出数组
返 回 值: 无
其    他: 无
******************************************************************/
void ConvUint32ToArray(uint32_t Val, uint8_t *OutBuf, uint8_t Mode)
{
	if (Mode == M_LITTLE_ENDIAN)
	{
		OutBuf[0] = (uint8_t)(Val & 0xFF);
		OutBuf[1] = (uint8_t)((Val >> 8) & 0xFF);
		OutBuf[2] = (uint8_t)((Val >> 16) & 0xFF);
		OutBuf[3] = (uint8_t)((Val >> 24) & 0xFF);
	}
	else if (Mode == M_BIG_ENDIAN)
	{
		OutBuf[0] = (uint8_t)((Val >> 24) & 0xFF);
		OutBuf[1] = (uint8_t)((Val >> 16) & 0xFF);
		OutBuf[2] = (uint8_t)((Val >> 8) & 0xFF);
		OutBuf[3] = (uint8_t)(Val & 0xFF);
	}
}

/******************************************************************
函数名称: void ConvUint16ToArray
功能描述: 从uint16转换为数组
参数输入: uint16_t	Val		要转换的32位值
		  uint8_t	Mode	大小端模式
参数输出: uint8_t	*OutBuf	输出数组
返 回 值: 无
其    他: 无
******************************************************************/
void ConvUint16ToArray(uint32_t Val, uint8_t *OutBuf, uint8_t Mode)
{
	if (Mode == M_LITTLE_ENDIAN)
	{
		OutBuf[0] = (uint8_t)(Val & 0xFF);
		OutBuf[1] = (uint8_t)((Val >> 8) & 0xFF);
	}
	else if (Mode == M_BIG_ENDIAN)
	{
		OutBuf[0] = (uint8_t)((Val >> 8) & 0xFF);
		OutBuf[1] = (uint8_t)(Val & 0xFF);
	}
}

/******************************************************************
函数名称: void ConvIPStrToArray
功能描述: 从IP格式字符串转换到数组
参数输入: char		*IPStr	要转换的IP字符串
		  uint8_t	Mode	大小端模式
参数输出: uint8_t	*OutBuf	输出数组
返 回 值: 无
其    他: 无
******************************************************************/
void ConvIPStrToArray(char *IPStr, uint8_t *OutBuf)
{
	uint16_t slen;
	
	char tmpip[16];
	char tmpport[6];
	uint8_t arrip[4];
	uint8_t arrport[2];
	uint16_t port;
	uint8_t pos;
	
	uint16_t i, j;
	
	/* 读取IP字符串 */
	i = 0;
	j = 0;
	slen = strlen(IPStr);
	pos = 0;
	for (i=0; i<(slen - 1); i++)
	{
		/* 处理IP分隔符 */
		if (IPStr[i] == '.')
		{
			memset(tmpip, 0x00, sizeof(tmpip));
			memcpy(tmpip, IPStr + pos, i - pos);
			arrip[j] = atol(tmpip);
			pos = i + 1;
			j++;
		}
		if (IPStr[i] == ':')
		{
			memset(tmpip, 0x00, sizeof(tmpip));
			memcpy(tmpip, IPStr + pos, i - pos);
			arrip[j] = atol(tmpip);
			memset(tmpport, 0x00, sizeof(tmpport));
			memcpy(tmpport, IPStr + i + 1, (slen - i) - 1);
			port = atol(tmpport);
			ConvUint16ToArray(port, arrport, M_LITTLE_ENDIAN);
		}
	}
	
	/* 输出 */
	memcpy(OutBuf, arrip, 4);
	memcpy(OutBuf + 4, arrport, 2);
}

/******************************************************************
函数名称: uint16_t ConvPortArrayToUint16
功能描述: 从端口数组转换到uint16
参数输入: uint8_t	*PortBuf要转换的端口数据格式
参数输出: 
返 回 值: uint16_t	端口号(uint16)
其    他: 无
******************************************************************/
uint16_t ConvPortArrayToUint16(uint8_t *PortBuf)
{
	uint16_t porttmp;
	
	porttmp = 0;
	porttmp += PortBuf[0] * 100;
	porttmp += PortBuf[1];
	
	return porttmp;
}

/******************************************************************
函数名称: void ConvUint16ToPortBuf
功能描述: 从uint16转换到端口数组
参数输入: uint16_t	端口号(uint16)
参数输出: uint8_t	*PortBuf要转换的端口数据格式
返 回 值: 
其    他: 无
******************************************************************/
void ConvUint16ToPortBuf(uint16_t Port, uint8_t *PortBuf)
{
	uint16_t porttmp;
	
	porttmp = Port % 100;
	PortBuf[0] = (porttmp >> 8) & 0xFF;
	porttmp = Port / 100;
	PortBuf[1] = (porttmp) & 0xFF;
}


/******************************************************************
函数名称: void ConvIPStrToIPAddr
功能描述: 从IP格式字符串转换到IP地址格式
参数输入: char		*IPStr	要转换的IP字符串
		  uint8_t	Mode	大小端模式
参数输出: IPAddr_t	*IPaddr	输出IP
返 回 值: 无
其    他: 无
******************************************************************/
void ConvIPStrToIPAddr(char *IPStr, uint8_t Mode, IPAddr_t *IPaddr)
{
	
}

/******************************************************************
函数名称: void ConvStrToPhonenum
功能描述: 从手机号字符串转换到手机号格式
参数输入: char		*Str	要转换的字符串
参数输出: uint8_t	*Pnum	手机号格式
返 回 值: 无
其    他: 无
******************************************************************/
void ConvStrToPhonenum(char *Str, uint8_t *Pnum)
{
	uint8_t i, j;
	uint8_t num[6];
	char ascl, asch;
	
	i = 0;
	j = 0;
	memset(num, 0x00, sizeof(num));
	for (i=0; i<12; i++)
	{
		if (Str[i] != '\0')
		{
			if ((i % 2) != 0)
			{
				Pnum[j] &= 0x0F;
				Pnum[j] |= ((Str[i] - '0') << 8) & 0xF0;
			}
			else
			{
				Pnum[j] &= 0xF0;
				Pnum[j] |= ((Str[i] - '0') & 0x0F);
				j++;
			}
		}
		else
			break;
	}
	Pnum[5] &= 0xF0;
	Pnum[5] |= 0x0F;
	
}

/******************************************************************
函数名称: uint16_t MergePathAndFilename
功能描述: 组合路径和文件名
参数输入: const char	*Path		文件路径
		 const char	*Filename	文件名
参数输出: char		*FullName	完整文件名
返 回 值: 无
其    他: 无
******************************************************************/
uint16_t MergePathAndFilename(const char *Path, const char *Filename, char *FullName)
{
	uint16_t plen=0;
	uint16_t flen=0;
	uint16_t i;
	uint8_t	slash=0;
	uint16_t ret;
	
	if ((Path == NULL) || (Filename == NULL))
	{
		FullName = NULL;
		return 0;
	}
	
	plen = strlen(Path);
	flen = strlen(Filename);
	
	/* 处理路径 */
	if (Path[plen-1] == '\\')		//最后是'\'结尾 不增加
	{
		memcpy(FullName, Path, plen);
	}
	else							//最后不是'\'结尾 需要增加
	{
		memcpy(FullName, Path, plen);
		FullName[plen] = '\\';
		plen += 1;
	}
	/* 处理文件名 */
	memcpy(FullName + plen, Filename, flen);
	return (plen + flen);
}

/******************************************************************
函数名称: uint16_t SpiltPathAndFilename
功能描述: 拆分路径和文件名
参数输入: char		*FullName	完整文件名
参数输出: char		*Path		文件路径
		 char		*Filename	文件名
返 回 值: 无
其    他: 无
******************************************************************/
uint16_t SpiltPathAndFilename(char *FullName, char *Path, char *Filename)
{
	uint16_t i, j;
	uint16_t slen, plen, flen;
	uint16_t p;
	uint16_t ret=0;
	
	if (FullName == NULL)
	{
		Path = NULL;
		Filename = NULL;
		return 0;
	}
	
	slen = strlen(FullName);
	j = 0;
	plen = 0;
	flen = 0;
	for (i=slen; i>0; i--)
	{
		if (FullName[i] == '/')
		{
			p = i;
			flen = j;
			plen = slen - p;
			break;
		}
		j++;
	}
	
	memcpy(Filename, FullName + p, flen);
	memcpy(Path, FullName, plen);
	return p;
}

/******************************************************************
函数名称: void GetExtname
功能描述: 获取扩展名
参数输入: const char		*sFilename	文件名字符串
参数输出:       char		*Filename	文件名
		       uint16_t	*FilenameLen文件名长度
			   char		*Extname	扩展名
返 回 值: 扩展名字符串
其    他: 无
******************************************************************/
void GetExtname(const char *sFilename, char *Filename, uint16_t *FilenameLen, char *Extname)
{
	uint16_t i, j, k;
	uint16_t extpos;
	
	i = 0;
	j = 0;
	extpos = 0;
	k = 0;
	
	while ((sFilename[i] != 0x00) || (i <= 32))		//文件名长度超过32个字节会被截取
	{
		if (sFilename[i] == '.')		//搜索第一个'.'作为扩展名分隔符
			extpos = i;
		else if (extpos != 0)
		{
			Extname[j] = sFilename[i];
			j++;
			if (j > 3)
				break;
		}
		else if (extpos == 0)
		{
			Filename[k] = sFilename[i];
			k++;
			if (k >= 32)
			{
				*FilenameLen = k;
				break;
			}
		}
		i++;
	}

}

/******************************************************************
函数名称: int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
功能描述: 7bit解码
参数输入: pSrc-源编码串指针 pDst-目标字符串指针 nSrcLength-源编码串长度
参数输出: 
返 回 值: 目标字符串长度
其    他: 无
******************************************************************/
int gsmDecode7bit(const unsigned char* pSrc, char* pDst, int nSrcLength)
{
    int nSrc;        	//源字符串的计数值
    int nDst;        	//目标解码串的计数值
    int nByte;       	//当前正在处理的组内字节的序号,范围是0-6
    unsigned char nLeft;//上一字节残余的数据
   
    //计数值初始化
    nSrc = 0;
    nDst = 0;
   
    //组内字节序号和残余数据初始化
    nByte = 0;
    nLeft = 0;
   
    //将源数据每7个字节为一组，解压缩成8个字节
    //循环该处理过程，直至源数据被处理完
    //如果分组不到7字节,也能正确处理
    while(nSrc<nSrcLength)
    {
        //将源字节右边部分与残余数据相加，去掉最高位，得到一个目标解码字节
        *pDst = ((*pSrc << nByte) | nLeft) & 0x7f;
        //将该字节剩下的左边部分，作为残余数据保存起来
        nLeft = *pSrc >> (7-nByte);
   
        //修改目标串的指针和计数值
        pDst++;
        nDst++;
   
        //修改字节计数值
        nByte++;
   
        //到了一组的最后一个字节
        if(nByte == 7)
        {
            //额外得到一个目标解码字节
            *pDst = nLeft;
   
            //修改目标串的指针和计数值
            pDst++;
            nDst++;
   
            //组内字节序号和残余数据初始化
            nByte = 0;
            nLeft = 0;
        }
   
        //修改源串的指针和技术值
        pSrc++;
        nSrc++;
    }
   
    *pDst = 0;
   
    //返回目标串长度
    return nDst;
}
/**
 * @brief	十六进制转换为BCD
 *
 * @param	
 *
 * @return  
 */
uint8_t HexToBCD(uint8_t val)	
{
	uint8_t tmp;

	tmp = ( val /10 );
	tmp = ( tmp << 4 );
	tmp += ( val % 10 );

	return tmp;
}
/**
 * @brief	十六进制时间转换为BCD格式
 *
 * @param	
 *
 * @return  
 */
void TimeHexToBcd(HexDateTime_t *dest ,uint8_t *src)
{
	dest->Year = HexToBCD(*src++);
	dest->Month= HexToBCD(*src++);
	dest->Day= HexToBCD(*src++);
	dest->Hour= HexToBCD(*src++);
	dest->Minute= HexToBCD(*src++);
	dest->Second = HexToBCD(*src);
}
/******************************************************************
函数名称: int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
功能描述: 7bit编码
参数输入: pSrc-源字符串指针 pDst-目标编码串指针 nSrcLength-源字符串长度
参数输出: 
返 回 值: 目标编码长度
其    他: 无
******************************************************************/
int gsmEncode7bit(const char* pSrc, unsigned char* pDst, int nSrcLength)
{
    int nSrc;        	//源字符串的计数值
    int nDst;        	//目标编码串计数值
    int nChar;       	//当前正在处理的组内字符字节的序号,范围是0-7
    unsigned char nLeft;//上一字节残余的数据
   
    // 计数值初始化
    nSrc = 0;
    nDst = 0;
   
    // 将源串每8个字节分为一组,压缩成7个字节
    // 循环该处理过程，直至源串被处理完
    // 如果分组不到8字节,也能正确处理
    while(nSrc<nSrcLength)
    {
        //取源字符串的计数值的最低3位
        nChar = nSrc & 7;
   
        //处理源串的每个字节
        if(nChar == 0)
        {
            //组内第一个字节,只是保存起来,待处理下一个字节时使用
            nLeft = *pSrc;
        }
        else
        {
            //组内其他字节,将其右边部分与残余数相加,得到一个目标编码字节
            *pDst = (*pSrc << (8-nChar)) | nLeft;
   
            //将该字节剩下的左边部分作为残余数据保存起来
            nLeft = *pSrc >> nChar;
            //修改目标串的指针和计数值
			pDst++;
            nDst++;
        }
       
        //修改源串的指针和计数值
        pSrc++; 
		nSrc++;
    }
   
    //返回目标串长度
    return nDst;
}

void SystimeToBCDTime(HexDateTime_t *dest,HexDateTime_t src )
{
	dest->Year = HexToBCD( src.Year );
	dest->Month= HexToBCD( src.Month );
	dest->Day= HexToBCD( src.Day);
	dest->Hour= HexToBCD( src.Hour );
	dest->Minute= HexToBCD( src.Minute );
}



/****************************************************************************
** 函数名称：CheckSum
** 函数功能：校验数据,返回累加和
** 入口参数：需要校验的字符串指针
** 出口参数：无
****************************************************************************/
uint8_t CheckSumNoC(uint8_t *Array,uint16_t len_Array)
{
	uint16_t i;
	uint8_t Check = 0;

	//计算校验和
	for(i = 0;i<len_Array;i++)
	{
		Check += Array[i];
	}
	
	return Check;
}


/**
 * @brief	ASCII转换为16进制
 *
 * @param	huart	串口句柄
 *
 * @return  
 */
uint8_t StrToTime(uint8_t* pStr)
{
	return ( (pStr[1]-0x30) + (pStr[0]-0x30) * 10 );
}



void* mem_mem(const void* haystack, size_t haystack_len,
              const void* needle, size_t needle_len)
{
    if (haystack == NULL || needle == NULL || haystack_len < needle_len)
        return NULL;

    const unsigned char* h = (const unsigned char*)haystack;
    const unsigned char* n = (const unsigned char*)needle;

    for (size_t i = 0; i <= haystack_len - needle_len; ++i) {
        if (memcmp(h + i, n, needle_len) == 0)
            return (void*)(h + i);
    }
    return NULL;
}