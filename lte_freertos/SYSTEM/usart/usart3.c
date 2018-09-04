#include "sys.h"
#include "usart3.h"	
#include "dma.h"

////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��	  
#endif

extern int device_data_filter(u8* msg, u16 len);
 
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART3_RX_BUF[USART3_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.

  
void uart3_init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	USART_DeInit(USART3);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//ʹ��USART3��GPIOBʱ��
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	
	//USART3_TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //USART3_TXD(PB.10)   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.10
   
	//USART3_RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;//USART3_RXD(PB.11)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOB, &GPIO_InitStructure);//��ʼ��GPIOB.11  

	//Usart3 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3 ;//��ռ���ȼ�3
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

	USART_Init(USART3, &USART_InitStructure); //��ʼ������3
	//USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//�������ڿ����ж�
	USART_DMACmd(USART3,USART_DMAReq_Rx,DISABLE);   //�򿪴���3DMA����
	USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3
	
	DMA_Config(DMA1_Channel3, (u32)&USART3->DR, (u32)USART3_RX_BUF, USART3_REC_LEN);//����DMA
	
}



/*******************************************************************************
* ������  : USART_SendData
* ����    : USART1�������ݻ���������
* ����    : *buff�����ݻ�����ָ�룬len���������ݳ���
* ���    : ��
* ����    : �� 
* ˵��    : ��
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

void USART3_IRQHandler(void)                	//����1�жϷ������
{
	u16 usart3_rec_cnt = 0;
	//USART_ITConfig(USART3, USART_IT_IDLE, DISABLE);//�رմ��ڿ����ж�
	if(USART_GetITStatus(USART3,USART_IT_IDLE) != RESET)//���յ�һ֡����
  {  
		//���IDLE�ж�
    USART_ReceiveData(USART3);

    usart3_rec_cnt =USART3_REC_LEN - DMA_GetCurrDataCounter(DMA1_Channel3); //���㱾֡���ݳ���

		device_data_filter(USART3_RX_BUF, usart3_rec_cnt);

    USART_ClearITPendingBit(USART3,USART_IT_IDLE);     //����жϱ��
    DMA_Enable(DMA1_Channel3, USART3_REC_LEN);                       //�ָ�DMAָ��,�ȴ���һ��*/
  }
	
	if(USART_GetITStatus(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//����
  {
		USART_ClearITPendingBit(USART3, USART_IT_PE | USART_IT_FE | USART_IT_NE);
  }
	
//	USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);//�������ڿ����ж�
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




