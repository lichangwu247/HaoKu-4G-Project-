#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
 
#define LED0 PAout(1)// PA8
#define LED0_OFF()  GPIO_SetBits(GPIOA,GPIO_Pin_1)						 //PA.8 输出高//灯灭
#define LED0_ON()   GPIO_ResetBits(GPIOA,GPIO_Pin_1)						 //PA.8 输出低//灯亮
 
void LED_Init(void);//初始化

		 				    
#endif
