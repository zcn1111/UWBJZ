/******************************************************************
版权所有:Copyright (C), 2015, 山西世恒技术有限公司
项目名称: IV型车载
文件名称: DTU_Update.c
作    者: GX
版    本: Ver1.0
完成日期:
功能描述: DTU 升级
其他说明:
功能列表:
******************************************************************/

#include "DTU_Update.h"

#include "g_func.h"
#include "g_GlobalValDef.h"
#include "string.h"

#include "BSP.h"

#include "DTU_Comm.h"
#include "DTU_Process.h"
#include "Update.h"

/********* 变量定义 **********/



/********* 缓存变量定义 **********/

/********* 函数声明 **********/
int16_t Locate_RecvAskPosition(void);


/******************************************************************
函数名称: int16_t DTU_RecvUpdateCmd
功能描述: DTU接收升级命令
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_RecvUpdateCmd(void)
{
//	int16_t updret;
//	
//	int16_t ret;
//	
//	updret = Update_RecvData(&g_UpdatePara, g_DtuDataBuf);			//升级命令接收
//	/* 包头错误 */
//	if (updret == UPD_ERR_PKTHEAD)
//	{
//		ret = DTU_COMM_ERR_NOHEADER;
//	}
//	else if (updret == UPD_ERR_CHKSUM)
//	{
//		ret = DTU_COMM_ERR_CHKSUM;
//	}
//	else if (updret == UPD_ERR_PROGTYPE)
//	{
//		ret = UPD_STA_FINISH;				//类型错误 结束升级
//	}
//	
//	ret = updret;
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_RecvUpdateData
功能描述: DTU接收升级数据
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_RecvUpdateData(void)
{
//	int16_t updret;
//	
//	int16_t ret;
//	
//	updret = Update_RecvData(&g_UpdatePara, g_DtuDataBuf);			//升级命令接收
//	/* 包头错误 */
//	if (updret == UPD_ERR_PKTHEAD)
//	{
//		ret = DTU_COMM_ERR_NOHEADER;
//	}
//	else if (updret == UPD_ERR_CHKSUM)
//	{
//		ret = DTU_COMM_ERR_CHKSUM;
//	}
//	else if (updret == UPD_ERR_PROGTYPE)
//	{
//		ret = UPD_STA_FINISH;				//类型错误 结束升级
//	}
//	
//	ret = updret;
//	return ret;
}

/******************************************************************
函数名称: int16_t DTU_SendUpdateRequest
功能描述: DTU发送升级索要
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_SendUpdateRequest(void)
{
//	memset(g_DtuDataBuf, 0x00, sizeof(g_DtuDataBuf));
//	Update_GenerateRequest(&g_UpdatePara, g_DtuDataBuf);
//	DTU_SendDataByInCommon(g_DtuDataBuf, 16, g_MobileInFlash.BJnum, "EE");
//	
//	return DTU_COMM_OK;
}

/******************************************************************
函数名称: int16_t DTU_SendUpdateFinish
功能描述: DTU发送升级结束
参数输入: 无
参数输出: 无
返 回 值: int16_t 错误代码
其    他: 无
******************************************************************/
int16_t DTU_SendUpdateFinish(void)
{
//	memset(g_DtuDataBuf, 0x00, sizeof(g_DtuDataBuf));
//	Update_GenerateFinish(&g_UpdatePara, g_DtuDataBuf);
//	DTU_SendDataByInCommon(g_DtuDataBuf, 16, g_MobileInFlash.BJnum, "EE");
//	
//	return DTU_COMM_OK;
}



