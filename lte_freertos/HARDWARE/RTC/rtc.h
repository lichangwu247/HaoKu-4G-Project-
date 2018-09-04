#ifndef __RTC_H
#define __RTC_H
#include "sys.h"
#include "time.h"	

u8 RTC_Init(void);
void Clock_SetTime(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec, s8 zone);
u8 Clock_Get(struct tm *local);
u32 Unix_Get(void);
#endif
