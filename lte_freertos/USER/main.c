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

//任务优先级
#define START_TASK_PRIO			1
//任务堆栈大小	
#define START_STK_SIZE 			256  
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//设置系统中断优先级分组4	
  PVD_Configuration();
	delay_init();	    				//延时函数初始化
	AT24CXX_Init();           //初始化EEPROM
	set_bin_mark(read_bin_mark());   //读取当前固件标记

	uart1_init(230400);				//初始化打印串口
	uart2_init(115200);	      //通讯模块串口设置
	uart3_buadRate(0xB0);	//根据参数设置设备连接串口波特率 38400bps
  COM_DBG("\nVersion: %d.%d.%d\n",IOT_VERSION_MAJOR, IOT_VERSION_MINOR, IOT_VERSION_REVISION);
	firmware_data_read();     //读取参数 
	iot_data_read();
	COM_DBG("\ndName: %s\n", g_iot_info.dname);
	
	LED_Init();		  					//初始化LED
	RTC_Init();               //初始化RTC
  
	user_selector_init();     //投币器初始化
	
	prizeInfo_read();         //读取出奖二维码加密信息

	MOBILE_CTR_Config();      //通讯模块使用IO初始化
	MOBILE_POWER(MOBILEON);   //通讯模块上电
	
	Timer2_Init_Config(1000-1,7200-1);//初始化定时器2，超时100ms,用于MOBILE接收超时
#ifdef USERIF_PRINTF
	TIM3_Int_Init(10-1,720-1);		//初始化定时器3，定时器周期100us
#endif
  TIM5_Int_Init(50-1,7200-1);		//初始化定时器5，定时器周期5ms
	backup_read();                //读取未输出的脉冲数，继续输出（针对脉冲类型）
	coin_out_action(g_service_info.coin_res);   //输出上次未输出的脉冲数
	
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
		while(1);
}



//开始任务任务函数
void start_task(void *pvParameters)
{
  taskENTER_CRITICAL();           //进入临界区
    //创建任务
	start_Mobile_task();
	
#ifdef USE_GPRS		 //如果使用GPRS定位功能，在user_config.h中打开定义
	start_Mobile_GPRS_task();
#endif
	
	start_MqttClient_task();
	start_Msg_task();
	USART_DMACmd(USART3,USART_DMAReq_Rx,ENABLE);

#ifdef USERIF_PRINTF
  xTaskCreate((TaskFunction_t )vTaskTaskUserIF,  			//任务函数
                (const char*    )"IF_task", 			//任务名称
                (uint16_t       )TaskUserIF_STK_SIZE,		//任务堆栈大小
                (void*          )NULL,						//传递给任务函数的参数
                (UBaseType_t    )TaskUserIF_TASK_PRIO,		//任务优先级
                (TaskHandle_t*  )&TaskUserIF_Handler); 	//任务句柄	
#endif
	vTaskDelete(StartTask_Handler); //删除开始任务
  taskEXIT_CRITICAL();            //退出临界区
}



/*
*********************************************************************************************************
*	函 数 名: vTaskTaskUserIF
*	功能说明: 接口消息处理。
*	形    参: pvParameters 是在创建该任务时传递的形参
*	返 回 值: 无
*   优 先 级: 1  (数值越小优先级越低，这个跟uCOS相反)
*********************************************************************************************************
*/
/*
*                任务运行状态的定义如下，跟上面串口打印字母B, R, D, S对应：
*                    #define tskBLOCKED_CHAR		   ( 'B' )  阻塞
*                    #define tskREADY_CHAR		     ( 'R' )  就绪
*                    #define tskDELETED_CHAR		   ( 'D' )  删除
*                    #define tskSUSPENDED_CHAR	   ( 'S' )  挂起
*/

#ifdef USERIF_PRINTF

void vTaskTaskUserIF(void *pvParameters)
{
	uint8_t pcWriteBuffer[500];

	while(1)
	{
			COM_DBG("=================================================\r\n");
			COM_DBG("任务名      任务状态 优先级   剩余栈 任务序号\r\n");
			vTaskList((char *)&pcWriteBuffer);
			COM_DBG("%s\r\n", pcWriteBuffer);
				
			COM_DBG("\r\n任务名       运行计数         使用率\r\n");
			vTaskGetRunTimeStats((char *)&pcWriteBuffer);
			COM_DBG("%s\r\n", pcWriteBuffer);
		  
		  COM_DBG("任务状态:   R-就绪  B-阻塞  S-挂起  D-删除\n"); 
			
			vTaskDelay(20000);
	}
}

#endif

