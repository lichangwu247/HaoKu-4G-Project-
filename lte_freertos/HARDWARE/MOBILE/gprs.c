#include "device.h"
#include "mobile_startup.h"
#include "usr_task.h"
#include "stm32f10x.h"
#include "gprs.h"
#include "usart1.h"
#include "mobile.h"
#include "iot.h"
#include "stdio.h"
#include "math.h"
#include "protobuf.h"

#define 	PI           	 3.1415626
#define 	EARTH_RADIUS   6378.137   //地球近似半径 uint: km

MOBILE_AT_SATRTUP Get_NEW_Location_CMD;

extern GPRS_Location GPRS_Location_Data;
extern Device_GPRS_Base_Location  device_gprs_base_locate;


// 求弧度
double radian(double d)
{
    return  d * PI / 180.0;   //角度 1度=PI/180
}

double get_distance(double lat1, double lng1, double lat2, double lng2)
{
		double radLat1 = radian(lat1);
    double radLat2 = radian(lat2);
    double a = radLat1 - radLat2;
    double b = radian(lng1) - radian(lng2);
    
    double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));
    
    dst = dst * EARTH_RADIUS;
    dst= round(dst * 10000) / 10000;
    return dst;
		
}

void vMOBILE_Start_GPRS_Task(void *arg)
{
	
	u8 gprs_sendbuf[21]={0};
	u8 set_default_value=0;
	u8 iot_sendbuf[128];
	u8 sub_iot_sendbuf[64];
	u8 num,sub_num;
	char order_num[1] = {0};
	double lat1 = 0;
  double lng1 = 0;//经度1，纬度1
  double lat2 = 0;
  double lng2 = 0;//经度2，纬度2
	
	vTaskSuspend(NULL);  //挂起任务 ，
	
	TickType_t xTicksToWait = 90000 / portTICK_PERIOD_MS; /* convert milliseconds to ticks 60S */ 
	TimeOut_t xTimeOut;
	vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */	
	
	Get_NEW_Location_CMD.cmdType=0;
	memcpy(Get_NEW_Location_CMD.cmd,"AT+ZGRUN=1\r",sizeof(Get_NEW_Location_CMD.cmd));
	memcpy(Get_NEW_Location_CMD.res,"+ZGPSR: ",sizeof(Get_NEW_Location_CMD.res));
	Get_NEW_Location_CMD.callback=GPRS_GET_CUR_LOCT;
	Get_NEW_Location_CMD.timeout=15000;//设置超时时间 15s
	Get_NEW_Location_CMD.tryCnt=1;

	while(1)
	{
	//	USART1_SendData("vMOBILE_Start_GPRS_Task!!!",28);
		vTaskDelay(50);	//延时50ms，也就是50个时钟节拍
		if(device_gprs_base_locate.DATA_STATUS_FLAG==1)   //如果服务器发来数据
		{
			xTicksToWait = (device_gprs_base_locate.CHECK_TIMES*1000) / portTICK_PERIOD_MS; /* convert milliseconds to ticks 60S */ 
			vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */	
			device_gprs_base_locate.DATA_STATUS_FLAG=0;
		}
		else 
		{
			set_default_value=1;
		}
		if(set_default_value==1)
		{
			set_default_value=0;
			memset(&device_gprs_base_locate,0,sizeof(device_gprs_base_locate));
			
			device_gprs_base_locate.SHOP_LONGTITUDE=2258.8719/100;  //默认经度
			device_gprs_base_locate.SHOP_LATITUDE=11321.6597/100;		//默认纬度
			device_gprs_base_locate.PORT_ID=1;
			device_gprs_base_locate.CHECK_TIMES=60; //位置检测时间间隔 60s
			device_gprs_base_locate.CHECK_RANGE=100;//检测位置范围 100m
			device_gprs_base_locate.WARN_FLAG=0;
		}
		
		if(xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) != pdFALSE)//如果定时器溢出
		{
		  if(0 != at_send_cmd(0,Get_NEW_Location_CMD.cmd,Get_NEW_Location_CMD.res,Get_NEW_Location_CMD.callback,Get_NEW_Location_CMD.timeout,Get_NEW_Location_CMD.tryCnt))
			{
			  //如果失败，重新初始化GPRS 
			}
			else
			{
				 lat1 = device_gprs_base_locate.SHOP_LATITUDE;
				 lng1 = device_gprs_base_locate.SHOP_LONGTITUDE;//经度1，纬度1
				 lat2 = GPRS_Location_Data.latitude/100;
				 lng2 = GPRS_Location_Data.longtitude/100;//经度2，纬度2
				
				//比较当前位置与基地址位置，计算是否超出范围，并发送给设备与服务器
				
				 double dst = get_distance(lat1, lng1, lat2, lng2);
				 COM_DBG("get_distance :%lf\r\n",dst);
				
				 if((dst*1000)>device_gprs_base_locate.CHECK_RANGE)
				 {
						device_gprs_base_locate.WARN_FLAG=1;
					  order_num[0]=device_gprs_base_locate.WARN_FLAG;
					 
					 if(0 == is_iot_works())
					 {
						 memcpy(gprs_sendbuf,(u8*)&GPRS_Location_Data.latitude,sizeof(GPRS_Location_Data.latitude));
						 memcpy(gprs_sendbuf+sizeof(GPRS_Location_Data.latitude),(u8*)&GPRS_Location_Data.longtitude,sizeof(GPRS_Location_Data.longtitude));
						 memcpy(gprs_sendbuf+sizeof(GPRS_Location_Data.latitude)+sizeof(GPRS_Location_Data.longtitude),(u8*)&device_gprs_base_locate.WARN_FLAG,sizeof(device_gprs_base_locate.WARN_FLAG));
						 
					   device_cmd_send(DEVICE_GPRS_REQUEST,device_gprs_base_locate.PORT_ID,sizeof(GPRS_Location_Data.latitude)+sizeof(GPRS_Location_Data.longtitude)+sizeof(device_gprs_base_locate.WARN_FLAG),gprs_sendbuf);
				   }
					 
					 sub_num = SerializeToOstream(sub_iot_sendbuf, "%h%h%s",3619,strlen(order_num), order_num);
					 num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s",
														1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)device_gprs_base_locate.PORT_ID,
														3,sub_num, sub_iot_sendbuf);

					 if(0 == iot_cloudpub( C2STRANSFER, QOS1, num, iot_sendbuf, 1))
					 {
						 COMDBG_INFO("\n1 ranking request: %s\r\n", order_num);
					 }
				 }
				 else
				 {
						device_gprs_base_locate.WARN_FLAG=0;
				 }
			}

			
			xTicksToWait = 90000 / portTICK_PERIOD_MS;
			vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */	
		}
	}
}



