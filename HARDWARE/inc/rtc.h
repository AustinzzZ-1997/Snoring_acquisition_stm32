#ifndef _RTC_H
#define _RTC_H

#include "sys.h"

u8 RTC_LSI_Init(u32 timestamp);
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week);
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm);
void PrintTime(void);
u8 RTC_HSE_Init(u32 timestamp);

#endif
