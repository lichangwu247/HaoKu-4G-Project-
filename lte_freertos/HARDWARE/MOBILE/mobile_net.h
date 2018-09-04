#ifndef MOBILE_NET_H
#define MOBILE_NET_H
#include "sys.h" 
#include "MQTTFreeRTOS.h"
/**
 * @brief The structure of MQTT network connection used in the MQTT library. The user has to allocate memory for this structure.
 */

int mobile_net_connect(Network* n, unsigned char type, char *addr, int port);
int mobile_net_read(Network* n, unsigned char *pRecvBuffer, int recvBufferlen, int timeOutMs);
int mobile_net_write(Network* n, unsigned char* pSendBuffer, int sendBufferlen, int timeout_ms);
int mobile_net_disconnect(Network* n);

#endif
