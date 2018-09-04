#include "sys.h"
#include "usart3.h"	
#include "dma.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用	  
#endif

extern int device_data_filter(u8* msg, u16 len);
 
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

  
void uart3_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	USART_DeInit(USART3);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能USART3，GPIOB时钟
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	//USART3_TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //USART3_TXD(PB.10)   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.10
   
	//USART3_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//USART3_RXD(PB.11)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);//初始化GPIOB.11  

	//Usart3 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
	//USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//串口波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

	USART_Init(USART3, &USART_InitStructure); //初始化串口3
	//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//开启串口空闲中断
	USART_DMACmd(USART3,USART_DMAReq_Rx,DISABLE);   //打开串口3DMA接收
	USART_Cmd(USART3, ENABLE);                    //使能串口3
	
	DMA_Config(DMA1_Channel3, (u32)&USART3->DR, (u32)USART3_RX_BUF, USART3_REC_LEN);//配置DMA
	
}



/*******************************************************************************
* 函数名  : USART_SendData
* 描述    : USART1发送数据缓冲区数据
* 输入    : *buff：数据缓冲区指针，len：发送数据长度
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void USART3_SendData(u8* buff, u16 len)
{    
	u16 i;
	USART_GetFlagStatus(USART3, USART_FLAG_TC);
	for(i=0; i<len; i++)  
	{
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET); 
	    USART_SendData(USART3, buff[i]);   
	}
}



void USART3_SendByte(u8 byte)
{    
	USART_GetFlagStatus(USART3, USART_FLAG_TC);
	
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC)==RESET); 
	   USART_SendData(USART3 ,byte);   
}

void USART3_IRQHandler(void)                	//串口1中断服务程序
{
	u16 usart3_rec_cnt = 0;
	//USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);//关闭串口空闲中断
	if(USART_GetITStatus(USART3,USART_IT_IDLE) != RESET)//接收到一帧数据
  {  
		//清除IDLE中断
    USART_ReceiveData(USART3);

    usart3_rec_cnt =USART3_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel3); //计算本帧数据长度

		device_data_filter(USART3_RX_BUF, usart3_rec_cnt);

    USART_ClearITPendingBit(USART3,USART_IT_IDLE);     //清除中断标记
    DMA_Enable(DMA1_Channel3, USART3_REC_LEN);                       //恢复DMA指针,等待下一次*/
  }
	
	if(USART_GetITStatus(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//出错
  {
		USART_ClearITPendingBit(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE);
  }
	
//	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//开启串口空闲中断
} 



void uart3_buadRate(u8 rateType)
{
	switch(rateType)
	{
		case 0xB0:
			uart3_init(38400);
		break;
		
		case 0xB1:
			uart3_init(115200);
		break;
		
		default:
			uart3_init(38400);
		break;		
	}	
}




