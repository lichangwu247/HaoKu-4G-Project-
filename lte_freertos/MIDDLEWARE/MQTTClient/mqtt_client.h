/******************************************************************************

******************************************************************************/
#ifndef __MQTT_CLIENT_H__
#define __MQTT_CLIENT_H__

#include "MQTTClient.h"

extern MQTTClient client;

void Mqtt_Clinet_Task(void *pvParameters);
void beat_task(void * pvParameters);

#endif

