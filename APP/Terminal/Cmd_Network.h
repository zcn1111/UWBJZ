/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CMD_NETWORK_H
#define __CMD_NETWORK_H

/* 引用头文件 */
#include "TerminalConf.h"


/******* 数据结构体定义 *******/

/******* 外部变量声明 *******/

/******* 外部函数声明 *******/

int16_t Cmd_SendDataByNetwork(uint8_t Argc, char *Argv[]);
int16_t Cmd_GetCH395Version(uint8_t Argc, char *Argv[]);
int16_t Cmd_GetCH395EthInfo(uint8_t Argc, char *Argv[]);
int16_t Cmd_CH395Init(uint8_t Argc, char *Argv[]);
int16_t Cmd_CH395Connect(uint8_t Argc, char *Argv[]);


#endif /* __CMD_NETWORK_H */
