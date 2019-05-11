#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"
#include "tpad.h"
#include "beep.h"
#include "myiic.h"
#include "timer.h"
#include "ISD4004.H"


/************************************************
 ALIENTEK ս��STM32F103������ʵ��0
 ����ģ��
 ע�⣬�����ֲ��е��½������½�ʹ�õ�main�ļ� 
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 �������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/
//void test(u32 x)
//{	
//	u32 y;
//	 GPIO_InitTypeDef  GPIO_InitStructure;
// 	
// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);	 //ʹ��PB,PE�˿�ʱ��
//	
// GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;				 //LED0-->PB.5 �˿�����
// GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
// GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.5

//	y=((u8)((x)>>16));
//	GPIOB->BRR=y;
//	
//}

 int main(void)
 {	
	//��������
	vu8 key=0;
	//u8 timecount=10;


	//������ʼ��
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
 	LED_Init();			     //LED�˿ڳ�ʼ��
    TPAD_Init(72);			//��ʼ����������
	BEEP_Init();	//��������ʼ��
	TIM3_Int_Init(4999,7199);//10Khz�ļ���Ƶ�ʣ�������5000Ϊ500ms 
	KEY_Init();//������ʼ��
	ISD4004_Init();
	 

	//���Թ��̴���
	//����LED0,LED1�ͷ�����Ӳ��״̬
	TIM_Cmd(TIM3, DISABLE);  //ʧ��TIMx
	LED0=1;//��
	LED1=1;
	BEEP=0;
	delay_ms(1000);
	LED0=0;//��
	LED1=0;
	BEEP=1;
	delay_ms(500);
	printf("\r\nս��STM32������ ����ʵ��\r\n");  //���Դ���ͨ��״̬
	printf("����ԭ��@ALIENTEK\r\n\r\n");
	BEEP=0; //��
	LED0=1;
	LED1=1;
	
	//���Զ�ʱ���Ĵ���
	//TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx
	
	
	while(1)
	{
//				//���Դ�������Ӳ��״̬
//		if(TPAD_Scan(0))
//		{
//			LED0=0;
//			LED1=0;
//			delay_ms(100);
//			ISD4004_Rec(0); //��ʼ¼��
//			delay_ms(100);
//		}
		
		//������������Ӳ��״̬
 		key=KEY_Scan(1);	//�õ���ֵ
	   	if(key)
		{						   
			switch(key)
			{				 
				case WKUP_PRES:	//���Ʒ�����
					BEEP=1;
				ISD4004_Rec(0x0000); //��ʼ¼��
					break;
				
				case KEY2_PRES:	//����LED0��ת
					LED0=!LED0;
				ISD4004_PlayRec(0x0000);
					break;
				
				case KEY1_PRES:	//����LED1��ת	 
					LED1=!LED1;
				ISD4004_StopMode();
				BEEP=0;
					break;
				
				case KEY0_PRES:	//ͬʱ����LED0,LED1��ת 
					LED0=!LED0;
					LED1=!LED1;
				ISD4004_StopPWRDN();
				BEEP=0;
					
               // delay_ms(200); 
					break;
			}
		}else 
		{
			
		}
//		BEEP=0;
//		LED0=1;
//		LED1=1;
	}
 }