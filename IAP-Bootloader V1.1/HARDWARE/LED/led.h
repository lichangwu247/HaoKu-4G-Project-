#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
 
#define LED0 PAout(1)// PA8
#define LED0_OFF()  GPIO_SetBits(GPIOA,GPIO_Pin_1)						 //PA.8 �����//����
#define LED0_ON()   GPIO_ResetBits(GPIOA,GPIO_Pin_1)						 //PA.8 �����//����
 
void LED_Init(void);//��ʼ��

		 				    
#endif
