
#include "WT588F02b.h"
#include "BSP.h"


 
 
/*--------------------------------------
 ;模块名称:Line_2A_WT588F(UC8 DDATA)
 ;功
能:实现二线串口通信函数
;入
;出
参:DDATA为发送数据
参:
 ;CLK_2A
 ;P_DATA_2A
时钟线
数据线
;-------------------------------------*/

void Line_2A_WT588F(uint8_t DDATA)
{
	uint8_t S_DATA,j;
	uint8_t B_DATA;

	CLK_2A(1);           	//拉高CLK
	DATA_2A(1);
	S_DATA=DDATA;          	//拉高DATA
	CLK_2A(0);              //拉低CLK
	delay_ms(5);            //延时5ms
	B_DATA=S_DATA&0X01;
	for(j = 0; j < 8; j++)
	{
		CLK_2A(0);             //拉低CLK		
		DATA_2A(B_DATA);       //传输数据一位
		delay_us(300);         //延时300us		
		CLK_2A(1);             //拉高CLK
		delay_us(300);         //延时300us
		S_DATA=S_DATA>> 1;
		B_DATA=S_DATA&0X01;
	}
	DATA_2A(1);
	CLK_2A(1);
}
 
//  void Line_1A_WT588F(uint8_t DDATA)
// {
// uint8_t S_DATA,j;
// uint8_t B_DATA;
// S_DATA= DDATA;
// P_DATA(0);
// delay_ms(5);
// B_DATA=S_DATA&0X01;
// for(j = 0; j < 8; j++)
// {
// if(B_DATA== 1)
// {
// P_DATA(1);
// delay_us(600);
// P_DATA(0);
// delay_us(200);
// }
// else
// {
// P_DATA(1);
// delay_us(200);
// P_DATA(0);
// delay_us(600);
// }
// S_DATA=S_DATA>> 1;
// //延时5ms
// //延时600us
// //延时200us
// //延时200us
// //延时600us-第 12 页
//B_DATA=S_DATA&0X01;
// }
// P_DATA(1);
// }








