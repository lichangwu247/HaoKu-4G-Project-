#ifndef __HEX_H_  
#define __HEX_H_  
#include "stm32f10x.h"

typedef uint32_t UINT;
typedef uint8_t  BYTE;

#define BINBUFF_SIZE  512
#define HEXBUFF_SIZE  1200
//BINBUFF_SIZE <= (HEXBUFF_SIZE/45) * 16 
//hex �ļ���ʽ ÿ��ACII�������45�ֽ�  ��������16λ 

typedef struct{
  int len; //bin�ļ���С  
  UINT startAddress; //ˢд����ʼ��ַ
	UINT sAddr_change; //��ƫ�Ƶ�ַʱ ����Ƿ�仯
	UINT appAddress; //ˢд����ʼ��ַ
  UINT offset;       //ƫ�Ƶ�ַ
	UINT percent;       //��ɰٷֱȣ�1035 = 10.35%��
	UINT byte_used;     //���δ���������ʹ�õ����ֽ���
  BYTE *pContent;     //ת��������� 	
}HexToBinData;  
  
typedef struct{  
  BYTE data[16];//����  
  BYTE len;   //���ݳ���  
  UINT pos;   //ƫ�Ƶ�ַ  
  BYTE type;  //����  
}HexLinData;  

extern HexToBinData g_bin_info;

int ConvertHexToBin(char *str,HexToBinData *pData);

#endif

