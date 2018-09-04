

/* �����ֹ�ݹ���� ----------------------------------------------------------*/
#ifndef _UPGRADE_H
#define _UPGRADE_H

/* ������ͷ�ļ� --------------------------------------------------------------*/
#include "stm32f10x.h"

/* ���Ͷ��� ------------------------------------------------------------------*/
typedef enum{
	CLOSE = 0,
	OPEN
} FTP_STATUS;

typedef enum{
	UPGRADE_IDLE = 0,
	UPGRADING= 1,
  UPGRADE_SUCCESS = 10001,//�����ɹ�
	UPGRADE_FTP_ERROR = 10002,//FTP����ʧ��
	UPGRADE_FILE_ERROR = 10003,//��ȡ�ļ���С����
	UPGRADE_DOWNLOAD_ERROR = 10004,//���ش���
} FIRMWARE_STATUS;


typedef struct{
	uint32_t version;//��ǰ�����汾
	FIRMWARE_STATUS status;      //����״̬
	u8 reboot;//�������Ƿ�����������Ч���粻������Ч����һ���ϵ���Ч
	u8 isError;//�����Ƿ�������
	char *path;  //����·��
	char *filename;  //�����ļ��� 	
	char *username;  //�û���
	char *password;  //����
	uint32_t total_len;   //�ܳ���  
  uint32_t offset;    //�ļ�ƫ�Ƶ�ַ
	//uint32_t byte_used;//��ǰ�����ļ���Ч�ֽ���(���Ѿ�������ֽڣ������ٴδ���)
	char *pHex;     //���յ���HEX����
}upgrade;  



/* ����------------------------------------------------------------------*/

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

#define FLASH_APP0_OFFSET		1024*12      //APP0�ж������ض�λƫ����
#define FLASH_APP1_OFFSET		1024*132     //APP1�ж������ض�λƫ����
#define FLASH_DATA_OFFSET		1024*252     //DATA��λƫ����


//����0x08000000~0x8003000�Ŀռ�ΪIAPʹ��
#define FLASH_APP0_ADDR		((u32)0x8000000 + FLASH_APP0_OFFSET)	//��һ��Ӧ�ó�����ʼ��ַ(�����FLASH)
#define FLASH_APP1_ADDR		((u32)0x8000000 + FLASH_APP1_OFFSET)  	//�ڶ���Ӧ�ó�����ʼ��ַ(�����FLASH)

#define FLASH_USERBIN_MARK_ADDR		((u32)0x8000000 + FLASH_DATA_OFFSET)//���һҳ��д��ʼ��ַ
#define FLASH_DATA_ADDR       		((u32)0x8000000 + FLASH_DATA_OFFSET)

void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	//��ָ����ַ��ʼ,д��bin
int  set_upgrade_info(char *path, char* filename,char* username,char* password, uint32_t version, uint32_t reboot);
int  upgrade_firmware_check(void);
void upgrade_firmware(void *pvParameters);

#endif /* _UPGRADE_H*/

/**** Copyright (C)2016  All Rights Reserved **** END OF FILE ****/
