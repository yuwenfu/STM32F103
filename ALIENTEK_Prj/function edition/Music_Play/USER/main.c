#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "lcd.h"  
#include "key.h"     
#include "usmart.h" 
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "text.h" 
#include "vs10xx.h"
#include "mp3player.h"	
#include "SendData.h"
#include "sram.h"	 
 
 
/************************************************
 ALIENTEK战舰STM32开发板实验43
 音乐播放器 实验
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


 int main(void)
 {	 
//	FIL fp1; FIL fp2; UINT btw1;UINT btw2;
//	 u8 buf1[50],buf2[50];
 
	delay_init();	    	 //延时函数初始化	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	 	//串口初始化为115200
	usmart_dev.init(72);		//初始化USMART
	 LED_Init();		  			//初始化与LED连接的硬件接口
	KEY_Init();					//初始化按键
	LCD_Init();			   		//初始化LCD     
	W25QXX_Init();				//初始化W25Q128
 	VS_Init();	  				//初始化VS1053 
	//FSMC_SRAM_Init();			//初始化外部SRAM  
	my_mem_init(SRAMIN);		//初始化内部内存池
	//my_mem_init(SRAMEX);		//初始化外部内存池
	exfuns_init();				//为fatfs相关变量申请内存  
 	f_mount(fs[0],"0:",1); 		//挂载SD卡 
 	f_mount(fs[1],"1:",1); 		//挂载FLASH.
	POINT_COLOR=RED;       
 	while(font_init()) 				//检查字库
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//清除显示	     
	}
 	Show_Str(30,50,200,16,"战舰 STM32开发板",16,0);				    	 
	Show_Str(30,70,200,16,"音乐播放器实验",16,0);				    	 
	Show_Str(30,90,200,16,"正点原子@ALIENTEK",16,0);				    	 
	Show_Str(30,110,200,16,"2015年1月20日",16,0);
	Show_Str(30,130,200,16,"KEY0:NEXT   KEY2:PREV",16,0);
	Show_Str(30,150,200,16,"KEY_UP:VOL+ KEY1:VOL-",16,0);
	while(1)
	{
  		LED1=0; 	  
		Show_Str(30,170,200,16,"存储器测试...",16,0);
		printf("Ram Test:0X%04X\r\n",VS_Ram_Test());//打印RAM测试结果	    
		Show_Str(30,170,200,16,"正弦波测试...",16,0); 	 	 
		VS_Sine_Test();	   
		Show_Str(30,170,200,16,"<<音乐播放器>>",16,0); 		 
		LED1=1;
		mp3_play();
		
		
// 数据发送程序
//		if(DataSendInit()==0)
//		{
//			LCD_ShowString(30,330,310,16,24,"In main function!");
//			LED0=0;
//			LED1=0;
//				
//		}
//		while(1){LED1=0;delay_ms(500);LED1=1;}
//	
		
		
	//fatfs测试代码	
		///FIL fp1; FRESULT state01; UINT btw1,btw2;u8 buf1[50],buf2[50];	
		//这是文件操作
//		LCD_ShowString(30,330,310,16,24,"In main function!");
//		Show_Str(30,400,310,24,"文件操作",24,0);
//		f_open(&fp1,(const TCHAR*)"0:/TEXT/测试文本001.txt",FA_OPEN_EXISTING|FA_READ);
//		f_read(&fp1,buf1,30,&btw1);
//		f_close(&fp1);
//	
//		
//		f_open(&fp2,(const TCHAR*)"0:/TEXT/test001.txt",FA_OPEN_EXISTING|FA_READ);
//		f_read(&fp2,buf2,30,&btw2);
//		f_close(&fp2);
//	
//		Show_Str(30,450,310,24,buf1,24,0);
//		Show_Str(30,490,310,24,buf2,24,0);
	
	}			
}


















