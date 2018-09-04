#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART3_REC_LEN  	200  	//�����������ֽ��� 200
extern u8  USART3_RX_BUF[USART3_REC_LEN]; //���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 


void uart3_init(u32 bound);
void uart3_buadRate(u8 rateType);
void USART3_SendData(u8* buff, u16 len);
void USART3_SendByte(u8 byte);
#endif


