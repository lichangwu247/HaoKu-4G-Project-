#ifndef  _IOT_H_
#define  _IOT_H_

#include "MQTTClient.h"
#include "stm32f10x.h"

//iot information

//测试环境测试用
/*#define PRODUCT_KEY    "TBuPGFTrzHn"
#define PRODUCT_SECRET ""
#define DEVICE_NAME    "SHTEST003"
#define DEVICE_SECRET  "GEO8uiCqChD4lSy9xyKB8TGceqGvLfsi"*/


//开发环境测试用
/*#define PRODUCT_KEY    "iM0jQ4sRZ8X"
#define PRODUCT_SECRET ""
#define DEVICE_NAME    "SDK001"
#define DEVICE_SECRET  "8043TZjeAyPhytf0FRteKO8VeIF2o1Ws"*/


//正式环境
#define PRODUCT_KEY    "TBuPGFTrzHn"
#define PRODUCT_SECRET ""
#define DEVICE_NAME    "SHTEST002"
#define DEVICE_SECRET  "7TeLo3fAMVmeuW8PjFmb62xbCqyzLlCY"

/**
 *TOPIC FOR ROUTINE
 **/
//c2s/   device/firmware to server(cloud)
#define  C2STRANSFER           "transfer_data"
#define  C2SDEV_SETTING        "device_setting"
#define  C2SFIRMWARE_UPGRADE   "firmware_upgrade_progress"
#define  C2SGPRS_RSSI          "firmware_signal"
#define  C2SDEV_INCOME         "device_income"
#define  C2SDEV_PRIZE          "device_prize"
#define  C2SDEV_COIN           "insert_coin"
#define  C2SDEV_ERROR          "device_error"
#define  C2SDEV_STATUS         "device_status"
#define  C2SFIRMWARE_ONLINE    "firmware_online"
#define  C2SS2C_STATUS         "s2c_status"
#define  C2SDEV_ACCOUNTS       "device_accounts"


//代码号 别名
#define  DEV_COIN             0x01
#define  DEV_PRIZE            0x02
#define  DEV_STATUS           0x03
#define  DEV_SETTING          0x04
#define  FIRMWARE_ONLINE      0x05
#define  FIRMWARE_UPGRADE     0x06
#define  TRANSFER_DATA        0x07
#define  DEV_ACCOUNTS         0x08
#define  FIRMWARE_SETTING     0x09


//s2c/   server(cloud)  to device/firmware          代码号
#define  S2CDEV_COIN             "insert_coin"      //0x01
#define  S2CDEV_PRIZE            "device_prize"     //0x02
#define  S2CDEV_STATUS           "device_status"    //0x03
#define  S2CDEV_SETTING          "device_setting"   //0x04
#define  S2CFIRMWARE_ONLINE      "firmware_online"  //0x05
#define  S2CFIRMWARE_UPGRADE     "firmware_upgrade" //0x06
#define  S2CTRANSFER_DATA        "transfer_data"    //0x07
#define  S2CDEV_ACCOUNTS         "device_accounts"    //0x08
#define  S2CFIRMWARE_SETTING     "firmware_settings"  //0x09


/**
*MQTT订阅topic消息
*1.AT+CLOUDSUB=<topic>,<qos>
*2.	<topic>:申请订阅的topic，字符串类型
*3.	<qos>:该topic对应的qos等级，数字类型，目前支持QoS级别是0和1
**/


#define  IOT_MAX_LENGTH ( 64 + 1 )

typedef struct 
{
  char pkey[IOT_MAX_LENGTH];//aliyun iot product key
	char psrt[IOT_MAX_LENGTH];//aliyun iot product screct
	char dname[IOT_MAX_LENGTH];//aliyun iot device name
	char dsrt[IOT_MAX_LENGTH];//aliyun iot device screct
}iot_device;


typedef struct
{
	double SHOP_LONGTITUDE;
	double SHOP_LATITUDE;
	uint32_t CHECK_TIMES;
	uint32_t CHECK_RANGE;
	u8		WARN_FLAG;
	uint32_t  PORT_ID;
	u8		DATA_STATUS_FLAG;
}Device_GPRS_Base_Location;



typedef struct 
{
  char userIdLast[12];   //玩家ID
	char userIdnew[12];    //玩家ID
	char orderLast[23];    //order
	char orderNew[23];     //order
	u8 isForce;            //是否强制投币
	u8  isPlay;            //是否继续允许投币
}userId;

typedef struct 
{
	char OrderIdForCmp[23];
  userId port[16];
}Insert_service;


typedef struct
{
	u8 type;//1:串口; 2:脉冲
	u8 signalThreshold;
	u8 timeThreshold;
} firmware;



extern firmware g_firmware_info;
extern Insert_service coin_service;
extern iot_device g_iot_info;


int iot_info_set(char* pkey, char* psrt, char* dname, char* dsrt);
int aliyun_iot_info_gen(char *mqtt_client_id, char *mqtt_username, char *mqtt_password);
int IOT_MC_Subscribe(MQTTClient* c, messageHandler messageHandler);
int IOT_MC_Publish(MQTTClient* c, enum QoS, const char* topicName, char* data, u16 len);
int IOT_MC_Unsubscribe(MQTTClient* c);

u8 is_iot_works(void);
int iot_cloudpub(char* topic, enum QoS qos, u8 msg_len, unsigned char* msg, u8 times);
int iot_msg_parse(char* topic, u16 msg_len, unsigned char* msg);
int iot_com_process(int topicnum, uint16_t len, uint8_t *msg);

void vIot_Task(void);



/*
?	MQTT 收到pub消息命令：+CLOUDPUBLISH
1.	+CLOUDPUBLISH:<packId>,<topic>,<msg_len>,<msg>
2.	<packId>:数据包id
3.	<topic>:接收到的topic名字
4.	<msg_len>:接收到的消息长度
5.	<msg>:接收到的消息内容
*/


int s2c_firmware_online(uint16_t len, uint8_t *msg);
int s2c_insert_coin_response(u8 port, char* order_num, u8 order_num_len, u16 coin, u8 isnetwork_cmd);
int s2c_insert_response(u8 port, char* order_num, u8 order_num_len, u16 status, u8 isnetwork_cmd);
int s2c_firmware_upgrade(uint16_t len, uint8_t *msg);//debug

int c2s_firmware_upgrade_progress(uint32_t percent);

int c2s_s2c_status(u8 port, uint32_t s2c_type, u8 result);


#endif

