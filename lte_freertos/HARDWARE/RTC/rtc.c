#include "delay.h"
#include "rtc.h"
#include "sys.h"


/*
初始化rtc时钟
*/
/************************************************
函数名称 ： RTC_Configuration
功    能 ： RTC配置
参    数 ： 无
返 回 值 ： 无
作    者 ： 
*************************************************/
void RTC_config(void)
{  
    u16 i = 0;  
    /* Enable PWR and BKP clocks */  
    /* PWR时钟（电源控制）与BKP时钟（RTC后备寄存器）使能 */  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);  
  
    /* Allow access to BKP Domain */  
    /*使能RTC和后备寄存器访问 */  
    PWR_BackupAccessCmd(ENABLE);  
  
    /* Reset Backup Domain */  
    /* 将外设BKP的全部寄存器重设为缺省值 */  
    BKP_DeInit();  
  
    /* Enable LSE */  
    /* 使能LSE（外部32.768KHz低速晶振）*/  
	  
    RCC_LSEConfig(RCC_LSE_ON);  
    
    /* Wait till LSE is ready */  
    /* 等待外部晶振震荡稳定输出 */  
    for (i = 0;i < 500;i++) //500次检测，如果LSE
    //仍然没有起振，证明这玩意有问题，跳出循环  
    {  
        if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)  
            break;  
        delay_ms(10); //10ms延时  
    }  
    COM_DBG("LSE oscillation: %dms\r\n",i*10);
    if (i >= 500)  
    {  
        //使用外部高速晶振 128分频  
        RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
        COM_DBG("using HSE\r\n");
    }else{  
        /* Select LSE as RTC Clock Source */  
        /*使用外部32.768KHz晶振作为RTC时钟 */                           
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
        COM_DBG("using LSE\r\n");			
    }  
  
    /* Enable RTC Clock */  
    /* 使能 RTC 的时钟供给 */  
    RCC_RTCCLKCmd(ENABLE);  
  
    /* Wait for RTC registers synchronization */  
    /*等待RTC寄存器同步 */  
    RTC_WaitForSynchro();  
  
    /* Wait until last write operation on RTC registers has finished */  
    /* 等待上一次对RTC寄存器的写操作完成 */  
    RTC_WaitForLastTask();  
    
    /* Set RTC prescaler: set RTC period to 1sec */  
    /* 32.768KHz晶振预分频值是32767,
如果对精度要求很高可以修改此分频值来校准晶振 */ 
    RTC_EnterConfigMode();/// 允许配置			
    if (i < 500) //LSE不能正常  
        RTC_SetPrescaler(32767); 
		/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */  
    else  
        RTC_SetPrescaler(62499); //8M / 128 = 62500，则这里应该填为62499  
  
    /* Wait until last write operation on RTC registers has finished */  
    /* 等待上一次对RTC寄存器的写操作完成 */  
    RTC_WaitForLastTask(); 
    RTC_ExitConfigMode(); //退出配置模式  		
}





/*
初始化rtc
*/
u8 RTC_Init(void)
{
	u16 i = 0;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);    //使能PWR和BKP外设时钟   
  PWR_BackupAccessCmd(ENABLE);                                                //使能后备寄存器访问 
	//检查是不是第一次配置时钟 
  if (BKP_ReadBackupRegister(BKP_DR1) != 0x5AA5)                              //从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
  {
		BKP_DeInit();                                                           //复位备份区域    
    RCC_LSEConfig(RCC_LSE_ON);                                              //设置外部低速晶振(LSE),使用外设低速晶振
        
		/* Wait till LSE is ready */  
		/* 等待外部晶振震荡稳定输出 */  
		for (i = 0;i < 500;i++) //500次检测，如果LSE
		//仍然没有起振，证明这玩意有问题，跳出循环  
		{
			if (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
				break;
			delay_ms(10); //10ms延时  
		}
		COM_DBG("LSE oscillation: %dms\r\n",i*10);
		if (i >= 500)
		{
			//使用外部高速晶振 128分频  
			RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
			COM_DBG("using HSE\r\n");
		}else{  
			/* Select LSE as RTC Clock Source */  
			/*使用外部32.768KHz晶振作为RTC时钟 */                           
			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
			COM_DBG("using LSE\r\n");
		}
		/* Enable RTC Clock */  
		/* 使能 RTC 的时钟供给 */  
		RCC_RTCCLKCmd(ENABLE);  
			
		/* Wait for RTC registers synchronization */  
		/*等待RTC寄存器同步 */  
		RTC_WaitForSynchro();  
			
		/* Wait until last write operation on RTC registers has finished */  
		/* 等待上一次对RTC寄存器的写操作完成 */  
		RTC_WaitForLastTask();  
				
		/* Set RTC prescaler: set RTC period to 1sec */  
		/* 32.768KHz晶振预分频值是32767,
		如果对精度要求很高可以修改此分频值来校准晶振 */ 
		RTC_EnterConfigMode();/// 允许配置			
		if (i < 500) //LSE不能正常 
			RTC_SetPrescaler(32767); 
		/* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */  
		else
			RTC_SetPrescaler(62499); //8M / 128 = 62500，则这里应该填为62499  
			
		/* Wait until last write operation on RTC registers has finished */  
		/* 等待上一次对RTC寄存器的写操作完成 */  
		RTC_WaitForLastTask();
    Clock_SetTime(2017,2,18,15,30,50,8);          //设置时间  
    RTC_ExitConfigMode();                       //退出配置模式  
    BKP_WriteBackupRegister(BKP_DR1, 0x5AA5);   //向指定的后备寄存器中写入用户程序数据
  }else{
		RTC_WaitForSynchro();
  }
	
	RTC_ITConfig(RTC_IT_SEC, ENABLE);   //使能RTC秒中断
  RTC_WaitForLastTask();              //等待最近一次对RTC寄存器的写操作完成                         
  return 0;
}


//设置时间
void Clock_SetTime(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec, s8 zone)
{
    time_t unix_time;     //定义time_t类型的设置时间结构体
    struct tm tm_Set_Time;      //定义tm结构的设置时间结构体

    tm_Set_Time.tm_year = (u32)(syear-1900); //从1900年算起
    tm_Set_Time.tm_mon  = (u32)(smon-1);     //月
    tm_Set_Time.tm_mday = (u32)sday;         //日
    tm_Set_Time.tm_hour = (u32)hour;         //时
    tm_Set_Time.tm_min  = (u32)min;          //分
    tm_Set_Time.tm_sec  = (u32) sec;         //秒

    unix_time = mktime(&tm_Set_Time);   //取得计数初始值
	
	  COM_DBG("unix_timestamp: %d\r\n", unix_time);
	
	  unix_time -= 60*60*zone;            //时区偏差校正
	
    if(unix_time!=0xFFFFFFFF)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);    //使能PWR&BKP外设时钟  
        PWR_BackupAccessCmd(ENABLE);        //使能RTC和后备寄存器访问 
        RTC_SetCounter(unix_time);    //设置RTC计数器值

        RTC_WaitForLastTask();              //等待最后一次对RTC寄存器写操作完成
    }
}


//读取时间 日历形式
u8 Clock_Get(struct tm *local)
{
	time_t timestamp;

  timestamp = RTC_GetCounter();                     //读取RTC计数器

  local = localtime(&timestamp);                      //把UNIX转换成日历时间
	return 0;
}


//读取时间 UNIX形式
u32 Unix_Get(void)
{
    return RTC_GetCounter();                        //返回RTC计数值
}


//更新时间
void RTC_IRQHandler(void)
{        
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)   //秒中断
    {                           
       //to do...
    }
    RTC_ClearITPendingBit(RTC_IT_SEC|RTC_IT_OW);//清中断
    RTC_WaitForLastTask();                                           
}


