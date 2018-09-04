#ifndef __USART2_H
#define __USART2_H
#include "ringbuf.h"
#include "stdio.h"	
#include "sys.h" 


#define USART2_REC_LEN  			1200  	//定义最大接收字节数 1200

extern RINGBUF Mobile_RxRingBuff;
void USART2_SendData(u8* buff, u16 len);
void USART2_SendByte(u8 byte);
void USART2_PutString (uint8_t *s);
void uart2_init(u32 bound);

#endif


