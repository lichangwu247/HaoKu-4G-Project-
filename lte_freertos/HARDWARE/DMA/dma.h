#ifndef __DMA_H
#define	__DMA_H	   
#include "sys.h"


void DMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);//����DMA1_CHx

void DMA_Enable(DMA_Channel_TypeDef*DMA_CHx, u16 cndtr);//ʹ��DMA1_CHx
		   
#endif




