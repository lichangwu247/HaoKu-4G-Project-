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


//����ģ��GPRS
void start_Mobile_GPRS_task(void)
{
		taskENTER_CRITICAL();           //�����ٽ���
	  xTaskCreate((TaskFunction_t )vMOBILE_Start_GPRS_Task,  			//������
                (const char*    )"MOBILE_GPRS_task", 			//��������
                (uint16_t       )MOBILE_GPRS_STK_SIZE,		//�����ջ��С
                (void*          )NULL,						//���ݸ��������Ĳ���
                (UBaseType_t    )MOBILE_GPRS_TASK_PRIO,		//�������ȼ�
                (TaskHandle_t*  )&MOBILE_GPRS_Task_Handler); 	//������	
    taskEXIT_CRITICAL();            //�˳��ٽ���								
}


//����ģ������������
void start_Mobile_task(void)
{
	if(MOBILETask_Handler == NULL)
	{
		taskENTER_CRITICAL();           //�����ٽ���
	  xTaskCreate((TaskFunction_t )vMOBILE_StartupTask,  			//������
                (const char*    )"MOBILE_task", 			//��������
                (uint16_t       )MOBILE_STK_SIZE,		//�����ջ��С
                (void*          )NULL,						//���ݸ��������Ĳ���
                (UBaseType_t    )MOBILE_TASK_PRIO,		//�������ȼ�
                (TaskHandle_t*  )&MOBILETask_Handler); 	//������	
    taskEXIT_CRITICAL();            //�˳��ٽ���								
	}
}


//����MQTT�ͻ���������
void start_MqttClient_task(void)
{
  xTaskCreate((TaskFunction_t )Mqtt_Clinet_Task,  			//������
                (const char*    )"MQTTClient_task", 			//��������
                (uint16_t       )MQTT_CLIENT_STK_SIZE,		//�����ջ��С
                (void*          )NULL,						//���ݸ��������Ĳ���
                (UBaseType_t    )MQTT_CLIENT_TASK_PRIO,		//�������ȼ�
                (TaskHandle_t*  )&MQTTClinetTask_Handler); 	//������								

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
		iot_msgInfoProc(iotInfoList);//�豸��Ϣ������
		eqpt_msgInfoProc(eqptInfoList);//IOT��Ϣ������
		device_coin_report_or_store(); //ʵ���Ͷ�Ҵ��� �ϴ��򱾵ش洢
		device_stored_report();        //���洢���洢���ݷ�������̨

		if (TimerIsExpired(&tcycleTimer))//5ms��ʱ�������
		{
			TimerInit(&tcycleTimer);
			TimerCountdown(&tcycleTimer, 5);
			timerSCnt += 5;
			if(timerSCnt % 60 == 0)
			{
				//updata_mobile_signal();
				if(g_firmware_info.type == 1)
				{
					check_device(1,1,0);//�������豸����״̬
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

//�����豸��Ϣ����
void start_Msg_task(void)
{
  xTaskCreate((TaskFunction_t )vMsg_Task,  			//������
                (const char*    )"Msg_task", 			//��������
                (uint16_t       )MSG_STK_SIZE,		//�����ջ��С
                (void*          )NULL,						//���ݸ��������Ĳ���
                (UBaseType_t    )MSG_TASK_PRIO,		//�������ȼ�
                (TaskHandle_t*  )&MSGTask_Handler); 	//������								

}

//�����豸��Ϣ����
void start_Upgrade_task(void)
{
	if(UPGRADETask_Handler == NULL)
	{
		taskENTER_CRITICAL();           //�����ٽ���
		xTaskCreate((TaskFunction_t )upgrade_firmware,  			//������
									(const char*    )"Upgrade_task", 			//��������
									(uint16_t       )UPGRADE_STK_SIZE,		//�����ջ��С
									(void*          )NULL,						//���ݸ��������Ĳ���
									(UBaseType_t    )UPGRADE_TASK_PRIO,		//�������ȼ�
									(TaskHandle_t*  )&UPGRADETask_Handler); 	//������								
		taskEXIT_CRITICAL();            //�˳��ٽ���
	}								
}


