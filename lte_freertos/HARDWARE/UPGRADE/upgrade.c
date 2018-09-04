#include  "mqtt_client.h"
#include  "mobile.h"
#include  "iot.h"
#include  "usr_task.h"
#include  "hexbin.h"
#include  "upgrade.h"
#include  "string.h"
#include  "stdlib.h"
#include  "stdio.h"
#include  "sys.h"
#include  "flash.h"

#define UPGRADE_LIB_DBG( format, ...  )    printf( format, ## __VA_ARGS__  )

/*#define  AT_FTP_OPEN     "AT$MYFTPOPEN=%d,%s,%s,%s,%d,%d,%d\r"
#define  AT_FTP_CLOSE    "AT$MYFTPCLOSE\r"
#define  AT_FTP_SIZE     "AT$MYFTPSIZE=%s\r"
#define  AT_FTP_GET      "AT$MYFTPGET=%s,%d,%d\r"
#define  AT_FTP_CHECK    "AT$MYFTPOPEN?\r"*/

#define  AT_FTP_OPEN     "AT$ZFTPCFG=%s,%s,%s\r"
#define  AT_FTP_CLOSE    "AT$ZFTPCLOSE\r"
#define  AT_FTP_SIZE     "AT$ZFTPSIZE=%s\r"
#define  AT_FTP_GET      "AT$ZFTPGET=%s,%d,%d\r"


FTP_STATUS ftp_status = CLOSE;

upgrade upgrade_info;

static u32 binmark = 0xffffffff;
 
/************************************************
函数名称 ： ftp_open
功    能 ： 打开FTP
参    数 ： 无
返 回 值 ： 无
作    者 ： 
*************************************************/
int ftp_open(char* path, char* username, char* password)
{
	int rc;
  char cmd_buf[256];
	
	sprintf(cmd_buf, AT_FTP_OPEN, path, username, password);
#if defined(MQTT_TASK)
		MutexLock(&client.mutex);
#endif

	rc = at_send_cmd(0,(u8*)cmd_buf, (u8*)"OK", NULL, 8000, 3);
#if defined(MQTT_TASK)
		MutexUnlock(&client.mutex);
#endif
	return 	rc;
}



int set_ftp_status(FTP_STATUS status)
{
	ftp_status = status;
	return 0;	
}



FTP_STATUS get_ftp_status(void)
{
	return ftp_status;	
}



int ftp_close(void)
{
	int rc;
	
#if defined(MQTT_TASK)
		MutexLock(&client.mutex);
#endif
	
	rc = at_send_cmd(0,(u8*)AT_FTP_CLOSE, (u8*)"OK", NULL, 2000, 1);	
#if defined(MQTT_TASK)
		MutexUnlock(&client.mutex);
#endif
	return 	rc;
}

u32 ftpFileSize = 0;

/*
	获取ftp 文件大小
*/
int ftp_getSize(uint8_t *s, uint16_t cnt)
{
	char ftpSize[10] = {0};
	char *phead =NULL,*pst =NULL;
	pst = (char*)s;
	u8 i = 0;
	
	phead = strstr((char*)pst, "$ZFTPSIZE: ");
	
	if(phead == NULL)
		return -1;
	phead += strlen("$ZFTPSIZE: ");
	
	do{
		if(*phead >= '0' && *phead <= '9')
		{
			ftpSize[i] = *phead;
			phead++;
			i++;
		}else{
			WRITE_IOT_DEBUG_LOG("ftpSize: %s\r\n", ftpSize);
			break;	
		}
	}while(i< 10 && (phead - pst) < cnt);
	
	ftpFileSize = atoi(ftpSize);
	
	return 0;
}



int ftp_file_size(char* file_name, u32* size)
{
	int rc;
  char cmd_buf[256];
	ftpFileSize = 0;
	
	sprintf(cmd_buf, AT_FTP_SIZE, file_name);
	
#if defined(MQTT_TASK)
		MutexLock(&client.mutex);
#endif
	
	rc = at_send_cmd(0,(u8*)cmd_buf, (u8*)"$ZFTPSIZE:", ftp_getSize, 2000, 3);
#if defined(MQTT_TASK)
		MutexUnlock(&client.mutex);
#endif
	*size = ftpFileSize;
	return 	rc;
}



int ftp_file_get(char* file_name, u32 offset, u32 data_lenth)
{
	int rc;
  char cmd_buf[256];
	if((offset == 0) && (data_lenth == 0))
		sprintf(cmd_buf, "AT$MYFTPGET=%s\r", file_name);
	else
		sprintf(cmd_buf, AT_FTP_GET, file_name, offset, data_lenth);
	
#if defined(MQTT_TASK)
		MutexLock(&client.mutex);
#endif
	
	rc = at_send_cmd(1,(u8*)cmd_buf, (u8*)"OK", NULL, 5000, 3);
	
#if defined(MQTT_TASK)
		MutexUnlock(&client.mutex);
#endif
	return 	rc;
}

   
//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 appsize)
{
	int32_t i;
	uint32_t flash_addr,RamSource;
	flash_addr = appxaddr;
	RamSource = (uint32_t)appbuf;
	UPGRADE_LIB_DBG("flash writing...\r\n");
	FLASH_Unlock();						//解锁
	for (i = 0; i < appsize; i += 4)
	{
		
		FLASH_ProgramWord(flash_addr, *(uint32_t*)RamSource);
		if (*(uint32_t*)flash_addr != *(uint32_t*)RamSource)
		{
			UPGRADE_LIB_DBG("flash write error\r\n");
			upgrade_info.isError = 1;
		}
		flash_addr += 4;
    RamSource += 4;
	}
	FLASH_Lock();//上锁
}


//WriteAddr:起始地址
//WriteData:要写入的数据
void write_bin_mark(u32 userbin)   	
{	
	FLASH_Unlock();						//解锁
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	FLASH_ErasePage(FLASH_USERBIN_MARK_ADDR);//擦除区
	FLASH_ProgramWord(FLASH_USERBIN_MARK_ADDR, userbin);

	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);
	FLASH_Lock();//上锁
	
	UPGRADE_LIB_DBG("binmark: 0x%4X\r\n",*(uint32_t*)FLASH_USERBIN_MARK_ADDR);	
}

u32 read_bin_mark(void)   	
{
	return STMFLASH_ReadWord(FLASH_USERBIN_MARK_ADDR);
}

u32 get_bin_mark(void)   	
{
	return binmark;
}

void set_bin_mark(u32 bin)   	
{
	binmark = bin;
}

//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
void write_firmware(HexToBinData *pData)
{
	u32 appxaddr;
	appxaddr = pData->startAddress + pData->offset;//debug
	UPGRADE_LIB_DBG("appxaddr: %8X\r\n", appxaddr);
	UPGRADE_LIB_DBG("len: %d\r\n", pData->len);
	
	vTaskSuspendAll();
	iap_write_appbin(appxaddr, pData->pContent, pData->len);
	xTaskResumeAll();
	
	//free(pData->pContent);
	vPortFree(pData->pContent);
	//COM_DBG("pData->len: %d\r\n", pData->len);
}


int is_upgrading(void)
{
	if(UPGRADE_IDLE != upgrade_info.status)
		return -1;
	else
		return 0;
}



//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
//u8 upgrade_firmware_check(upgrade *pData)
int upgrade_firmware_check(void)
{
	if(UPGRADE_IDLE != upgrade_info.status)
		return 0;
	
	if(USERBIN1 == get_bin_mark())
	{
		STMFLASH_Erase(6,60);
	}else{
		STMFLASH_Erase(66,60);
	}
	
	upgrade_info.status = UPGRADING;
	ftp_close();//bin
	if(0 != ftp_open(upgrade_info.path, upgrade_info.username, upgrade_info.password))
	{
		upgrade_info.status = UPGRADE_FTP_ERROR;
		goto exit;
	}
	
	if(0 != ftp_file_size(upgrade_info.filename, &upgrade_info.total_len))
	{
		upgrade_info.status = UPGRADE_FILE_ERROR;
		goto exit;
	}
	
	if(upgrade_info.total_len == 0)
	{
		upgrade_info.status = UPGRADE_FILE_ERROR;
		goto exit;
	}
	
	upgrade_info.pHex = (char*)malloc(sizeof(char)*HEXBUFF_SIZE);
	if(upgrade_info.pHex == NULL)
	{
		UPGRADE_LIB_DBG("upgrade malloc fail\r\n");
		upgrade_info.status = UPGRADE_DOWNLOAD_ERROR;
		goto exit;
	}
	
	UPGRADE_LIB_DBG("total_line: %d\r\n", upgrade_info.total_len);
	exit:
	if(upgrade_info.status != UPGRADING)
	{
		return c2s_firmware_upgrade_progress(upgrade_info.status);
	}
	
	start_Upgrade_task();
		
	return 0;
}



//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
int upgrade_firmware_download(upgrade *pData, HexToBinData *hData,uint32_t ByteNum)
{
	u32 res_len = 0;
	if(pData->status != UPGRADING)
		return -1;
	
	COM_DBG("upgrade percent: %d\r\n",hData->percent);
  if(hData->percent >= 10000)
	{
		pData->status = UPGRADE_SUCCESS;
		return 0;
	}
	
	if(pData->pHex == NULL)
		pData->pHex = (char*)malloc(sizeof(char)*HEXBUFF_SIZE);
	if(pData->pHex == NULL)
	{
		COM_DBG("upgrade download malloc fail\r\n");
		return -1;
	}
	memset(pData->pHex, 0x0, HEXBUFF_SIZE);
	UPGRADE_LIB_DBG("hData->byte_used: %d\r\n", hData->byte_used);
	pData->offset += hData->byte_used;
	UPGRADE_LIB_DBG("pData->offset: %d\r\n", pData->offset);
	
	res_len = pData->total_len - pData->offset;
	if(res_len > ByteNum)		res_len = ByteNum;
	UPGRADE_LIB_DBG("pData->res_len: %d\r\n", res_len);

	if(0 != ftp_file_get(pData->filename, pData->offset, res_len))
	{
	  pData->status = UPGRADE_DOWNLOAD_ERROR;
		return -1;
	}
	UPGRADE_LIB_DBG("pData->pHex:\r%s", pData->pHex);
	
	return 0;
}

//appxaddr:应用程序的起始地址
//appbuf:应用程序CODE.
//appsize:应用程序大小(字节).
u8 upgrade_firmware_done(void)
{	
	free(upgrade_info.pHex);
	free(upgrade_info.path);
	free(upgrade_info.filename);
	free(upgrade_info.username);
	free(upgrade_info.password);
	memset(&upgrade_info, 0, sizeof(upgrade));
	//ftp_close();
	return 0;
}



int set_upgrade_info(char *path, char* filename,char* username,char* password, uint32_t version, uint32_t reboot)
{
	upgrade_info.version = version; 
	upgrade_info.reboot = reboot;
	
	UPGRADE_LIB_DBG("path: %s\n",path);
	UPGRADE_LIB_DBG("filename: %s\n",filename);
	UPGRADE_LIB_DBG("username: %s\n",username);
	UPGRADE_LIB_DBG("password: %s\n",password);
	
	upgrade_info.path = (char*)malloc(sizeof(char)*32);
	memset(upgrade_info.path, 0x0, 32);
	if(upgrade_info.path == NULL)
	{
		UPGRADE_LIB_DBG("upgrade download malloc fail\r\n");
		return -1;
	}
	if(strlen(path) > 31)
	{
		UPGRADE_LIB_DBG("upgrade path too long\r\n");
		return -1;
	}
	strncpy(upgrade_info.path, path,strlen(path));
	
	upgrade_info.filename = (char*)malloc(sizeof(char)*80);
	memset(upgrade_info.filename, 0x0, 80);
	if(upgrade_info.filename == NULL)
	{
		UPGRADE_LIB_DBG("upgrade download malloc fail\r\n");
		return -1;
	}
	if(strlen(filename) > 79)
	{
		UPGRADE_LIB_DBG("upgrade filename too long\r\n");
		return -1;
	}
	strncpy(upgrade_info.filename, filename,strlen(filename));
	
	upgrade_info.username = (char*)malloc(sizeof(char)*32);
	memset(upgrade_info.username, 0x0, 32);
	if(upgrade_info.username == NULL)
	{
		UPGRADE_LIB_DBG("upgrade download malloc fail\r\n");
		return -1;
	}
	if(strlen(username) > 31)
	{
		UPGRADE_LIB_DBG("upgrade filename too long\r\n");
		return -1;
	}
	strncpy(upgrade_info.username, username,strlen(username));
	
	upgrade_info.password = (char*)malloc(sizeof(char)*32);
	memset(upgrade_info.password, 0x0, 32);
	if(upgrade_info.password == NULL)
	{
		UPGRADE_LIB_DBG("upgrade download malloc fail\r\n");
		return -1;
	}
	if(strlen(password) > 31)
	{
		UPGRADE_LIB_DBG("upgrade password too long\r\n");
		return -1;
	}
	strncpy(upgrade_info.password, password,strlen(password));
	
	return 0;
}


void upgrade_firmware(void *pvParameters)
{
	u8 bin_effective = 0;
  u8 bin_flag = 0;
	u8 upgrade_report = 0;
 	while(1)
	{
		switch(upgrade_info.status)
		{
			case UPGRADE_IDLE:
				vTaskDelete( NULL );			            //删除本任务
			break;
			
			case UPGRADING:
				UPGRADE_LIB_DBG("\r\nUPGRAD_Phase: UPGRADING\r\n");
				if(bin_flag == 0)
				{
					upgrade_firmware_download(&upgrade_info, &g_bin_info, 962);
				}else{
					upgrade_firmware_download(&upgrade_info, &g_bin_info, 990);
				}

				if(*upgrade_info.pHex != 0)
				{
					//COM_DBG("upgrade_info.HEX: %s\r\n", upgrade_info.pHex);
					ConvertHexToBin(upgrade_info.pHex,&g_bin_info);
					UPGRADE_LIB_DBG("hex:\r\n");
					DBG_HEX(g_bin_info.pContent, g_bin_info.len);//bin
					if(bin_flag == 0)
					{
						UPGRADE_LIB_DBG("\r\nbinmark: 0x%4X\r\n",get_bin_mark());
						UPGRADE_LIB_DBG("\r\nstartAddress: 0x%8X\r\n",g_bin_info.startAddress);
						UPGRADE_LIB_DBG("\r\nFLASH_APP1_ADDR: 0x%8X\r\n",FLASH_APP1_ADDR&0XFFFF0000);
						UPGRADE_LIB_DBG("\r\nFLASH_APP0_ADDR: 0x%8X\r\n",FLASH_APP0_ADDR&0XFFFF0000);
						if(((USERBIN0 == get_bin_mark()) && (g_bin_info.startAddress == (FLASH_APP1_ADDR&0XFFFF0000))) 
							|| ((USERBIN1 == get_bin_mark()) && (g_bin_info.startAddress == (FLASH_APP0_ADDR&0XFFFF0000))))
							bin_effective = 1;
						else
							upgrade_info.status = UPGRADE_FILE_ERROR;
						bin_flag = 1;
					}
					if(bin_effective == 1)
					{
						write_firmware(&g_bin_info);
						if(g_bin_info.percent < 10000)
						{
							upgrade_report++;
							if(upgrade_report >= 10)
							{
								upgrade_report = 0;
								float lenf,offsetf,percentf;//
								//计算完成百分比
								offsetf = upgrade_info.offset;
								lenf = g_bin_info.byte_used;
								percentf	= (offsetf + lenf)/upgrade_info.total_len;
								percentf *= 10000;
								g_bin_info.percent = (u32)percentf;
								UPGRADE_LIB_DBG("percent: %d\r\n", g_bin_info.percent/100);
								c2s_firmware_upgrade_progress(g_bin_info.percent);
							}
						}else if(g_bin_info.percent == 10000){
							upgrade_info.status = UPGRADE_SUCCESS;
							UPGRADE_LIB_DBG("percent: %d\r\n", g_bin_info.percent/100);
							c2s_firmware_upgrade_progress(g_bin_info.percent);
						}
					}
				}
			break;	
			
			case UPGRADE_SUCCESS:
				UPGRADE_LIB_DBG("\r\nUPGRAD_Phase: SUCESS\r\n");

				if(upgrade_info.isError == 1)
				{
					c2s_firmware_upgrade_progress(UPGRADE_DOWNLOAD_ERROR);
				}else{
					if(USERBIN1 == get_bin_mark())
					{
						write_bin_mark(USERBIN0);
					}else{
						write_bin_mark(USERBIN1);
					}
					c2s_firmware_upgrade_progress(UPGRADE_SUCCESS);
				}				
				
				if(upgrade_info.reboot == 1)
				{
					Sys_Soft_Reset();
				}
				upgrade_firmware_done();
				bin_flag = 0;
				bin_effective =0;
			break;
			
			case UPGRADE_FTP_ERROR:
				UPGRADE_LIB_DBG("\r\nUPGRAD_Phase: FTP_ERROR\r\n");
				upgrade_firmware_done();
				ftp_close();
				c2s_firmware_upgrade_progress(UPGRADE_FTP_ERROR);
				bin_flag = 0;
				bin_effective =0;
			break;
			
			case UPGRADE_FILE_ERROR:
				UPGRADE_LIB_DBG("\r\nUPGRAD_Phase: FILE_ERROR\r\n");
				upgrade_firmware_done();
				ftp_close();
				c2s_firmware_upgrade_progress(UPGRADE_FILE_ERROR);
				bin_flag = 0;
				bin_effective =0;
			break;
			
			case UPGRADE_DOWNLOAD_ERROR:
				UPGRADE_LIB_DBG("\r\nUPGRAD_Phase: DOWNLOAD_ERROR\r\n");
				upgrade_firmware_done();
				ftp_close();
				c2s_firmware_upgrade_progress(UPGRADE_DOWNLOAD_ERROR);
				bin_flag = 0;
				bin_effective =0;
			break;	

			default:
			break; 
			
		}
	}
}



/**** Copyright (C)2016  All Rights Reserved **** END OF FILE ****/
