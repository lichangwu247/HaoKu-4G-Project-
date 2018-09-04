#include "iot.h"
#include "routine.h"
#include "selector.h"
#include "usr_task.h"
#include "prize.h"
#include "mobile.h"
#include "rtc.h"
#include "delay.h"
#include "24Cxx.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "led.h"
#include "timer.h"
#include "info_storage.h"
#include "hash_crypt.h"
#include "pvd.h"
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"
#include "dbg.h"
#include "string.h"

#include "usr_task.h"

#include "flash.h"

//�������ȼ�
#define START_TASK_PRIO			1
//�����ջ��С	
#define START_STK_SIZE 			256  
//������
TaskHandle_t StartTask_Handler;
//������
void start_task(void *pvParameters);


#ifdef USERIF_PRINTF
	#define TaskUserIF_TASK_PRIO  5
	#define TaskUserIF_STK_SIZE   360
	TaskHandle_t TaskUserIF_Handler;
	void vTaskTaskUserIF(void *pvParameters);
#endif


int main(void)
{
	SCB->VTOR = FLASH_BASE | FLASH_APP_OFFSET; /* Vector Table Relocation in Internal FLASH. */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//����ϵͳ�ж����ȼ�����4	
  PVD_Configuration();
	delay_init();	    				//��ʱ������ʼ��
	AT24CXX_Init();           //��ʼ��EEPROM
	set_bin_mark(read_bin_mark());   //��ȡ��ǰ�̼����

	uart1_init(230400);				//��ʼ����ӡ����
	uart2_init(115200);	      //ͨѶģ�鴮������
	uart3_buadRate(0xB0);	//���ݲ��������豸���Ӵ��ڲ����� 38400bps
  COM_DBG("\nVersion: %d.%d.%d\n",IOT_VERSION_MAJOR, IOT_VERSION_MINOR, IOT_VERSION_REVISION);
	firmware_data_read();     //��ȡ���� 
	iot_data_read();
	COM_DBG("\ndName: %s\n", g_iot_info.dname);
	
	LED_Init();		  					//��ʼ��LED
	RTC_Init();               //��ʼ��RTC
  
	user_selector_init();     //Ͷ������ʼ��
	
	prizeInfo_read();         //��ȡ������ά�������Ϣ

	MOBILE_CTR_Config();      //ͨѶģ��ʹ��IO��ʼ��
	MOBILE_POWER(MOBILEON);   //ͨѶģ���ϵ�
	
	Timer2_Init_Config(1000-1,7200-1);//��ʼ����ʱ��2����ʱ100ms,����MOBILE���ճ�ʱ
#ifdef USERIF_PRINTF
	TIM3_Int_Init(10-1,720-1);		//��ʼ����ʱ��3����ʱ������100us
#endif
  TIM5_Int_Init(50-1,7200-1);		//��ʼ����ʱ��5����ʱ������5ms
	backup_read();                //��ȡδ��������������������������������ͣ�
	coin_out_action(g_service_info.coin_res);   //����ϴ�δ�����������
	
	//������ʼ����
    xTaskCreate((TaskFunction_t )start_task,            //������
                (const char*    )"start_task",          //��������
                (uint16_t       )START_STK_SIZE,        //�����ջ��С
                (void*          )NULL,                  //���ݸ��������Ĳ���
                (UBaseType_t    )START_TASK_PRIO,       //�������ȼ�
                (TaskHandle_t*  )&StartTask_Handler);   //������              
    vTaskStartScheduler();          //�����������
		while(1);
}



//��ʼ����������
void start_task(void *pvParameters)
{
  taskENTER_CRITICAL();           //�����ٽ���
    //��������
	start_Mobile_task();
	
#ifdef USE_GPRS		 //���ʹ��GPRS��λ���ܣ���user_config.h�д򿪶���
	start_Mobile_GPRS_task();
#endif
	
	start_MqttClient_task();
	start_Msg_task();
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);

#ifdef USERIF_PRINTF
  xTaskCreate((TaskFunction_t )vTaskTaskUserIF,  			//������
                (const char*    )"IF_task", 			//��������
                (uint16_t       )TaskUserIF_STK_SIZE,		//�����ջ��С
                (void*          )NULL,						//���ݸ��������Ĳ���
                (UBaseType_t    )TaskUserIF_TASK_PRIO,		//�������ȼ�
                (TaskHandle_t*  )&TaskUserIF_Handler); 	//������	
#endif
	vTaskDelete(StartTask_Handler); //ɾ����ʼ����
  taskEXIT_CRITICAL();            //�˳��ٽ���
}



/*
*********************************************************************************************************
*	�� �� ��: vTaskTaskUserIF
*	����˵��: �ӿ���Ϣ����
*	��    ��: pvParameters ���ڴ���������ʱ���ݵ��β�
*	�� �� ֵ: ��
*   �� �� ��: 1  (��ֵԽС���ȼ�Խ�ͣ������uCOS�෴)
*********************************************************************************************************
*/
/*
*                ��������״̬�Ķ������£������洮�ڴ�ӡ��ĸB, R, D, S��Ӧ��
*                    #define tskBLOCKED_CHAR		   ( 'B' )  ����
*                    #define tskREADY_CHAR		     ( 'R' )  ����
*                    #define tskDELETED_CHAR		   ( 'D' )  ɾ��
*                    #define tskSUSPENDED_CHAR	   ( 'S' )  ����
*/

#ifdef USERIF_PRINTF

void vTaskTaskUserIF(void *pvParameters)
{
	uint8_t pcWriteBuffer[500];

	while(1)
	{
			COM_DBG("=================================================\r\n");
			COM_DBG("������      ����״̬ ���ȼ�   ʣ��ջ �������\r\n");
			vTaskList((char *)&pcWriteBuffer);
			COM_DBG("%s\r\n", pcWriteBuffer);
				
			COM_DBG("\r\n������       ���м���         ʹ����\r\n");
			vTaskGetRunTimeStats((char *)&pcWriteBuffer);
			COM_DBG("%s\r\n", pcWriteBuffer);
		  
		  COM_DBG("����״̬:   R-����  B-����  S-����  D-ɾ��\n"); 
			
			vTaskDelay(20000);
	}
}

#endif

