#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "upgrade.h"

#define IOT_VERSION_MAJOR		    1U
#define IOT_VERSION_MINOR		    2U
#define IOT_VERSION_REVISION	  1U


#define UART_DEBUG      //�궨����Ч��ʹ�ô���1��ӡ�������ݣ����򲻴�ӡ


/*********************************************************************************/
//������ƺ궨��
//�̼���Ӧ��ţ�0����Ӧ����0����ʹ�õĹ̼��� 1����Ӧ����1����ʹ�õĹ̼�
//ͨ��USERBIN �궨�壬���Ʋ�����Ӧ�����Ĺ̼���ͬʱ���ƹ̼�������
#define USERBIN    1

/*****************************************************/

#ifdef UART_DEBUG
  #define DBG_PRINTF
  #define IOT_DBG_PRINTF
	#define MQTT_DEBUG_ON
#endif


#if USERBIN == 0
  #define FLASH_APP_OFFSET    FLASH_APP0_OFFSET
#else
  #define FLASH_APP_OFFSET    FLASH_APP1_OFFSET
#endif


#define PRINTF_BRIEF         //��Ҫ��Ϣ��ӡ������ģʽ��ʹ��
//#define USERIF_PRINTF         //������Ϣ��ӡ������ʱ��Ҫע�͵�

#define VERSION_NUM   (IOT_VERSION_MAJOR * 100 + IOT_VERSION_MINOR * 10 + IOT_VERSION_REVISION)

#define EE_WRITE_IOT     //ʹ����дiot.h�ļ��еĹ̼���Ϣ

//#define USE_GPRS      //�����Ƿ�ʹ��GPRS����

#endif

