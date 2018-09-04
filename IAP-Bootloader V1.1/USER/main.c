#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "stmflash.h"
#include "iap.h"
#include "led.h"
/****************************************************************************/

//#define RESTORE

int main(void)
{		
	u32 user_bin_flag = 0xFFFFFFFF;
	//u32 user_bin_flag = 0x5A5A5A5A;

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(230400);	//串口初始化为230400bps
	delay_init();	   	 	//延时初始化 

#ifdef RESTORE
	FLASH_Unlock();						//解锁
	FLASH_ErasePage(FLASH_USERBIN_MARK_ADDR);//擦除区
	FLASH_ProgramWord(FLASH_USERBIN_MARK_ADDR,user_bin_flag);
	FLASH_Lock();//上锁
#endif
	
	user_bin_flag = STMFLASH_ReadWord(FLASH_USERBIN_MARK_ADDR);
	
	if(0x5A5A5A5A == user_bin_flag){
		printf(" FIRMWARE 1\r\n");
		if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
		{
			iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP1代码
		}
	}else{
		printf(" FIRMWARE 0\r\n");
		if(((*(vu32*)(FLASH_APP0_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
		{	
			iap_load_app(FLASH_APP0_ADDR);//执行FLASH APP代码
		}
	}	 
}

