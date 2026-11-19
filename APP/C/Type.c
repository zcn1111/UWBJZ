#include "BSP.h"
#include "math.h"
#include "Type.h"
#include "g_GlobalValDef.h"
#include "app_dtu.h"
/******************************************************************
函数名称: void InitDebugCmdPara
功能描述: 初始化调试命令参数
参数输入: DbgCmdPara_t	*Para	调试命令参数
参数输出: 无
返 回 值: 无
其    他:
******************************************************************/
void InitDebugCmdPara(DbgCmdPara_t *Para)
{
	memset(Para->CmdName, 0x00, 20);	//命令字初始化
	Para->cbCmdFunc = NULL;				//回调函数初始化
	Para->cbCmdHelpFunc = NULL;			//帮助函数初始化
}
/******************************************************************
函数名称: int16_t ExecDebugCmdFromCmdTable
功能描述: 命令表数据解析
参数输入: DbgCmdGroups_t		CmdTable	命令表
		  uint8_t			*InBuf		输入缓存
参数输出: 
返 回 值: 
其    他:
******************************************************************/
int16_t ExecDebugCmdFromCmdTable(DbgCmdGroups_t CmdTable, uint8_t *InBuf)
{
	uint16_t i, j;
	uint16_t slen;
	int16_t dbgret=DBGRET_OK;
	
	DbgCmdPara_t	para;
	char	*argv[10];
	uint8_t argc=0;
	char	*pstrchr=NULL;
	char 	*ptr=NULL;
	
	uint8_t			step;
	
	/* 初始化 */
	InitDebugCmdPara(&para);
//	for (i=0; i<10; i++)
//	{
//		argv[i] = pvPortMalloc(128);
//	}
	
	/* 处理接收字符 分离命令字和参数 */
	slen = strlen((char*)InBuf);
	if (slen == 0)
		return DBGRET_ERR_NODATA;
	else 
	{
		pstrchr = (char*)InBuf;
	}
	
	step = 0;
	i = 0;
	/* 按空格拆分字符串 */
	ptr = NULL;
	do
	{
		argv[i] = strtok_r(pstrchr, " ", &ptr);	//按空格拆分
		if (argv[i] != NULL)
		{
			i++;
			pstrchr = NULL;
		}
	} while (argv[i] != NULL);
	argc = i;
	
	slen = strlen(argv[0]);
	if (slen >= 20) slen = 20;
	for (i=0; i<slen; i++)
	{
		para.CmdName[i] = argv[0][i];
	}
	
	/* 查询命令表 执行命令 */
	dbgret = DBGRET_ERR_NOMATCH;
	for (i=0; i<CmdTable.CmdTotalNbr; i++)
	{
		if (strcasecmp(CmdTable.CmdPara[i].CmdName, para.CmdName) == 0)
		{
			dbgret = DBGRET_OK;
			dbgret = CmdTable.CmdPara[i].cbCmdFunc(argc-1, argv);
			break;
		}
	}
	return dbgret;
}

/******************************************************************
函数名称: void DebugRecvProcess(void)
功能描述: 调试数据接收函数
参数输入: 无
参数输出: uint8_t *OutBuf
返 回 值: 
其    他:
******************************************************************/
int16_t DebugRecvProcess(void)
{
	uint16_t i, j;
	uint16_t size;
	uint16_t finflag;
	char ch;
	
	size = GetRingUsedLen(&g_Uart3RxRing);
	if (size == 0)
		return DBGRET_ERR_NODATA;
	finflag = 0;
	j = 0;
	memset(g_DebugDatBuf, 0x00, sizeof(g_DebugDatBuf));
	for (i=0; i<size; i++)
	{
		RingBufPop(&g_Uart3RxRing, (uint8_t*)&ch, 1);
		switch (finflag)
		{
			case 0:
				if (ch == 0x0D)
				{
					finflag = 1;
				}
				else
				{
					g_DebugDatBuf[j] = ch;
					finflag = 0;
					j++;
				}
				break;
			case 1:
				if (ch == 0x0A)
				{
					finflag = 2;
					if (i == (size - 1))
					{
//						if (CommMode == 2)
//						{
//							g_DebugDatBuf[j] = 0x0D;
//							g_DebugDatBuf[j+1] = 0x0A;
//							BleSize = j+2;
//						}
					}
				}
				else
				{
					g_DebugDatBuf[j] = ch;
					finflag = 0;
					j++;
				}
				break;
			case 2:
				finflag = 3;
				break;
			default:
				break;
		}
		if (finflag == 3)
		{
//			if (CommMode == 1)
//			{
//				g_DebugDatBuf[j+1] = 0x0D;
//				g_DebugDatBuf[j+2] = 0x0A;
//				BleSize = j+2;
//			}

			break;
		}
	}
	
	if (CommMode == 1)
	{
		BleSize = j;
	}
	else if ((CommMode == 2) || (RtkCommMode == 1))
	{
		g_DebugDatBuf[j] = 0x0D;
		g_DebugDatBuf[j + 1] = 0x0A;
		BleSize = j + 2;
	}
	return DBGRET_OK;
}
void TypeProcess(void)
{
	int16_t dbgret;
	uint16_t i;
	/* Debug */
	dbgret = DebugRecvProcess();
	if (dbgret == DBGRET_OK)
	{
		if ((CommMode == 1) || (CommMode == 2))
		{
			/* 退出转发模式 */
			if (strcasecmp((char*)g_DebugDatBuf, "!BLE") == 0)
			{
				debug_printf("BLE exit mode!\r\n");
				if (CommMode == 2)
				{
					HAL_GPIO_WritePin(BT_MODESW_PORT, BT_MODESW_PIN, GPIO_PIN_RESET);
					BSP_UartSetupPara(&hLPUART1, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);	//切换波特率
					BSP_UartBindInstance(&hLPUART1, LPUART1);
				}
				CommMode = 0;
			}
			else
			{
				if (CommMode == 2)
				{
					HAL_UART_Transmit(&hLPUART1, g_DebugDatBuf, BleSize, 1000);
				}
				else if (CommMode == 1)
				{
					HAL_UART_Transmit(&hLPUART1, g_DebugDatBuf, BleSize, 1000);
				}
			}
		}
		if (RtkCommMode == 1)
		{
			/* 退出转发模式 */
			if (strcasecmp((char*)g_DebugDatBuf, "!RTK\r\n") == 0)
			{
				debug_printf("RTK exit trans mode!\r\n");
				RtkCommMode = 0;
			}
			else
			{
				HAL_UART_Transmit(&hUSART1, g_DebugDatBuf, BleSize, 1000);
			}
		}
		if ((CommMode == 0) && (RtkCommMode == 0))
		{
			/* 列出命令 */
			if (strcasecmp((char*)g_DebugDatBuf, "cmdlist") == 0)
			{
				ListRegisteredCmd();
			}
			/* 进入数据转发模式 */
			else if (strcasecmp((char*)g_DebugDatBuf, "#BLE") == 0)
			{
				HAL_GPIO_WritePin(BT_MODESW_PORT, BT_MODESW_PIN, GPIO_PIN_RESET);
				BSP_UartSetupPara(&hLPUART1, 115200, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
				BSP_UartBindInstance(&hLPUART1, LPUART1);
				debug_printf("BLE trans mode!\r\n");
				CommMode = 1;
			}
			else if (strcasecmp((char*)g_DebugDatBuf, "##BLE") == 0)
			{
				BSP_UartSetupPara(&hLPUART1, 9600, UART_WORDLENGTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
				BSP_UartBindInstance(&hLPUART1, LPUART1);
				HAL_GPIO_WritePin(BT_MODESW_PORT, BT_MODESW_PIN, GPIO_PIN_SET);
				debug_printf("BLE command mode!\r\n");
				CommMode = 2;
			}
			else if (strcasecmp((char*)g_DebugDatBuf, "#RTK") == 0)
			{
				debug_printf("RTK trans mode!\r\n");
				RtkCommMode = 1;
			}
			else if(memcmp(g_DebugDatBuf,"#Cnum_Write,",12) == 0)
			{//设置本机号
				for(i=0;i<6;i++)
				{					
					g_MobileInFlash.mobileno[4][i] = Asc2Bcd(g_DebugDatBuf[12+i*2],g_DebugDatBuf[13+i*2]); 
				}
				WriteMobileNoToFLASH();         /*存入flash*/					
				CelluarStep = STA_CLOSE_TCP;
				debug_printf("Cnum_Write,OK!\r\n");
			}
			else if(memcmp(g_DebugDatBuf,"#Cnum_Read",10) == 0)
			{//读取本机号
				uint8_t MobNum[12];
				ReadMobileNoFromFLASH();
				for(i=0;i<6;i++)
				{					
					Bcd2Asc(g_MobileInFlash.mobileno[4][i],&MobNum[i*2]);
				}
				debug_printf("Cnum_Read,%s,OK!\r\n",MobNum);					
			}
			else if(memcmp(g_DebugDatBuf,"#ID_Write,",10) == 0)
			{//设置ID号
				for(i=0;i<7;i++)
				{					
					g_IDInFlash.IDnum[i] = Asc2Bcd(g_DebugDatBuf[10+i*2],g_DebugDatBuf[11+i*2]); 
				}						
				WriteIDToFlash();
				g_ChangeIPFlag = 1;
				CelluarStep = STA_CLOSE_TCP;
				debug_printf("ID_Write,OK!\r\n");
			}
			else if(memcmp(g_DebugDatBuf,"#ID_Read",8) == 0)
			{//读取ID号
				uint8_t MobNum[14];
				ReadIDFromFlash();
				for(i=0;i<7;i++)
				{					
					Bcd2Asc(g_IDInFlash.IDnum[i],&MobNum[i*2]);
				}					
				debug_printf("ID_Read,%s,OK!\r\n",MobNum);	
			}
			else if(memcmp(g_DebugDatBuf,"#Equipnum_Write,",16) == 0)
			{//写设备编号		
				
				memset(EquipmentNum,0x00,sizeof(EquipmentNum));
				memcpy(EquipmentNum,g_DebugDatBuf+16,sizeof(EquipmentNum));      
				Write_EQUIPNUM_EEPROM();
				debug_printf("Equipnum_Write,OK!\r\n");
			}
			else if(memcmp(g_DebugDatBuf,"#Equipnum_Read",14) == 0)
			{//读取设备编号
				Read_EQUIPNUM_EEPROM();
				debug_printf("Equipnum_Read,%s,OK!\r\n",EquipmentNum);	
			}
			else if(memcmp(g_DebugDatBuf,"#HWVer_Write,",13) == 0)
			{//写入硬件版本号		
				
				memset(HardwareVer,0x00,sizeof(HardwareVer));
				memcpy(HardwareVer,g_DebugDatBuf+13,sizeof(HardwareVer));      				
				Write_HARDWAREVER_EEPROM();
				debug_printf("HWVer_Write,OK!\r\n");
			}
			else if(memcmp(g_DebugDatBuf,"#HWVer_Read",11) == 0)
			{//读取硬件版本号
				Read_HARDWAREVER_EEPROM();
				debug_printf("#HWVer_Read,%s,OK!\r\n",HardwareVer);	
			}
			else if(memcmp(g_DebugDatBuf,"#IP_Write,",10) == 0)
			{//写入IP		
				uint8_t ip_hex[4];
				uint16_t port_hex;
				ReadServerInfoPar();  
				uint8_t cmpipbuf[2][6];												
				memset(cmpipbuf[0],0,sizeof(cmpipbuf[0]));
				memset(cmpipbuf[1],0xff,sizeof(cmpipbuf[1]));								

				int ip0, ip1, ip2, ip3, port0;
				// 解析格式：固定前缀 + 数字.数字.数字.数字:数字
				if (sscanf((char *)g_DebugDatBuf, "#IP_Write,%d.%d.%d.%d:%d", &ip0, &ip1, &ip2, &ip3, &port0) != 5)
				{
					debug_printf("IP_Write,ERR!\r\n");
				}
				else
				{
					if (ip0 < 0 || ip0 > 255 || ip1< 0 || ip1 > 255 || ip2 < 0 || ip2 > 255 || ip3 < 0 || ip3 > 255 ||
						port0 < 0 || port0 > 65535) 
					{
						debug_printf("IP_Write,ERR!\r\n");
					}
					else
					{
						ip_hex[0] = (uint8_t)ip0;
						ip_hex[1] = (uint8_t)ip1;
						ip_hex[2] = (uint8_t)ip2;
						ip_hex[3] = (uint8_t)ip3;
						port_hex = (uint16_t)port0;	
						if( memcmp(ip_hex,cmpipbuf[0],4)== 0 || memcmp((uint8_t *)&port_hex,cmpipbuf[0],2)== 0 
												|| memcmp(ip_hex,cmpipbuf[1],4) == 0 || memcmp((uint8_t *)&port_hex,cmpipbuf[1],2)== 0 )
						{
							debug_printf("IP_Write,ERR!\r\n");
						}
						else
						{
							memcpy(g_ServerInfo.g_ServerIPAddr[2],ip_hex,4);
							g_ServerInfo.g_ServerPortNo[2] = port_hex;
							WriteServerInfoPar();
							debug_printf("IP_Write,OK!\r\n");							
							ReadServerInfoPar();                             //从Flash读出
							g_ChangeIPFlag = 1;
							CmpIPAndPort();                                         //若有全零或全F的替换为默认值
							CelluarStep = STA_CLOSE_TCP;//重连服务器					
						}				      				
					}
				}
			}
			else if(memcmp(g_DebugDatBuf,"#IP_Read",8) == 0)
			{//读IP
				ReadServerInfoPar();
				debug_printf("IP_Read,%d.%d.%d.%d:%d,OK!\r\n",g_ServerInfo.g_ServerIPAddr[2][0],g_ServerInfo.g_ServerIPAddr[2][1],g_ServerInfo.g_ServerIPAddr[2][2],g_ServerInfo.g_ServerIPAddr[2][3],g_ServerInfo.g_ServerPortNo[2]);	
			}
			else if(memcmp(g_DebugDatBuf,"#ICCID_Read",11) == 0)
			{//读IP				
				debug_printf("ICCID_Read,%s,OK!\r\n",iccidbuff);	
			}
			else
			{
//				/* 处理命令对应功能 */				
//				dbgret = ExecDebugCmdFromCmdTable(g_CmdTable, g_DebugDatBuf);
//				if ((dbgret & DBGRET_ERR_NOMATCH) == DBGRET_ERR_NOMATCH)
//				{
//					debug_printf("Command not found!\r\n");
//				}
			}
		}
	}
}

