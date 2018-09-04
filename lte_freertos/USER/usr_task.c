#include "info_storage.h"
#include "iot.h"
#include "device.h"
#include "upgrade.h"
#include "usr_task.h"
#include "routine.h"
#include "mqtt_client.h"
#include "mobile.h"
#include "mobile_startup.h"
#include "sys.h"
#include "MQTTFreeRTOS.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gprs.h"

#define MQTT_CLIENT_TASK_PRIO		4
#define MOBILE_TASK_PRIO	      5
#define MSG_TASK_PRIO		        3
#define UPGRADE_TASK_PRIO		    4

#define MOBILE_GPRS_TASK_PRIO	  5

#define MQTT_CLIENT_STK_SIZE    2048
#define MOBILE_STK_SIZE         256
#define MSG_STK_SIZE 		        1024
#define UPGRADE_STK_SIZE 		    256

#define MOBILE_GPRS_STK_SIZE    256

TaskHandle_t MQTTClinetTask_Handler = NULL;
TaskHandle_t MOBILETask_Handler = NULL;
TaskHandle_t MSGTask_Handler = NULL;
TaskHandle_t UPGRADETask_Handler = NULL;

TaskHandle_t MOBILE_GPRS_Task_Handler = NULL;


//创建模块GPRS
void start_Mobile_GPRS_task(void)
{
		taskENTER_CRITICAL();           //进入临界区
	  xTaskCreate((TaskFunction_t )vMOBILE_Start_GPRS_Task,  			//任务函数
                (const char*    )"MOBILE_GPRS_task", 			//任务名称
                (uint16_t       )MOBILE_GPRS_STK_SIZE,		//任务堆栈大小
                (void*          )NULL,						//传递给任务函数的参数
                (UBaseType_t    )MOBILE_GPRS_TASK_PRIO,		//任务优先级
                (TaskHandle_t*  )&MOBILE_GPRS_Task_Handler); 	//任务句柄	
    taskEXIT_CRITICAL();            //退出临界区								
}


//创建模块启动任务函数
void start_Mobile_task(void)
{
	if(MOBILETask_Handler == NULL)
	{
		taskENTER_CRITICAL();           //进入临界区
	  xTaskCreate((TaskFunction_t )vMOBILE_StartupTask,  			//任务函数
                (const char*    )"MOBILE_task", 			//任务名称
                (uint16_t       )MOBILE_STK_SIZE,		//任务堆栈大小
                (void*          )NULL,						//传递给任务函数的参数
                (UBaseType_t    )MOBILE_TASK_PRIO,		//任务优先级
                (TaskHandle_t*  )&MOBILETask_Handler); 	//任务句柄	
    taskEXIT_CRITICAL();            //退出临界区								
	}
}


//创建MQTT客户端任务函数
void start_MqttClient_task(void)
{
  xTaskCreate((TaskFunction_t )Mqtt_Clinet_Task,  			//任务函数
                (const char*    )"MQTTClient_task", 			//任务名称
                (uint16_t       )MQTT_CLIENT_STK_SIZE,		//任务堆栈大小
                (void*          )NULL,						//传递给任务函数的参数
                (UBaseType_t    )MQTT_CLIENT_TASK_PRIO,		//任务优先级
                (TaskHandle_t*  )&MQTTClinetTask_Handler); 	//任务句柄								

}

void vMsg_Task(void)
{
	uint32_t timerSCnt = 0;
	Timer tcycleTimer;
	TimerInit(&tcycleTimer);
	TimerCountdown(&tcycleTimer, 5);
	
  eqptInfoList = list_new();
	iotInfoList = list_new();
	
	while(1)
	{
		iot_msgInfoProc(iotInfoList);//设备消息链表处理
		eqpt_msgInfoProc(eqptInfoList);//IOT消息链表处理
		device_coin_report_or_store(); //实体币投币处理 上传或本地存储
		device_stored_report();        //将存储器存储内容发送至后台

		if (TimerIsExpired(&tcycleTimer))//5ms定时到后进入
		{
			TimerInit(&tcycleTimer);
			TimerCountdown(&tcycleTimer, 5);
			timerSCnt += 5;
			if(timerSCnt % 60 == 0)
			{
				//updata_mobile_signal();
				if(g_firmware_info.type == 1)
				{
					check_device(1,1,0);//更新与设备连接状态
				}

				/*if(mobile_get_signal() < g_firmware_info.signalThreshold)
					firmware_signal();*/
				
				if(g_firmware_info.type == 1)
				{
					if(device.timeout >= 3 && device.status != 106)
					{
						if(0 == device_status_report(106))
						{
							device.status = 106;
						}
					}	
					device.timeout ++;				
				}
			}
		}
	}	
}

//创建设备消息处理
void start_Msg_task(void)
{
  xTaskCreate((TaskFunction_t )vMsg_Task,  			//任务函数
                (const char*    )"Msg_task", 			//任务名称
                (uint16_t       )MSG_STK_SIZE,		//任务堆栈大小
                (void*          )NULL,						//传递给任务函数的参数
                (UBaseType_t    )MSG_TASK_PRIO,		//任务优先级
                (TaskHandle_t*  )&MSGTask_Handler); 	//任务句柄								

}

//创建设备消息处理
void start_Upgrade_task(void)
{
	if(UPGRADETask_Handler == NULL)
	{
		taskENTER_CRITICAL();           //进入临界区
		xTaskCreate((TaskFunction_t )upgrade_firmware,  			//任务函数
									(const char*    )"Upgrade_task", 			//任务名称
									(uint16_t       )UPGRADE_STK_SIZE,		//任务堆栈大小
									(void*          )NULL,						//传递给任务函数的参数
									(UBaseType_t    )UPGRADE_TASK_PRIO,		//任务优先级
									(TaskHandle_t*  )&UPGRADETask_Handler); 	//任务句柄								
		taskEXIT_CRITICAL();            //退出临界区
	}								
}


