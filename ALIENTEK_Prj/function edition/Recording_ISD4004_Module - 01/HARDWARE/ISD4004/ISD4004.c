#include "ISD4004.h"


//////////////////////////////////////////////////////////////////////////////////	 
//ISD4004驱动代码	   
//修改日期:2019/5/2
//版本：V1.0								  
////////////////////////////////////////////////////////////////////////////////// 

/*****************************************************************
函数名称：ISD4004_Init(void)
入口参数：无
出口参数：无
功能：初始化时钟以及与ISD4004相连接的IO口
******************************************************************/
void ISD4004_Init(void)
{
	
	//初始化PC口，设置CS、INT、RAC
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB,PE端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_4|GPIO_Pin_5;				 //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
	
	GPIO_SetBits(GPIOB,GPIO_Pin_12); 
	
	W25QXX_CS=1;				//SPI FLASH不选中
	ISD4004_CS = 1;
	SPI2_Init();		   	//初始化SPI
	SPI2_SetSpeed(SPI_BaudRatePrescaler_256);//设置为18M时钟,高速模式;APB2时钟速度为36M
	
}

/********************************************************************
函数名称：ISD4004_PwrUp()
入口参数：无
出口参数：无
功能：ISD4004的上电指令
********************************************************************/
void ISD4004_PwrUp(void)
{
	ISD4004_CS = 1;
	delay_ms(1);
	ISD4004_CS = 0; //使能器件
	SPI2_ReadWriteByte(ISD4004_PwrUpEnable);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	delay_ms(1);
	ISD4004_CS = 1;
}

/*****************************************************************
函数名称：ISD4004_StopMode(void)
入口参数：无
出口参数：无
功能：停止当前操作
********************************************************************/
void ISD4004_StopMode(void)
{
	ISD4004_CS = 1;
	delay_ms(1);
	ISD4004_CS = 0; //使能器件
	SPI2_ReadWriteByte(ISD4004_StopCrtWr);
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(0x00);
	delay_ms(1);
	ISD4004_CS = 1;	
}
/*****************************************************************
函数名称：ISD4004_Rec(u16 addr)
入口参数：u16 addr
出口参数：无
功能：从address处开始录音
********************************************************************/
void ISD4004_Rec(u16 addr)
{
	ISD4004_PwrUp();
	delay_ms(10);
	ISD4004_PwrUp();
	delay_ms(10);
	ISD4004_CS = 1;
	delay_ms(1);
	ISD4004_CS = 0; //使能器件	
	SPI2_ReadWriteByte(ISD4004_SetRecAddr);
	SPI2_ReadWriteByte(addr>>8);
	SPI2_ReadWriteByte(addr);
	SPI2_ReadWriteByte(ISD4004_RecAtCrtAddr);
	//SPI2_ReadWriteByte(addr>>8);
	//SPI2_ReadWriteByte(addr);	
	delay_ms(1);
	ISD4004_CS = 1;	
}	

/*****************************************************************
函数名称：ISD4004_PlayRec(u16 addr)
入口参数：u16 addr
出口参数：无
功能：从address处开始放音
********************************************************************/

void ISD4004_PlayRec(u16 addr)
{
	ISD4004_PwrUp();
	delay_ms(10);
	ISD4004_PwrUp();
	delay_ms(10);
	ISD4004_CS = 1;
	ISD4004_CS = 0; //使能器件	
	SPI2_ReadWriteByte(ISD4004_SetPlayAddr);
	SPI2_ReadWriteByte(addr>>8);
	SPI2_ReadWriteByte(addr);
	SPI2_ReadWriteByte(ISD4004_PlayAtCrtAddr);
	//SPI2_ReadWriteByte(addr>>8);
	//SPI2_ReadWriteByte(addr);
	ISD4004_CS = 1;		
	
	
}





