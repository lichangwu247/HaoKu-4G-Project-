#include "mobile_net.h"
#include "mobile.h"
#include "usart2.h"
#include "ringbuf.h"
#include "delay.h"
#include "MQTTFreeRTOS.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include <string.h>
#include <stdio.h>

RINGBUF TCP0_RxRingBuff;
RINGBUF TCP1_RxRingBuff;

uint8_t TCP_RxBuff[2][1536] = {0};
RINGBUF *TCP_RxRingBuff[2] = {&TCP0_RxRingBuff, &TCP1_RxRingBuff};



#define NETTYPE_MIN(x,y) (((x)<(y))?(x):(y))

// tcp connect
// AT+ZIPOPEN=<Socket id>,<Protocol Type>,<Remote Address>,<Remote port>
#define AT_TCP_SOCKET_CONNECT_FMT "AT+ZIPOPEN=%d,%d,%s,%d\r\n"

// tcp fast close
// AT+ZIPCLOSE=<Socket id>
#define AT_TCP_SOCKET_CLOSE_FMT "AT+ZIPCLOSE=%d\r\n"

// tcp send data 
// AT+ZIPSENDRAW=<socket_ID>,<len>
#define AT_TCP_SOCKET_SEND_DATA_FMT "AT+ZIPSENDRAW=%d,%d\r\n"




/*
connect to server
*/
int mobile_net_connect(Network* n, unsigned char type, char *addr, int port)
{
	char cmd[100] = {0};
	char res[20] = {0};
	
	if(NULL == addr)
	{
		return -1;
	}
																		//socket:1 ,type:0 tcp,addr: TBuPGFTrzHn.iot-as-mqtt.cn-shanghai.aliyuncs.com,port:1883
	snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_CONNECT_FMT, n->my_socket, type, addr, port);
	snprintf(res, sizeof(res), "+ZIPSTAT: %d,1", n->my_socket);
	
	if(0 == at_send_cmd(0,(u8*)cmd, (u8*)res, NULL, 10000, 1))
	{
		RINGBUF_Init(TCP_RxRingBuff[n->my_socket-1], TCP_RxBuff[n->my_socket-1], sizeof(TCP_RxBuff[n->my_socket-1]));
		return 0;
	}

	return -1;
}




/*
read the data tcp/udp
*/
int mobile_net_read(Network* n, unsigned char *pRecvBuffer, int recvBufferlen, int timeOutMs)
{
	if(!pRecvBuffer)
	{
		COM_DBG("param error, recvbuffer is null\r\n");
		return -1;
	}
	
	int i =0;
	int readLen = 0;
	int dataLen = 0;
	
	/*Timer tMobileAckTimer;
	
	TimerInit(&tMobileAckTimer);
	TimerCountdownMS(&tMobileAckTimer, timeOutMs);

	do{*/
		dataLen = RINGBUF_GetFill(TCP_RxRingBuff[n->my_socket - 1]);
		
		if(dataLen > 0)
		{
			//COM_DBG("TCP Ring len: %d", dataLen);
			// read mqtt data from ringbuffer
			readLen = NETTYPE_MIN(recvBufferlen,dataLen);
			//COM_DBG("datLen: %d\r\n", dataLen);
			for(i = 0; i< readLen; i++)
			{
				RINGBUF_Get(TCP_RxRingBuff[n->my_socket -1], pRecvBuffer + i);
			}
			return readLen;
		}
	//}while(!TimerIsExpired(&tMobileAckTimer));

	return 0;
}


/*
write the data tcp/udp
*/
//int mobile_net_write(unsigned char socket, unsigned char *pSendBuffer, int sendBufferlen, int timeOutMs)
int mobile_net_write(Network* n, unsigned char* pSendBuffer, int sendBufferlen, int timeout_ms)
{
	char cmd[128] = {0};
	char res[20] = {0};
	
	if(!pSendBuffer)
	{
		COM_DBG("param error, sendbuffer is null\r\n");
		return -1;
	}
	
	if(sendBufferlen >1024 || sendBufferlen <= 0)
	{
		COM_DBG("param error, sendBufferlen is wrong\r\n");
		return -1;
	}
	
	//send tcp data at cmd
	snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_SEND_DATA_FMT, n->my_socket, sendBufferlen);	

	COMM_Puts((uint8_t*)cmd);

	/*if(0 != at_send_cmd(0,(u8*)cmd, "OK", NULL, 300, 1))
		return -1;*/
	delay_xms(100);

	COMM_PutCs(pSendBuffer, sendBufferlen);

	snprintf(res, sizeof(res), "+ZIPSENDRAW: %d,%d", n->my_socket, sendBufferlen);	

	if(at_wait_ack(res, NULL, timeout_ms - 100) != 0)
		return -1;

	return sendBufferlen;
}


/*
disconnect the data tcp/udp
*/
int mobile_net_disconnect(Network* n)
{
	char cmd[32] = {0};
	
	snprintf(cmd, sizeof(cmd), AT_TCP_SOCKET_CLOSE_FMT, n->my_socket);
  if(0 == at_send_cmd(0,(u8*)cmd, (u8*)"OK", NULL, 1000, 1))
	{
		RINGBUF_Clear(TCP_RxRingBuff[n->my_socket-1]);
		return 0;
	}

	return -1;
}

