#ifndef _TIMER_H
#define _TIMER_H

#include "sys.h"

extern void TIM7_Timeout_Init(u16 arr, u16 psc);
extern void TIM5_Int_Init(u16 arr, u16 psc);
extern u8 TIM7_flag;
#endif
