#ifndef _4G_UART_H
#define _4G_UART_H

#include "sys.h"

#define UART4_RX_EN 		1
#define EN_UART4_DMA_TX		1

#define UART4_MAX_SEND_LEN		100
#define UART4_MAX_RECV_LEN		100
#define UART_DMA_TX_LEN      4000

extern vu16 UART4_RX_STA;
extern u8 UART_DMA_TX_BUF[UART_DMA_TX_LEN];	//DMA·¢ËÍ»º³å
extern void uart4_init(u32 bound);
//extern void UART4_DMA_TX_Config(void *memaddr, u32 txlen);
extern void u4_printf(char* fmt,...);
void UART4_DMA_Send(u16 len);

extern u8  UART4_RX_BUF[UART4_MAX_RECV_LEN]; 			
extern u8  UART4_TX_BUF[UART4_MAX_SEND_LEN]; 

#endif
