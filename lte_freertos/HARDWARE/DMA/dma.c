#include "dma.h"


/*
*DMA 与串口对应关系
*UART1-TX--->DMA1_Channel4    UART1-RX--->DMA1_Channel5
*UART2-TX--->DMA1_Channel7    UART2-RX--->DMA1_Channel6
*UART3-TX--->DMA1_Channel2    UART3-RX--->DMA1_Channel3
*UART4-TX--->DMA2_Channel5    UART4-RX--->DMA2_Channel2
*/

//DMA1的各通道配置
//这里的传输形式是固定的,这点要根据不同的情况来修改
//此处针对串口接收进行设置
//从外设->存储器模式/8位数据宽度/存储器增量模式
//DMA_CHx:DMA通道CHx
//cpar:外设地址
//cmar:存储器地址
//cndtr:数据传输量 
void DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
	DMA_InitTypeDef DMA_InitStructure;
	
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
  DMA_DeInit(DMA_CHx);   //将DMA的通道1寄存器重设为缺省值
	
  DMA_InitStructure.DMA_PeripheralBaseAddr = cpar; //DMA外设基地址
  DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //传输方向，外设至内存
  DMA_InitStructure.DMA_BufferSize = cndtr;  //DMA缓冲大小
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址不变
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址自增
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte; //数据宽度8
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度8
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;//DMA_Mode_Normal;  //正常DMA模式
  DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA中优先级
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //无内存至内存传输
  DMA_Init(DMA_CHx, &DMA_InitStructure);  //初始化
	
  DMA_Cmd(DMA_CHx, ENABLE);  //驱动DMA通道		
} 



//开启一次DMA传输
void DMA_Enable(DMA_Channel_TypeDef*DMA_CHx, u16 cndtr)
{ 
	DMA_Cmd(DMA_CHx, DISABLE );  //关闭USART1 TX DMA1 所指示的通道      
 	DMA_SetCurrDataCounter(DMA_CHx,cndtr);//DMA通道的DMA缓存的大小
 	DMA_Cmd(DMA_CHx, ENABLE);  //使能USART1 TX DMA1 所指示的通道 
}	  

 
