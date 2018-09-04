#ifndef __DBG_H__
#define __DBG_H__

#include "user_config.h"
#include "usart1.h"
#include "sys.h"
#include "stdio.h"
#include "string.h"


//将打印函数映射到串口
#define dbg_write_char(c)    USART1_SendByte(c)

#ifdef DBG_PRINTF
    #define DBG_HEX(p,l)    dbghex_printf(p,l)
    #define DBG_ONEHEX(c)   hex_printf(c)
    #define DBG_STRING(p,l) dbgchar_printf(p,l)
		#define DBG_CHR(c)    dbg_write_char(c)
#else
    #define DBG_HEX(p,l)    
    #define DBG_ONEHEX(c)   
    #define DBG_STRING(p,l) 
#endif

#ifdef IOT_DBG_PRINTF
    #define WRITE_IOT_DEBUG_LOG( format, ... )    printf( format, ## __VA_ARGS__  )
		#define COM_DBG( format, ... )                printf( format, ## __VA_ARGS__  )
#else
    #define WRITE_IOT_DEBUG_LOG
		#define COM_DBG
#endif



#ifdef PRINTF_BRIEF
    #define COMDBG_HEX(p,l)            dbghex_printf(p,l)
    #define COMDBG_ONEHEX(c)           hex_printf(c)
    #define COMDBG_STRING(p,l)         dbgchar_printf(p,l)
	  #define COMDBG_INFO(format, ... )  printf( format, ## __VA_ARGS__ )
#else
    #define COMDBG_HEX(p,l)    
    #define COMDBG_ONEHEX(c)   
    #define COMDBG_STRING(p,l)
	  #define COMDBG_INFO(format, ... )
#endif

u8  dbghex_printf(unsigned char* packet, int len);
u8  hex_printf(unsigned char c);
u8  dbgchar_printf(unsigned char* packet, int len);

#endif

