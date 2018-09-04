/*----------------------------------------------------------------------------

  ----------------------------------------------------------------------------*/
/* 包含的头文件 --------------------------------------------------------------*/
#include "pvd.h"
#include "sys.h"
#include "selector.h"
/************************************************
函数名称 ： RTC_Configuration
功    能 ： RTC配置
参    数 ： 无
返 回 值 ： 无
作    者 ： 
*************************************************/
void PVD_Configuration(void)
{
  EXTI_InitTypeDef EXTI_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);

  /* Configure EXTI Line16(PVD Output) to generate an interrupt on rising and
     falling edges */
  EXTI_ClearITPendingBit(EXTI_Line16); 
  EXTI_InitStructure.EXTI_Line = EXTI_Line16;// PVD连接至EXIT16 
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;//中断模式
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; //高到低电压触发中断//EXTI_Trigger_Falling;//下降沿触发
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;// 使能
  EXTI_Init(&EXTI_InitStructure);

  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Enable the PVD Interrupt */ //设置PVD中断
  NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	  /* Configure the PVD Level to 2.9V */
  PWR_PVDLevelConfig(PWR_PVDLevel_2V9);// 设置监控值
	
	PWR_BackupAccessCmd(ENABLE);                   //允许操作后备区域
	if(BKP_ReadBackupRegister(BKP_DR9) != 0xA5A5)
  {
    BKP_WriteBackupRegister(BKP_DR9, 0xA5A5);
		BKP_WriteBackupRegister(BKP_DR5, 0x0000);
		BKP_WriteBackupRegister(BKP_DR6, 0x0000);
		BKP_WriteBackupRegister(BKP_DR7, 0x0000);
		BKP_WriteBackupRegister(BKP_DR8, 0x0000);
  }	
	
  /* Enable the PVD Output */
  PWR_PVDCmd(ENABLE);// 使能PVD
}


/************************************************
* Function Name  : PVD_IRQHandler
* Description    : This function handles pvd interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None

*************************************************/
void PVD_IRQHandler(void)
{

  if(EXTI_GetITStatus(EXTI_Line16) != RESET)
  {
    //掉电处理，存储关键信息
		COM_DBG("PWROFF......");
		PWR_BackupAccessCmd(ENABLE);                   //允许操作后备区域
		
		BKP_WriteBackupRegister(BKP_DR5, g_service_info.ctrStatus);
		BKP_WriteBackupRegister(BKP_DR6, g_service_info.coin_res);
		BKP_WriteBackupRegister(BKP_DR7, g_service_info.tbout_has);
		BKP_WriteBackupRegister(BKP_DR8, g_service_info.coin_has);

		/* Clear the Key Button EXTI line pending bit */
		EXTI_ClearITPendingBit(EXTI_Line16);
  }
}	
 
extern service  g_service_info;

void backup_read(void)
{
	u16 backup_flag;
	PWR_BackupAccessCmd(ENABLE);                   //允许操作后备区域
	
	backup_flag = BKP_ReadBackupRegister(BKP_DR9);
	
	COM_DBG("backup_flag: %04X\n",backup_flag);
	if(backup_flag == 0xA5A5)
	{
		/* 读取掉电保存信息 */
		g_service_info.ctrStatus = BKP_ReadBackupRegister(BKP_DR5);
		g_service_info.coin_res = BKP_ReadBackupRegister(BKP_DR6);
		g_service_info.tbout_has = BKP_ReadBackupRegister(BKP_DR7);
		g_service_info.coin_has = BKP_ReadBackupRegister(BKP_DR8);
		
		COM_DBG("info.status: %d\n",g_service_info.ctrStatus);
		COM_DBG("info.coin_res: %d\n",g_service_info.coin_res);
		COM_DBG("info.tbout_has: %d\n",g_service_info.tbout_has);
		COM_DBG("info.coin_has: %d\n",g_service_info.coin_has);
	}
}



/**** Copyright (C)2016  All Rights Reserved **** END OF FILE ****/
