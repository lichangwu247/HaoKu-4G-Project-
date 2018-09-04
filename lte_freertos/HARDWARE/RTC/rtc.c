#include "delay.h"
#include "rtc.h"
#include "sys.h"


/*
��ʼ��rtcʱ��
*/
/************************************************
�������� �� RTC_Configuration
��    �� �� RTC����
��    �� �� ��
�� �� ֵ �� ��
��    �� �� 
*************************************************/
void RTC_config(void)
{  
    u16 i = 0;  
    /* Enable PWR and BKP clocks */  
    /* PWRʱ�ӣ���Դ���ƣ���BKPʱ�ӣ�RTC�󱸼Ĵ�����ʹ�� */  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  
  
    /* Allow access to BKP Domain */  
    /*ʹ��RTC�ͺ󱸼Ĵ������� */  
    PWR_BackupAccessCmd(ENABLE);  
  
    /* Reset Backup Domain */  
    /* ������BKP��ȫ���Ĵ�������Ϊȱʡֵ */  
    BKP_DeInit();  
  
    /* Enable LSE */  
    /* ʹ��LSE���ⲿ32.768KHz���پ���*/  
	  
    RCC_LSEConfig(RCC_LSE_ON);  
    
    /* Wait till LSE is ready */  
    /* �ȴ��ⲿ�������ȶ���� */  
    for (i = 0;i < 500;i++) //500�μ�⣬���LSE
    //��Ȼû������֤�������������⣬����ѭ��  
    {  
        if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)  
            break;  
        delay_ms(10); //10ms��ʱ  
    }  
    COM_DBG("LSE oscillation: %dms\r\n",i*10);
    if (i >= 500)  
    {  
        //ʹ���ⲿ���پ��� 128��Ƶ  
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
        COM_DBG("using HSE\r\n");
    }else{  
        /* Select LSE as RTC Clock Source */  
        /*ʹ���ⲿ32.768KHz������ΪRTCʱ�� */                           
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        COM_DBG("using LSE\r\n");			
    }  
  
    /* Enable RTC Clock */  
    /* ʹ�� RTC ��ʱ�ӹ��� */  
    RCC_RTCCLKCmd(ENABLE);  
  
    /* Wait for RTC registers synchronization */  
    /*�ȴ�RTC�Ĵ���ͬ�� */  
    RTC_WaitForSynchro();  
  
    /* Wait until last write operation on RTC registers has finished */  
    /* �ȴ���һ�ζ�RTC�Ĵ�����д������� */  
    RTC_WaitForLastTask();  
    
    /* Set RTC prescaler: set RTC period to 1sec */  
    /* 32.768KHz����Ԥ��Ƶֵ��32767,
����Ծ���Ҫ��ܸ߿����޸Ĵ˷�Ƶֵ��У׼���� */ 
    RTC_EnterConfigMode();/// ��������			
    if (i < 500) //LSE��������  
        RTC_SetPrescaler(32767); 
		/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */  
    else  
        RTC_SetPrescaler(62499); //8M / 128 = 62500��������Ӧ����Ϊ62499  
  
    /* Wait until last write operation on RTC registers has finished */  
    /* �ȴ���һ�ζ�RTC�Ĵ�����д������� */  
    RTC_WaitForLastTask(); 
    RTC_ExitConfigMode(); //�˳�����ģʽ  		
}





/*
��ʼ��rtc
*/
u8 RTC_Init(void)
{
	u16 i = 0;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);    //ʹ��PWR��BKP����ʱ��   
  PWR_BackupAccessCmd(ENABLE);                                                //ʹ�ܺ󱸼Ĵ������� 
	//����ǲ��ǵ�һ������ʱ�� 
  if (BKP_ReadBackupRegister(BKP_DR1) != 0x5AA5)                              //��ָ���ĺ󱸼Ĵ����ж�������:��������д���ָ�����ݲ����
  {
		BKP_DeInit();                                                           //��λ��������    
    RCC_LSEConfig(RCC_LSE_ON);                                              //�����ⲿ���پ���(LSE),ʹ��������پ���
        
		/* Wait till LSE is ready */  
		/* �ȴ��ⲿ�������ȶ���� */  
		for (i = 0;i < 500;i++) //500�μ�⣬���LSE
		//��Ȼû������֤�������������⣬����ѭ��  
		{
			if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
				break;
			delay_ms(10); //10ms��ʱ  
		}
		COM_DBG("LSE oscillation: %dms\r\n",i*10);
		if (i >= 500)
		{
			//ʹ���ⲿ���پ��� 128��Ƶ  
			RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
			COM_DBG("using HSE\r\n");
		}else{  
			/* Select LSE as RTC Clock Source */  
			/*ʹ���ⲿ32.768KHz������ΪRTCʱ�� */                           
			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
			COM_DBG("using LSE\r\n");
		}
		/* Enable RTC Clock */  
		/* ʹ�� RTC ��ʱ�ӹ��� */  
		RCC_RTCCLKCmd(ENABLE);  
			
		/* Wait for RTC registers synchronization */  
		/*�ȴ�RTC�Ĵ���ͬ�� */  
		RTC_WaitForSynchro();  
			
		/* Wait until last write operation on RTC registers has finished */  
		/* �ȴ���һ�ζ�RTC�Ĵ�����д������� */  
		RTC_WaitForLastTask();  
				
		/* Set RTC prescaler: set RTC period to 1sec */  
		/* 32.768KHz����Ԥ��Ƶֵ��32767,
		����Ծ���Ҫ��ܸ߿����޸Ĵ˷�Ƶֵ��У׼���� */ 
		RTC_EnterConfigMode();/// ��������			
		if (i < 500) //LSE�������� 
			RTC_SetPrescaler(32767); 
		/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */  
		else
			RTC_SetPrescaler(62499); //8M / 128 = 62500��������Ӧ����Ϊ62499  
			
		/* Wait until last write operation on RTC registers has finished */  
		/* �ȴ���һ�ζ�RTC�Ĵ�����д������� */  
		RTC_WaitForLastTask();
    Clock_SetTime(2017,2,18,15,30,50,8);          //����ʱ��  
    RTC_ExitConfigMode();                       //�˳�����ģʽ  
    BKP_WriteBackupRegister(BKP_DR1, 0x5AA5);   //��ָ���ĺ󱸼Ĵ�����д���û���������
  }else{
		RTC_WaitForSynchro();
  }
	
	RTC_ITConfig(RTC_IT_SEC, ENABLE);   //ʹ��RTC���ж�
  RTC_WaitForLastTask();              //�ȴ����һ�ζ�RTC�Ĵ�����д�������                         
  return 0;
}


//����ʱ��
void Clock_SetTime(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec, s8 zone)
{
    time_t unix_time;     //����time_t���͵�����ʱ��ṹ��
    struct tm tm_Set_Time;      //����tm�ṹ������ʱ��ṹ��

    tm_Set_Time.tm_year = (u32)(syear-1900); //��1900������
    tm_Set_Time.tm_mon  = (u32)(smon-1);     //��
    tm_Set_Time.tm_mday = (u32)sday;         //��
    tm_Set_Time.tm_hour = (u32)hour;         //ʱ
    tm_Set_Time.tm_min  = (u32)min;          //��
    tm_Set_Time.tm_sec  = (u32) sec;         //��

    unix_time = mktime(&tm_Set_Time);   //ȡ�ü�����ʼֵ
	
	  COM_DBG("unix_timestamp: %d\r\n", unix_time);
	
	  unix_time -= 60*60*zone;            //ʱ��ƫ��У��
	
    if(unix_time!=0xFFFFFFFF)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);    //ʹ��PWR&BKP����ʱ��  
        PWR_BackupAccessCmd(ENABLE);        //ʹ��RTC�ͺ󱸼Ĵ������� 
        RTC_SetCounter(unix_time);    //����RTC������ֵ

        RTC_WaitForLastTask();              //�ȴ����һ�ζ�RTC�Ĵ���д�������
    }
}


//��ȡʱ�� ������ʽ
u8 Clock_Get(struct tm *local)
{
	time_t timestamp;

  timestamp = RTC_GetCounter();                     //��ȡRTC������

  local = localtime(&timestamp);                      //��UNIXת��������ʱ��
	return 0;
}


//��ȡʱ�� UNIX��ʽ
u32 Unix_Get(void)
{
    return RTC_GetCounter();                        //����RTC����ֵ
}


//����ʱ��
void RTC_IRQHandler(void)
{        
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)   //���ж�
    {                           
       //to do...
    }
    RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);//���ж�
    RTC_WaitForLastTask();                                           
}


