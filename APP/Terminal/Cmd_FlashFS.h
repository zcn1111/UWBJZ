/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMD_FLASHFS_H
#define __CMD_FLASHFS_H

/* 引用头文件 */
#include "TerminalConf.h"


/******* 数据结构体定义 *******/

/******* 外部变量声明 *******/

/******* 外部函数声明 *******/
int16_t Cmd_MountNand(uint8_t Argc, char *Argv[]);
int16_t Cmd_UnMountNand(uint8_t Argc, char *Argv[]);
int16_t Cmd_FormatNand(uint8_t Argc, char *Argv[]);
int16_t Cmd_ListDir(uint8_t Argc, char *Argv[]);
int16_t Cmd_DeleteFile(uint8_t Argc, char *Argv[]);
int16_t Cmd_GetFreeSpace(uint8_t Argc, char *Argv[]);
int16_t Cmd_WriteFile(uint8_t Argc, char *Argv[]);
int16_t Cmd_ReadFile(uint8_t Argc, char *Argv[]);


#endif /* __CMD_FLASHFS_H */
