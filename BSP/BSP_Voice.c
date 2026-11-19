#include "BSP.h"
#include "g_GlobalValDef.h"
#include "Drv_W25Qxx_QSPI.h"
#include "Update.h"

uint16_t SEND_SUM=0;   //发送的校验和
uint16_t REC_SUM=0;    //接收的校验和
uint8_t DATA_SONG[512]; //存放音频数据



void Get512Byte(uint16_t num)
{
	W25qxx_ReadBytes((uint8_t*)&DATA_SONG , FLASH_VOICE_START + num*512 , 256);	
	W25qxx_ReadBytes((uint8_t*)&DATA_SONG+256 , FLASH_VOICE_START + num*512+256 , 256);	
}


/*--------------------------------------
函数名称:SPI_WT588E_COMMAND(UC8DDATA,UC8Delay_Cnt)
功能:实现SPI控制指令,SPI模式0
入参: DDATA 为发送数据 Delay_Cnt为发送速度
出参:
----------------------------------------*/
void SPI_WT588E_COMMAND(uint8_t DDATA,uint8_t Delay_Cnt)
{
	uint8_t  S_DATA, j;
	bool      B_DATA;
	S_DATA =DDATA;
	B_DATA=S_DATA&0X80;
	for(j = 0; j < 8; j++)
	{
		VOICE_SI(B_DATA);   //传输数据一位
        VOICE_SCK(1);       //时钟线拉高
		delay_us(Delay_Cnt);
		VOICE_SCK(0);       //时钟线拉低
		delay_us(Delay_Cnt);
		S_DATA=S_DATA << 1;
		B_DATA=S_DATA&0X80;
	}
}
/*--------------------------------------
函数名称:SPI_WT588E_BYTE(UC8Delay_Cnt)
功能:    实现SPI接受数据校验,SPI模式1
入参:    Delay_Cnt 为发送速度
出参:    R_DATA为接收的数据
----------------------------------------*/
uint8_t SPI_WT588E_BYTE(uint8_t Delay_Cnt)
{
	uint8_t R_DATA, j;
	VOICE_SI(0);
	for(j = 0; j < 8; j++)
	{
		VOICE_SCK(1);                //时钟线拉高
		delay_us(Delay_Cnt);   
		VOICE_SCK(0);                //时钟线拉低
		R_DATA=R_DATA<<1;           
		if(VOICE_SO() == 1)           //接收一位数据
		{
			R_DATA =R_DATA|0x01;
		}
		else
		{
			R_DATA =R_DATA&0xFE;
		}
		delay_us(Delay_Cnt);
	}
	return R_DATA;
}



 /*--------------------------------------
函数名称:WT588E_PLAY(UC8DDATA)
功能:语音芯片播放指令
入参:
出参:
 ----------------------------------------*/
void WT588E_PLAY(uint8_t DDATA)
{
	VOICE_SCK(0);
	VOICE_CS(0);
	delay_ms(5);
	SPI_WT588E_COMMAND(0xF0, 100);
	SPI_WT588E_COMMAND(DDATA, 100);
	VOICE_CS(1);
}
 /*--------------------------------------
函数名称:DATA_CHANGE_All_START
功能:    语音数据全部更换握手指令
入参:
出参:
----------------------------------------*/
void DATA_CHANGE_All_START()
{
	VOICE_SCK(0);
	VOICE_CS(0);
	delay_ms(5);
	SPI_WT588E_COMMAND(0xE1, 100);
	delay_us(20);
	SPI_WT588E_COMMAND(0xFF,100);
	SEND_SUM=0x01E0;
	VOICE_CS(1);
}
 
/*--------------------------------------
函数名称:DATA_CHANGE_SINGLE_START
功能:    语音数据全部更换握手指令
入参:
出参:
----------------------------------------*/
void DATA_CHANGE_SINGLE_START(uint8_t Addr)
{
	VOICE_SCK(0);
	VOICE_CS(0);
	delay_ms(5);
	SPI_WT588E_COMMAND(0xE0, 100);
	delay_us(20);
	SPI_WT588E_COMMAND(Addr, 100);
	SEND_SUM=0xE0+Addr;
	VOICE_CS(1);
}
/*--------------------------------------
函数名称:DATA_CHANGE_End()
功能:    语音数据更换结束指令
入参:
出参:
----------------------------------------*/
void DATA_CHANGE_End()
{
	VOICE_SCK(0);
	VOICE_CS(0);
	delay_ms(5);
	SPI_WT588E_COMMAND(0xEF, 20);
	VOICE_CS(1);
}
/*--------------------------------------
函数名称:DATA_CHANGE_PACK_START
功能:    语音数据更换一个包的开始指令
入参:
出参:    1: 校验成功 0:校验失败
----------------------------------------*/
bool DATA_CHANGE_PACK_START()
{
	uint16_t SUM_TEMP=0;
	REC_SUM=0;
	VOICE_SCK(0);
	VOICE_CS(0);
	delay_ms(5);
	SPI_WT588E_COMMAND(0xDF, 20);
	delay_us(20);
	SUM_TEMP=SPI_WT588E_BYTE(20);
	REC_SUM=SUM_TEMP;
	delay_us(20);
	SUM_TEMP=SPI_WT588E_BYTE(20);
	SUM_TEMP=SUM_TEMP<<8;
	REC_SUM=REC_SUM+SUM_TEMP;
	VOICE_CS(1);
	if(SEND_SUM == REC_SUM)
	{
		return true;
	}
	else
	{
		return false;
	}
	
}
/*--------------------------------------
函数名称:DATA_CHANGE_PACK_SEND
功能:    语音数据发送一个包
入参:    DATA_BUF bin 文件存放区
         DATA_LEN 这个包的数据长度（完整512BYTE）
出参:1: 校验成功 0:校验失败
----------------------------------------*/
void DATA_CHANGE_PACK_SEND(uint8_t* DATA_BUF,uint16_t DATA_LEN)
{
	uint16_t SUM_TEMP =0;
	uint16_t j;
	SEND_SUM= 0;
	VOICE_CS(0);
	
	for(j = 0; j < DATA_LEN; j++)
	{
		delay_us(20);
		SPI_WT588E_COMMAND(*(DATA_BUF + j), 2);
		SUM_TEMP=*(DATA_BUF +j);
		j++;
		delay_us(20);
		SPI_WT588E_COMMAND(*(DATA_BUF + j), 2);
		SUM_TEMP=SUM_TEMP+(*(DATA_BUF+j)<<8);
		SEND_SUM=SEND_SUM+SUM_TEMP;
	}
	VOICE_CS(1);
}
///*--------------------------------------
//函数名称:DATA_CHANGE_All()
//功能:    语音数据全部更换
//入参:
//出参:    1: 更换成功 0:更换失败
//----------------------------------------*/
//bool DATA_CHANGE()
//{
//	uint16_t i =0;
//	DATA_CHANGE_All_START();
//	delay_ms(30);
//	for(i = 0; i < PACK_LEN; i++)
//	{
//		delay_ms(16);
//		Get512Byte(i);
//		if(DATA_CHANGE_PACK_START() == 1)
//		{
//			delay_ms(1);
//			DATA_CHANGE_PACK_SEND(DATA_SONG, 512);
//		}
//		else
//		{
//			return false;
//		}
//	}
//	DATA_CHANGE_End();
//	return true;
//}
/*--------------------------------------
函数名称:DATA_CHANGE_All()
功能:    语音数据全部更换
入参:
出参:    1: 更换成功 0:更换失败
----------------------------------------*/
bool DATA_CHANGE()
{
	uint16_t i =0;
	uint16_t PACK_LEN;
	PACK_LEN = ( VoiceFileSize.FileSize * 496 + 511) /512;
	DATA_CHANGE_All_START();
	delay_ms(30);
	for(i = 0; i < PACK_LEN; i++)
	{
		delay_ms(16);
		Get512Byte(i);
		if(DATA_CHANGE_PACK_START() == 1)
		{
			delay_ms(1);
			DATA_CHANGE_PACK_SEND(DATA_SONG, 512);
		}
		else
		{
			return false;
		}
	}
	DATA_CHANGE_End();
	return true;
}
/*--------------------------------------
函数名称:DATA_CHANGE_SINGLE(UC8Addr)
功能:    语音数据单首更换
入参:
出参:    1: 更换成功 0:更换失败
----------------------------------------*/
bool DATA_CHANGE_SINGLE(uint8_t Addr)
{
	uint16_t PACK_LEN;
	uint16_t i =0;
	DATA_CHANGE_SINGLE_START(Addr);
	delay_ms(30);
	for(i = 0; i < PACK_LEN; i++)
	{
		delay_ms(16);
		Get512Byte(i);
		if(DATA_CHANGE_PACK_START() == 1)
		{
			delay_ms(1);
			DATA_CHANGE_PACK_SEND(DATA_SONG, 512);
		}
		else
		{
			return false;
		}
	}
	DATA_CHANGE_End();
	return true;
}


void JudgeUpdateVoice(void)
{
	if(g_ShouldUpdateVoiceFlag == 0 && g_UpdateVoiceTypeFlag == 0 && g_UpFlag == 0)
	{
		ReadVoiceFileSizeFromFlash();
		if(VoiceFileSize.FileSize != 0 && VoiceFileSize.FileSize != 0xFFFF)
		{
			g_ShouldUpdateVoiceFlag = 1;
			g_UpFlag = 1;
			
		}			
	}
	else if(g_ShouldUpdateVoiceFlag == 1 && g_UpFlag == 1)
	{
		if( DATA_CHANGE()== true)
		{
			g_UpFlag = 0;
			VoiceFileSize.FileSize = 0;	
			WriteVoiceFileSizeToFlash();
			g_UpdateVoiceTypeFlag = 0;	
			g_ShouldUpdateVoiceFlag=0;
			//是否需要重启语音模块
		}	
	}
}









