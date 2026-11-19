/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMD_SYS_H
#define __CMD_SYS_H

/* 引用头文件 */
#include "TerminalConf.h"


/******* 数据结构体定义 *******/

/******* 外部变量声明 *******/

/******* 外部函数声明 *******/
int16_t Cmd_GetFreeHeap(uint8_t Argc, char *Argv[]);
int16_t Cmd_GetTaskStateList(uint8_t Argc, char *Argv[]);
int16_t Cmd_SoftReset(uint8_t Argc, char *Argv[]);


#endif /* __CMD_SYS_H */
