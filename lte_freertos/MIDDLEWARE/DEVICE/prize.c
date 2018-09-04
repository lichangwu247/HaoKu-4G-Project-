/* 包含的头文件 --------------------------------------------------------------*/
#include "iot.h"
#include "rtc.h"
#include "24cxx.h"
#include "prize.h"
#include "hash_crypt.h"
#include "sys.h"
#include "FreeRTOS.h"


char payQRcode[16][payQRcodeLen + 4]={0};

char MD5keyId[KEYLEN + 4] = {0};  //秘钥id
char MD5key[KEYLEN + 4] = {0};  //秘钥
char prizeUrl[prizeUrlLen + 4] = {0};  //链接 
char devicePrizeUrl[prizeUrlLen + 4] = {0};  //链接 


#define  PRIZE1_FMT    "%s?keyId=%s&orderId=%s&time=%d&type=%d&value=%d&sign=%s"
/*例子
http://wx.uuudian.com/device/scanPrize.do? 
keyId=12345678999999&orderId=2017072811263000000001
&type=53time=1501212466&value=3&sign=2B621F3132E5F2DD77AE53A133953DDF
*/

#define  PRIZE2_FMT    "%s?deviceName=%s&keyId=%s&time=%d&type=%d&value=%d&sign=%s"
/*例子
http://wx.uuudian.com/device/scanPrize.do?
deviceName=HK888|1&keyId=12345678999999&type=53
&time=1501212466&value=3&sign=2B621F3132E5F2DD77AE53A133953DDF
*/


#define  PRIZE3_FMT    "%s?keyId=%s&orderId=%s&seq=%d&time=%d&type=%d&value=%d&sign=%s"
/*example
http://wx.uuudian.com/device/scanPrize.do?
keyId=123456789&orderId=2017072811263000000001&seq=2147483647&time=1501212466&type=1&
value=3&sign=2B621F3132E5F2DD77AE53A133953DDF
*/


int prizeInfo_save(u32 addr, u8 *data, u16 len)	//在指定地址写入prize info
{
	taskENTER_CRITICAL();           //进入临界区
	AT24CXX_Write(addr, data, len);
	taskEXIT_CRITICAL();            //退出临界区
	return 0;
}


int MD5Key_read(char *key)	//在指定地址读取MD5key info
{
	taskENTER_CRITICAL();           //进入临界区
	memset(key, 0x0, KEYLEN + 4);
  AT24CXX_Read(MD5KEY_DATA_ADDR, (uint8_t*)key, (KEYLEN+4));
	taskEXIT_CRITICAL();            //退出临界区

	if(strlen(key) > KEYLEN)
	{
		COM_DBG("MD5 Key too long: %d (32MAX)!\r\n", strlen(key));
    memset(key, 0x0, KEYLEN + 4);
		AT24CXX_Write(MD5KEY_DATA_ADDR, (uint8_t*)key, (KEYLEN + 4));
		return -1;
	}
		return 0;
}


int MD5KeyId_read(char *key)	//在指定地址读取MD5key info
{
	taskENTER_CRITICAL();           //进入临界区
	memset(key, 0x0, KEYLEN + 4);
	AT24CXX_Read( MD5KEY_ID_DATA_ADDR, (uint8_t*)key, (KEYLEN+4));
	taskEXIT_CRITICAL();            //退出临界区

	if(strlen(key) > KEYLEN)
	{
		COM_DBG("MD5 Key ID too long: %d (32MAX)!\r\n", strlen(key));
    memset(key, 0x0, KEYLEN + 4);
		AT24CXX_Write(MD5KEY_ID_DATA_ADDR, (u8*)key, (KEYLEN + 4));
		return -1;
	}
	return 0;
}



int prizeUrl_read(char *url)	//在指定地址读取MD5key info
{
	taskENTER_CRITICAL();           //进入临界区
	memset(url, 0x0, prizeUrlLen + 4);
	AT24CXX_Read(URL_DATA_ADDR, (uint8_t*)url,  (prizeUrlLen + 4));
	taskEXIT_CRITICAL();            //退出临界区

	if(strlen(url) > prizeUrlLen)
	{
		COM_DBG("MD5 URL too long: %d (64MAX)!\r\n", strlen(url));
    memset(url, 0x0, prizeUrlLen + 4);
		AT24CXX_Write(URL_DATA_ADDR, (u8*)url, (prizeUrlLen + 4));		
		return -1;
	}
		return 0;
}


int prizeInfo_read(void)	//在指定地址读取prize info
{
	memset(MD5key, 0x0, sizeof(MD5key));
	memset(MD5keyId, 0x0, sizeof(MD5keyId));
	memset(prizeUrl, 0x0, sizeof(prizeUrl));
	if(0 == MD5Key_read(MD5key))	//读取MD5 key
	{
		COM_DBG("MD5Key: %s\r\n",MD5key);
	}
  if(0 == MD5KeyId_read(MD5keyId))		//读取MD5 key ID
	{
		COM_DBG("MD5keyId: %s\r\n",MD5keyId);
	}
  if(0 == prizeUrl_read(prizeUrl))	//读取URL
	{
		COM_DBG("prizeUrl: %s\r\n",prizeUrl);
	}
	return 0;
}



int prize_qrcode1_gen(char* qrcode, u8* qrcode_len, char *keyId, char *key, char *url,u8 type, u32 vaule, char* orderId)
{
	if((qrcode == NULL) || (keyId == NULL) || (key == NULL) || (url == NULL))
	{
		return -1;
	}
	
	char md5_buff[72];
	uint32_t timestamp = 0;
	timestamp = Unix_Get();
	char hash[36] = {0};
	
	//sprintf(md5_buff, "%s%d%d%s", orderId, timestamp, vaule, key);
	sprintf(md5_buff, "%s%d%d%d%s", orderId, timestamp, type,vaule, key);
	
	md5_calc((uint8_t*)md5_buff, strlen(md5_buff),(uint8_t*)hash);
	
	sprintf(qrcode, PRIZE1_FMT, url, keyId, orderId, timestamp, type, vaule, hash);
	*qrcode_len = strlen(qrcode);
	
	return 0;
}



int prize_qrcode2_gen(char* qrcode, u8* qrcode_len, char *keyId, char *key, char *url,u8 type, u32 vaule, u8 port)
{
	if((qrcode == NULL) || (keyId == NULL) || (key == NULL) || (url == NULL))
	{
		return -1;
	}
	
	char md5_buff[72];
	char dviceName[12] = {0};
	uint32_t timestamp = 0;
	timestamp = Unix_Get();
	char hash[36] = {0};
	
	sprintf(dviceName, "%s|%d", g_iot_info.dname, port);
	sprintf(md5_buff, "%s%d%d%d%s", dviceName, timestamp, type,vaule, key);
	
	md5_calc((uint8_t*)md5_buff, strlen(md5_buff),(uint8_t*)hash);
	
	sprintf(qrcode, PRIZE2_FMT, url, dviceName, keyId, timestamp, type, vaule, hash);
	*qrcode_len = strlen(qrcode);
	
	return 0;
}



int prize_qrcode3_gen(char* qrcode, u8* qrcode_len, char *keyId, char *key, char *url,u8 type, u32 vaule, char* orderId, u32 seq)
{
	if((qrcode == NULL) || (keyId == NULL) || (key == NULL) || (url == NULL))
	{
		return -1;
	}

	char hash[36] = {0};
	char md5_buff[72];
	uint32_t timestamp = 0;
	timestamp = Unix_Get();
	
	sprintf(md5_buff, "%s%d%d%d%d%s", orderId, seq, timestamp, type,vaule, key);

	md5_calc((uint8_t*)md5_buff, strlen(md5_buff),(uint8_t*)hash);
	
	sprintf(qrcode, PRIZE3_FMT, url, keyId, orderId, seq, timestamp, type, vaule, hash);
	*qrcode_len = strlen(qrcode);
	
	return 0;
}


/**** Copyright (C)2016  All Rights Reserved **** END OF FILE ****/
