#ifndef __GFUNC_STRING_H
#define __GFUNC_STRING_H

/* 引用头文件 */
#include "stm32l4xx.h"

/******* 宏定义 *******/
/* 大小端 */
#define M_LITTLE_ENDIAN		0
#define M_BIG_ENDIAN		1

/******* 外部变量声明 *******/

/******* 外部函数声明 *******/

uint8_t CalcSpiltChrNum(char *InStr, uint16_t SLen, char SpiltChr);
void	GetStrAfterSpiltChr(char *InStr, uint16_t SLen, char SpiltChr, uint8_t SpiltNum, char *OutStr);

#endif /* __GFUNC_STRING_H */

