#ifndef __USART1_H
#define __USART1_H
#include "stdio.h"	
#include "sys.h" 

#define USART1_REC_LEN  			200  	//�����������ֽ��� 200
#define EN_USART1_RX 			    0		//ʹ�ܣ�1��/��ֹ��0������1����

void uart1_init(u32 bound);
//void uart1_buadRate(u8 rateType);
void USART1_SendByte(u8 byte);
void USART1_SendData(u8* buff, u16 len);
#endif


