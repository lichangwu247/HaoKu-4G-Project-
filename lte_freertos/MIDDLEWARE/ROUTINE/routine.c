#include "iot.h"
#include "device.h"
#include "routine.h"
#include "sys_list.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sys.h"


list_t *   iotInfoList;
list_t *   eqptInfoList;

int push_iotInfo_to(list_t* InfoList, int topicnum, int msglen, unsigned char* msg, list_node_t** node)
{
    if(msglen < 0 || msglen > 512)
    {
        COM_DBG("the param of len is error!");
        return -1;
    }

    //开辟内存空间
    IOT_INFO_S *pubInfo = (IOT_INFO_S*)malloc(sizeof(IOT_INFO_S)+msglen + 1);//多一位为0，方便字符串数据操作
    if(NULL == pubInfo)
    {
        COM_DBG("run aliyun_iot_memory_malloc is error!");
        return -1;
    }

    pubInfo->msglen = msglen;
		pubInfo->topicnum = topicnum;
    pubInfo->msg = (unsigned char*)pubInfo + sizeof(IOT_INFO_S);

    //复制保存消息内容
    memcpy(pubInfo->msg,msg,msglen);

    //创建保存结点
    *node = list_node_new(pubInfo);
    if(NULL == *node)
    {
        COM_DBG("run list_node_new is error!");
        return -1;
    }

    //将结点放入链表
    list_rpush(InfoList,*node);

    return 0;
}

/***********************************************************
* 名称: iot_msgInfoProc
* 描述: 处理iot消息链表

************************************************************/
int iot_msgInfoProc(list_t* InfoList)
{
    int rc = 0;

    do
    {
			  if(0 == InfoList->len)
        {
            break;
        }

        list_iterator_t *iter = list_iterator_new(InfoList, LIST_TAIL);
        list_node_t *node = NULL;
        list_node_t *tempNode = NULL;

        for (;;)
        {
            node = list_iterator_next(iter); 

            if (NULL != tempNode)
            {
							free(tempNode->val);
              list_remove(InfoList, tempNode);
              tempNode = NULL;
            }

            if (NULL == node)
            {
                //轮询结束
                break;
            }

            IOT_INFO_S *pubInfo = (IOT_INFO_S *) node->val;//将取出的节点值经过强制转换后，
            if (NULL == pubInfo)
            {
                COM_DBG("node's value is invalid!");
                tempNode = node;
                continue;
            }
						                
						iot_com_process(pubInfo->topicnum, pubInfo->msglen, pubInfo->msg);
            //状态异常则继续循环
            /*MQTTClientState state = aliyun_iot_mqtt_get_client_state(pClient);
            if(state != CLIENT_STATE_CONNECTED)
            {
                continue;
            }*/

            tempNode = node;
        }

        list_iterator_destroy(iter);

    }while(0);

    return rc;
}


/***********************************************************
* 名称: push_eqptInfo_to
* 描述: 将设备消息push进链表
*
************************************************************/
int push_eqptInfo_to(list_t* InfoList, uint8_t port, uint8_t cmd, int msglen, unsigned char* msg,
                        uint8_t isnetwork_cmd, list_node_t** node)
{
    if(msglen < 0 || msglen > 512)
    {
        COM_DBG("the param of len is error!");
        return -1;
    }
  //  DEVICE_DBG("Get malloc \n\r");
    //开辟内存空间
    EQPT_INFO_S *repubInfo = (EQPT_INFO_S*)malloc(sizeof(EQPT_INFO_S)+msglen);
    if(NULL == repubInfo)
    {
        COM_DBG("run aliyun_iot_memory_malloc is error!");
        return -1;
    }
		
		
    repubInfo->isnetwork_cmd = isnetwork_cmd;
		repubInfo->port = port;
    repubInfo->cmd = cmd;
    repubInfo->msglen = msglen;
    repubInfo->msg = (unsigned char*)repubInfo + sizeof(EQPT_INFO_S);
		
    //复制保存消息内容
    memcpy(repubInfo->msg,msg,msglen);
		
    
    //创建保存结点
    *node = list_node_new(repubInfo);
		
		COM_DBG("Node Adress: %d!\r\n",*node);
		
    if(NULL == *node)
    {
        COM_DBG("run list_node_new is error!");
        return -1;
    }
		
    //将结点放入链表
	
    list_rpush(InfoList,*node);

    return 0;
}


/***********************************************************
* 名称: eqpt_msgInfoProc
* 描述: 处理device消息链表

************************************************************/
int eqpt_msgInfoProc(list_t* InfoList)
{
    int rc = 0;

    do
    {
				if(0 == InfoList->len)
        {
            break;
        }

        list_iterator_t *iter = list_iterator_new(InfoList, LIST_TAIL);
        list_node_t *node = NULL;
        list_node_t *tempNode = NULL;

        for (;;)
        {
            node = list_iterator_next(iter);

            if (NULL != tempNode)
            {
							free(tempNode->val);
              list_remove(InfoList, tempNode);
              tempNode = NULL;
            }

            if (NULL == node)
            {
                //轮询结束
                break;
            }

            EQPT_INFO_S *subInfo = (EQPT_INFO_S *) node->val;
            if (NULL == subInfo)
            {
                COM_DBG("node's value is invalid!");
                tempNode = node;
                continue;
            }
						
						//状态异常则继续循环
            /*MQTTClientState state = aliyun_iot_mqtt_get_client_state(pClient);
            if(state != CLIENT_STATE_CONNECTED)
            {
                continue;
            }*/
						device_com_process(subInfo->port, subInfo->cmd, subInfo->msglen, subInfo->msg, subInfo->isnetwork_cmd);

            tempNode = node;
        }

        list_iterator_destroy(iter);

    }while(0);

    return rc;
}


