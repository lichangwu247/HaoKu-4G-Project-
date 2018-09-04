#include "mqtt_client.h"
#include "ringbuf.h"
#include "delay.h"
#include "mobile.h"
#include "dma.h"
#include "usart2.h"
#include "MQTTFreeRTOS.h"
#include "time.h"
#include "sys.h"

#define AT_RX_BUFF_LEN    256


uint8_t AT_RxBuff[AT_RX_BUFF_LEN] = {0};
RINGBUF AT_RxRingBuff;

#define NETTYPE_MIN(x,y) (((x)<(y))?(x):(y))


char mobileIMEI[20] = {0};
char mobileSimCCID[24] = {0};
u8 mobile_csq = 0;
u8 AT_cmd_now = 0;

GPRS_Location GPRS_Location_Data;

	
extern upgrade upgrade_info;
extern RINGBUF *TCP_RxRingBuff[2];
extern void Clock_SetTime(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec, s8 zone);

/*******************************************************************************
* 函数名  : MOBILE_CTR_Config
* 描述    : LED IO配置
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : 
*******************************************************************************/
void MOBILE_CTR_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;				//定义一个GPIO_InitTypeDef类型的GPIO初始化结构体
	
	RCC_APB2PeriphClockCmd(MOBILE_RCC1, ENABLE);			//使能GPIOA的外设时钟	
	
	GPIO_InitStructure.GPIO_Pin = MOBILE_EN;				    //选择要初始化的GPIOB引脚(PA1)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置引脚工作模式为通用推挽输出 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(MOBILE_PORT1, &GPIO_InitStructure);			//调用库函数中的GPIO初始化函数，初始化GPIOB中的PB5,PB6,PB7,PB8引脚

	RCC_APB2PeriphClockCmd(MOBILE_RCC2, ENABLE);			//使能GPIOB的外设时钟	
	
	GPIO_InitStructure.GPIO_Pin = MOBILE_RST;				    //选择要初始化的GPIOB引脚(PB9)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置引脚工作模式为通用推挽输出 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(MOBILE_PORT2, &GPIO_InitStructure);			//调用库函数中的GPIO初始化函数，初始化GPIOB中的PB5,PB6,PB7,PB8引脚

  GPIO_SetBits(MOBILE_PORT2,MOBILE_RST);						//上电默认关闭	
  MOBILE_POWER(MOBILEOFF);										      //上电默认关闭
	
	RINGBUF_Init(&AT_RxRingBuff, AT_RxBuff, sizeof(AT_RxBuff));
}



/*MOBILE_RST*/
void MOBILE_RESET_ON(void)
{
	GPIO_ResetBits(MOBILE_PORT2,MOBILE_RST);
}


void MOBILE_RESET_OFF(void)
{
	GPIO_SetBits(MOBILE_PORT2,MOBILE_RST);
}


/*MOBILE_EN*/
void MOBILE_POWER(uint8_t status)
{
	if(status)
		GPIO_SetBits(MOBILE_PORT1,MOBILE_EN);
	else
		GPIO_ResetBits(MOBILE_PORT1,MOBILE_EN);
}


void COMM_Puts(uint8_t *s)
{
	USART2_PutString(s);
}


void COMM_PutCs(uint8_t *s, u16 len)
{
	u16 i;
	for (i = 0; i<len; i++)
	{
		USART2_SendByte(s[i]);
		DBG_CHR(s[i]);	//debug
	}	
}



/*
	Check AT command return value
*/
int at_wait_ack(char *pAck, int (*callback)(uint8_t* s, uint16_t cnt), int timeout_ms)
{
	int recvLen = 0, dataLen;
	
	char ack_str[AT_RX_BUFF_LEN] = {0};
	u16 i;
	u8 c;
	
	if(!pAck)
	{
		WRITE_IOT_DEBUG_LOG("ack buffer is null");
		return -1;
	}
	
	TickType_t xTicksToWait = timeout_ms / portTICK_PERIOD_MS; /* convert milliseconds to ticks */
	TimeOut_t xTimeOut;
	vTaskSetTimeOutState(&xTimeOut); /* Record the time at which this function was entered. */
	
	do
	{
		//read at buffer data
		//delay_xms(10);
		delay_ms(10);
		dataLen = RINGBUF_GetFill(&AT_RxRingBuff);
		if(dataLen)
		{
			for(i = 0; i < dataLen; i++)
			{
				RINGBUF_Get(&AT_RxRingBuff, &c);
				if(recvLen < AT_RX_BUFF_LEN)
					ack_str[recvLen++] = c;
			}
			ack_str[recvLen] = '0';
			
			/*WRITE_IOT_DEBUG_LOG("ack_str: start\r\n");*/
			
			DBG_STRING((uint8_t*)ack_str, recvLen);
		/*	WRITE_IOT_DEBUG_LOG("ack_str: end \r\n");*/

			if(strlen(ack_str) && strstr(ack_str, pAck))
			{
				if(callback != NULL)
				{
					callback((uint8_t*)ack_str, recvLen);
				}
				//COM_DBG("ack done?\r\n");
				return 0;
			}
		}
	//	DBG_STRING("RUN Here!!", 10);
	} while (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE);

	return -1;
}



/*
	send at cmd by uart and check ack value
u8 cmd:在接收到数据时，针对无识别字内容进行辨别为何命令内容
*/
int at_send_cmd(u8 cmd, u8 *pCmd, u8 *pAck, int (*callback)(uint8_t * s, uint16_t cnt), int timeOutMs, uint32_t tryCnt)
{
	if(!pAck)
	{
		WRITE_IOT_DEBUG_LOG("param error, cmd or ack value is null");
		return -1;
	}
	u8 times = 1;
	AT_cmd_now = cmd;
	
	if(tryCnt > times)
		times = tryCnt;
	
	do{
			vTaskDelay(5);	//延时5ms，也就是5个时钟节拍
			RINGBUF_Clear(&AT_RxRingBuff);
			COMM_PutCs(pCmd, strlen((char*)pCmd));
			if(0 == at_wait_ack((char*)pAck, callback, timeOutMs))
			{
				AT_cmd_now = 0;
				return 0;
			}	
	}while(--times);
	AT_cmd_now = 0;
	return -1;
}



void TCP1_Disconnected(void)
{
	client.connState = TCP_DISCONNECTED;
	client.isconnected = 0;
	COM_DBG("connState: %d\r\n", client.connState );
	COM_DBG("isconnected: %d\r\n", client.isconnected );
	COM_DBG("TCP 1 Disconnected...\r\n");
}


void TCP2_Disconnected()
{
	COM_DBG("TCP 2 Disconnected...\r\n");
}


void Hang_Up(void)
{
	//COMM_PutCs("ATH\r\n", strlen("ATH\r\n"));
	//COMM_PutCs("AT+CHUP\r\n", strlen("AT+CHUP\r\n"));
}


/*
查找字符串中第N次出现特定字符的位置
*/

int chrlookfor(const char *s, char c, int n)
{
    int flag = 0;
    int index = 0;
    if (NULL == s)
    {
        return -1;
    }
    while (*s != '\0')
    {
        if (*s == c)
        {
            ++flag;
            ++index;
            ++s;
            if (flag == n)
            {
                return index;
            }
        }
        ++s;
        ++index;
    }
    return index;
}

/*
filter the at data, to differ th tcp data and at ack
<CR><LF>+ZIPRECV: <Socket id>,<Remote IP>,<Remote port>,<Data len>,<Data><CR><LF>
提取网络数据
*/

int moblie_data_filter(void)
{
	char *phead =NULL,*pst =NULL,*pt =NULL, *pend = NULL;
	int i, rport, tcpdatalen, cshift;
	int rip[4] = {0};
	static u16 resreadlen = 0;
	static int socket;
	u16 dataLen = 0,readlen;
	u8 tcp_At_data[1200] = {0};
	u8 c;
	
	dataLen = RINGBUF_GetFill(&Mobile_RxRingBuff);
	readlen = NETTYPE_MIN(dataLen,resreadlen);
	
	if(readlen)
	{
			for(i = 0; i < readlen; i++)
			{
				RINGBUF_Get(&Mobile_RxRingBuff, &c);
				RINGBUF_Put(TCP_RxRingBuff[socket - 1], c);
			}
			resreadlen -= readlen;
			dataLen -= readlen;
	}
	
	if(dataLen == 0)
		return 0;
	
	for(i = 0; i < dataLen; i++)
	{
		RINGBUF_Get(&Mobile_RxRingBuff, &c);
		tcp_At_data[i] = c;
	}

	if(AT_cmd_now == 1 && upgrade_info.pHex != NULL)
	{
		memcpy(upgrade_info.pHex, tcp_At_data, dataLen);
		AT_cmd_now = 0;
	}
	
	pst =  (char*)tcp_At_data;
	pend = pst + dataLen;
	if((phead = strstr((char*)pst, "+ZGPSR: "))!= NULL)
	{
		goto hare;
	}
	if(((phead = strstr((char*)pst, "$GPRMC"))!= NULL)&&((phead = strstr((char*)pst, "V"))!= NULL))
	{
	//	DBG_STRING("Data_filter",11);
		return 0;
	}
	hare:
	//+ZIPSTAT: 1,0//bin，在TCP链接断开时，设置状态
	if((phead = strstr((char*)pst, "+ZIPSTAT: 1,0")) != NULL)
		TCP1_Disconnected();
	
	/*if((phead = strstr((char*)pst, "+ZIPSTAT: 2,0")) != NULL)
		TCP2_Disconnected();*/
	
	if((phead = strstr((char*)pst, "RING")) != NULL)
		Hang_Up();

	phead = strstr((char*)pst, "+ZIPRECV: ");
	
	if(phead != NULL)
	{
		do{
			
			for(i = 0; i < phead - pst; i++)
			{
				if(RINGBUF_GetFill(&AT_RxRingBuff) >= AT_RX_BUFF_LEN - 1 )
				{
					RINGBUF_Get(&AT_RxRingBuff, &c);
				}
				RINGBUF_Put(&AT_RxRingBuff, pst[i]);
			}
			
			phead += strlen("+ZIPRECV: ");
			sscanf(phead, "%d,%d.%d.%d.%d,%d,%d", &socket, rip, rip+1, rip+2, rip+3, &rport, &tcpdatalen);
			
			cshift = chrlookfor(phead, ',',4);
			
			pt = phead + cshift;
			
			dataLen -= (pt- pst);
			
			readlen = NETTYPE_MIN(dataLen,tcpdatalen);
		
			for(i = 0; i < readlen; i++)
			{
				RINGBUF_Put(TCP_RxRingBuff[socket - 1], pt[i]);
			}
			resreadlen = tcpdatalen - readlen;
			pst = pt + readlen;
		}while((phead = strstr((char*)pst, "+ZIPRECV: ")) != NULL);
		
		for(i = 0; i < pend - pst; i++)
		{
			if(RINGBUF_GetFill(&AT_RxRingBuff) >= AT_RX_BUFF_LEN - 1 )
			{
				RINGBUF_Get(&AT_RxRingBuff, &c);
			}
			RINGBUF_Put(&AT_RxRingBuff, pst[i]);
		}		
	}else{
		for(i = 0; i < dataLen; i++)
		{
			if(RINGBUF_GetFill(&AT_RxRingBuff) >= AT_RX_BUFF_LEN - 1 )
			{
				RINGBUF_Get(&AT_RxRingBuff, &c);
			}
			RINGBUF_Put(&AT_RxRingBuff, pst[i]);
		}
	}
	return 0;
}

/*
	获取模块IMEI
*/
int mobile_getIMEI(uint8_t *s, uint16_t cnt)
{
	char *phead =NULL,*pst =NULL;
	pst = (char*)s;
	u8 i = 0;
	
	phead = strstr((char*)pst, "IMEI: ");
	if(phead != NULL)
	{
		phead += strlen("IMEI: ");
	}else{
		phead = strstr((char*)pst, "ESN: ");
		if(phead != NULL)
			phead += strlen("ESN: ");
	}
	
	if(phead == NULL)
		return -1;
	
	do{
		if(*phead >= '0' && *phead <= '9')
		{
			mobileIMEI[i] = *phead;
			phead++;
			i++;
		}else{
			WRITE_IOT_DEBUG_LOG("IMEI: %s\r\n", mobileIMEI);
			break;	
		}
	}while(i< 20 && (phead - pst) < cnt);
	
	return 0;
}

//回调函数定义
//uint8_t *s,接收到的字符串指针， uint16_t cnt，接收到的数据长度
int GPRS_GET_CUR_LOCT(uint8_t *s, uint16_t cnt)
{
	char *phead =NULL,*pst =NULL;
		
		char UTC[11]={0};
		char latitude[11]={0};
		char longtitude[11]={0};
		char hdop[3]={0};
		char altitude[10]={0};
		char fix[2]={0},cog[6]={0},spkm[3]={0},spkn[3]={0};
		char Data[6]={0},nsat[2]={0};
	
	pst = (char*)s;
	phead=strstr((char *)s,"+ZGPSR: ");
	phead+=strlen("+ZGPSR: ");
	if(phead!=NULL)
	{
		sscanf((const char *)phead,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,]",UTC,latitude,longtitude,hdop,altitude,fix,cog,spkm,spkn,Data,nsat);
		memset(&GPRS_Location_Data,0,sizeof(GPRS_Location));
		
		
		GPRS_Location_Data.UTC=strtod(UTC,NULL);
		GPRS_Location_Data.latitude=strtod(latitude,NULL);
		GPRS_Location_Data.longtitude=strtod(longtitude,NULL);
		GPRS_Location_Data.hdop=strtod(hdop,NULL);
		GPRS_Location_Data.altitude=strtod(altitude,NULL);
		
/*	COM_DBG(UTC,sizeof(UTC));
		COM_DBG("\r\n");
		COM_DBG(latitude,sizeof(latitude));
		COM_DBG("\r\n");
		COM_DBG(longtitude,sizeof(longtitude));
		COM_DBG("\r\n");
		COM_DBG(hdop,sizeof(hdop));
		COM_DBG("\r\n");
		COM_DBG(altitude,sizeof(altitude));
		COM_DBG("\r\n");
		COM_DBG(fix,sizeof(fix));
		COM_DBG("\r\n");
		COM_DBG(cog,sizeof(cog));
		COM_DBG("\r\n");
		COM_DBG(spkm,sizeof(spkm));
		COM_DBG("\r\n");
		COM_DBG(spkn,sizeof(spkn));
		COM_DBG("\r\n");
		COM_DBG(Data,sizeof(Data));
		COM_DBG("\r\n");
		COM_DBG(nsat,sizeof(nsat));
		COM_DBG("\r\n");
*/
	}
	return 0;
}


/*
	获取SIM卡CCID
*/
int mobile_getCCID(uint8_t *s, uint16_t cnt)
{
	char *phead =NULL,*pst =NULL;
	pst = (char*)s;
	u8 i = 0;
	
	phead = strstr((char*)pst, "+ZGETICCID: ");
	if(phead == NULL)
		return -1;
		
	phead += strlen("+ZGETICCID: ");
	
	do{
		if(*phead >= '0' && *phead <= 'z')
		{
			mobileSimCCID[i] = *phead;
			phead++;
			i++;
		}else{
			WRITE_IOT_DEBUG_LOG("CCID: %s\r\n", mobileSimCCID);
			break;	
		}
	}while(i< 24 && (phead - pst) < cnt);
	
	return 0;
}



/*
获取网络时间，用于rtc校准
*/
int mobile_getCCLK(uint8_t *s, uint16_t cnt)
{
	char *phead =NULL,*pst =NULL;
	pst = (char*)s;
	int year = 0, month = 0, day = 0, hour = 0, Min = 0, sec = 0, zone = 0;
	
	phead = strstr((char*)pst, "+CCLK: \"");
	if(phead == NULL)
		return -1;
	
	phead += strlen("+CCLK: \"");
	                                                                  //Fu：如果4G模块发过来的时间格式包含zone,即时区
	if(6 == sscanf(phead, "%d/%d/%d,%d:%d:%d",                       //if(7 == sscanf(phead, "%d/%d/%d,%d:%d:%d%d", 
		&year, &month, &day, &hour, &Min, &sec))                      //&year, &month, &day, &hour, &Min, &sec, &zone))
	{
		WRITE_IOT_DEBUG_LOG("clock: %d/%d/%d,%d:%d:%d\r\n",         //WRITE_IOT_DEBUG_LOG("clock: %d/%d/%d,%d:%d:%d %d\r\n",
		          year, month, day, hour, Min, sec);               //year, month, day, hour, Min, sec, zone);
		year += 2000;
		Clock_SetTime(year, month, day, hour, Min, sec, 0);
	}else{
		return -1;	
	}
	
	return 0;	
}



/*
获取信号值
*/
int mobile_getCSQ(uint8_t *s, uint16_t cnt)
{
	char *phead =NULL,*pst =NULL;
	pst = (char*)s;
	int rssi = 0, ber = 0;
	
	phead = strstr((char*)pst, "+CSQ: ");
	if(phead == NULL)
		return -1;
	
	phead += strlen("+CSQ: ");
	
	if(2 == sscanf(phead, "%d,%d", &rssi, &ber))
	{
		if(ber != 0 && rssi != 99)
			mobile_csq = rssi;
		else
			mobile_csq = 0;
		WRITE_IOT_DEBUG_LOG("csq: %d\r\n", mobile_csq);
	}else{
		return -1;	
	}
	return 0;	
}


u8 mobile_get_signal(void )
{
	return mobile_csq;
}


int updata_mobile_signal(void)
{
	if(client.connState != MQTT_RUNING)
		return -1;
	
#if defined(MQTT_TASK)
		MutexLock(&client.mutex);
#endif
	
	at_send_cmd(0,"AT+CSQ\r", "+CSQ: ", mobile_getCSQ, 300, 1);

#if defined(MQTT_TASK)
		MutexUnlock(&client.mutex);
#endif
	
	return 0;
}


int updata_mobile_time(void)
{
#if defined(MQTT_TASK)
		MutexLock(&client.mutex);
#endif

	at_send_cmd(0,"AT+CCLK?\r", "+CCLK: ", mobile_getCCLK, 500, 1);

#if defined(MQTT_TASK)
		MutexUnlock(&client.mutex);
#endif
	return 0;
}


