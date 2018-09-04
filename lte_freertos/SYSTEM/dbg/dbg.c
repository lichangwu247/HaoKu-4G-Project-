#include "dbg.h"
#include "usart1.h"


//16进制打印字符串内容
u8  dbghex_printf(unsigned char* packet, int len)
{
	u16 k;
	for (k = 0; k<len; k++)
	{
		const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
		dbg_write_char(hex[packet[k]>>4]);
		dbg_write_char(hex[packet[k]&0xF]);
	    dbg_write_char( ' '); 
	}
	dbg_write_char('\r');
	dbg_write_char('\n');
	return 0;
}

//16进制打印字符
u8 hex_printf(unsigned char c)
{
	const char hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	dbg_write_char(hex[c>>4]);
	dbg_write_char(hex[c&0xF]);
	dbg_write_char( ' '); 
	return 0;
}


//打印字符串
u8 dbgchar_printf(unsigned char* packet, int len)
{
	u16 k;
	for (k = 0; k<len; k++)
	{
	  dbg_write_char(packet[k]);
	}
	dbg_write_char('\r');
	dbg_write_char('\n');
	return 0;
}

