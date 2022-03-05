#ifndef _NTP_H
#define _NTP_H

#include "sys.h"

typedef struct {
	u32  integer;
    u32  fraction;
} NTP_Time;

//48个字节
typedef struct {
	u32 control;
	u32 root_delay;
	u32 root_dispersion;
	u32 reference_identifier;
    NTP_Time reference_timestamp;
    NTP_Time originate_timestamp;		//离开客户端时的时间
    NTP_Time receive_timestamp;			//服务器接收到报文的时间
    NTP_Time transmit_timestamp;		//离开服务器时的时间（可认为是当前时间，忽略传输时间）
} NTP_Packet;

//32位数据的大小端转换
#define BLSWAP(N)	((((unsigned int)(N) & 0xff000000) >> 24) | \
                    (((unsigned int)(N) & 0x00ff0000) >> 8) | \
                    (((unsigned int)(N) & 0x0000ff00) << 8) | \
                    (((unsigned int)(N) & 0x000000ff) << 24))

					
u8 NTP_Req(void);
u8 timeUpdate(void);
					
#endif
