#ifndef __HEX_H_  
#define __HEX_H_  
#include "stm32f10x.h"

typedef uint32_t UINT;
typedef uint8_t  BYTE;

#define BINBUFF_SIZE  512
#define HEXBUFF_SIZE  1200
//BINBUFF_SIZE <= (HEXBUFF_SIZE/45) * 16 
//hex 文件格式 每行ACII长度最大45字节  有限数据16位 

typedef struct{
  int len; //bin文件大小  
  UINT startAddress; //刷写的起始地址
	UINT sAddr_change; //多偏移地址时 标记是否变化
	UINT appAddress; //刷写的起始地址
  UINT offset;       //偏移地址
	UINT percent;       //完成百分比（1035 = 10.35%）
	UINT byte_used;     //本次传输内容中使用到的字节数
  BYTE *pContent;     //转化后的内容 	
}HexToBinData;  
  
typedef struct{  
  BYTE data[16];//数据  
  BYTE len;   //数据长度  
  UINT pos;   //偏移地址  
  BYTE type;  //类型  
}HexLinData;  

extern HexToBinData g_bin_info;

int ConvertHexToBin(char *str,HexToBinData *pData);

#endif

