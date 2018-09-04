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

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(230400);	//���ڳ�ʼ��Ϊ230400bps
	delay_init();	   	 	//��ʱ��ʼ�� 

#ifdef RESTORE
	FLASH_Unlock();						//����
	FLASH_ErasePage(FLASH_USERBIN_MARK_ADDR);//������
	FLASH_ProgramWord(FLASH_USERBIN_MARK_ADDR,user_bin_flag);
	FLASH_Lock();//����
#endif
	
	user_bin_flag = STMFLASH_ReadWord(FLASH_USERBIN_MARK_ADDR);
	
	if(0x5A5A5A5A == user_bin_flag){
		printf(" FIRMWARE 1\r\n");
		if(((*(vu32*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
		{
			iap_load_app(FLASH_APP1_ADDR);//ִ��FLASH APP1����
		}
	}else{
		printf(" FIRMWARE 0\r\n");
		if(((*(vu32*)(FLASH_APP0_ADDR+4))&0xFF000000)==0x08000000)//�ж��Ƿ�Ϊ0X08XXXXXX.
		{	
			iap_load_app(FLASH_APP0_ADDR);//ִ��FLASH APP����
		}
	}	 
}

