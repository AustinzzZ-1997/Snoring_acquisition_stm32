#include "4g_uart.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
vu8 uart4_tx_flag;
static void _UART4_DMA_TX_Config(void);

void uart4_init(u32 bound)
{
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 	//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);	//使能USART4时钟
 
	//串口4对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_UART4); 	//GPIOA0复用为USART4_RX
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_UART4); 	//GPIOA1复用为USART4_TX
	
	//USART4端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; 	//GPIOA0与GPIOA1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 			//上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); 					//初始化PA0，PA1

   //USART4 初始化设置
	USART_InitStructure.USART_BaudRate = bound;				//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;		//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(UART4, &USART_InitStructure); 				//初始化串口4
	
	USART_Cmd(UART4, ENABLE);  								//使能串口4
	
#if UART4_RX_EN  //如果使能了接收,则需配置接收中断
//	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);			//开启接收中断

	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;		//串口4中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;	//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							//根据指定的参数初始化NVIC寄存器、
#endif

#if EN_UART4_DMA_TX
//	不用使用串口单独的中断，直接使用DMA传输完成中断
//	/* Enable the USART1 Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//抢占优先级
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	USART_ClearFlag(USART1, USART_FLAG_TC);				//清除串口发送完成标志位
//	USART_ITConfig(USART1, USART_IT_TC, ENABLE);		// 使能串口发送完成中断	
	
	_UART4_DMA_TX_Config();								//串口4 DMA配置
	USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);		//使能UART4的DMA发送接口
#endif

}

#if EN_UART4_DMA_TX		//如果使能了DMA方式发送

u8 UART_DMA_TX_BUF[UART_DMA_TX_LEN];	//DMA发送缓冲

static void _UART4_DMA_TX_Config(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);			//DMA1时钟使能
	
	//查表知UART4_TX对应着DMA1_Stream4,通道4
	DMA_Cmd(DMA1_Stream4, DISABLE);									//关闭DMA传输
	DMA_DeInit(DMA1_Stream4);										//重置DMA1配置
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  				//通道选择
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR;		//DMA外设地址
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&UART_DMA_TX_BUF;			//DMA 存储器0地址UART4_TX_BUF
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;			//存储器到外设模式
	DMA_InitStructure.DMA_BufferSize = UART4_MAX_SEND_LEN;						//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;			//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	//存储器数据长度:8位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;					// 使用普通模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;			//中等优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;		//存储器突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//外设突发单次传输
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);						//初始化DMA Stream
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);					//配置DMA发送完成后产生中断
		
}
#endif


vu16 UART4_RX_STA = 0;
u8  UART4_RX_BUF[UART4_MAX_RECV_LEN]; 				
u8  UART4_TX_BUF[UART4_MAX_SEND_LEN]; 


void u4_printf(char* fmt,...)  
{  
	u16 i,j;
	va_list ap;
	va_start(ap,fmt);
	vsprintf((char*)UART4_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)UART4_TX_BUF);
	for(j=0;j<i;j++)
	{
		while(USART_GetFlagStatus(UART4,USART_FLAG_TC)==RESET);
		USART_SendData(UART4,(uint8_t)UART4_TX_BUF[j]);
	}
}


void UART4_IRQHandler(void)
{
	u8 res;
	u16 t;

	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		res =USART_ReceiveData(UART4);
//		printf("%c",res);
		if((UART4_RX_STA&0x8000)==0)		//如果接收未完成，完成时最高位为1
		{
			if(UART4_RX_STA<UART4_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM_SetCounter(TIM7,0);
				if(UART4_RX_STA==0) 				//使能定时器7的中断 
				{
//					TIM_SetCounter(TIM7,0);			//计数器清空
					TIM_Cmd(TIM7,ENABLE);			//使能定时器7
				}
				UART4_RX_BUF[UART4_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				TIM_Cmd(TIM7,DISABLE);				//如果超出长度，则提前关闭超时定时器
				UART4_RX_STA|=1<<15;				//强制标记接收完成
			}
		}
//		else 
//		{
//		  for(t=0;t<(UART4_RX_STA&0x3fff);t++)
//			printf("%c",UART4_RX_BUF[t]);
//			UART4_RX_STA=0;
//		}
		
	}
}


//void DMA1_Stream4_IRQHandler(void)
//{
//	if(DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4)==SET)
//	{
//		DMA_Cmd(DMA1_Stream4, DISABLE);
//		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
//		//printf("\r\nEOL\r\n");
//		uart4_tx_flag = 1;
//	}
//}

void UART4_DMA_Send(u16 len)
{
	UART4_RX_STA = 0;
	DMA_SetCurrDataCounter(DMA1_Stream4, len);	//设置要传输的字节数
	DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
	DMA_Cmd(DMA1_Stream4, ENABLE);				//开启一次DMA传输
//	while(uart4_tx_flag==0);					//等待DMA传输完成
//	uart4_tx_flag = 0;							//重置发送标记
}
