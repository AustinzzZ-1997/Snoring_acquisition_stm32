#ifndef _WIFI_H
#define _WIFI_H

#include "sys.h"
#include "4g_uart.h"

extern u32 timestamp;
extern void wifi_Init(void);
u8 getTime(void);
u8 getTimeFromNTPServer(void);
u8 wifi_settansper(void);
void wifi_esctansper(void);
	
#endif
