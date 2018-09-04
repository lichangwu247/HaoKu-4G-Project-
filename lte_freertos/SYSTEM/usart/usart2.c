#include "sys.h"
#include "usart2.h"	
#include "timer.h"	

RINGBUF Mobile_RxRingBuff;
u8 USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART2_REC_LEN个字节.

////////////////////////////////////////////////////////////////////////////////// 	 
//如果使用os,则包括下面的头文件即可.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOS使用	
#endif


void uart2_init(u32 bound)
{
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*复位串口2*/
 	USART_DeInit(USART2);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART2，GPIOA时钟
  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//USART2_TXD(PA.2)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.2
   
	//USART2_RXD(PA.3)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA.3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.3  

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级3
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
	USART_Init(USART2, &USART_InitStructure); //初始化串口2
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			//使能串口2接收中断
	USART_Cmd(USART2, ENABLE);                    //使能串口2

	USART_ClearFlag(USART2, USART_FLAG_TC);					//清除发送完成标志,防止首字节发不出去
	
	RINGBUF_Init(&Mobile_RxRingBuff, USART2_RX_BUF, sizeof(USART2_RX_BUF));
}



/*******************************************************************************
* 函数名  : USART2_SendData
* 描述    : USART2发送数据缓冲区数据
* 输入    : *buff：数据缓冲区指针，len：发送数据长度
* 输出    : 无
* 返回    : 无 
* 说明    : 无
*******************************************************************************/
void USART2_SendData(u8* buff, u16 len)
{    
	u16 i;
	USART_GetFlagStatus(USART2, USART_FLAG_TC);
	for(i=0; i<len; i++)  
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET); 
	    USART_SendData(USART2 ,buff[i]);	
	}
}


void USART2_SendByte(u8 byte)
{    
	USART_GetFlagStatus(USART2, USART_FLAG_TC);
	
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET); 
	   USART_SendData(USART2 ,byte);   
}



void USART2_PutString (uint8_t *s) 
{
   while(*s)
	{
		USART2_SendByte(*s++);
	}
}

void USART2_IRQHandler(void)                	//串口2中断服务程序
{
  u8  data;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到一帧数据
  {  
    data = USART_ReceiveData(USART2);
		RINGBUF_Put(&Mobile_RxRingBuff, data);
/*		if(RINGBUF_Put(&Mobile_RxRingBuff, data)!=0)
		{
		  DBG_STRING("ring buffer is full!!\r\n", 24);
			RINGBUF_Clear(&Mobile_RxRingBuff);
		}
		*/
		DBG_CHR(data);//debug
		Timer2_Start();
  }
	
	if(USART_GetITStatus(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//出错
  {
		USART_ClearITPendingBit(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE);
  }
} 



