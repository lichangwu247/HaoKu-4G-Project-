#include "sys.h"
#include "usart1.h"	
////////////////////////////////////////////////////////////////////////////////// 	 
//���ʹ��os,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "FreeRTOS.h"					//FreeRTOSʹ��	  
#endif


//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;
FILE __stdin;
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 

void _ttywrch(int ch)
{
    ch=ch;
}

int _ferror(FILE *f) {  
  /* Your implementation of ferror */  
  return EOF;  
} 

//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}

#endif 

 
 
  
void uart1_init(u32 bound)
{
	//GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	USART_DeInit(USART1);
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
	//USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 4;//��ռ���ȼ�3
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

	USART_Init(USART1, &USART_InitStructure); //��ʼ������1
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
  //USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);//�������ڿ����ж�
	//USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);   //�򿪴���1DMA����
	USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ���1
//  USART_ITConfig(USART1, USART_IT_IDLE, DISABLE);//�������ڿ����ж�
	//DMA_Config(DMA1_Channel5, (u32)&USART1->DR, (u32)USART1_RX_BUF, USART1_REC_LEN);//����DMA
}



/*******************************************************************************
* ������  : USART_SendData
* ����    : USART1�������ݻ���������
* ����    : *buff�����ݻ�����ָ�룬len���������ݳ���
* ���    : ��
* ����    : �� 
* ˵��    : ��
*******************************************************************************/
void USART1_SendData(u8* buff, u16 len)
{    
	u16 i;
	USART_GetFlagStatus(USART1, USART_FLAG_TC);
	for(i=0; i<len; i++)  
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); 
	    USART_SendData(USART1, buff[i]);   
	}
}


void USART1_SendByte(u8 byte)
{    
	USART_GetFlagStatus(USART1, USART_FLAG_TC);
	
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC)==RESET); 
	   USART_SendData(USART1, byte);   
}



#if EN_USART1_RX   //���ʹ���˽���
//����3�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART1_RX_BUF[USART1_REC_LEN];     //���ջ���,���USART3_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART1_RX_STA = 0;       //����״̬���	  

void USART1_IRQHandler(void)                	//����3�жϷ������
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�
	{
  		 
  }

	if(USART_GetITStatus(USART1, USART_IT_PE | USART_IT_FE | USART_IT_NE) != RESET)//����
  {
		USART_ClearITPendingBit(USART1, USART_IT_PE | USART_IT_FE | USART_IT_NE);
  }		
} 
#endif	




