#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "upgrade.h"

#define IOT_VERSION_MAJOR		    1U
#define IOT_VERSION_MINOR		    2U
#define IOT_VERSION_REVISION	  1U


#define UART_DEBUG      //宏定义生效则使用串口1打印调试数据，否则不打印


/*********************************************************************************/
//代码控制宏定义
//固件对应编号，0：对应分区0所能使用的固件； 1：对应分区1所能使用的固件
//通过USERBIN 宏定义，控制产生对应分区的固件，同时控制固件的名称
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


#define PRINTF_BRIEF         //简要信息打印，发布模式下使用
//#define USERIF_PRINTF         //任务信息打印，发布时需要注释掉

#define VERSION_NUM   (IOT_VERSION_MAJOR * 100 + IOT_VERSION_MINOR * 10 + IOT_VERSION_REVISION)

#define EE_WRITE_IOT     //使能烧写iot.h文件中的固件信息

//#define USE_GPRS      //定义是否使用GPRS功能

#endif

