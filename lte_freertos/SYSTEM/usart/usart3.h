#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define USART3_REC_LEN  	200  	//定义最大接收字节数 200
extern u8  USART3_RX_BUF[USART3_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 


void uart3_init(u32 bound);
void uart3_buadRate(u8 rateType);
void USART3_SendData(u8* buff, u16 len);
void USART3_SendByte(u8 byte);
#endif


