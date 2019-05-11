#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "tpad.h"
#include "beep.h"
#include "myiic.h"
#include "timer.h"


/************************************************
 ALIENTEK 战舰STM32F103开发板实验0
 工程模板
 注意，这是手册中的新建工程章节使用的main文件 
 技术支持：www.openedv.com
 淘宝店铺：http://eboard.taobao.com 
 关注微信公众平台微信号："正点原子"，免费获取STM32资料。
 广州市星翼电子科技有限公司  
 作者：正点原子 @ALIENTEK
************************************************/


 int main(void)
 {	
	//变量定义
	vu8 key=0;
	//u8 timecount=10;


	//函数初始化
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
 	LED_Init();			     //LED端口初始化
    TPAD_Init(72);			//初始化触摸按键
	BEEP_Init();	//蜂鸣器初始化
	TIM3_Int_Init(4999,7199);//10Khz的计数频率，计数到5000为500ms 
	KEY_Init();//按键初始化
	 

	//测试工程代码
	//测试LED0,LED1和蜂鸣器硬件状态
	TIM_Cmd(TIM3, DISABLE);  //失能TIMx
	LED0=1;//关
	LED1=1;
	BEEP=0;
	delay_ms(1000);
	LED0=0;//开
	LED1=0;
	BEEP=1;
	delay_ms(1000);
	printf("\r\n战舰STM32开发板 串口实验\r\n");  //测试串口通信状态
	printf("正点原子@ALIENTEK\r\n\r\n");
	BEEP=0; //关
	LED0=1;
	LED1=1;
	
	//测试定时器的代码
	//TIM_Cmd(TIM3, ENABLE);  //使能TIMx
	
	
	while(1)
	{
		//测试触摸按键硬件状态
		if(TPAD_Scan(0))
		{
			//LED0=0;
			LED1=0;
			BEEP=0;
			delay_ms(250);
			//LED0=1;
			LED1=1;
			BEEP=1;
			delay_ms(250);
			
		}
		
		//测试三个按键硬件状态
 		key=KEY_Scan(1);	//得到键值
	   	if(key)
		{						   
			switch(key)
			{				 
				case WKUP_PRES:	//控制蜂鸣器
					BEEP=1;
                delay_ms(200); 
					break;
				case KEY2_PRES:	//控制LED0翻转
					LED0=!LED0;
                delay_ms(200); 
					break;
				case KEY1_PRES:	//控制LED1翻转	 
					LED1=!LED1;
                delay_ms(200); 
					break;
				case KEY0_PRES:	//同时控制LED0,LED1翻转 
					LED0=!LED0;
					LED1=!LED1;
                delay_ms(200); 
					break;
			}
		}else 
		{
		}
		BEEP=0;
	}
 }
