#ifndef __INFO_STORAGE_H
#define __INFO_STORAGE_H

#include "device.h"
#include "sys.h"

//I2C EEPROM 存储信息数据地址列表

#define  I2CADDR_IOT_INFO         0    //阿里云IOT密钥等信息，总长260字节（各段65字节）

#define  I2CADDR_FIRMWARE_FLAG    260  //备份有效标记
#define  I2CADDR_FIRMWARE_BASE    261  //地址
#define  I2CADDR_FIRMWARE_OFFSET  3    //信号上报阈值


#define  I2CADDR_SELECTOR_FLAG    267  //备份有效标记
#define  I2CADDR_SELECTOR_BASE    268  //地址
#define  I2CADDR_SELECTOR_OFFSET  4    //投币参数（极性，脉宽，脉冲间隔）

#define  I2CADDR_RECORD_F      416
#define  I2CADDR_RECORD_W      418  //记录过的总数
#define  I2CADDR_RECORD_S      419  //当前有效的记录数量

#define  I2CADDR_RECORD_BASE   420  //记录基地址

#define  MAX_RECORD  44//((2048 - 420)/40)

void iot_data_write(void);
void iot_data_read(void);
void firmware_data_read(void);
void firmware_data_write(void);
void selector_data_read(void);
void selector_data_write(void);


int record_read(record* rec);
int record_read_success(void);
int record_write(record* rec);


#endif

