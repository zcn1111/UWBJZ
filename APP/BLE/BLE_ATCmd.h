#ifndef __BLE_ATCMD_H
#define __BLE_ATCMD_H

/* 引用头文件 */
#include "stm32l4xx.h"

#include "BSP.h"
#include "DTU_Comm.h"
#include "g_GlobalValDef.h"

#include "BLE_Comm.h"

/******* 宏定义 *******/
#define ATCMD_MAXNUM		20		//最大注册命令个数
#define ATPARA_MAXNUM		10		//最大AT参数个数
#define ATRESP_MAXNUM		6		//最大响应处理个数
#define AT_ARGVLEN_MAXNUM	16		//最大AT参数数据长度
#define AT_KEYLEN_MAXNUM	20		//最大AT指令关键字长度
#define AT_RESPLEN_MAXNUM	128		//最大AT响应数据长度


/* 命令类型 */
#define ATCMD_TYP_NONE		0x00		//无
#define ATCMD_TYP_SET		0x01		//命令可设置
#define ATCMD_TYP_QUERY		0x02		//命令可查询

/******* 状态定义 *******/
typedef enum
{
	ATRET_OK				= 0,	/* 0 = 成功 */
	ATRET_MEM_FAIL			= 1,	/* 1 = 申请内存失败 */
	ATRET_SEND_FAIL			= 2,	/* 2 = 发送失败 */
	ATRET_RECV_FAIL			= 3,	/* 3 = 接收失败 */
	ATRET_CHK_ERR			= 4,	/* 4 = 校验和错误 */
	ATRET_LEN_ERR			= 5,	/* 5 = 数据长度错误 */
	ATRET_NODATA_ERR		= 6,	/* 6 = 无数据 */
	ATRET_KEYWORD_ERR		= 7,	/* 7 = 关键字错误 */
	ATRET_TYPE_ERR			= 8,	/* 8 = 类型错误 */
	ATRET_INCOMPLETE_ERR	= 9,	/* 9 = 不完整的响应数据 */
	ATRET_FUNC_OVERMAX		= 20,	/* 20 = 注册函数超过最大数量 */
	ATRET_FUNC_NOTMATCH		= 21,	/* 21 = 函数不匹配 */
	ATRET_FUNC_REPEAT		= 22,	/* 22 = 函数重复 */
	ATRET_RESP_OK			= 100,	/* 100 = AT响应 OK */
	ATRET_RESP_ERROR		= 101, 	/* 101 = AT响应 ERROR */
	ATRET_RESP_OTHER		= 102,	/* 102 = AT响应 其他 */
	ATRET_RESP_PARA			= 110,	/* 110 = AT响应 参数 */
} ATResult;

/* AT关键字参数表结构体 */
typedef struct
{
	uint8_t Argc;
	char	*Argv[ATPARA_MAXNUM];
} ATCmdParaTable_t;

/* AT指令参数 */
typedef struct
{
	char 		CmdKeyword[AT_KEYLEN_MAXNUM];	//命令关键字 最大关键字长度=AT_KEYLEN_MAXNUM
	uint8_t		CmdType;						//命令类型
	uint8_t		Argc;							//命令参数个数
	char		*Argv[ATPARA_MAXNUM];			//命令参数 最大参数个数=ATPARA_MAXNUM
	
	ATResult	(*cbATFunc)(uint8_t Argc, char *Argv[]);	//回调函数
} ATCmd_t;

/* AT指令组 */
typedef struct
{
	uint8_t		CmdNbr;
	ATCmd_t		ATCmdTable[ATCMD_MAXNUM];
} ATCmdGroups_t;

/******* 数据结构体定义 *******/

/* 新增蓝牙协议 */


/******* 外部变量声明 *******/
extern ATCmdGroups_t	g_ATCmdTable;

extern char			*g_ATRespones[ATRESP_MAXNUM];
extern char			*g_ATCmdArgv[ATPARA_MAXNUM];


/******* 外部函数声明 *******/

ATResult AT_InitATCmdPara(ATCmd_t *Para);
ATResult AT_RegisterATCmd(char *KeyWord, uint8_t CmdType, uint8_t ParaArgc, char *ParaArgv[], ATResult (*cbAtCmdFunc)(uint8_t Argc, char *Argv[]), ATCmdGroups_t *ATCmdGrp);
ATResult AT_InitATCmdGroup(ATCmdGroups_t *ATCmdGrp);
ATResult AT_InitATMem(char *ptrMem[], uint8_t MaxNum, uint16_t MemSize);
ATResult AT_UnInitATMem(char *ptrMem[], uint8_t MaxNum);


ATResult AT_AnaysisCmd(char *InBuf, uint16_t BufLen, char *RespWord, ATCmdGroups_t *ATCmdGrp);
ATResult AT_GenerateCmd(char *KeyWord, uint8_t CmdType, ATCmdGroups_t *ATCmdGrp, char *OutBuf, uint16_t *BufLen);

ATResult AT_BLEBaudrate(uint8_t Argc, char *Argv[]);
ATResult AT_BLEPassword(uint8_t Argc, char *Argv[]);
ATResult AT_BLERole(uint8_t Argc, char *Argv[]);
ATResult AT_BLEMaxconn(uint8_t Argc, char *Argv[]);

#endif /* __BLE_ATCMD_H */

