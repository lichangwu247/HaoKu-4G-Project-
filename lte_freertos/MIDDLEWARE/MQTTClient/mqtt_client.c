#include "usr_task.h"
#include "mobile_startup.h"
#include "mqtt_client.h"
#include "MQTTClient.h"
#include "led.h"
#include "device.h"
#include "iot.h"

MQTTClient client;


// Callback when receiving subscribed message
void messageArrived(MessageData* data)
{
	COM_DBG("Message arrived on topic %.*s: %.*s\n", data->topicName->lenstring.len, data->topicName->lenstring.data,
		data->message->payloadlen, data->message->payload);
	
	iot_msg_parse(data->topicName->lenstring.data, data->message->payloadlen, data->message->payload);
}


// Callback when MqttClientDisconnected
void MqttClientDisconnected(void* parm)
{
	MQTTClient* c = (MQTTClient*)parm;
	COM_DBG("MqttClientDisconnected\r\n");
	c->connState = MQTT_DISCONNECTED;
	
}


// Callback when MqttClientConnected
void MqttClientConnected(void* parm)
{
	MQTTClient* c = (MQTTClient*)parm;
	COM_DBG("MqttClientConnected\r\n");
	device.status = 106;//离线重上线后 同步后台状态 避免状态不同步
}


// Callback when MqttClientInited
void MqttClientInited(void* parm)
{
	firmware_first_pub();
	if(g_firmware_info.type == 1)
		check_device(1, 2, 1);
	COM_DBG("MqttClientInited!\r\n");
}



void Mqtt_Clinet_Task(void *pvParameters)
{
	//MQTTClient client;
	Network network;
	unsigned char sendbuf[1050], readbuf[1050];
	char mqtt_client_id[100] ={0};
	char mqtt_username[36] ={0};
	char mqtt_password[44] ={0};
	char mqtt_host[100] ={0};
	int rc = 0;
	u8 tryCnt = 0;
	
	vTaskSuspend(NULL);
	
	MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer; //定义MQTT 协议包连接数据结构体

	pvParameters = 0;
	NetworkInit(&network);  // 初始化网络
	MQTTClientInit(&client, &network, 5000, sendbuf, sizeof(sendbuf), readbuf, sizeof(readbuf));//初始化读写缓存
	MQTTClientCallbackInit(&client, MqttClientConnected, MqttClientDisconnected, MqttClientInited);//初始化MQTT回调函数
	client.defaultMessageHandler =  messageArrived;  //定义接收MQTT消息回调函数
	
	aliyun_iot_info_gen(mqtt_client_id, mqtt_username, mqtt_password);//阿里云信息生成 ，用户名，用户密码，mqtt客户端ID
	
	snprintf(mqtt_host, sizeof(mqtt_host),"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com", g_iot_info.pkey);
	if ((rc = NetworkConnect(&network, mqtt_host, 1883)) == rSUCCESS)
		client.connState = TCP_CONNECTED;
	else{
		client.connState = TCP_DISCONNECTED;
		goto exit;
	}
	COM_DBG("TCP Connected\r\n");
	LedSetStatus(1000,1000,0xffffffff);
	vTaskDelay(20);	//延时20ms，也就是20个时钟节拍
	
#if defined(MQTT_TASK)
	if ((rc = MQTTStartTask(&client)) != pdPASS)
		COM_DBG("Return code from start tasks is %d\n", rc);
#endif
	//设置MQTT协议数据包基本参数
  connectData.willFlag = 0;
  connectData.MQTTVersion = 3;
  connectData.clientID.cstring = mqtt_client_id;
  connectData.username.cstring = mqtt_username;
  connectData.password.cstring = mqtt_password;
  connectData.keepAliveInterval = 60;
  connectData.cleansession = 1;

	if ((rc = MQTTConnect(&client, &connectData)) == rSUCCESS)//MQTT连接
		client.connState = MQTT_CONNECTED;
	else{
		client.connState = MQTT_DISCONNECTED;
		goto exit;	
	}
	LedSetStatus(500,500,0xffffffff);
	vTaskDelay(20);	//延时20ms，也就是20个时钟节拍
	
	COM_DBG("\nMQTT Connected\r\n");
		
	if((rc = IOT_MC_Subscribe(&client, messageArrived)) == rSUCCESS)
		client.connState = MQTT_SUB_SUCCESS;
	else{
		client.connState = MQTT_SUB_FAIL;
		goto exit;
	}

	COM_DBG("\nMQTT Subscribe Successful\r\n");
	LedSetStatus(1000,0,0xffffffff);
	client.connState = MQTT_RUNING;
	
	exit:
	while (1)
	{
		vTaskDelay(200);	//延时200ms，也就是200个时钟节拍
		switch (client.connState)
		{
				default:
					LedSetStatus(0,1000,0xffffffff);
				  client.connState = MODULE_INIT;
				break;

				case MODULE_INIT:
					COM_DBG("MODULE_INIT\r\n");
					LedSetStatus(2000,1000,0xffffffff);
				  Sys_Soft_Reset();// 系统重启
					//start_Mobile_task();   //启动模块初始化连接任务
				  //vTaskSuspend(NULL);    //挂起本任务，连接状态为disconnect
				  client.connState = TCP_DISCONNECTED;   //客户端连接状态设置为 TCP断开
				break;
				
				case TCP_CONNECTED:
					COM_DBG("TCP_CONNECTED\r\n");
					LedSetStatus(1000,1000,0xffffffff);
					if ((rc = MQTTConnect(&client, &connectData)) == rSUCCESS)//如果客户端状态为 TCP连接状态，则连接MQTT
						client.connState = MQTT_CONNECTED;
					else
						client.connState = MQTT_DISCONNECTED;
				break;
				
				case TCP_DISCONNECTED:  //
					COM_DBG("TCP_DISCONNECTED\r\n");
					LedSetStatus(500,2000,0xffffffff);
					tryCnt++;
					if ((rc = NetworkConnect(&network, mqtt_host, 1883)) == rSUCCESS)// 如果客户端为TCP断开，重新连接网络
					{
						client.connState = TCP_CONNECTED;
						tryCnt = 0;
					}else{
						client.connState = TCP_DISCONNECTED;
						if(tryCnt >= 3)
						{
							tryCnt = 0;
							client.connState = MODULE_INIT;
						}
					}
				break;
				
				case MQTT_CONNECTED:
					COM_DBG("MQTT_CONNECTED\r\n");
					LedSetStatus(500,500,0xffffffff);
					if((rc = IOT_MC_Subscribe(&client, messageArrived)) == rSUCCESS) //如果客户端为MQTT 连接状态，则订阅信息
						client.connState = MQTT_SUB_SUCCESS;
					else
						client.connState = MQTT_SUB_FAIL;
				break;
				
				case MQTT_DISCONNECTED:  
					COM_DBG("MQTT_DISCONNECTED\r\n");
					rc = MQTTDisconnect(&client);  				//如果MQTT状态为断开连接状态，则断开客户端
				  if(rc == rSUCCESS)
						client.connState = TCP_DISCONNECTED;
					else{
						client.ipstack->disconnect(&network);
						client.connState = TCP_DISCONNECTED;
					}
				break;
					
				case MQTT_SUB_SUCCESS:				
					COM_DBG("MQTT_SUB_SUCCESS\r\n");
				  COM_DBG("MQTT_RUNING\r\n");
					client.connState = MQTT_RUNING;
				  LedSetStatus(1000,0,0xffffffff); //
				break;
				
				case MQTT_SUB_FAIL:
					COM_DBG("MQTT_SUB_FAIL\r\n");
					client.connState = MQTT_DISCONNECTED;
				break;
				
				case MQTT_RUNING:
					vTaskDelay(300);	//延时300ms，也就是300个时钟节拍
				break;				
		}		
	}
	
}


