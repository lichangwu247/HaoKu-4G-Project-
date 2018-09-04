#include "dma.h"


/*
*DMA �봮�ڶ�Ӧ��ϵ
*UART1-TX--->DMA1_Channel4    UART1-RX--->DMA1_Channel5
*UART2-TX--->DMA1_Channel7    UART2-RX--->DMA1_Channel6
*UART3-TX--->DMA1_Channel2    UART3-RX--->DMA1_Channel3
*UART4-TX--->DMA2_Channel5    UART4-RX--->DMA2_Channel2
*/

//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�˴���Դ��ڽ��ս�������
//������->�洢��ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_CHx:DMAͨ��CHx
//cpar:�����ַ
//cmar:�洢����ַ
//cndtr:���ݴ����� 
void DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
	DMA_InitTypeDef DMA_InitStructure;
	
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
  DMA_DeInit(DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ
	
  DMA_InitStructure.DMA_PeripheralBaseAddr = cpar; //DMA�������ַ
  DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���䷽���������ڴ�
  DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA�����С
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //�����ַ����
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ����
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //���ݿ��8
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��8
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//DMA_Mode_Normal;  //����DMAģʽ
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA�����ȼ�
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //���ڴ����ڴ洫��
  DMA_Init(DMA_CHx, &DMA_InitStructure);  //��ʼ��
	
  DMA_Cmd(DMA_CHx, ENABLE);  //����DMAͨ��		
} 



//����һ��DMA����
void DMA_Enable(DMA_Channel_TypeDef*DMA_CHx, u16 cndtr)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //�ر�USART1 TX DMA1 ��ָʾ��ͨ��      
 	DMA_SetCurrDataCounter(DMA_CHx,cndtr);//DMAͨ����DMA����Ĵ�С
 	DMA_Cmd(DMA_CHx, ENABLE);  //ʹ��USART1 TX DMA1 ��ָʾ��ͨ�� 
}	  

 
