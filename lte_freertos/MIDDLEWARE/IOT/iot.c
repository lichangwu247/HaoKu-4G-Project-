#include "prize.h"
#include "routine.h"
#include "MQTTClient.h"
#include "mqtt_client.h"
#include "iot.h"
#include "device.h"
#include "selector.h"
#include "flash.h"
#include "rtc.h"
#include "chip.h"
#include "mobile.h"
#include "info_storage.h" 
#include "protobuf.h"
#include "sys_list.h" 
#include "hash_crypt.h"
#include "sys.h"


#define  IOT_TOPIC_SUB      "/%s/%s/s2c/%s"

#define  IOT_TOPIC_PUB      "/%s/%s/c2s/%s"

#define  IOT_TOPIC_UNSUB    "/%s/%s/s2c/%s"

Insert_service coin_service;
iot_device     g_iot_info;
firmware       g_firmware_info;

Device_GPRS_Base_Location  device_gprs_base_locate;

int iot_info_set(char* pkey, char* psrt, char* dname, char* dsrt)
{
	memset(g_iot_info.pkey,0,IOT_MAX_LENGTH);
	memcpy(g_iot_info.pkey,pkey,strlen(pkey));
	memset(g_iot_info.psrt,0,IOT_MAX_LENGTH);
	memcpy(g_iot_info.psrt,psrt,strlen(psrt));
	memset(g_iot_info.dname,0,IOT_MAX_LENGTH);
	memcpy(g_iot_info.dname,dname,strlen(dname));
	memset(g_iot_info.dsrt,0,IOT_MAX_LENGTH);
	memcpy(g_iot_info.dsrt,dsrt,strlen(dsrt));

	return 0;
}


int aliyun_iot_info_gen(char *mqtt_client_id, char *mqtt_username, char *mqtt_password)
{
	char content[100] ={0};
	char client_id[100] ={0};
	char username[36] ={0};
	char password[44] ={0};
	char chip_id[32] = {0};

	uint32_t timestamp = Unix_Get();

	Get_ChipID(chip_id);
	COM_DBG("id: %s\r\n", chip_id);
#ifdef EE_WRITE_IOT  //烧写产品密钥等信息，
	iot_info_set(PRODUCT_KEY, PRODUCT_SECRET, DEVICE_NAME, DEVICE_SECRET);
	iot_data_write();//写入存储器，IOT会读取此信息
#endif
	memset((u8*)&g_iot_info, 0x0, sizeof(g_iot_info));
  iot_data_read();//读出信息

	// Unique client ID
  snprintf(client_id, sizeof(client_id),
				"%s|securemode=3,signmethod=hmacsha1,timestamp=%d|", chip_id, timestamp);
			
  snprintf(username, sizeof(username),"%s&%s", g_iot_info.dname, g_iot_info.pkey);
			
	snprintf(content, sizeof(content),"clientId%sdeviceName%sproductKey%stimestamp%d", 
				           chip_id, g_iot_info.dname, g_iot_info.pkey, timestamp);

	hmac_sha1_calc((uint8_t* )content, strlen(content), (uint8_t* )g_iot_info.dsrt, strlen(g_iot_info.dsrt), (uint8_t* )mqtt_password);
	
	strncpy(mqtt_client_id, client_id, strlen(client_id));	
  strncpy(mqtt_username, username, strlen(username));	
  strncpy(mqtt_password, password, strlen(password));		
	COM_DBG("mqtt_client_id: %s\r\n", mqtt_client_id);
	COM_DBG("mqtt_username: %s\r\n", mqtt_username);
	COM_DBG("mqtt_password: %s\r\n", mqtt_password);
  return 0;	
}


/*
IOT topic sub
*/

int IOT_MC_Subscribe(MQTTClient* c, messageHandler messageHandler)
{
	int rc = FAILURE;
	char topic[64];
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_COIN);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_PRIZE);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_STATUS);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_SETTING);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CFIRMWARE_ONLINE);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CFIRMWARE_UPGRADE);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CTRANSFER_DATA);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_ACCOUNTS);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	snprintf(topic, sizeof(topic), IOT_TOPIC_SUB, g_iot_info.pkey, g_iot_info.dname, S2CFIRMWARE_SETTING);
	if ((rc = MQTTSubscribe(c, topic, QOS1, messageHandler)) != rSUCCESS)
		return rc;
	
	if (c->initedCb)
		c->initedCb(c);
	
	return rSUCCESS;	
}


/*
IOT topic pub
*/
int IOT_MC_Publish(MQTTClient* c, enum QoS qos, const char* topicName, char* data, u16 len)
{
	MQTTMessage message; //定义MQTT信息结构体
	char topic[64];
									//将 产品密钥、设备名，话题名 按照 IOT_TOPIC_PUB 放入topic中
	snprintf(topic, sizeof(topic), IOT_TOPIC_PUB, g_iot_info.pkey, g_iot_info.dname, topicName);
	
	message.qos = qos;  //封装信息
	message.retained = 0;
	message.payload = data;
	message.payloadlen = len;
									
	return MQTTPublish(c, topic, &message);
}


u8 is_iot_works(void)
{
	return MQTTIsConnected(&client);
}


int iot_cloudpub(char* topic, enum QoS qos, u8 msg_len, unsigned char* msg, u8 times)
{
	int rc;
	
	if(is_iot_works() != 1)
		return -1;
	
	do{										//MQTT客户端 ,QOS,话题，信息，长度
		rc = IOT_MC_Publish(&client, qos, topic, (char*)msg, msg_len);
		if(rc == 0)
			return rc;
	}while(--times);
	
	return rc;
	//return IOT_MC_Publish(&client, qos, topic, (char*)msg, msg_len);
}





/*
IOT topic unsub
*/

int IOT_MC_Unsubscribe(MQTTClient* c)
{
	int rc = FAILURE;
	char topicFliter[64];
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_COIN);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_PRIZE);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_STATUS);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_SETTING);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CFIRMWARE_ONLINE);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CFIRMWARE_UPGRADE);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CTRANSFER_DATA);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CDEV_ACCOUNTS);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	snprintf(topicFliter, sizeof(topicFliter), IOT_TOPIC_UNSUB, g_iot_info.pkey, g_iot_info.dname, S2CFIRMWARE_SETTING);
	if ((rc = MQTTUnsubscribe(c, topicFliter)) != rSUCCESS)
		return rc;
	
	return rSUCCESS;	
}


/*
?	MQTT 收到pub消息命令：
1.  
2.	<topic>:接收到的topic名字
3.	<msg_len>:接收到的消息长度
4.	<msg>:接收到的消息内容
*/
int iot_msg_parse(char* topic, u16 msg_len, unsigned char* msg)
{
	int topicnum;
	list_node_t *node = NULL;
/*************************************************/
	if(NULL == topic)
	{
		return -1;
	}
	
	if ((char *)strstr(topic, S2CDEV_COIN) != NULL){
		  topicnum = DEV_COIN;
		}else if ((char *)strstr(topic, S2CDEV_PRIZE) != NULL){
			topicnum = DEV_PRIZE;
		}else if ((char *)strstr(topic, S2CDEV_STATUS) != NULL){
		  topicnum = DEV_STATUS;
		}else if ((char *)strstr(topic, S2CDEV_SETTING) != NULL){
			topicnum = DEV_SETTING;		
		}else if ((char *)strstr(topic, S2CFIRMWARE_ONLINE) != NULL){
			topicnum = FIRMWARE_ONLINE;		
		}else if ((char *)strstr(topic, S2CFIRMWARE_UPGRADE) != NULL){
			topicnum = FIRMWARE_UPGRADE;		
		}else if ((char *)strstr(topic, S2CTRANSFER_DATA) != NULL){
			topicnum = TRANSFER_DATA;		
		}else if ((char *)strstr(topic, S2CDEV_ACCOUNTS) != NULL){
			topicnum = DEV_ACCOUNTS;		
		}else if ((char *)strstr(topic, S2CFIRMWARE_SETTING) != NULL){
			topicnum = FIRMWARE_SETTING;		
		}		
		
	if (0 != push_iotInfo_to(iotInfoList, topicnum, msg_len, (unsigned char *)msg, &node))
	{
		COM_DBG("push publish into to pubInfolist failed!\n");
		return -1;
  }
	return 0;
}




/*
具体消息处理 对于s2c/topic
*/

//用于自环测试/脉冲型设备 模拟设备上分回传消息
int s2c_insert_coin_response(u8 port, char* order_num, u8 order_num_len, u16 coin, u8 isnetwork_cmd)
{
	u8 num;
	unsigned char sendbuf[128];

	num = SerializeToOstream(sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i", 
	        1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3,order_num_len, order_num, 4,4,(uint32_t)coin, 5,4,(uint32_t)mobile_get_signal());
	return iot_cloudpub(C2SDEV_COIN, QOS1, num, sendbuf, 1);
}


//用于串口上币反馈
int s2c_insert_response(u8 port, char* order_num, u8 order_num_len, u16 status, u8 isnetwork_cmd)
{
	u8 num;
	unsigned char sendbuf[128];

	num = SerializeToOstream(sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i", 
	        1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3,order_num_len, order_num, 7,4,(uint32_t)status, 5,4,(uint32_t)mobile_get_signal());
	return iot_cloudpub(C2SDEV_COIN, QOS1, num, sendbuf, 1);
}


/*************************************************************
 *  名称:	device_account_get_response()
 *  功能:  上报升级firmware的万分比
 *	输入:  percent-完成万分比 
 *	输出:  TRUE-发送成功  FASLE-发送失败
 *************************************************************/
int c2s_firmware_upgrade_progress(uint32_t percent)
{
	u8 num;
	unsigned char sendbuf[64];
  if(percent > 10000)//超过10000为升级状态代码
	{
		num = SerializeToOstream(sendbuf, "%h%h%l%h%h%i", 
	       1,8,(uint64_t)Unix_Get(),3,4, (uint32_t)percent);
	}else{
		num = SerializeToOstream(sendbuf, "%h%h%l%h%h%i", 
	       1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)percent);
	}

	return iot_cloudpub(C2SFIRMWARE_UPGRADE, QOS1, num, sendbuf, 2);
}


// service to client intsert coin
int s2c_insert_coin(uint16_t len, uint8_t *msg)
{
	uint16_t datalen = 0,field = 0;
	uint32_t coin_num = 0,port_id = 0,time_dif = 0;
	uint8_t  order_num_len;//订单长度
	uint8_t  userId_len;//ID长度
	char* order_num;//订单编号
	char* userId;//订单编号
	uint64_t timestamp,timestamp_now;
	static char order_num_last[23] = {0};
	u8 msg_buff[25] = {0};
	u8 temp = 0;
	uint32_t insertForce = 0,isPlay = 1;
	u8 i;
//	list_node_t *node = NULL;
	record rec;
	
	/*u16 num =0;
	u8 iot_sendbuf[128];*/

	void* value;
	
	//static uint32_t coin_times = 0;
	//coin_times++;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
			  memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2:
			  memcpy(&port_id,value,sizeof(uint32_t));
			  COM_DBG("port_id: %d\n", port_id);
		  break;
			
			case 3:
				order_num = (char*)value;
			  order_num_len = datalen;
			  COM_DBG("order_num: ");
			  DBG_STRING((unsigned char*)order_num, order_num_len);
			  if(0 == strncmp(order_num_last, order_num, order_num_len))
					return -1;
				memset(order_num_last, 0, 23);
				memcpy(order_num_last, order_num, order_num_len);
				
				memcpy(coin_service.port[port_id - 1].orderNew, order_num_last, order_num_len);
				if((*(coin_service.OrderIdForCmp) == 0)&&(order_num_len <= 22))
				{
				   memcpy(coin_service.OrderIdForCmp, order_num_last, order_num_len);	
					 COM_DBG("OrderIdForCmp: %s\r\n", coin_service.OrderIdForCmp);			 
				}
		  break;
			
			case 4:
			  memcpy(&coin_num,value,sizeof(uint32_t));
			  COM_DBG("coin_num: %d\n", coin_num);
		  break;
			
			case 5:
				userId = (char*)value;
			  userId_len = datalen;
			  COM_DBG("userId: ");
			  DBG_STRING((unsigned char*)userId, userId_len);
			  memset(coin_service.port[port_id - 1].userIdnew, 0x0, 12);
			  memcpy(coin_service.port[port_id - 1].userIdnew, userId, userId_len);
		  break;
				
			case 6:
				memcpy(&insertForce,value,sizeof(uint32_t));
			  COM_DBG("insertForce: %d\n", insertForce);
			  coin_service.port[port_id - 1].isForce =  insertForce;
		  break;
			
			case 7:
				memcpy(&isPlay,value,sizeof(uint32_t));
			  COM_DBG("isPlay: %d\n", isPlay);
			  coin_service.port[port_id - 1].isPlay =  isPlay;
		  break;

			default:
			break;	
		}
	}
	
/*************************************************/
	//测试返回值为不合法情况（非1/2）
	/*if(1)
	{
		num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i",
					  1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port_id,3,strlen(order_num), order_num, 
					  7,4,(uint32_t)200);
		return iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1);
	}	*/
	

	
/*************************************************/
	timestamp_now = Unix_Get();
	COM_DBG("timestamp_now: %lld\n", timestamp_now);
	if(timestamp_now >= timestamp)
	{
		time_dif = timestamp_now - timestamp;
		//time_dif = 33;//测试超时用
	}else{
		time_dif = timestamp - timestamp_now;
		//time_dif = 34;//测试超时用
	}
	COM_DBG("time_dif: %d\n", time_dif);
	if(time_dif > g_firmware_info.timeThreshold)
	{
		list_node_t *node = NULL;
		memcpy(msg_buff, order_num_last, 22);
		memcpy(msg_buff + 22, &temp, 1);
		memcpy(msg_buff + 23, &time_dif, sizeof(uint32_t));
		push_eqptInfo_to(eqptInfoList, port_id, DEVICE_COIN, 27, msg_buff, 1, &node);
		updata_mobile_time();     //网络时间超太多，网络对时
		return -1;
	}
	
	if(g_firmware_info.type == 1)
	{
			device_insert_coin(port_id, order_num, order_num_len, coin_num);//正式串口
	}else{
		if(0 != s2c_insert_coin_response(port_id, order_num_last, order_num_len, coin_num,1))
		{
			if(0 != s2c_insert_coin_response(port_id, order_num_last, order_num_len, 0,1))
			{
				memset((u8*)&rec, 0x0, sizeof(record));
				rec.num = 0;//为成功上币，同步后台  如不成功则存储起来   后续发送
				memcpy(rec.orderNum, order_num_last, order_num_len);
				rec.seq = 0;
				rec.timestamp = Unix_Get();
				rec.type = 0;//上分动作
				for(i = 0; i< (sizeof(record) - 2);i++)
				{
					rec.sum +=  *((u8*)&rec + i); 
				}
				record_write(&rec);
			}
			
			return -1;//脉冲型设备，先响应命令再上分
		}	
		
		//coin_num = coin_times;
		
		//COM_DBG("coin_times: %d\n", coin_times);

		if(coin_num != 0)
		{
			g_service_info.coin_res +=  coin_num;
			coin_out_action(coin_num);
		}
	}
	return 0;
}


int s2c_device_prize(uint16_t len, uint8_t *msg)
{
	uint16_t datalen = 0,field = 0,time_dif = 0;
	uint32_t prize_type = 0,prize_num = 0,port_id = 0;
	uint64_t timestamp,timestamp_now;
	uint8_t  service_id_len;//订单长度
  char* service_id;//订单编号
	static char service_id_last[23] = {0};

	void* value;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
			  memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2:
			  memcpy(&port_id,value,sizeof(uint32_t));
			  COM_DBG("port_id: %d\n", port_id);
		  break;
			
			case 3:
				service_id = (char*)value;
			  service_id_len = datalen;
			  COM_DBG("service_id: ");
			  DBG_STRING((unsigned char*)service_id, service_id_len);
			  if(NULL == strncmp(service_id_last, service_id, service_id_len))
					return -1;
				memset(service_id_last, 0, 23);
				memcpy(service_id_last, service_id, service_id_len);
		  break;
			
			case 4:
			  memcpy(&prize_type,value,sizeof(uint32_t));
			  COM_DBG("prize_type: %d\n", prize_type);
		  break;
			
			case 5:
			  memcpy(&prize_num,value,sizeof(uint32_t));
			  COM_DBG("prize_num: %d\n", prize_num);
		  break;	
			
			default:
			break;	
		}
	}
	/*************************************************/
	timestamp_now = Unix_Get();
	COM_DBG("timestamp_now: %lld\n", timestamp_now);
	if(timestamp_now >= timestamp)
	{
		time_dif = timestamp_now - timestamp;
		//time_dif = 20;//测试超时用
	}else{
		time_dif = timestamp - timestamp_now;
		//time_dif = 21;//测试超时用
	}
	COM_DBG("time_dif: %d\n", time_dif);
	if(time_dif > g_firmware_info.timeThreshold)
	{
		updata_mobile_time();//网络时间超太多，网络对时
		return -1;
	}
	
	
/*************************************************/

	switch (prize_type)
	{
			case 1://实体币
				if(0 != s2c_insert_coin_response(port_id, service_id_last, service_id_len, prize_num,1))
					return -1;
				//g_service_info.coin_res +=  prize_num;
				//coin_out_action(prize_num);
				COM_DBG("prize_type: %d\n", prize_type);
			  COM_DBG("prize_num: %d\n", prize_num);
			break;			
			
			case 51://礼品
				COM_DBG("prize_type: %d\n", prize_type);
			  COM_DBG("prize_num: %d\n", prize_num);
			break;
			
			case 52://彩票
				COM_DBG("prize_type: %d\n", prize_type);
			  COM_DBG("prize_num: %d\n", prize_num);
		  break;
			
			case 53://积分
				COM_DBG("prize_type: %d\n", prize_type);
			  COM_DBG("prize_num: %d\n", prize_num);
		  break;

			default:
			break;	
	}

	return 0;
}



int s2c_device_status(uint16_t len, uint8_t *msg)
{
	uint16_t datalen = 0,field = 0,time_dif = 0;
	uint32_t port_id = 0,status;
	uint64_t timestamp,timestamp_now;

	void* value;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
			  memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2:
			  memcpy(&port_id,value,sizeof(uint32_t));
			  COM_DBG("port_id: %d\n", port_id);
		  break;
			
			case 3:
			  memcpy(&status,value,sizeof(uint32_t));
			  COM_DBG("status: %d\n", status);
		  break;
			
			default:
			break;	
		}
	}
	
	/*************************************************/
	timestamp_now = Unix_Get();
	COM_DBG("timestamp_now: %lld\n", timestamp_now);
	if(timestamp_now >= timestamp)
	{
		time_dif = timestamp_now - timestamp;
		//time_dif = 20;//测试超时用
	}else{
		time_dif = timestamp - timestamp_now;
		//time_dif = 21;//测试超时用
	}
	COM_DBG("time_dif: %d\n", time_dif);
	if(time_dif > g_firmware_info.timeThreshold)
	{
		updata_mobile_time();//网络时间超太多，网络对时
		return -1;
	}
	
/*************************************************/
	//g_service_info.ctrStatus = status;
/*************************************************/
  COM_DBG("device status: %d\n", status);
	switch(status)
	{
		
		case 201://启动设备
				
		break;			
			
		case 202://停止设备

		break;
			
		case 203://重启设备

		break;
			
		case 204://设备恢复出厂设置
				
		break;
			
	  case 205://设备校准
		  set_device_status(port_id,0,2);		
		break;
						
		case 206://设备清除历史数据
			set_device_status(port_id, 1,2);		
		break;
		
		case 207://设备状态查询动作发起
			check_device(port_id, 2, 1);
		break;
		
		default:
		break;	
	}
	
	return 0;
}


int s2c_device_setting(uint16_t len, uint8_t *msg)
{
	uint16_t datalen = 0,field = 0, sub_len = 0,time_dif = 0;
	uint32_t port_id = 0,port_type = 0,action_type = 0;
	uint64_t timestamp,timestamp_now;
	
	uint8_t  selector_flag = 0;
	
	u8 *sub_msg;

	void *value;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
				memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2:
				memcpy(&port_id,value,sizeof(uint32_t));
			  COM_DBG("port_id: %d\n", port_id);
		  break;
			
			case 3:
				memcpy(&port_type,value,sizeof(uint32_t));
			  COM_DBG("port_type: %d\n", port_type);
		  break;
			
			case 4:
				memcpy(&action_type,value,sizeof(uint32_t));
			  COM_DBG("action_type: %d\n", action_type);
		  break;				
			
			case 5:
				sub_len = datalen;
			  sub_msg = (u8*)value;

			if(2 == port_type){
				while(sub_len > 4)
				{
					value = ParseForStream(&sub_len, &sub_msg, &field, &datalen);
					if(value == NULL)
						break;
					switch (field)
						{
							case 3101://投币器脉冲极性
								if(g_selector_info.polarity != *(uint32_t*)value)
									selector_flag = 1;
									g_selector_info.polarity = *(uint32_t*)value;
									COM_DBG("s_polarity: %d\n", g_selector_info.polarity);			
							break;
									
							case 3102://投币器脉冲宽度
								if(g_selector_info.width != *(uint32_t*)value)
									selector_flag = 1;
									g_selector_info.width = *(uint32_t*)value;
									COM_DBG("s_width: %d\n", g_selector_info.width);		
							break;
									
							case 3103://投币器脉冲间隔
								if(g_selector_info.interval != *(uint32_t*)value)
									selector_flag = 1;
									g_selector_info.interval = *(uint32_t*)value;
									COM_DBG("s_interval: %d\n", g_selector_info.interval);		
							break;
								
							case 3104://彩票口脉冲极性
								/*if(g_tb_out_info.polarity != *(uint32_t*)value)
									tb_out_flag = 1;
									g_tb_out_info.polarity = *(uint32_t*)value;
									COM_DBG("t_polarity: %d\n", g_tb_out_info.polarity);*/			
							break;
									
							case 3105://彩票口脉冲宽度
								/*if(g_tb_out_info.width != *(uint32_t*)value)
									tb_out_flag = 1;
									g_tb_out_info.width = *(uint32_t*)value;
									COM_DBG("t_width: %d\n", g_tb_out_info.width);*/			
							break;
									
							case 3106://彩票口脉冲间隔
								/*if(g_tb_out_info.interval != *(uint32_t*)value)
									tb_out_flag = 1;
									g_tb_out_info.interval = *(uint32_t*)value;
									COM_DBG("t_interval: %d\n", g_tb_out_info.interval);*/			
							break;
								
	/****************************************************************/			
							default:
							break;	
						}
					}
			}
			break;
			
			default:
			break;	
		}
	}
	
	if(action_type == 1)
	{
		device_params_get(port_id,0, NULL, 1);//获取设备参数，用于同步	
		return 0;
	}
	
	if(action_type != 2)
	{
	   return 0;
	}
	
	/*************************************************/
	timestamp_now = Unix_Get();
	COM_DBG("timestamp_now: %lld\n", timestamp_now);
	if(timestamp_now >= timestamp)
	{
		time_dif = timestamp_now - timestamp;
		//time_dif = 20;//测试超时用
	}else{
		time_dif = timestamp - timestamp_now;
		//time_dif = 21;//测试超时用
	}
	COM_DBG("time_dif: %d\n", time_dif);
	if(time_dif > g_firmware_info.timeThreshold)
	{
		updata_mobile_time();//网络时间超太多，网络对时
		return -1;
	}
	
	if(1 == port_type)
	{
		//dbghex_printf(sub_msg,sub_len);//测试用
		return device_params_set(port_id, sub_len,sub_msg, 1);
		
	}else if(2 == port_type){
		if(selector_flag == 1)
		{
			selector_data_write();
			coin_out_action(0);//立马更新极性
			COM_DBG("selector polarity: %d\n", g_selector_info.polarity);
			COM_DBG("selector width: %d\n", g_selector_info.width);
			COM_DBG("selector interval: %d\n", g_selector_info.interval);		
		}
	}
	return 0;
}


/*
上线后发送固件配置固件类型变量   达到固件只需一个版本
*/
int s2c_firmware_online(uint16_t len, uint8_t *msg)
{
	uint8_t  selector_flag = 0,firmware_flag = 0;
	uint16_t datalen = 0,field = 0, sub_len = 0;
	uint32_t signalThreshold, timeThreshold, status, type;
	uint64_t timestamp;
	u8* test_string, *sub_msg;

	void* value;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
			  memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);				
			break;
			
			case 2:
			  memcpy(&status,value,sizeof(uint32_t));
			  COM_DBG("device status: %d\n", status);
		  break;
			
			case 3:
				sub_len = datalen;
			  sub_msg = (u8*)value;

  			while(sub_len > 4)
					{
						value = ParseForStream(&sub_len, &sub_msg, &field, &datalen);
						if(value == NULL)
							break;
						switch (field)
						{
							  case 3000://信号强度变化上传阈值
								  memcpy(&signalThreshold,value,sizeof(uint32_t));
									COM_DBG("signalThreshold: %d\n", signalThreshold);
								  if(g_firmware_info.signalThreshold != signalThreshold)
										firmware_flag = 1;
									g_firmware_info.signalThreshold = signalThreshold;
								break;
									
								case 3001://时间阈值
								  memcpy(&timeThreshold,value,sizeof(uint32_t));
									COM_DBG("timeThreshold: %d\n", timeThreshold);
								  if(g_firmware_info.timeThreshold != timeThreshold)
										firmware_flag = 1;
								  g_firmware_info.timeThreshold = timeThreshold;				
								break;
								
								case 3101://投币器脉冲极性
									if(g_selector_info.polarity != *(uint32_t*)value)
										selector_flag = 1;
								  g_selector_info.polarity = *(uint32_t*)value;
									COM_DBG("s_polarity: %d\n", g_selector_info.polarity);			
								break;
								
								case 3102://投币器脉冲宽度
									if(g_selector_info.width != *(uint32_t*)value)
										selector_flag = 1;
									g_selector_info.width = *(uint32_t*)value;
									COM_DBG("s_width: %d\n", g_selector_info.width);		
								break;
								
								case 3103://投币器脉冲间隔
									if(g_selector_info.interval != *(uint32_t*)value)
										selector_flag = 1;
									g_selector_info.interval = *(uint32_t*)value;
									COM_DBG("s_interval: %d\n", g_selector_info.interval);			
								break;
								
								case 3104://彩票口脉冲极性
									/*if(g_tb_out_info.polarity != *(uint32_t*)value)
										tb_out_flag = 1;
									g_tb_out_info.polarity = *(uint32_t*)value;
									COM_DBG("t_polarity: %d\n", g_tb_out_info.polarity);*/			
								break;
								
								case 3105://彩票口脉冲宽度
									/*if(g_tb_out_info.width != *(uint32_t*)value)
										tb_out_flag = 1;
									g_tb_out_info.width = *(uint32_t*)value;
									COM_DBG("t_width: %d\n", g_tb_out_info.width);*/			
								break;
								
								case 3106://彩票口脉冲间隔
									/*if(g_tb_out_info.interval != *(uint32_t*)value)
										tb_out_flag = 1;
									g_tb_out_info.interval = *(uint32_t*)value;
									COM_DBG("t_interval: %d\n", g_tb_out_info.interval);*/			
								break;
							
							case 9999:
								test_string = value;
								COM_DBG("test_string: ");
								DBG_STRING(test_string, datalen);
							break;
							
							default:
							break;	
				  }
			  }
		  break;
				
			case 4:
				memcpy(&type,value,sizeof(uint32_t));
				COM_DBG("signalThreshold: %d\n", signalThreshold);
				if(g_firmware_info.type != type)
					firmware_flag = 1;
					g_firmware_info.type = type;
			  COM_DBG("device type: %d\n", type);
		  break;
			
			default:
			break;	
		}
	}

/*************************************************/

	if(firmware_flag == 1)
	{
		g_firmware_info.type = type;
		g_firmware_info.signalThreshold = signalThreshold;
		g_firmware_info.timeThreshold = timeThreshold;
		firmware_data_write();
	}
	
	
	if(selector_flag == 1)
	{
		selector_data_write();
		coin_out_action(0);//立马更新极性
		COM_DBG("selector polarity: %d\n", g_selector_info.polarity);
		COM_DBG("selector width: %d\n", g_selector_info.width);
		COM_DBG("selector interval: %d\n", g_selector_info.interval);		
	}
	return 0;
}


//服务器下发的数据接收处理函数  (len 数据长度，uint8_t *msg数据指针)
int s2c_transfer_data(uint16_t len, uint8_t *msg)
{
	uint16_t datalen = 0,field = 0, sub_len = 0;
	uint32_t port_id = 0,race_rank = 0,race_bonus = 0,race_bdiff = 0,race_no = 0;
	uint64_t timestamp;
	u8* qrcode_link = NULL, *sub_msg;
	u8* MD5key_p = NULL;
	u8* MD5keyId_p = NULL;
	u8* PrizeUrl_p = NULL;
	u8* devicePrizeUrl_p = NULL;
	u8* race_nikename_p = NULL;
	uint64_t timestamp_now = 0;
	
	u8  MD5key_len, MD5keyId_len, PrizeUrl_len ,qrcode_link_len, devicePrizeUrl_len,race_nikename_len;
	char strcmp_temp[72] ={0};
	char race_nikename[52] ={0};
	u8 send_flag = 0;
	u8 race_send_flag = 0;
	void* value;
 
	memset(&device_gprs_base_locate,0,sizeof(device_gprs_base_locate));
	
/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);//取出每个域中的数据
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:    //第一个域表示时间戳
				memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2://第三个域表示商品ID
				memcpy(&port_id,value,sizeof(uint32_t));
			  COM_DBG("port_id: %d\n", port_id);
		  break;
			
			case 3:  //第三个域表示 用户数据
			  sub_len = datalen;  //取出用户数据长度 
			  sub_msg = (u8*)value;

			while(sub_len > 4) //用户数据长度小于4
			{
					value = ParseForStream(&sub_len, &sub_msg, &field, &datalen);//获取用户数据域中的数据段，（子串长度，子串数据，子串口的域号，数据长度）返回
					if(value == NULL)
						break;
					switch (field)
					{
						case 3401:
							qrcode_link = value;
							qrcode_link_len = datalen;
							COM_DBG("qrcode_link: ");
							DBG_STRING(qrcode_link, qrcode_link_len);
						  if(qrcode_link_len <= payQRcodeLen)
							{
								memset(payQRcode[port_id-1],0x0, payQRcodeLen + 4);
								memcpy(payQRcode[port_id-1],qrcode_link, qrcode_link_len);
							}		  
						break;
						case 3605:
							PrizeUrl_p = value;
							PrizeUrl_len = datalen;
							COM_DBG("PrizeUrl: ");
							DBG_STRING(PrizeUrl_p, PrizeUrl_len);
						break;
						case 3606:
							MD5key_p = value;
							MD5key_len = datalen;
							COM_DBG("MD5key: ");
							DBG_STRING(MD5key_p, MD5key_len);
						break;
						case 3607:
							MD5keyId_p = value;
							MD5keyId_len = datalen;
							COM_DBG("MD5keyId: ");
							DBG_STRING(MD5keyId_p, MD5keyId_len);
						break;
									
						case 3608://设备生成二维码URL
							devicePrizeUrl_p = value;
							devicePrizeUrl_len = datalen;
							WRITE_IOT_DEBUG_LOG("Device PrizeUrl: ");
							DBG_STRING(devicePrizeUrl_p, devicePrizeUrl_len);
						break;

						case 3609://昵称
							race_nikename_p = value;
							race_nikename_len = datalen;
							WRITE_IOT_DEBUG_LOG("nikename: ");
							DBG_STRING(race_nikename_p, race_nikename_len);
							race_send_flag = 1;
							if(race_nikename_len <= 48)
								memcpy(race_nikename, race_nikename_p, race_nikename_len);
						break;

						case 3610://排名
						  memcpy(&race_rank,value,sizeof(uint32_t));
							race_send_flag = 1;
							WRITE_IOT_DEBUG_LOG("race_rank: %d\n", race_rank);
						break;
						
						case 3611://当前积分
						  memcpy(&race_bonus,value,sizeof(uint32_t));
							race_send_flag = 1;
							WRITE_IOT_DEBUG_LOG("race_bonus: %d\n", race_bonus);
						break;

						case 3612://与上一名积分差额
						  memcpy(&race_bdiff,value,sizeof(uint32_t));
							race_send_flag = 1;
							WRITE_IOT_DEBUG_LOG("race_bdiff: %d\n", race_bdiff);
						break;
						
						case 3614://当前期数  为0  不在竞赛
						  memcpy(&race_no,value,sizeof(uint32_t));
							race_send_flag = 1;
							WRITE_IOT_DEBUG_LOG("race_no: %d\n", race_no);
						break;						
	   			 //获取定位基地址
						case 3615:
							memcpy(&device_gprs_base_locate.SHOP_LONGTITUDE,value,sizeof(uint32_t));
							WRITE_IOT_DEBUG_LOG("SHOP_LONGTITUDE: %f\n", device_gprs_base_locate.SHOP_LONGTITUDE);
						break;
						case 3616:
							memcpy(&device_gprs_base_locate.SHOP_LATITUDE,value,sizeof(uint32_t));
							WRITE_IOT_DEBUG_LOG("SHOP_LATITUDE: %f\n", device_gprs_base_locate.SHOP_LATITUDE);
							break;
						case 3617:
							memcpy(&device_gprs_base_locate.CHECK_TIMES,value,sizeof(uint32_t));
							WRITE_IOT_DEBUG_LOG("CHECK_TIMES: %d\n", device_gprs_base_locate.CHECK_TIMES);
							break;
						case 3618:
							memcpy(&device_gprs_base_locate.CHECK_RANGE,value,sizeof(uint32_t));
						  memcpy(&device_gprs_base_locate.PORT_ID,&port_id,sizeof(uint32_t));
						  device_gprs_base_locate.DATA_STATUS_FLAG=1;
							WRITE_IOT_DEBUG_LOG("CHECK_RANGE: %d\n", device_gprs_base_locate.CHECK_RANGE);
							break;
						
						default:
						break;		
			    }
		  }
		  break;
			
			default:
			break;	
		}
	}
	
/*************************************************/
	if(NULL != qrcode_link)
  {
     device_qrcode_set(port_id,qrcode_link_len,qrcode_link,1); 
  }
	
	if((NULL != PrizeUrl_p) && (PrizeUrl_len <= prizeUrlLen))
  {
		memset(strcmp_temp, 0x0,sizeof(strcmp_temp));
		memcpy(strcmp_temp, PrizeUrl_p, PrizeUrl_len);
		if(0 != strcmp(prizeUrl,strcmp_temp))
		{
			memset(prizeUrl, 0x0, sizeof(prizeUrl));
			memcpy(prizeUrl,PrizeUrl_p, PrizeUrl_len);
			prizeInfo_save(URL_DATA_ADDR, (u8*)prizeUrl, sizeof(prizeUrl));
		}
  }
	
	if((NULL != MD5key_p) && (MD5key_len <= KEYLEN))
  {
		memset(strcmp_temp, 0x0,sizeof(strcmp_temp));
		memcpy(strcmp_temp, MD5key_p, MD5key_len);
		if(0 != strcmp(MD5key,strcmp_temp))
		{
			memset(MD5key, 0x0, sizeof(MD5key));
			memcpy(MD5key, MD5key_p, MD5key_len);
			prizeInfo_save(MD5KEY_DATA_ADDR, (u8*)MD5key, sizeof(MD5key));
		}
  }
	
	if((NULL != MD5keyId_p) && (MD5keyId_len <= KEYLEN))
  {
		memset(strcmp_temp, 0x0,sizeof(strcmp_temp));
		memcpy(strcmp_temp, MD5keyId_p, MD5key_len);
		if(0 != strcmp(MD5keyId,strcmp_temp))
		{
			memset(MD5keyId, 0x0, sizeof(MD5key));
			memcpy(MD5keyId, MD5keyId_p, MD5keyId_len);
			prizeInfo_save(MD5KEY_ID_DATA_ADDR, (u8*)MD5keyId, sizeof(MD5keyId));
		}
  }
	
	if((NULL != devicePrizeUrl_p) && (devicePrizeUrl_len <= prizeUrlLen))
  {
		memset(strcmp_temp, 0x0,sizeof(strcmp_temp));
		memcpy(strcmp_temp, devicePrizeUrl_p, devicePrizeUrl_len);

		if(0 != strcmp(devicePrizeUrl,strcmp_temp))
		{
			memset(devicePrizeUrl, 0x0, sizeof(devicePrizeUrl));
			memcpy(devicePrizeUrl,devicePrizeUrl_p, devicePrizeUrl_len);
		}
		send_flag = 1;
  }

	if(send_flag == 1)
	{
		
		device_prize_set(port_id, 2, strlen(MD5key), MD5key, 1);
		vTaskDelay(1000);//Fu：延迟1000ms，解决27命令发的URL,KeyId,KEY，种子时间等内容，发得太快的问题

		device_prize_set(port_id, 1, strlen(MD5keyId), MD5keyId, 1);
		vTaskDelay(1000);//Fu：延迟1000ms，解决27命令发的URL,KeyId,KEY，种子时间等内容，发得太快的问题

		device_prize_set(port_id, 0, strlen(devicePrizeUrl), devicePrizeUrl, 1);
		vTaskDelay(1000);//Fu：延迟1000ms，解决27命令发的URL,KeyId,KEY，种子时间等内容，发得太快的问题

		timestamp_now = Unix_Get();//获取当前时间
		
	  device_prize_set(port_id, 3, sizeof(timestamp_now), (char*)&timestamp_now, 1);
	}


	if(1 == race_send_flag)
	{
		device_race_set(port_id, race_no, race_rank, 
			                     race_bonus, race_bdiff, race_nikename, race_nikename_len);

		race_no = 0;
		race_rank = 0;
		race_bonus = 0;
		race_bdiff = 0;
		memset(race_nikename,0x0,sizeof(race_nikename));
	}

	return 0;
}



int s2c_get_device_accounts(uint16_t len, uint8_t *msg)
{
	uint16_t datalen = 0,field = 0;
	uint32_t port_id = 0;
	uint64_t timestamp;

	void* value;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
				memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2:
				memcpy(&port_id,value,sizeof(uint32_t));
			  COM_DBG("port_id: %d\n", port_id);
		  break;
			
			default:
			break;	
		}
	}
  //device_account_get_response(1, msg_account, sizeof(msg_account), 1);
	return device_account_get(port_id, 0, NULL, 1);
}


int s2c_firmware_upgrade(uint16_t len, uint8_t *msg)
{
	uint16_t datalen = 0,field = 0;
	uint32_t version,reboot = 0;
	uint64_t timestamp;
	u8* url;
	u8* sdk_version;
	u16 url_length;
	char type[12]={0};
	char host_port[64]={0};
	char filename[80]={0};
	char username[32]={0};
	char password[32]={0};
	

	void* value;
	
	if(0 != is_upgrading())
		return 0;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
				memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2:
				memcpy(&version,value,sizeof(uint32_t));
			  COM_DBG("version: %d\n", version);
		  break;
			
			case 3:
				url = value;
			  url_length = datalen;
			  COM_DBG("url: ");
			  DBG_STRING(url, url_length);
			  url[url_length] = 0;
		  break;
			
			case 4:
				memcpy(&reboot,value,sizeof(uint32_t));
			  COM_DBG("reboot: %d\n",reboot);
		  break;
			
			case 5:
				sdk_version = value;
			  COM_DBG("sdk_version: ");
			  DBG_STRING(sdk_version, datalen);
		  break;

			case 6:
				url = value;
			  url_length = datalen;
			  COM_DBG("sdk_url: ");
			  DBG_STRING(url, url_length);
			  url[url_length] = 0;
		  break;
			
			default:
			break;	
		}
	}

/*************************************************/
	if(version != VERSION_NUM)
	{
		if(5 == sscanf((char*)url, "%[^:]://%[^:]:%[^@]@%[^/]/%s",type, username, password, host_port, filename))
		{
			COM_DBG("type: %s\n",type);
			COM_DBG("USERNAME: %s\n",username);
			COM_DBG("PASSWORD: %s\n",password);
			COM_DBG("HOST: %s\n",host_port);
			COM_DBG("filename: %s\n",filename);
		}
		/*strcpy(type,"ftp");
		strcpy(username,"root");
		strcpy(password,"Haoku_123");
		strcpy(host_port,"39.108.235.86:21");
		strcpy(filename,"firmware_version/MOBILE/ttt");
		reboot = 1;*/
		
		if(USERBIN1 == get_bin_mark())
		{
			strcat(filename,"/usrbin0.hex");
			//STMFLASH_Erase(6,60);
		}else{
			strcat(filename,"/usrbin1.hex");
			//STMFLASH_Erase(66,60);
		}
		COM_DBG("filename: %s\n",filename);
		if(0 != strcmp(type,"ftp"))
			return -1;
		
  	if(0 == set_upgrade_info(host_port, filename, username, password, version, 1))
		{
			upgrade_firmware_check();
		}	
	}
	return 0;
}


int s2c_firmware_setting(uint16_t len, uint8_t *msg)
{
	uint8_t  firmware_flag = 0;
	uint16_t datalen = 0,field = 0, sub_len = 0,time_dif = 0;
	uint32_t signalThreshold, timeThreshold;
	uint64_t timestamp,timestamp_now;
	u8 *sub_msg;

	void* value;

/*************************************************/
	while(len > 4)
	{
		value = ParseForStream(&len, &msg, &field, &datalen);
		if(value == NULL)
			return -1;
		switch (field)
		{
			case 1:
			  memcpy(&timestamp,value,sizeof(uint64_t));
			  COM_DBG("timestamp: %lld\n", timestamp);
			break;
			
			case 2:
				sub_len = datalen;
			  sub_msg = (u8*)value;

  			while(sub_len > 4)
					{
						value = ParseForStream(&sub_len, &sub_msg, &field, &datalen);
						if(value == NULL)
							break;
						switch (field)
						{
							  case 3000://信号强度变化上传阈值
								  memcpy(&signalThreshold,value,sizeof(uint32_t));
									COM_DBG("signalThreshold: %d\n", signalThreshold);
								  if(g_firmware_info.signalThreshold != signalThreshold)
										firmware_flag = 1;
									g_firmware_info.signalThreshold = signalThreshold;
								break;
									
								case 3001://时间阈值
								  memcpy(&timeThreshold,value,sizeof(uint32_t));
									COM_DBG("timeThreshold: %d\n", timeThreshold);
								  if(g_firmware_info.timeThreshold != timeThreshold)
										firmware_flag = 1;
								  g_firmware_info.timeThreshold = timeThreshold;					
								break;
							
							default:
							break;	
				    }
			    }
		  break;
			
			default:
			break;	
		}
	}
	
/*************************************************/
	timestamp_now = Unix_Get();
	COM_DBG("timestamp_now: %lld\n", timestamp_now);
	if(timestamp_now >= timestamp)
	{
		time_dif = timestamp_now - timestamp;
		//time_dif = 20;//测试超时用
	}else{
		time_dif = timestamp - timestamp_now;
		//time_dif = 21;//测试超时用
	}
	COM_DBG("time_dif: %d\n", time_dif);
/*	if(time_dif > g_firmware_info.timeThreshold)
	{
		//updateTime();//网络时间超太多，网络对时
		return -1;
	}*/

/*************************************************/

	if(firmware_flag == 1)
	{
		g_firmware_info.signalThreshold = signalThreshold;
		g_firmware_info.timeThreshold = timeThreshold;
		firmware_data_write();
	}
	
	return 0;
}



/*************************************************************
 *  名称:	c2s_s2c_status()
 *  功能:  上报s2c命令执行结果
 *	输入:  s2c命令类型，结果result
 *	输出:  TRUE-发送成功  FASLE-发送失败
 *************************************************************/
int c2s_s2c_status(u8 port, uint32_t s2c_type, u8 result)
{
	u8 num;
	unsigned char sendbuf[64];
	
	if( FIRMWARE_SETTING == s2c_type)
	{
		num = SerializeToOstream(sendbuf, "%h%h%l%h%h%i%h%h%i", 
	       1,8,(uint64_t)Unix_Get(),2,4,s2c_type,4,4,(uint32_t)result);
	}else{
		num = SerializeToOstream(sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%i", 
	       1,8,(uint64_t)Unix_Get(),2,4,s2c_type,3,4,(uint32_t)port,4,4,(uint32_t)result);
	}

	//DBG_HEX((unsigned char*)iot_sendbuf, num);
	return iot_cloudpub(C2SS2C_STATUS, QOS1, num, sendbuf, 1);
}




/*
iot消息处理
*/
int iot_com_process(int topicnum, uint16_t len, uint8_t *msg)
{
	COM_DBG("iot cmd is: %d\r\n", topicnum);
	COM_DBG("iot len is: %d\r\n", len);
	COM_DBG("iot content is: ");
	DBG_HEX(msg, len);
	
	switch (topicnum)
	 {
		 case DEV_COIN://S2CDEV_COIN//insert_coin
			 s2c_insert_coin(len, msg);
		 break;
		 
		 case DEV_PRIZE://S2CDEV_PRIZE//device_prize
			 s2c_device_prize(len, msg); 
		 break;
		 
		 case DEV_STATUS://S2CDEV_STATUS//device_status
			 s2c_device_status(len, msg);
		 break;
		 
		 case DEV_SETTING://S2CDEV_SETTING//device_setting
			 s2c_device_setting(len, msg);	
		 break;
			 
		 case FIRMWARE_ONLINE://S2CFIRMWARE_ONLINE//firmware_online
			 s2c_firmware_online(len, msg);
		 break;
		 
		 case FIRMWARE_UPGRADE://S2CFIRMWARE_UPGRADE//firmware_upgrade
			 s2c_firmware_upgrade(len, msg);
		 break;
		 
		 case TRANSFER_DATA://S2CTRANSFER_DATA//transfer_data
			 s2c_transfer_data(len, msg);
		 break;
		 		 
		 case DEV_ACCOUNTS://S2CDEV_ACCOUNTS//device_accounts
			 s2c_get_device_accounts(len, msg);
		 break;
		 
		 case FIRMWARE_SETTING://S2CFIRMWARE_SETTING//firmware_settings
			 s2c_firmware_setting(len, msg);
		 break;
		 
		 default:
		 break;
	 }
	return 0;
}


