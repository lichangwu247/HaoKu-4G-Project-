#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"

void Timer2_Init_Config(u16 arr,u16 psc);
void Timer2_Start(void);
void Timer2_Stop(void);

void TIM3_Int_Init(u16 arr,u16 psc);
void TIM5_Int_Init(u16 arr,u16 psc);
#endif
