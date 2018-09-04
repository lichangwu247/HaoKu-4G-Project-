/******************************************************************************

******************************************************************************/
#ifndef __ROUTINE_H__
#define __ROUTINE_H__

#include <stdint.h>
#include "sys_list.h"

extern list_t *   iotInfoList;
extern list_t *  eqptInfoList;


typedef struct SUB_INFO
{
	int               msglen;
	int               topicnum;
	unsigned char*    msg;
}IOT_INFO_S;


typedef struct REPUBLISH_INFO
{
	uint8_t           isnetwork_cmd;
	uint8_t           port;
	uint8_t 	        cmd;
	int               msglen;
	unsigned char*    msg;
}EQPT_INFO_S;



/**
*0x00-0x0F:firmware cmd 
*0x10-0xFF:device cmd
 *
**/

int iot_msgInfoProc(list_t* InfoList);

int push_iotInfo_to(list_t* InfoList, int topicnum, int msglen, unsigned char* msg, list_node_t** node);

int eqpt_msgInfoProc(list_t* InfoList);

int push_eqptInfo_to(list_t* InfoList, uint8_t port, uint8_t cmd, int msglen, unsigned char* msg,
                        uint8_t isnetwork_cmd, list_node_t** node);


#endif


