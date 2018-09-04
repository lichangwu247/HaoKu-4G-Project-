

/* 定义防止递归包含 ----------------------------------------------------------*/
#ifndef _UPGRADE_H
#define _UPGRADE_H

/* 包含的头文件 --------------------------------------------------------------*/
#include "stm32f10x.h"

/* 类型定义 ------------------------------------------------------------------*/
typedef enum{
	CLOSE = 0,
	OPEN
} FTP_STATUS;

typedef enum{
	UPGRADE_IDLE = 0,
	UPGRADING= 1,
  UPGRADE_SUCCESS = 10001,//升级成功
	UPGRADE_FTP_ERROR = 10002,//FTP连接失败
	UPGRADE_FILE_ERROR = 10003,//获取文件大小错误
	UPGRADE_DOWNLOAD_ERROR = 10004,//下载错误
} FIRMWARE_STATUS;


typedef struct{
	uint32_t version;//当前升级版本
	FIRMWARE_STATUS status;      //升级状态
	u8 reboot;//升级后是否重启立马生效，如不立马生效则下一次上电生效
	u8 isError;//升级是否发生错误
	char *path;  //下载路径
	char *filename;  //下载文件名 	
	char *username;  //用户名
	char *password;  //密码
	uint32_t total_len;   //总长度  
  uint32_t offset;    //文件偏移地址
	//uint32_t byte_used;//当前接收文件有效字节数(即已经处理的字节，无需再次传输)
	char *pHex;     //接收到的HEX内容
}upgrade;  



/* 函数------------------------------------------------------------------*/

int ftp_open(char* path, char* username, char* password);
int ftp_close(void);
int ftp_file_size(char* file_name, u32* size);
int ftp_file_get(char* file_name, u32 offset, u32 data_lenth);

int set_ftp_status(FTP_STATUS status);
FTP_STATUS get_ftp_status(void);
void set_bin_mark(u32 bin);
u32 read_bin_mark(void);
u32 get_bin_mark(void);
int is_upgrading(void);

//12KB         //120KB        //120KB        //4KB    (total 256KB)
//bootloader  //firmware1   //firmware2   //data


#define USERBIN0		0xffffffff
#define USERBIN1		0x5A5A5A5A

#define FLASH_APP0_OFFSET		1024*12      //APP0中断向量重定位偏移量
#define FLASH_APP1_OFFSET		1024*132     //APP1中断向量重定位偏移量
#define FLASH_DATA_OFFSET		1024*252     //DATA定位偏移量


//保留0x08000000~0x8003000的空间为IAP使用
#define FLASH_APP0_ADDR		((u32)0x8000000 + FLASH_APP0_OFFSET)	//第一个应用程序起始地址(存放在FLASH)
#define FLASH_APP1_ADDR		((u32)0x8000000 + FLASH_APP1_OFFSET)  	//第二个应用程序起始地址(存放在FLASH)

#define FLASH_USERBIN_MARK_ADDR		((u32)0x8000000 + FLASH_DATA_OFFSET)//最后一页读写起始地址
#define FLASH_DATA_ADDR       		((u32)0x8000000 + FLASH_DATA_OFFSET)

void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//在指定地址开始,写入bin
int  set_upgrade_info(char *path, char* filename,char* username,char* password, uint32_t version, uint32_t reboot);
int  upgrade_firmware_check(void);
void upgrade_firmware(void *pvParameters);

#endif /* _UPGRADE_H*/

/**** Copyright (C)2016  All Rights Reserved **** END OF FILE ****/
