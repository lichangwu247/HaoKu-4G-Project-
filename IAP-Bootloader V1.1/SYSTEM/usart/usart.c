#include "sys.h"
#include "usart.h"	  

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
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 

void uart_init(u32 bound){
  GPIO_InitTypeDef  GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/*ʹ��USART3����ʱ��*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	
	/*��λ����3*/
 	USART_DeInit(USART1);  
	
	/*USART1_GPIO��ʼ������*/ 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;			//USART1_TXD(PA.9)     
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);				//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_TXD(PA.9)  
   
    
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//USART1_RXD(PA.10)
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;	//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);					//���ÿ⺯���е�GPIO��ʼ����������ʼ��USART1_RXD(PA.10)


   /*USART1 ��ʼ������*/
	USART_InitStructure.USART_BaudRate = bound;										//���ò�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						//8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							//1��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					//����ģʽ����Ϊ�շ�ģʽ
  USART_Init(USART1, &USART_InitStructure);										//��ʼ������3

  USART_Cmd(USART1, ENABLE);                    			//ʹ�ܴ��� 
	USART_ClearFlag(USART1, USART_FLAG_TC);					//���������ɱ�־,��ֹ���ֽڷ�����ȥ
}

