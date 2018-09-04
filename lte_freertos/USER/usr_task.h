#ifndef __USER_TASK_H__
#define __USER_TASK_H__
#include "FreeRTOS.h"
#include "task.h"

void start_Mobile_GPRS_task(void);
void start_Mobile_task(void);
void start_MqttClient_task(void);
void start_Msg_task(void);	
void start_Upgrade_task(void);


#endif

