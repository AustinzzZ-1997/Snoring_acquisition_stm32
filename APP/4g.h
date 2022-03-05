#ifndef _4G_H
#define _4G_H

#include "sys.h"
#include "4g_uart.h"


extern u8 FourG_Init(void);
extern u8 send_cmd(char *cmd, char *ack, u16 timeout, u8 crlf,u8 hex, char *endptr);

u8 FourG_Reset(void);
u8 enterAT(void);
u8 quitAT(void);
u8 enterATT(void);
u8 FourG_SetSOCK(char *sock, char *pto, char *addr, u16 port, u8 mode);

#endif
