# include <stdio.h>
# include <string.h>

typedef unsigned int uint32_t;

typedef unsigned short int uint16_t;

typedef unsigned char uint8_t;

#define POLY16    0xA001

uint16_t calculate_crc16(uint16_t crc,uint8_t *buf, int len)
{														 
	uint8_t i;
	while(len-- != 0)
	{
		crc ^= *buf;
		for(i = 0; i < 8; i++)
		{
			if((crc&0x0001) != 0) {crc>>=1; crc^=POLY16;} 
			else crc>>=1; 
		}
		buf++;
	}
	return crc;
}