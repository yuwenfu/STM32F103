#include "sys.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "delay.h"
#include "usart.h"
#include "exti.h"

#define A 0
#define B 1
 int main(void)
 {	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
    //BEEP_Init();
    KEY_Init();
    LED_Init();
    uart_init(115200);
    EXTIX_Init();
     LED0=1;
     LED1=0;
     
     while(1)
     {
        printf("Press LED BLINK!\r\n");
        delay_ms(2000);
		 #if A
			printf("Press A Works!\r\n");
		 #endif
		 		 #if B
			printf("Press B Works!\r\n");
		 #endif
     }
    
    

 }
