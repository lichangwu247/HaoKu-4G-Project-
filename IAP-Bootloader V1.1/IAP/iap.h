#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
	
typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.

//����0x08000000~0x08001BFF�Ŀռ�ΪIAPʹ��

#define FLASH_APP0_ADDR		((u32)0x8000000 + 1024*12)
//0x8001C00  	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
											
#define FLASH_APP1_ADDR		((u32)0x8000000 + 1024*132)
//0x8010C00  	//�ڶ���Ӧ�ó�����ʼ��ַ(�����FLASH)

#define FLASH_USERBIN_MARK_ADDR		((u32)0x8000000 + 1024*252)//�����ڶ�ҳ��д��ʼ��ַ

void iap_load_app(u32 appxaddr);			//ִ��flash�����app����

#endif

