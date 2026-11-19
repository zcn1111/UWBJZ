/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMD_NAND_H
#define __CMD_NAND_H

/* 引用头文件 */
#include "TerminalConf.h"


/******* 数据结构体定义 *******/

/******* 外部变量声明 *******/

/******* 外部函数声明 *******/
int16_t Cmd_EraseNand(uint8_t Argc, char *Argv[]);
int16_t Cmd_LowFormatNand(uint8_t Argc, char *Argv[]);
int16_t Cmd_CalcECC(uint8_t Argc, char *Argv[]);
int16_t Cmd_NandBlockInfo(uint8_t Argc, char *Argv[]);


#endif /* __CMD_NAND_H */
