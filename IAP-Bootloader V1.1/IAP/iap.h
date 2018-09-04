#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
	
typedef  void (*iapfun)(void);				//定义一个函数类型的参数.

//保留0x08000000~0x08001BFF的空间为IAP使用

#define FLASH_APP0_ADDR		((u32)0x8000000 + 1024*12)
//0x8001C00  	//第一个应用程序起始地址(存放在FLASH)
											
#define FLASH_APP1_ADDR		((u32)0x8000000 + 1024*132)
//0x8010C00  	//第二个应用程序起始地址(存放在FLASH)

#define FLASH_USERBIN_MARK_ADDR		((u32)0x8000000 + 1024*252)//倒数第二页读写起始地址

void iap_load_app(u32 appxaddr);			//执行flash里面的app程序

#endif

