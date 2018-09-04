#ifndef __INFO_STORAGE_H
#define __INFO_STORAGE_H

#include "device.h"
#include "sys.h"

//I2C EEPROM �洢��Ϣ���ݵ�ַ�б�

#define  I2CADDR_IOT_INFO         0    //������IOT��Կ����Ϣ���ܳ�260�ֽڣ�����65�ֽڣ�

#define  I2CADDR_FIRMWARE_FLAG    260  //������Ч���
#define  I2CADDR_FIRMWARE_BASE    261  //��ַ
#define  I2CADDR_FIRMWARE_OFFSET  3    //�ź��ϱ���ֵ


#define  I2CADDR_SELECTOR_FLAG    267  //������Ч���
#define  I2CADDR_SELECTOR_BASE    268  //��ַ
#define  I2CADDR_SELECTOR_OFFSET  4    //Ͷ�Ҳ��������ԣ�������������

#define  I2CADDR_RECORD_F      416
#define  I2CADDR_RECORD_W      418  //��¼��������
#define  I2CADDR_RECORD_S      419  //��ǰ��Ч�ļ�¼����

#define  I2CADDR_RECORD_BASE   420  //��¼����ַ

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

