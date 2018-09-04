#include "iot.h"
#include "selector.h"
#include "info_storage.h" 
#include "24cxx.h"

void iot_data_write(void)
{
	COM_DBG("save IOT\r\n");
	AT24CXX_Write(I2CADDR_IOT_INFO, (u8*)&g_iot_info, sizeof(iot_device));	
}


void iot_data_read(void)
{
		AT24CXX_Read(I2CADDR_IOT_INFO, (u8*)&g_iot_info, sizeof(iot_device));
}


void firmware_data_write(void)
{
	u8 flag = 0;
	taskENTER_CRITICAL();           //进入临界区

		flag = AT24CXX_ReadOneByte(I2CADDR_FIRMWARE_FLAG);
		if(0 == flag)
		{
			AT24CXX_Write(I2CADDR_FIRMWARE_BASE + I2CADDR_FIRMWARE_OFFSET, (u8*)&g_firmware_info, sizeof(firmware));
			flag = 1;
			AT24CXX_WriteOneByte(I2CADDR_FIRMWARE_FLAG, flag);
		}else{
			AT24CXX_Write(I2CADDR_FIRMWARE_BASE, (u8*)&g_firmware_info, sizeof(firmware));
			flag = 0;
			AT24CXX_WriteOneByte(I2CADDR_FIRMWARE_FLAG, flag);
		}
	taskEXIT_CRITICAL();            //退出临界区
}
	

void firmware_data_read(void)
{
	u8 flag = 0xFF;
		
	AT24CXX_Read(I2CADDR_FIRMWARE_FLAG, &flag, 1);
		if(0 == flag)
		{
			AT24CXX_Read(I2CADDR_FIRMWARE_BASE, (u8*)&g_firmware_info, sizeof(firmware));
		}else if(1 == flag){
			AT24CXX_Read(I2CADDR_FIRMWARE_BASE + I2CADDR_FIRMWARE_OFFSET, (u8*)&g_firmware_info, sizeof(firmware));
		}else{
			g_firmware_info.type = 0x2;//默认脉冲
			g_firmware_info.signalThreshold = 10;
			g_firmware_info.timeThreshold = 10;
			AT24CXX_Write(I2CADDR_FIRMWARE_BASE, (u8*)&g_firmware_info, sizeof(firmware));
			flag = 0;
			AT24CXX_WriteOneByte(I2CADDR_FIRMWARE_FLAG, flag);
		}
    COM_DBG("devicetype:%d\r\n",g_firmware_info.type);
}



void selector_data_write(void)
{
	u8 flag = 0;
	taskENTER_CRITICAL();           //进入临界区
		flag = AT24CXX_ReadOneByte(I2CADDR_SELECTOR_FLAG);
		if(0 == flag)
		{
			AT24CXX_Write(I2CADDR_SELECTOR_BASE + I2CADDR_SELECTOR_OFFSET, (u8*)&g_selector_info, sizeof(selector));
			flag = 1;
			AT24CXX_WriteOneByte(I2CADDR_SELECTOR_FLAG, flag);
		}else{
			AT24CXX_Write(I2CADDR_SELECTOR_BASE, (u8*)&g_selector_info, sizeof(selector));
			flag = 0;
			AT24CXX_WriteOneByte(I2CADDR_SELECTOR_FLAG, flag);
		}
		taskEXIT_CRITICAL();            //退出临界区
}
	

void selector_data_read(void)
{
	u8 flag = 0xFF;
		
	AT24CXX_Read(I2CADDR_SELECTOR_FLAG, &flag, 1);

		if(0 == flag)
		{
			AT24CXX_Read(I2CADDR_SELECTOR_BASE, (u8*)&g_selector_info, sizeof(selector));
		}else if(1 == flag){
			AT24CXX_Read(I2CADDR_SELECTOR_BASE + I2CADDR_SELECTOR_OFFSET, (u8*)&g_selector_info, sizeof(selector));
		}else{
			g_selector_info.polarity = 1;
			g_selector_info.width = 50;
			g_selector_info.interval = 200;
			AT24CXX_Write(I2CADDR_SELECTOR_BASE, (u8*)&g_selector_info, sizeof(selector));
			flag = 0;
			AT24CXX_WriteOneByte(I2CADDR_SELECTOR_FLAG, flag);
		}
}




u8 get_record_ws(u32 addr)
{
	u8 num = 0;
  taskENTER_CRITICAL();           //进入临界区
	AT24CXX_Read(addr, (u8*)&num, sizeof(u8));
	taskEXIT_CRITICAL();            //退出临界区
	return num;
}


u16 get_record_f(void)
{
	u16 num = 0;
  taskENTER_CRITICAL();           //进入临界区
	AT24CXX_Read(I2CADDR_RECORD_F,  (u8*)&num, sizeof(u16));
	taskEXIT_CRITICAL();            //退出临界区
	return num;
}

u16 set_record_f(u16 val)
{
  taskENTER_CRITICAL();           //进入临界区
	AT24CXX_Write(I2CADDR_RECORD_F,  (u8*)&val, sizeof(u16));
	taskEXIT_CRITICAL();            //退出临界区
	return val;
}

u8 set_record_ws(u32 addr,u8 num)
{
	taskENTER_CRITICAL();           //进入临界区
	AT24CXX_Write(addr, (u8*)&num, sizeof(u8));
	taskEXIT_CRITICAL();            //退出临界区
	return 0;
}


int record_read(record* rec)
{
	u8 num_s,num_w,i;
	u16 sum = 0;
	u32 record_readAddr;
	if(get_record_f() != 0x5A5A)
	{
		set_record_f(0x5A5A);
		set_record_ws(I2CADDR_RECORD_W,0);
		set_record_ws(I2CADDR_RECORD_S,0);
		return -1;
	}
	num_s = get_record_ws(I2CADDR_RECORD_S);
	num_w = get_record_ws(I2CADDR_RECORD_W);
	
	if(num_s >0)
	{
		if(num_w < num_s)
			num_w += MAX_RECORD;
		record_readAddr = I2CADDR_RECORD_BASE + (num_w - num_s)*sizeof(record);
		taskENTER_CRITICAL();           //进入临界区
		AT24CXX_Read(record_readAddr,  (u8*)rec, sizeof(record));
		taskEXIT_CRITICAL();            //退出临界区
		
	  for(i = 0; i< (sizeof(record) - 2);i++)
		{
			sum +=  *((u8*)rec + i); 
		}
		
		if(sum == rec->sum)
			return 0;
	}
	return -1;
}

int record_read_success(void)
{
	u8 num_s;
	num_s = get_record_ws(I2CADDR_RECORD_S);
	
	if(num_s >0)
		set_record_ws(I2CADDR_RECORD_S,--num_s);
	return 0;
}

int record_write(record* rec)
{
	u8 num_s,num_w;

	u32 record_writeAddr;
	if(get_record_f() != 0x5A5A)
	{
		set_record_f(0x5A5A);
		set_record_ws(I2CADDR_RECORD_W,0);
		set_record_ws(I2CADDR_RECORD_S,0);
		return 0;
	}
	num_s = get_record_ws(I2CADDR_RECORD_S);
	num_w = get_record_ws(I2CADDR_RECORD_W);
	
	record_writeAddr = I2CADDR_RECORD_BASE + num_w*sizeof(record);
	
	taskENTER_CRITICAL();           //进入临界区
	AT24CXX_Write(record_writeAddr,  (u8*)rec, sizeof(record));
	taskEXIT_CRITICAL();            //退出临界区
	num_w++;
	
	if(num_w >= MAX_RECORD)
			num_w = 0;
	
	set_record_ws(I2CADDR_RECORD_W,num_w);
	
	num_s++;
	if(num_s >= MAX_RECORD)
		set_record_ws(I2CADDR_RECORD_S,MAX_RECORD);
	else
		set_record_ws(I2CADDR_RECORD_S,num_s);
	return 0;
}
