#include "sys.h"
#include "usart2.h"	
#include "timer.h"	

RINGBUF Mobile_RxRingBuff;
u8 USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART2_REC_LEN���ֽ�.

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��	
#endif


void uart2_init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	/*��λ����2*/
 	USART_DeInit(USART2);	
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//ʹ��USART2��GPIOAʱ��
  
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//USART2_TXD(PA.2)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.2
   
	//USART2_RXD(PA.3)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;//PA.3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.3  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
	//USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART2, &USART_InitStructure); //��ʼ������2
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);			//ʹ�ܴ���2�����ж�
	USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2

	USART_ClearFlag(USART2, USART_FLAG_TC);					//���������ɱ�־,��ֹ���ֽڷ�����ȥ
	
	RINGBUF_Init(&Mobile_RxRingBuff, USART2_RX_BUF, sizeof(USART2_RX_BUF));
}



/*******************************************************************************
* ������  : USART2_SendData
* ����    : USART2�������ݻ���������
* ����    : *buff�����ݻ�����ָ�룬len���������ݳ���
* ���    : ��
* ����    : �� 
* ˵��    : ��
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

void USART2_IRQHandler(void)                	//����2�жϷ������
{
  u8  data;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�һ֡����
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
	
	if(USART_GetITStatus(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//����
  {
		USART_ClearITPendingBit(USART2, USART_IT_PE | USART_IT_FE | USART_IT_NE);
  }
} 



