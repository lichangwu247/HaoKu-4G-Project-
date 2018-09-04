#include "prize.h"
#include "routine.h"
#include "device.h"
#include "selector.h"
#include "prize.h"
#include "iot.h"
#include "MQTTClient.h"
#include "protobuf.h"
#include "mobile.h"
#include "info_storage.h"
#include "delay.h"
#include "rtc.h"
#include "sys.h"
#include "encoding.h"

DEVICE_INFO device;



int device_cmd_send(u8 cmd, u8 daddr, u8 msglen, u8* msg_content)
{
	u8 i, sum = 0, len = 0;
	u8 send_cmd_buff[256] = {0};

	send_cmd_buff[0] = 0x68;//帧头
	send_cmd_buff[1] = daddr;//目标地址
	send_cmd_buff[2] = cmd;//指令码
	send_cmd_buff[3] = msglen;//数据长度 暂定最大118
	
	if(msglen != 0)
	{
		for(i = 0; i < msglen; i++)
		{
			send_cmd_buff[4 + i] = msg_content[i];
		}
	}

	len = 4 +  msglen;
	
	for(i = 1; i < len; i++)
	{
		sum += send_cmd_buff[i];
	}

	send_cmd_buff[len] = ~sum + 1;//校验和
	len++;
	send_cmd_buff[len] = 0x16;//帧尾
	len++;

	/*************************************/

	DEVICE_SendCmd_Port(send_cmd_buff, len);

	/*************************************/	

	COM_DBG("Device S: ");
	DBG_HEX(send_cmd_buff, len);//debug
	return  0;
}


/*************************************************************
 *  名称：	device_cmd_and_wait_ack()
 *  功能：  依据协议封装消息体发送到机器串口，待返回消息后验证
            是否正确并将消息体，命令等存于链表eqptInfoList
 *	输入：  cmd-命令字
 *          saddr-源地址
 *          saddr-目标地址
 *          msglen-消息长度（消息体）
 *          msg_content-消息内容
 *          service_id-业务id 网络投币及prize传下来的id
 *          isnetworkcmd-是否为网络命令
 *          timeout-命令返回超时
 *	输出：  FALSE-超时  TRUE-执行完成
 *************************************************************/
int device_cmd_and_wait_ack(u8 cmd, u8 daddr, u8 msglen,	
          u8* msg_content, u8 isnetwork_cmd,	u8 timeout)
{
	if(timeout == 0) 
		return -1;
	
	device_cmd_send(cmd, daddr, msglen, msg_content);
	
	return  0;
}



/*
filter the device data
*/

int device_data_filter(u8* msg, u16 len)
{
  u8 i, sum;
	u16 reslen = len;
  uint8_t 	cmd,port;
	u8* protocol = msg;
	u8 length;//协议内容长度
	
	COM_DBG("Device R: ");
	DBG_STRING(msg, len);
	
	do{
		loop:
		i = 0;
		while(0x68 != protocol[i])
		{ 
			if(reslen>200) goto exit;
			if(i < reslen)
				i++;
			else
				goto exit;
		}
		
		DEVICE_DBG("68?: %02x\r\n", protocol[i]);
		protocol += i;
		reslen -= i;
		
		DEVICE_DBG("reslen: %d\r\n", reslen);
		
		port = protocol[1];
		cmd = protocol[2];
		length = protocol[3];
		DEVICE_DBG("port: %d\r\n", port);
		DEVICE_DBG("cmd: %d\r\n", cmd);
		DEVICE_DBG("length: %d\r\n", length);
		
		if(reslen < (length + 6))//长度不够，重新查找帧头
		{
			i = 0;
			protocol +=1;
			reslen -= 1;
			goto loop;
		}
		sum = 0;
		for(i = 1; i < (4 + length); i++)//协议内容检验
		{
			sum += protocol[i];
		}
		sum = ~sum + 1;//校验和		
		DEVICE_DBG("sum: %02x",sum);
		if((sum == protocol[4 + length]) && (0x16 == protocol[5 + length]))
		{
			DBG_HEX(protocol,length+6);
			//接收到的响应内容，放入接收链表，在main中循环处理。
			list_node_t *node = NULL;
			
			push_eqptInfo_to(eqptInfoList, port, cmd, length, (protocol + 4), 1, &node);
			
			protocol += (length + 6);
			reslen -= (length + 6);
			
		
		}else{
			protocol +=1;
			reslen -= 1;//帧不完整，重新找新的帧头
		}
	}while(reslen >= 6);//数据长度不满足最小帧长，退出
	exit:	
	return 0;
}




//主机上分命令
int device_insert_coin(u8 port, char *order_num, u8 oder_num_len, u16 coin)
{
	u8 eqt_buff[64]={0};
	u8 len;
	memcpy(eqt_buff, order_num, oder_num_len);
	memcpy(eqt_buff  + oder_num_len, &coin, sizeof(u16));
	len = oder_num_len + sizeof(coin);
	return device_cmd_and_wait_ack(DEVICE_COIN, port, len, eqt_buff, 1, 3);
}


/*************************************************************
 *  名称：	device_params_set()
 *  功能：  串口设备 参数网络设置
 *	输入：  param_type-参数类型
 *          param_value-参数值
 *          isnetworkcmd-是否为网络命令
 *          超时 1s
 *	输出：  FALSE-超时  TRUE-执行完成
 *************************************************************/
int device_params_set(u8 port, u8 msglen,	u8* msg_content, u8 isnetwork_cmd)
{
	return device_cmd_and_wait_ack(DEVICE_PARAM_SET, port, msglen, msg_content, isnetwork_cmd,3 );
}


int device_qrcode_set(u8 port, u8 msglen, u8* msg_content, u8 isnetwork_cmd)
{
	return device_cmd_and_wait_ack(DEVICE_QRCODE_SET, port, msglen, msg_content, isnetwork_cmd,2 );
}


int device_params_get(u8 port, u8 msglen,	u8* msg_content, u8 isnetwork_cmd)
{
	return device_cmd_and_wait_ack(DEVICE_PARAM_GET, port, msglen, msg_content, isnetwork_cmd,2 );
}



int device_account_get(u8 port, u8 msglen,	u8* msg_content, u8 isnetwork_cmd)
{
	return device_cmd_and_wait_ack(DEVICE_ACCOUNT_GET, port, msglen, msg_content, isnetwork_cmd,2 );
}




void device_request_signal(void)
{
	u8 buff[2];
	buff[0] = mobile_get_signal();
	if(is_iot_works())
	{
		buff[1] = 0;
	}else{
		buff[1] = 1;
	}
	
	device_cmd_send(DEVICE_REQUEST_SIGNAL, 1, sizeof(buff), buff);
}

/*********************************************************************/

extern char mobileIMEI[20];
extern char mobileSimCCID[24];


int firmware_first_pub(void)
{
	u16 num;
	unsigned char iot_sendbuf[128];
	num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%s%h%h%s%h%h%i", 
	          1,8,(uint64_t)Unix_Get(), 4,4,(uint32_t)VERSION_NUM, 
	          5,4,(uint32_t)mobile_get_signal(), 
						6,strlen(mobileSimCCID),mobileSimCCID,7,strlen(mobileIMEI),mobileIMEI,
						9,sizeof(uint32_t),(uint32_t)0);
										//MQTT 发送数据  （topic：话题 ,QOS1：交付质量等级,数据长度，内容，）
	return iot_cloudpub(C2SFIRMWARE_ONLINE, QOS1, num, iot_sendbuf, 3);
}


int firmware_signal(void)
{
	u16 num;
	unsigned char iot_sendbuf[128];
	num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i", 
							1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)mobile_get_signal());
	
	return iot_cloudpub(C2SGPRS_RSSI, QOS1, num, iot_sendbuf, 1);
}


int device_status_report(u8 status_code)
{
	u16 num = 0;
	u8 iot_sendbuf[64];
	
	num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i",
			      1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)1,3,sizeof(uint32_t), (uint32_t)status_code);
	
	return iot_cloudpub(C2SDEV_STATUS, QOS1, num, iot_sendbuf, 1);
}


int device_comm_timeout(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{	
	u8 cmd_timeout = 0;
	cmd_timeout = *msg; //超时的命令
	uint32_t device_cmd_unsuccess = 0;//超时命令
	
	if(len > 4)
		return -1;

	switch(cmd_timeout)
	{
		case DEVICE_CHECK:
		  if(106 != device.status)
			{
				device.status = 106;
				if(0 != device_status_report(device.status))
				{
					device.status = 101;
				}
			}
			memset(device.pn_sn,0x0,sizeof(device.pn_sn));
		break;

		case DEVICE_RESET:
			device_cmd_unsuccess = DEVICE_RESET;
		break;
		
		case DEVICE_COIN:
			device_cmd_unsuccess = DEVICE_COIN;
		  memset(coin_service.port[port - 1].userIdnew, 0x0, 12);
		  memset(coin_service.port[port - 1].orderNew, 0x0, 23);
		  coin_service.port[port - 1].isForce = 0;
		break;
		
		case DEVICE_ACCOUNT_GET:
			device_cmd_unsuccess = DEVICE_ACCOUNT_GET;			
		break;
		
		case DEVICE_PARAM_GET:
			device_cmd_unsuccess = DEVICE_PARAM_GET;
		break;
		
		case DEVICE_PARAM_SET:
			device_cmd_unsuccess = DEVICE_PARAM_SET;
		break;
		
		case DEVICE_FRONT_ACCOUNT_CLR:
			device_cmd_unsuccess = DEVICE_FRONT_ACCOUNT_CLR;
		break;
		
		case DEVICE_BACK_ACCOUNT_CLR:
			device_cmd_unsuccess = DEVICE_BACK_ACCOUNT_CLR;
		break;
		
		case DEVICE_INCREMENT_GET:
			device_cmd_unsuccess = DEVICE_INCREMENT_GET;
		break;
		
		case DEVICE_RESTORE:
			device_cmd_unsuccess = DEVICE_RESTORE;
		break;
		
		case DEVICE_ACCOUNT_CLR:
			device_cmd_unsuccess = DEVICE_ACCOUNT_CLR;
		break;
		
		case DEVICE_QRCODE_SET:
			device_cmd_unsuccess = DEVICE_QRCODE_SET;
		break;
	  case DEVICE_INCREMENT_REPORT:
			device_cmd_unsuccess = DEVICE_INCREMENT_REPORT;
		break;
		
		case DEVICE_REQUEST_SIGNAL:
			device_cmd_unsuccess = DEVICE_REQUEST_SIGNAL;
		break;
		
		case DEVICE_SLEEP:
			device_cmd_unsuccess = DEVICE_SLEEP;
		break;
		
		case DEVICE_WAKE:
			device_cmd_unsuccess = DEVICE_WAKE;
		break;
		
    case DEVICE_PRIZE_CLR:
			device_cmd_unsuccess = DEVICE_PRIZE_CLR;
		break;
		
		case DEVICE_ACCOUNT_SET:
			device_cmd_unsuccess = DEVICE_ACCOUNT_SET;	
		break;
		
		case DEVICE_PRIZE_REPORT:
			device_cmd_unsuccess = DEVICE_PRIZE_REPORT;		
		break;
		
		case DEVICE_SET_STATUS:
			device_cmd_unsuccess = DEVICE_SET_STATUS;	
		break;
		
		case DEVICE_PRIZE_QRCODE:
			device_cmd_unsuccess = DEVICE_PRIZE_QRCODE;	
		break;
		
		default:
		break;
	}

	if((device_cmd_unsuccess != 0) && (isnetwork_cmd ==1))
	{
		return 	c2s_s2c_status(port, device_cmd_unsuccess, 3);
	}
	return -1;
}


//主机查询链接指令响应//向topic c2s/device_status传
int device_check_response(u8* msg, u16 len, u8 isnetwork_cmd)
{
	u16 num = 0;
	u8 iot_sendbuf[128];
	char pn_sn[65] = {0};
	
	if(len > DEVICE_INFO_LEN)
		return -1;
	memcpy(pn_sn, msg, len);
	
		if(0 != strncmp(device.pn_sn, (char*)msg, len))
		{
			memset(device.pn_sn, 0x0, DEVICE_INFO_LEN + 1);
			memcpy(device.pn_sn, msg, len);
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s",
								1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)1, 5,len, device.pn_sn);

			COM_DBG("\ndevice info: %s\n", device.pn_sn);
			if(0 != iot_cloudpub(C2SDEV_STATUS, QOS1, num, iot_sendbuf, 1))
			{
					memset(device.pn_sn, 0x0, DEVICE_INFO_LEN + 1);
			}
		}
	return 0;	
}


//主机重启设备指令响应
int device_reset_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg; 
	if(isnetwork_cmd ==1)
	{
		return c2s_s2c_status(port, DEVICE_RESET, result);
	}
	return -1;
}


//主机上分命令确认
int insert_coin_confirm(u8 port, char *order_num, u8 oder_num_len, u8 confirm)
{
	u8 eqt_buff[64]={0};
	u8 len;
	u16 coin = 0;
	memcpy(eqt_buff, order_num, oder_num_len);
	memcpy(eqt_buff  + oder_num_len, &coin, sizeof(u16));
	len = oder_num_len + sizeof(coin);
  memcpy(eqt_buff  + len, &confirm, sizeof(u8));
	len +=  sizeof(confirm);
	return device_cmd_send(DEVICE_COIN, port, len, eqt_buff);
}



//机台上分失败命令确认
int fail_coin_confirm(u8 port, char *order_num, u8 oder_num_len, u16 confirm)
{
	u8 eqt_buff[64]={0};
	u8 len;
	u16 direction = 0;
	memcpy(eqt_buff, order_num, oder_num_len);
	memcpy(eqt_buff  + oder_num_len, &direction, sizeof(u16));
	len = oder_num_len + sizeof(direction);
  memcpy(eqt_buff  + len, &confirm, sizeof(u16));
	len +=  sizeof(confirm);
	return device_cmd_send(DEVICE_FAILCOIN_GET, port, len, eqt_buff);
}



//主机向设备上分指令响应
int device_coin_set_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	//static u8 times = 0;
	u8 code = 0,status = 0;
	u16 num = 0, direction = 0;
	u32 timeout = 0;
	u8 iot_sendbuf[128];
	char order_num[23] = {0};
	record rec;
	u8 i;
	
	if(len < 23)
		return -1;
	
	memcpy(order_num, msg, 22);
	memcpy((u8*)&code, (msg + 22), sizeof(code));
		
	/*if(times < 3){//test
			code = 1;
		}else if(times >= 3){
		    code = 2;
		}
		
		times++;*/
	
	COM_DBG("\ncode: %d\r\n", code);
	if(len == 27)
	{
		memcpy(&timeout, (msg + 23 ), 4);
		COM_DBG("\ntimeoutd: %d\r\n", timeout);
		num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i%h%h%i",
			       1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3,strlen(order_num), order_num, 
		         4,4,(uint32_t)0, 5,4,(uint32_t)mobile_get_signal(),6,4,(uint32_t)timeout,7,4,(uint32_t)1003);
		return iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1);
	}
	
	if(len == 25)
	{
		memcpy(&direction, (msg + 22), sizeof(direction));
		memcpy(&status, (msg + 24), sizeof(status));
		if(direction == 0x00)
				return -1;
			
		if(status == 0)
			status = 2;
		num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i",
					  1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3,strlen(order_num), order_num, 
					  7,4,(uint32_t)status);
		return iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1);
	}
	
	if(code == 0)
	{
		num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
			       1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
             3,strlen(order_num), order_num, 4,4,(uint32_t)0, 5,4,(uint32_t)mobile_get_signal(),
		         7,4, (uint32_t)1004);
		coin_service.port[port - 1].isForce = 0;
    return iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1);		
	}else if(code == 1){
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
			       1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
             3,strlen(order_num), order_num, 4,4,(uint32_t)1, 5,4,(uint32_t)mobile_get_signal(),
		         7,4, (uint32_t)0);
			if(0 == iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1))
			{
				memset(coin_service.port[port - 1].userIdLast, 0x0,12);
				memset(coin_service.port[port - 1].orderLast, 0x0,23);
				memcpy(coin_service.port[port - 1].userIdLast, coin_service.port[port - 1].userIdnew,12);
				memcpy(coin_service.port[port - 1].orderLast, coin_service.port[port - 1].orderNew,23);
				coin_service.port[port - 1].isForce = 0;
				insert_coin_confirm(port, order_num, strlen(order_num), 1);
				COM_DBG("\n1 order success: %s\n", order_num);
			}else{
				coin_service.port[port - 1].isForce = 0;
				insert_coin_confirm(port, order_num, strlen(order_num), 0);
				
				if(0 != s2c_insert_response(port, order_num, strlen(order_num), 2,1))
				{
					memset((u8*)&rec, 0x0, sizeof(record));
					rec.num = 2;
					memcpy(rec.orderNum, order_num, strlen(order_num));
					rec.seq = 0;
					rec.timestamp = Unix_Get();
					rec.type = 0;//上币
					for(i = 0; i< (sizeof(record) - 2);i++)
					{
						rec.sum +=  *((u8*)&rec + i); 
					}
					record_write(&rec);
			   }
				COM_DBG("\n1 order fail: %s\n", order_num);
			}		
	}else if(code == 2){
		if(coin_service.port[port - 1].isForce == 0){
			if(0 == strcmp(coin_service.port[port - 1].userIdnew, coin_service.port[port - 1].userIdLast))
			{
				if(coin_service.port[port - 1].isPlay == 0)
				{
					insert_coin_confirm(port, order_num, strlen(order_num), 0);
					num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
									 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
									 3,strlen(order_num), order_num, 4,4,(uint32_t)0, 5,4,(uint32_t)mobile_get_signal(),
									 7,4, (uint32_t)1005);
					return 	iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1);				
				}
				
					num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
								 3,strlen(order_num), order_num, 4,4,(uint32_t)1, 5,4,(uint32_t)mobile_get_signal(),
								 7,4, (uint32_t)0);
					if(0 == iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1))
					{
						insert_coin_confirm(port, order_num, strlen(order_num), 1);
						COM_DBG("\n2 order success: %s\n", order_num);
					}else{
						insert_coin_confirm(port, order_num, strlen(order_num), 0);
						
						if(0 != s2c_insert_response(port, order_num, strlen(order_num), 2,1))
						{
							memset((u8*)&rec, 0x0, sizeof(record));
							rec.num = 2;
							memcpy(rec.orderNum, order_num, strlen(order_num));
							rec.seq = 0;
							rec.timestamp = Unix_Get();
							rec.type = 0;//上币
							for(i = 0; i< (sizeof(record) - 2);i++)
							{
								rec.sum +=  *((u8*)&rec + i); 
							}
							record_write(&rec);
						}
						
						COM_DBG("\n2 order fail: %s\n", order_num);

					}		
			}else{
				num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
								 3,strlen(order_num), order_num, 4,4,(uint32_t)0, 5,4,(uint32_t)mobile_get_signal(),
								 7,4, (uint32_t)1002);
				iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1);
				insert_coin_confirm(port, order_num, strlen(order_num), 0);
			}
		}else if(coin_service.port[port - 1].isForce == 1){
			
			if(coin_service.port[port - 1].isPlay == 0)
			{
				insert_coin_confirm(port, order_num, strlen(order_num), 0);
				
				num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
									 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
									 3,strlen(order_num), order_num, 4,4,(uint32_t)0, 5,4,(uint32_t)mobile_get_signal(),
									 7,4, (uint32_t)1005);				
				return 	iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1);				
			}
			
			
				num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
								 3,strlen(order_num), order_num, 4,4,(uint32_t)1, 5,4,(uint32_t)mobile_get_signal(),
								 7,4, (uint32_t)0);
				if(0 == iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1))
				{
					coin_service.port[port - 1].isForce = 0;
					insert_coin_confirm(port, order_num, strlen(order_num), 1);
					COM_DBG("\n21 order success: %s\n", order_num);
				}else{
					coin_service.port[port - 1].isForce = 0;
					insert_coin_confirm(port, order_num, strlen(order_num), 0);
					
					if(0 != s2c_insert_response(port, order_num, strlen(order_num), 2,1))
					{
						memset((u8*)&rec, 0x0, sizeof(record));
						rec.num = 2;
						memcpy(rec.orderNum, order_num, strlen(order_num));
						rec.seq = 0;
						rec.timestamp = Unix_Get();
						rec.type = 0;//上币
						for(i = 0; i< (sizeof(record) - 2);i++)
						{
							rec.sum +=  *((u8*)&rec + i); 
						}
						record_write(&rec);
					}
					COM_DBG("\n21 order fail: %s\n", order_num);
				}
		}
	}
	return 0;
}


//主机获取设备账目信息指令响应
int device_account_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	//透传
	u8 num = 0;
	u8 iot_sendbuf[280];
	static u8 last_accounts[256] = {0};
	
	if(len > 256)
		return -1;

		if((0 != memcmp(last_accounts, &msg,len))|| (isnetwork_cmd == 1))
		{
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s",
									 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3, len, msg);
			if(0 != iot_cloudpub(C2SDEV_ACCOUNTS, QOS1, num, iot_sendbuf, 1))
			{
				memset(last_accounts, 0x0, 256);
			}else{
				memcpy(last_accounts, msg, len);;
			}
		}

	return 0;	
}

//主机查询设备参数指令响应
int device_param_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	//透传
	u8 num = 0;
	u8 iot_sendbuf[280];

	static u8 last_param[256];
	
	if(len > 256)
		return -1;
	
		if((0 != memcmp(last_param, msg,len))|| (isnetwork_cmd == 1))
		{
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s",
									 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3, len, msg);
			if(0 != iot_cloudpub(C2SDEV_SETTING, QOS1, num, iot_sendbuf, 1))
			{
				memset(last_param, 0x0, 256);
			}else{
				memcpy(last_param, msg, len);
			}
		}

	return 0;	
}
		 
//主机设置设备参数指令响应
int device_param_set_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_PARAM_SET, result);
	}
	return 0;	
}

		 
//主机清除设备面板账目指令响应
int device_front_account_clr_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_FRONT_ACCOUNT_CLR, result);
	}
	return -1;	
}

//主机清除设备后台账目指令响应
int device_back_account_clr_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_BACK_ACCOUNT_CLR, result);
	}
	return -1;	
}
						 
//主机查询设备增量指令响应
int device_increment_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_INCREMENT_GET, result);
	}
	return -1;	
}

								 
//主机恢复设备出厂设置指令响应
int device_restore_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_RESTORE, result);
	}
	return -1;		
}

//主机清除设备本地账目指令响应
int device_account_clr_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_ACCOUNT_CLR, result);
	}
	return -1;			
}

//主机发送二维码信息至设备指令响应
int device_qrcode_set_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_QRCODE_SET, result);
	}
	return -1;	
}


int device_prizeqrcode_set_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_PRIZE_QRCODE, result);
	}
	return -1;	
}


//设备主动上报增量（游戏币和出礼品等）
int device_increment_report(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	return 0;	
}
		 
//主机设置设备待机状态指令响应
int device_sleep_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_SLEEP, result);
	}
	return -1;	
}

//主机唤醒设备指令响应
int device_wake_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_WAKE, result);
	}
	return -1;	
}

//主机清设备奖池指令响应
int device_prize_clr_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_PRIZE_CLR, result);
	}
	return -1;	
}


//主机设置设备本地账目指令响应
int device_account_set_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 result = 0;
	result = *msg;

	if(isnetwork_cmd == 1)
	{
		return c2s_s2c_status(port, DEVICE_ACCOUNT_SET, result);
	}
	return -1;		
}



int device_prize_qrcode_send(u8 port, u8 type, u32 vaule, char* orderId)
{
	char qrcode[220];
	u8 qrcode_len = 0;
	if(orderId == 0)
	{
		if(0==  prize_qrcode2_gen(qrcode, &qrcode_len, MD5keyId, MD5key, prizeUrl,type, vaule, port))
		{
			device_cmd_send(DEVICE_PRIZE_QRCODE, port, qrcode_len, (u8*)qrcode);
		}
		return 0;
	}else if(strlen(orderId) == 22){
		if(0==  prize_qrcode1_gen(qrcode, &qrcode_len, MD5keyId, MD5key, prizeUrl,type, vaule, orderId))
	  {
			device_cmd_send(DEVICE_PRIZE_QRCODE, port, qrcode_len, (u8*)qrcode);
		}
		return 0;	
	}
	return 0;	
}


int device_prize_qrcode3_send(u8 port, u8 type, u32 vaule, char* orderId, u32 seq)
{
  char qrcode[220];
	u8 qrcode_len = 0;
	if(0 ==	prize_qrcode3_gen(qrcode, &qrcode_len, MD5keyId, MD5key, prizeUrl,type, vaule, orderId, seq))
	{
		device_cmd_send(DEVICE_PRIZE_QRCODE, port, qrcode_len, (u8*)qrcode);
	}
	return 0;	
}


//设备中奖出奖响应
int device_prize_report(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u16 type = 0,prize_type2_len=0,type2_data=0,type2=0,prize_buff[3]={0};
	u32 point = 0, seq = 0;
	u16 num = 0;
	u8 iot_sendbuf[128];
	u8 eqt_sendbuf[64];
	char order_num[23] = {0};
	u8 status = 0;
	
	if(len > 36)
		return -1;
	if(26 == len)
	{
			memcpy(order_num, msg, 22);
			memcpy((u8*)&point, (msg + 22), sizeof(point));
			
			COM_DBG("point: %d\n", point);
			
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3,strlen(order_num), order_num,
								 4,4,(uint32_t)53,5,4,point);	
  		
				 if(0 == iot_cloudpub(C2SDEV_PRIZE, QOS1, num, iot_sendbuf, 1))
				 { 
					 COM_DBG("prize upload sucess: %d\r\n",Unix_Get());
					 status = 1;
					 memcpy(eqt_sendbuf, order_num, strlen(order_num));
					 memcpy(eqt_sendbuf + strlen(order_num), &status, sizeof(status));
					 device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(status)),eqt_sendbuf);
					 return 0;
				 }else{
					 COM_DBG("prize upload fail: %d\r\n",Unix_Get());
					 status = 0;
					 memcpy(eqt_sendbuf, order_num, strlen(order_num));
					 memcpy(eqt_sendbuf + strlen(order_num), &status, sizeof(status));
					 device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(status)),eqt_sendbuf);
					 return -1;
				 }		 

	}else if(28 == len){
			memcpy(order_num, msg, 22);
			memcpy((u8*)&type, (msg + 22), sizeof(type));
			memcpy((u8*)&point, (msg + 24), sizeof(point));
		
		  COM_DBG("type: %d\n", type);
			COM_DBG("point: %d\n", point);
		  if(1 == type)
				type = 53;
			else if(2 == type)
				type =  52;
			else if(3 == type)
				type =  1;
			else if(4 == type)
				type =  51;
			
			if((strcmp(coin_service.OrderIdForCmp, order_num) > 0)
				|| (strlen(coin_service.port[port - 1].orderLast) == 0))
			{
				status = 0;
				memcpy(eqt_sendbuf, order_num, 22);
				memcpy(eqt_sendbuf + 22, &status, sizeof(status));
				device_cmd_send(DEVICE_PRIZE_REPORT, port, (22 + sizeof(status)),eqt_sendbuf);
				//delay_ms(PRIZE_DELAY_TIME);
				device_prize_qrcode_send(port, type, point, NULL);
				return 0;
			}
			
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
			           3,strlen(coin_service.port[port - 1].orderLast), coin_service.port[port - 1].orderLast,
								 4,4,(uint32_t)type,5,4,point);

				 if(0 == iot_cloudpub(C2SDEV_PRIZE, QOS1, num, iot_sendbuf, 1))
				 { 
					 COM_DBG("prize upload sucess: %d\r\n",Unix_Get());
					 status = 1;
					 memcpy(eqt_sendbuf, order_num, strlen(order_num));
					 memcpy(eqt_sendbuf + strlen(order_num), &status, sizeof(status));
					 device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(status)),eqt_sendbuf);
					 return 0;
				 }else{
					 COM_DBG("prize upload fail: %d\r\n",Unix_Get());
					 status = 0;
					 memcpy(eqt_sendbuf, order_num, strlen(order_num));
					 memcpy(eqt_sendbuf + strlen(order_num), &status, sizeof(status));
					 device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(status)),eqt_sendbuf);
					 //delay_ms(PRIZE_DELAY_TIME);
					 device_prize_qrcode_send(port, type, point, order_num);
					 return -1;
				 }		 
	}else if(32 == len){//对应协议2.4.0以上
			memcpy(order_num, msg, 22);
			memcpy((u8*)&type, (msg + 22), sizeof(type));
			memcpy((u8*)&point, (msg + 24), sizeof(point));
			memcpy((u8*)&seq, (msg + 28), sizeof(seq));
		
		  COMDBG_INFO("type: %d\n", type);
		  COMDBG_INFO("point: %d\n", point);
		  COMDBG_INFO("seq: %d\n", seq);
		  if(1 == type)
				type = 53;
			else if(2 == type)
				type =  52;
			else if(3 == type)
				type =  1;
			else if(4 == type)
				type =  51;

			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
			                     3,strlen(order_num), order_num, 4,4,(uint32_t)type,5,4,point, 7,sizeof(seq), seq);
			

			if(0 == iot_cloudpub(C2SDEV_PRIZE, QOS1, num, iot_sendbuf, 1))
			{
				COM_DBG("prize upload sucess: %d\r\n",Unix_Get());
				status = 1;
				memcpy(eqt_sendbuf, order_num, strlen(order_num));
				memcpy(eqt_sendbuf + strlen(order_num),  (u8*)&seq, sizeof(seq));
				memcpy(eqt_sendbuf + strlen(order_num) + sizeof(seq), &status, sizeof(status));
				device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(seq) + sizeof(status)),eqt_sendbuf);
				return 0;
			}else{
				COM_DBG("prize upload fail: %d\r\n",Unix_Get());
				status = 0;
				memcpy(eqt_sendbuf, order_num, strlen(order_num));
				memcpy(eqt_sendbuf + strlen(order_num),  (u8*)&seq, sizeof(seq));
				memcpy(eqt_sendbuf + strlen(order_num) + sizeof(seq), &status, sizeof(status));
				device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(seq) + sizeof(status)),eqt_sendbuf);
  			//delay_ms(PRIZE_DELAY_TIME);
				device_prize_qrcode3_send(port, type, point, order_num, seq);
				return -1;
			}		 
	}	else if(36 == len){
			memcpy(order_num, msg, 22);
			memcpy((u8*)&type, (msg + 22), sizeof(type));
			memcpy((u8*)&point, (msg + 24), sizeof(point));
			memcpy((u8*)&seq, (msg + 28), sizeof(seq));
			memcpy((u8*)&type2_data, (msg + 32), sizeof(type2_data));
		
		  COMDBG_INFO("type: %d\n", type);
		  COMDBG_INFO("point: %d\n", point);
		  COMDBG_INFO("seq: %d\n", seq);
		 if(1 == type)
				type = 53;
			else if(2 == type)
				type =  52;
			else if(3 == type)
				type =  1;
			else if(4 == type)
			{
				type =  51;
				COMDBG_INFO("prize_type2: %d\n",type2_data );
				type2=0x0CE5;//根据协议定义，封装数据
				type2=htons(type2);
				prize_type2_len=2;
			//	prize_type2_len=htons(prize_type2_len);
				
				prize_buff[0]=type2;
				prize_buff[1]=prize_type2_len;
				prize_buff[2]=type2_data;
			}
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%s%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
			                     3,strlen(order_num), order_num, 4,4,(uint32_t)type,5,4,point,6,sizeof(prize_buff),prize_buff,7,sizeof(seq), seq);
			
			if(0 == iot_cloudpub(C2SDEV_PRIZE, QOS1, num, iot_sendbuf, 1))
			{
				COM_DBG("prize upload sucess: %d\r\n",Unix_Get());
				status = 1;
				memcpy(eqt_sendbuf, order_num, strlen(order_num));
				memcpy(eqt_sendbuf + strlen(order_num),  (u8*)&seq, sizeof(seq));
				memcpy(eqt_sendbuf + strlen(order_num) + sizeof(seq), &status, sizeof(status));
				device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(seq) + sizeof(status)),eqt_sendbuf);
				return 0;
			}else{
				COM_DBG("prize upload fail: %d\r\n",Unix_Get());
				status = 0;
				memcpy(eqt_sendbuf, order_num, strlen(order_num));
				memcpy(eqt_sendbuf + strlen(order_num),  (u8*)&seq, sizeof(seq));
				memcpy(eqt_sendbuf + strlen(order_num) + sizeof(seq), &status, sizeof(status));
				device_cmd_send(DEVICE_PRIZE_REPORT, port, (strlen(order_num) + sizeof(seq) + sizeof(status)),eqt_sendbuf);
  			//delay_ms(PRIZE_DELAY_TIME);
				device_prize_qrcode3_send(port, type, point, order_num, seq);
				return -1;
			}			
	}
	
	return -1;
}


int device_status_set_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 status = 0;
	status = (u8)*msg;
	return c2s_s2c_status(port, DEV_STATUS, status);
}


int device_status_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	return 0;
}



int device_payqrcode_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 send_buff[65] ={0};
	if(payQRcode[port-1][0] != NULL)
	{
		send_buff[0] = 1;
		memcpy(send_buff +1,payQRcode[port-1],strlen(payQRcode[port-1]));
		device_cmd_send(DEVICE_PAY_QRCODE_GET, port, strlen(payQRcode[port-1]) + 1, send_buff);
	}else{
		send_buff[0] = 0;
		device_cmd_send(DEVICE_PAY_QRCODE_GET, port, 1, send_buff);
	}
	return 0;
}



int device_prize_qrcode_gen_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	char qrcode[220];
	u8 qrcode_len = 0;
	u16 type = 0;
	u32 vaule = 0;
	
	memcpy((u8*)&type, (msg), sizeof(type));
	memcpy((u8*)&vaule, (msg + 2), sizeof(vaule));
		
	COM_DBG("type: %d\n", type);
	COM_DBG("point: %d\n", vaule);
	if(1 == type)
		type = 53;
	else if(2 == type)
		type =  52;
	else if(3 == type)
		type =  1;
	else if(4 == type)
		type =  51;
	

	if(0 == prize_qrcode2_gen(qrcode, &qrcode_len, MD5keyId, MD5key, prizeUrl,type, vaule, port))
	{
			device_cmd_send(DEVICE_PRIZE_QRCODE_GEN, port, qrcode_len, (u8*)qrcode);
	}
	return 0;
}



int device_order_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 send_buff[65] ={0};
	u8 order_len =0;
	order_len = strlen(coin_service.port[port - 1].orderLast);

	memcpy(send_buff, coin_service.port[port - 1].orderLast, order_len);
	
	device_cmd_send(DEVICE_ORDER_GET, port, order_len, send_buff);
	return 0;	
}


int device_failprize_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
  u16 type = 0,prize_type2_len=0,type2_data=0,type2=0,prize_buff[3]={0};
	u32 point = 0, seq = 0;
	u16 num = 0;
	u8 iot_sendbuf[128];
	u8 eqt_sendbuf[64];
	char order_num[23] = {0};
	u8 status = 0;

	if(len == 1)
	{
	    u8 result = 0;
	    result = *msg;
		if(is_iot_works())
		{
			return c2s_s2c_status(port, DEVICE_FAILPRIZE_GET, result);
		}
		return -1;
	}
	
	if(len != 36)
		return -1;
		
	memcpy(order_num, msg, 22);
	memcpy((u8*)&type, (msg + 22), sizeof(type));
	memcpy((u8*)&point, (msg + 24), sizeof(point));
	memcpy((u8*)&seq, (msg + 28), sizeof(seq));
	memcpy((u8*)&type2_data, (msg + 32), sizeof(type2_data));
	
	COMDBG_INFO("type: %d\n", type);
	COMDBG_INFO("point: %d\n", point);
	COMDBG_INFO("seq: %d\n", seq);
	if(1 == type)
		type = 53;
	else if(2 == type)
		type =	52;
	else if(3 == type)
		type =	1;
	else if(4 == type)
	{
		type =  51;
		COMDBG_INFO("prize_type2: %d\n",type2_data );
		type2=0x0CE5;//根据协议定义，封装数据
		type2=htons(type2);
		prize_type2_len=2;
			//	prize_type2_len=htons(prize_type2_len);
				
		prize_buff[0]=type2;
		prize_buff[1]=prize_type2_len;
		prize_buff[2]=type2_data;
	}
	num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%s%h%h%i",
						1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
			      3,strlen(order_num), order_num, 4,4,(uint32_t)type,5,4,point,6,sizeof(prize_buff),prize_buff,7,sizeof(seq), seq);	
/*		type =	51;
	
	num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i%h%h%i%h%h%i",
						1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
						3,strlen(order_num), order_num, 4,4,(uint32_t)type,5,4,point, 7,sizeof(seq), seq);
*/			
	if(is_iot_works())
	{
		if(0 == iot_cloudpub(C2SDEV_PRIZE, QOS1, num, iot_sendbuf, 1))
		{ 
			COMDBG_INFO("prize upload sucess: %d\r\n",Unix_Get());
			status = 1;
			memcpy(eqt_sendbuf, order_num, strlen(order_num));
			memcpy(eqt_sendbuf + strlen(order_num),  (u8*)&seq, sizeof(seq));
			memcpy(eqt_sendbuf + strlen(order_num) + sizeof(seq), &status, sizeof(status));
			device_cmd_send(DEVICE_FAILPRIZE_GET, port, (strlen(order_num) + sizeof(seq) + sizeof(status)),eqt_sendbuf);
			return 0;
		}else{
			COMDBG_INFO("prize upload fail: %d\r\n",Unix_Get());
			status = 0;
			memcpy(eqt_sendbuf, order_num, strlen(order_num));
			memcpy(eqt_sendbuf + strlen(order_num),  (u8*)&seq, sizeof(seq));
			memcpy(eqt_sendbuf + strlen(order_num) + sizeof(seq), &status, sizeof(status));
			device_cmd_send(DEVICE_FAILPRIZE_GET, port, (strlen(order_num) + sizeof(seq) + sizeof(status)),eqt_sendbuf);
			return 0;
		}		 
	}else{
		COMDBG_INFO("prize no network: %d\r\n",Unix_Get());
		status = 0;
		memcpy(eqt_sendbuf, order_num, strlen(order_num));
		memcpy(eqt_sendbuf + strlen(order_num),  (u8*)&seq, sizeof(seq));
		memcpy(eqt_sendbuf + strlen(order_num) + sizeof(seq), &status, sizeof(status));
		device_cmd_send(DEVICE_FAILPRIZE_GET, port, (strlen(order_num) + sizeof(seq) + sizeof(status)),eqt_sendbuf);
		return 0;
	}	
}


int device_prize_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 send_buff[68] ={0};

	u16 type = 0xFFFF;

	memcpy((u8*)&type, (msg), sizeof(type));
	WRITE_IOT_DEBUG_LOG("type: %d\n", type);

	if(0 == type)
	{
		if(*devicePrizeUrl != 0)
		{
			memcpy(send_buff, (u8 *)&type, sizeof(type));
			memcpy(send_buff +2, devicePrizeUrl,strlen(devicePrizeUrl));
			device_cmd_send(DEVICE_PRIZE_GET, port, strlen(devicePrizeUrl) + 2, send_buff);
		}else{
		    memcpy(send_buff, (u8 *)&type, sizeof(type));
		    send_buff[sizeof(type)] = 0;
			device_cmd_send(DEVICE_PRIZE_GET, port, sizeof(type) + 1, send_buff);
		}
	}else if(1 == type){
	  if(*MD5keyId != 0)
		{
			memcpy(send_buff, (u8 *)&type, sizeof(type));
			memcpy(send_buff +2, MD5keyId, strlen(MD5keyId));
			device_cmd_send(DEVICE_PRIZE_GET, port, strlen(MD5keyId) + 2, send_buff);
		}else{
		    memcpy(send_buff, (u8 *)&type, sizeof(type));
		    send_buff[sizeof(type)] = 0;
			device_cmd_send(DEVICE_PRIZE_GET, port, sizeof(type) + 1, send_buff);
		}
	}else if(2 == type){
	  if(*MD5key != 0)
		{
			memcpy(send_buff, (u8 *)&type, sizeof(type));
			memcpy(send_buff +2, MD5key, strlen(MD5key));
			device_cmd_send(DEVICE_PRIZE_GET, port, strlen(MD5key) + 2, send_buff);
		}else{
		  memcpy(send_buff, (u8 *)&type, sizeof(type));
		  send_buff[sizeof(type)] = 0;
			device_cmd_send(DEVICE_PRIZE_GET, port, sizeof(type) + 1, send_buff);
		}
	}else if(3 == type){
	  uint64_t timestamp_now = Unix_Get();//机台生成二维码种子时间
	  memcpy(send_buff, (u8 *)&type, sizeof(type));
		memcpy(send_buff +2, (u8*)&timestamp_now, sizeof(timestamp_now));
		device_cmd_send(DEVICE_PRIZE_GET, port, sizeof(timestamp_now) + 2, send_buff);
	}
	
	return 0;
}





int device_prize_set_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
  u8 status = 0;
	status = *msg;
	return c2s_s2c_status(port, DEVICE_PRIZE_SET, status);
}




int device_failcoin_get_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u16 direction = 0,status = 0;
	u16 num = 0;
	u8 iot_sendbuf[128];
	char order_num[23] = {0};

	if(len == 1)
	{
	    u8 result = 0;
	    result = *msg;
		if(is_iot_works())
		{
			return c2s_s2c_status(port, DEVICE_FAILCOIN_GET, result);
		}
		return -1;
	}
	
	if(len != 26)
		return -1;
	
	memcpy(order_num, msg, 22);
	memcpy((u8*)&direction, (msg + 22), sizeof(direction));
	memcpy(&status, (msg + 24), sizeof(status));

	if(direction == 0x00)
		return -1;

	if(is_iot_works())
	{
		if(status == 0)
			status = 2;
		num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s%h%h%i",
				   1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,3,strlen(order_num), order_num, 
				   7,4,(uint32_t)status);

		if(0 == iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf, 1))
		{
			fail_coin_confirm(port, order_num, strlen(order_num), 1);
			COMDBG_INFO("\n1 order upload success: %s\r\n", order_num);
		}else{
			fail_coin_confirm(port, order_num, strlen(order_num), 0);
			COMDBG_INFO("\n1 order  upload  fail: %s\r\n", order_num);
		}		
	}else{
		fail_coin_confirm(port, order_num, strlen(order_num), 0);
		COMDBG_INFO("\n1 order upload fail: %s\n", order_num);
	}
	return 0;
}



int device_ranking_request_response(u8 port, u8* msg, u16 len, u8 isnetwork_cmd)
{
	u8 iot_sendbuf[128];
	u8 sub_iot_sendbuf[64];
	u8 eqt_sendbuf[64];
	u8 num,sub_num;
	char order_num[23] = {0};

	if(len != 22)
		return -1;

	if(0 == is_iot_works())
	{
	  eqt_sendbuf[0] = 0;
		return device_cmd_send(DEVICE_RANKING_REQUEST, port, 1, eqt_sendbuf);
	}

	memcpy(order_num, msg, 22);

	sub_num = SerializeToOstream(sub_iot_sendbuf, "%h%h%s",3613,strlen(order_num), order_num);

	num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%s",
						1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)port,
						3,sub_num, sub_iot_sendbuf);

	if(0 == iot_cloudpub( C2STRANSFER, QOS1, num, iot_sendbuf, 1))
	{
		COMDBG_INFO("\n1 ranking request: %s\r\n", order_num);
	}

	return 0;	
}



/*************************************************************
 *  名称：	device_com_process()
 *  功能：  依据协议处理链表eqptInfoList中接收到的消息
 *	输入：  cmd-命令字
 *          msg_ack_len-消息长度（消息体）
 *          msg_ack-消息内容
 *          service_id-业务id 网络投币及prize传下来的id
 *          isnetworkcmd-是否为网络命令
 *	输出：  TRUE-执行完成
 *************************************************************/
int device_com_process(u8 port, u8 cmd, u16 msg_ack_len,u8 *msg_ack, u8 isnetwork_cmd)
{
	COM_DBG("eqt port is: %d\r\n", port);
	COM_DBG("eqt cmd is: %d\r\n", cmd);
	COM_DBG("eqt content is: ");
	DBG_HEX(msg_ack, msg_ack_len);
	 switch (cmd)
	 {
		 /*case FIRMWARE_FIRST://firmware_online pub
			 firmware_first_pub(msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case GPRS_RSSI://gprs_rssi pub
       firmware_signal(msg_ack, msg_ack_len, isnetwork_cmd);
		 break;*/
			 
		 case DEV_COMM_TIMEOUT:
			 device_comm_timeout(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_CHECK://主机查询设备状态指令响应
			 device_check_response(msg_ack, msg_ack_len, isnetwork_cmd); 
		 break;
		 
		 case DEVICE_RESET://主机复位设备状态指令响应
			 device_reset_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case DEVICE_COIN://发送投币至设备指令响应
			 device_coin_set_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_ACCOUNT_GET://主机获取设备账目信息指令响应
			 device_account_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case DEVICE_PARAM_GET://主机获取设备参数指令响应
			 device_param_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);	
		 break;
		 
		 case DEVICE_PARAM_SET://主机设置设备参数指令响应
			 device_param_set_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;	
		 
		 case DEVICE_FRONT_ACCOUNT_CLR://主机清除设备面板账目指令响应
	     device_front_account_clr_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
				 
		 case DEVICE_BACK_ACCOUNT_CLR://主机清除设备后台账目指令响应
	     device_back_account_clr_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
						 
		 case DEVICE_INCREMENT_GET://主机查询设备增量指令响应
	     device_increment_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
								 
		 case DEVICE_RESTORE://主机复位设备指令响应
	     device_restore_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
										 
		 case DEVICE_ACCOUNT_CLR://主机清除设备本地账目指令响应
	     device_account_clr_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
												 
		 case DEVICE_QRCODE_SET://主机发送二维码信息至设备指令响应
	     device_qrcode_set_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_INCREMENT_REPORT://设备主动上报增量（游戏币和出礼品等）
       device_increment_report(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_REQUEST_SIGNAL://设备查询主机信号
			 device_request_signal();
		 break;
		 
		 case DEVICE_SLEEP://主机设置设备待机状态指令响应
       device_sleep_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_WAKE://主机唤醒设备指令响应
		   device_wake_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_PRIZE_CLR://主机清设备奖池指令响应
       device_prize_clr_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_ACCOUNT_SET://主机设置设备本地账目指令响应
			 device_account_set_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
				 
		 case DEVICE_PRIZE_REPORT://设备上报中奖信息
			 device_prize_report(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_SET_STATUS://设备命令操作状态信息
			 device_status_set_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_GET_STATUS://获取设备游戏状态
			 device_status_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break; 
		 
		 case DEVICE_PRIZE_QRCODE://云采器发送出奖二维码至设备
			 device_prizeqrcode_set_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 case DEVICE_PAY_QRCODE_GET://设备获取云采器保存的支付二维码内容
			 device_payqrcode_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;	
		 
		 case DEVICE_PRIZE_QRCODE_GEN://不联网设备，请求云采器生成退分二维码
			 device_prize_qrcode_gen_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
				 
		 case DEVICE_ORDER_GET://设备请求最后一次有效订单(订单缓冲在RAM)
			 device_order_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case DEVICE_FAILPRIZE_GET://云采器获取机台最后一次失败出奖，用于处理出奖
			 device_failprize_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case DEVICE_PRIZE_SET://云采器向设备发送，退分二维码生成所需信息
			 device_prize_set_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case DEVICE_PRIZE_GET://设备查询云采器，获取退分二维码生成所需信息
		 	 device_prize_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case DEVICE_FAILCOIN_GET://云采器获取机台最后一次失败上币，用于处理数据同步
		 	 device_failcoin_get_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;

		 case DEVICE_RANKING_REQUEST:
		 	 device_ranking_request_response(port, msg_ack, msg_ack_len, isnetwork_cmd);
		 break;
		 
		 default:
		 break;
	 }
	 
	 if(g_firmware_info.type == 1)
	 {
		 device.timeout = 0;
		 if(device.status != 101)
		 {
			 if(cmd != 0 && cmd < 0xE0)
			 {
					if(0 == device_status_report(101))
					{
						device.status = 101;
					}
			 }		 
		 }		 
	 }

	 return 0;
}



/*************************************************************
 *  名称：	check_device()
 *  功能：  串口设备 发送状态查询命令，如有返回正确响应 则为串口设备，否则标记为脉冲设备
 *	输入：  timeout-命令返回超时
 *	输出：  FALSE-超时  TRUE-执行完成
 *************************************************************/
int check_device(u8 port, u8 timeout, u8 isnetwork_cmd)
{
	return device_cmd_and_wait_ack(DEVICE_CHECK,
	            	port, strlen(g_iot_info.dname), (u8*)g_iot_info.dname, isnetwork_cmd, timeout);
}


/*************************************************************
 *  名称：	check_device()
 *  功能：  串口设备 发送状态查询命令，如有返回正确响应 则为串口设备，否则标记为脉冲设备
 *	输入：  timeout-命令返回超时
 *	输出：  FALSE-超时  TRUE-执行完成
 *************************************************************/
int set_device_status(u8 port, u8 act, u8 timeout)
{
	return device_cmd_and_wait_ack(DEVICE_SET_STATUS, port, sizeof(u8), &act, 0, timeout);
}



/*************************************************************
 *  名称：	device_coin_prize_report_or_store()
 *  功能：  针对非串口设备，上传实体投币和出奖信息，如网络连接
            正常则上传至后台，否则保存至flash，下一次联网时上传
 *	输入：  
 *	输出：  TRUE-执行完成
 *************************************************************/
int device_coin_report_or_store(void)
{
	u8 num = 0,i;
	u8 iot_sendbuf[42];
	record rec;
	
	u16 coin_has_sending = 0;
	u16 tbout_has_sending = 0;
	
	if(g_service_info.coin_has != 0)
	{
		coin_has_sending = g_service_info.coin_has;
		if(is_iot_works())
		{
			
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%i",
									 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)1,
									 3,4, (uint32_t)1, 4,4,(uint32_t)coin_has_sending);
			if(0 == iot_cloudpub(C2SDEV_INCOME, QOS1, num, iot_sendbuf, 1))
			{
				g_service_info.coin_has -= coin_has_sending;
			}else{
				  memset((u8*)&rec, 0x0, sizeof(record));
					rec.num = coin_has_sending;//
					rec.seq = 0;
					rec.timestamp = Unix_Get();
					rec.type = 1;//实体投币
					for(i = 0; i< (sizeof(record) - 2);i++)
					{
						rec.sum +=  *((u8*)&rec + i); 
					}
					record_write(&rec);
					g_service_info.coin_has -= coin_has_sending;			
			}
		}else{
			memset((u8*)&rec, 0x0, sizeof(record));
			rec.num = coin_has_sending;//
			rec.seq = 0;
			rec.timestamp = Unix_Get();
			rec.type = 1;//实体投币
			for(i = 0; i< (sizeof(record) - 2);i++)
			{
				rec.sum +=  *((u8*)&rec + i); 
			}
			record_write(&rec);
			g_service_info.coin_has -= coin_has_sending;
		}
	}
	

	if(g_service_info.tbout_has != 0)
	{
		tbout_has_sending = g_service_info.tbout_has;
		if(is_iot_works())
		{
			tbout_has_sending = g_service_info.tbout_has;
			num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%i",
								 1,8,(uint64_t)Unix_Get(),2,4, (uint32_t)1,
								 4,4, (uint32_t)51, 5,4,(uint32_t)tbout_has_sending);
			if(0 == iot_cloudpub(C2SDEV_PRIZE, QOS1, num, iot_sendbuf, 1))
			{
				g_service_info.tbout_has -= tbout_has_sending;
			}else{
				  memset((u8*)&rec, 0x0, sizeof(record));
					rec.num = tbout_has_sending;//
					rec.seq = 0;
					rec.timestamp = Unix_Get();
					rec.type = 2;//退彩
					for(i = 0; i< (sizeof(record) - 2);i++)
					{
						rec.sum +=  *((u8*)&rec + i); 
					}
					record_write(&rec);
					g_service_info.tbout_has -= tbout_has_sending;			
			}
		}else{
			memset((u8*)&rec, 0x0, sizeof(record));
			rec.num = tbout_has_sending;//
			rec.seq = 0;
			rec.timestamp = Unix_Get();
			rec.type = 2;//退彩
			for(i = 0; i< (sizeof(record) - 2);i++)
			{
				rec.sum +=  *((u8*)&rec + i); 
			}
			record_write(&rec);
			g_service_info.tbout_has -= tbout_has_sending;
		}
	}
	
	return 0;	
}


int device_prize_set(u8 port, u16 type, u8 msglen, char* msg_content, u8 isnetwork_cmd)
{
  u8 eqt_buff[68]={0};
	u8 len = 0;

	memcpy(eqt_buff, (u8*)&type, sizeof(type));
	memcpy(eqt_buff + sizeof(type), msg_content, msglen);

	len = sizeof(type) + msglen;

	if(0 != device_cmd_and_wait_ack(DEVICE_PRIZE_SET, port, len, eqt_buff, isnetwork_cmd,1 ))
	{
		//WRITE_IOT_ERROR_LOG("please check sim card");
		return -1;
	}
	return 0;
}



int device_race_set(u8 port, u32 race_no, u32 race_rank, u32 race_bonus,  u32 race_bdiff,char* nikename, u8 nikename_len)
{
  u8 eqt_buff[76]={0};
	u8 len = 0;

	eqt_buff[0] = 1;
    len = 1;
	

	if(race_no == 0)
	{
	  memcpy(eqt_buff + len, (u8*)&race_no, sizeof(race_no));
		len +=  sizeof(race_no);
		device_cmd_and_wait_ack(DEVICE_RANKING_REQUEST, port, len, eqt_buff, 0, 0);
		return 0;
	}else{
     memcpy(eqt_buff +len, (u8*)&race_no, sizeof(race_no));
		len +=  sizeof(race_no);
    memcpy(eqt_buff +len, (u8*)&race_rank, sizeof(race_rank));
		len +=  sizeof(race_rank);
    memcpy(eqt_buff +len, (u8*)&race_bonus, sizeof(race_bonus));
		len +=  sizeof(race_bonus);
    memcpy(eqt_buff +len, (u8*)&race_bdiff, sizeof(race_bdiff));
		len +=  sizeof(race_bdiff);
    memcpy(eqt_buff +len, (u8*)nikename, nikename_len);
		len +=  nikename_len;
		
		device_cmd_and_wait_ack(DEVICE_RANKING_REQUEST, port, len, eqt_buff, 0, 0);
		return 0;
	}
}



/*************************************************************
 *  名称：	device_stored_report()
 *  功能：  针对非串口设备，网络连接正常时，上传firmware中
            实体投币和出奖保存信息，每调用一次传一条
 *	输入：  
 *	输出：  TRUE-执行完成
 *************************************************************/
int device_stored_report(void)
{
	u8 num = 0;
	u8 iot_sendbuf[42];
	record rec;
	
	if(is_iot_works() != 1)
		return -1;
	
	if(record_read(&rec) == 0)    //读取存储信息
	{
		COM_DBG("rec.timestamp: %d\n",rec.timestamp);
		COM_DBG("rec.num: %d\n",rec.num);
		COM_DBG("rec.seq: %d\n",rec.seq);
		COM_DBG("rec.type: %d\n",rec.type);
		COM_DBG("rec.orderNum: %s\n", rec.orderNum);
		
		switch(rec.type)
		{
			case 0://wechat insert
				if(rec.num == 0)
				{
					num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%i",
									 1,8,(uint64_t)rec.timestamp,2,4, (uint32_t)1,
									 3,4, (uint32_t)1, 4,4,(uint32_t)rec.num);
				}else{
					num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%i",
									 1,8,(uint64_t)rec.timestamp,2,4, (uint32_t)1,
									 3,4, (uint32_t)1, 7,4,(uint32_t)rec.num);
				}
				
				if(0 == iot_cloudpub(C2SDEV_COIN, QOS1, num, iot_sendbuf,1))
					record_read_success();
			break;
			
			case 1://real coin
				num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%i",
									 1,8,(uint64_t)rec.timestamp,2,4, (uint32_t)1,
									 3,4, (uint32_t)1, 4,4,(uint32_t)rec.num);
				if(0 == iot_cloudpub(C2SDEV_INCOME, QOS1, num, iot_sendbuf,1))
					record_read_success();				
			break;
				
			case 2://退彩
				num = SerializeToOstream(iot_sendbuf, "%h%h%l%h%h%i%h%h%i%h%h%i",
								 1,8,(uint64_t)rec.timestamp,2,4, (uint32_t)1,
								 4,4,(uint32_t)51, 5,4,(uint32_t)rec.num);
				if(0 == iot_cloudpub(C2SDEV_PRIZE, QOS1, num, iot_sendbuf,1))
					record_read_success();				
			break;
			
			default:
				
			break;
		}
	}else{
		record_read_success();
	}
	
	return 0;	
}



