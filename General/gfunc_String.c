/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: gfunc_String.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: 通用处理函数(字符串处理类)
其他说明:
功能列表:
******************************************************************/

#include "gfunc_String.h"

#include "string.h"


/******************************************************************
函数名称: uint8_t CalcSpiltChrNum
功能描述: 计算分割字符总个数
参数输入: 
参数输出: 
返 回 值: uint8_t 返回分隔符个数 0=没有找到
其    他: 无
******************************************************************/
uint8_t CalcSpiltChrNum(char *InStr, uint16_t SLen, char SpiltChr)
{
	uint16_t i;
	
	uint8_t num;
	
	num = 0;
	/* 检查分割符 0x00不计算 */
	if (SpiltChr == 0x00)
	{
		return 0;
	}
	
	/* 计算分割符 */
	for (i=0; i<SLen; i++)
	{
		if (InStr[i] == SpiltChr) num++;
	}
	return num;
}

/******************************************************************
函数名称: void GetStrAfterSpiltChr
功能描述: 获取指定分隔符后的字符串
参数输入: 
参数输出: char*	OutStr	返回字符串地址 NULL=未找到对应的分隔符
返 回 值: char* 返回字符串 NULL=未找到对应分隔符
其    他: 无
******************************************************************/
void GetStrAfterSpiltChr(char *InStr, uint16_t SLen, char SpiltChr, uint8_t SpiltNum, char *OutStr)
{
	uint16_t i;
	uint8_t spiltcnt;
	uint16_t stpos, edpos;
	uint8_t step;
	
	char *ptr;
	
	ptr = NULL;
	/* 检查分割符 0x00不计算 */
	if ((SpiltChr == 0x00) || (SpiltNum == 0))
	{
		OutStr = NULL;
		return;
	}
	
	/* 查找分割符位置 */
	spiltcnt = 0;
	stpos = 0;
	edpos = 0;
	step = 0;
	for (i=0; i<SLen; i++)
	{
		/* 第一步：查找指定分隔符记录数据起始位置 */
		if (step == 0)
		{
			if (InStr[i] == SpiltChr)
			{
				spiltcnt++;
				if (spiltcnt == SpiltNum)
				{
					stpos = i + 1;
					step = 1;
				}
			}
		}
		/* 第二步：查找下一个分隔符或数据结尾 */
		else if (step == 1)
		{
			if (InStr[i] == SpiltChr)
			{
				edpos = i - 1;
				break;
			}
			else if ((InStr[i] == '\r') && (InStr[i+1] == '\n'))
			{
				edpos = i - 1;
			}
			else if (i == (SLen - 1))
			{
				edpos = i - 1;
			}
			
		}
	}
	
	/* 输出字符串 */
	/* 没有找到字符 */
	if (step == 0)
	{
		OutStr = NULL;
	}
	/* 找到空字符串 */
	if (edpos < stpos) memcpy(OutStr, "\0", 1);		//两个分隔符之间没有字符
	/* 找到非空字符串 */
	else
	{
		memcpy(OutStr, InStr + stpos, (edpos - stpos + 1));
		OutStr[(edpos - stpos) + 2] = '\0';
	}
}






