#ifndef _NTP_H
#define _NTP_H

#include "sys.h"

typedef struct {
	u32  integer;
    u32  fraction;
} NTP_Time;

//48���ֽ�
typedef struct {
	u32 control;
	u32 root_delay;
	u32 root_dispersion;
	u32 reference_identifier;
    NTP_Time reference_timestamp;
    NTP_Time originate_timestamp;		//�뿪�ͻ���ʱ��ʱ��
    NTP_Time receive_timestamp;			//���������յ����ĵ�ʱ��
    NTP_Time transmit_timestamp;		//�뿪������ʱ��ʱ�䣨����Ϊ�ǵ�ǰʱ�䣬���Դ���ʱ�䣩
} NTP_Packet;

//32λ���ݵĴ�С��ת��
#define BLSWAP(N)	((((unsigned int)(N) & 0xff000000) >> 24) | \
                    (((unsigned int)(N) & 0x00ff0000) >> 8) | \
                    (((unsigned int)(N) & 0x0000ff00) << 8) | \
                    (((unsigned int)(N) & 0x000000ff) << 24))

					
u8 NTP_Req(void);
u8 timeUpdate(void);
					
#endif
