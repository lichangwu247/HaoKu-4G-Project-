#ifndef __DEVICE_H__
#define __DEVICE_H__


#include "usart3.h"
#include "sys.h"

#define  PRIZE_DELAY_TIME   3
#define DEVICE_DBG( format, ... )               printf( format, ## __VA_ARGS__  )
//////////////////////////////////////////////////////////////////
//device or firmware  to pub iot CODE
#define  FIRMWARE_FIRST     0xE1
#define  GPRS_RSSI          0xE2
#define  DEV_COMM_TIMEOUT   0xE3
#define  DEV_INCOME         0x09
#define  DEV_ERROR          0x10


#define    DEVICE_CHECK        0x01
#define    DEVICE_RESET        0x02
#define    DEVICE_COIN         0x03
#define    DEVICE_ACCOUNT_GET  0x04
#define    DEVICE_PARAM_GET    0x05
#define    DEVICE_PARAM_SET    0x06
#define    DEVICE_FRONT_ACCOUNT_CLR  0x07
#define    DEVICE_BACK_ACCOUNT_CLR   0x08
#define    DEVICE_INCREMENT_GET      0x09
#define    DEVICE_RESTORE            0x0B
#define    DEVICE_ACCOUNT_CLR    0x0C
#define    DEVICE_QRCODE_SET     0x10
#define    DEVICE_INCREMENT_REPORT   0x13
#define    DEVICE_REQUEST_SIGNAL     0x19
#define    DEVICE_SLEEP       0x1A
#define    DEVICE_WAKE        0x1B
#define    DEVICE_PRIZE_CLR   0x1C
#define    DEVICE_ACCOUNT_SET      0x1D
#define    DEVICE_PRIZE_REPORT     0x1E
#define    DEVICE_SET_STATUS       0x1F

#define    DEVICE_GET_STATUS       0x21
#define    DEVICE_PRIZE_QRCODE     0x22
#define    DEVICE_PAY_QRCODE_GET   0x23
#define    DEVICE_PRIZE_QRCODE_GEN 0x24
#define    DEVICE_ORDER_GET        0x25

#define    DEVICE_FAILPRIZE_GET    0x26
#define    DEVICE_PRIZE_SET        0x27
#define    DEVICE_PRIZE_GET        0x28
#define    DEVICE_FAILCOIN_GET     0x29

#define    DEVICE_RANKING_REQUEST  0x30

#define    DEVICE_GPRS_REQUEST  0x31

#define DEVICE_INFO_LEN     64

#define DEVICE_CHECK_TIME          6   //60秒
#define NTP_QUERY_TIME             360 //更新获取账目时间间隔3600秒

//接口函数-发送命令函数(串口发送一串字符串)
#define DEVICE_SendCmd_Port(buff, len)		do{ vTaskSuspendAll();\
	                                            USART3_SendData(buff, len);\
                                               xTaskResumeAll();\
                                            }while(0)

typedef struct{
	char pn_sn[DEVICE_INFO_LEN + 1];
	u8 status;
	u8 timeout;     //数据通讯超时，超过90s没有任何数据，标记为离线
} DEVICE_INFO;


typedef struct
{
	u32 timestamp;
  u32 num;
	u32 seq;
	u8  orderNum[24];
	u16 type;
  u16 sum;	
} record;



extern DEVICE_INFO device;

typedef struct 
{
  uint64_t happen_time;
	u16 prize;
	u16 coin;
}save_info;

int device_cmd_send(u8 cmd, u8 daddr, u8 msglen, u8* msg_content);

int device_com_process(u8 port, u8 cmd, u16 msg_ack_len,u8 *msg_ack, u8 isnetwork_cmd);

int device_status_report(u8 status_code);
int check_device(u8 port, u8 timeout, u8 isnetwork_cmd);
int device_coin_prize_report_or_store(void);
int device_stored_report(void);
int device_params_set(u8 port, u8 msglen,	u8* msg_content, u8 isnetwork_cmd);
int device_insert_coin(u8 port, char *order_num, u8 oder_num_len, u16 coin);
int device_account_get(u8 port, u8 msglen,	u8* msg_content, u8 isnetwork_cmd);
int device_params_get(u8 port, u8 msglen,	u8* msg_content, u8 isnetwork_cmd);
int set_device_status(u8 port, u8 act, u8 timeout);
int device_qrcode_set(u8 port, u8 msglen, u8* msg_content, u8 isnetwork_cmd);
int device_coin_report_or_store(void);

int firmware_first_pub(void);
int firmware_signal(void);

int device_prize_set(u8 port, u16 type, u8 msglen, char* msg_content, u8 isnetwork_cmd);
int device_race_set(u8 port, u32 race_no, u32 race_rank, u32 race_bonus,  u32 race_bdiff, char* nikename, u8 nikename_len);

#endif

