

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _PRIZE_H
#define _PRIZE_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "sys.h"

/* 类型定义 ------------------------------------------------------------------*/

#define KEYLEN   32
#define KEYLEN   32
#define payQRcodeLen   64
#define prizeUrlLen    64


extern char payQRcode[16][payQRcodeLen + 4];

extern char MD5keyId[KEYLEN + 4];  //秘钥id
extern char MD5key[KEYLEN + 4];  //秘钥
extern char prizeUrl[prizeUrlLen + 4];  //链接 
extern char devicePrizeUrl[prizeUrlLen + 4];  //?? 

#define FLASH_DATA_OFFSET   1024*252

/* 函数-----------------------------------------------------------------------*/
//EEPROM 地址
#define MD5KEY_DATA_ADDR       		276//((u32)0x8000000 + FLASH_DATA_OFFSET + 4)//
#define MD5KEY_ID_DATA_ADDR       312//((u32)0x8000000 + FLASH_DATA_OFFSET + 40)//
#define URL_DATA_ADDR       		  348//((u32)0x8000000 + FLASH_DATA_OFFSET + 76)//

int prizeInfo_save(u32 addr, u8 *data, u16 len);
int MD5Key_read(char *MD5key);	    //读取MD5 key
int MD5KeyId_read(char *MD5keyId);	//读取MD5 key ID
int prizeUrl_read(char *prizeUrl);	//读取URL

int prizeInfo_read(void);

int prize_qrcode1_gen(char* qrcode, u8* qrcode_len, char *keyId, char *key, char *url,u8 type, u32 vaule, char* orderId);	//生成
int prize_qrcode2_gen(char* qrcode, u8* qrcode_len, char *keyId, char *key, char *url,u8 type, u32 vaule, u8 port);
int prize_qrcode3_gen(char* qrcode, u8* qrcode_len, char *keyId, char *key, char *url,u8 type, u32 vaule, char* orderId, u32 seq);
#endif /* _PRIZE_H*/

/**** Copyright (C)2016  All Rights Reserved **** END OF FILE ****/
