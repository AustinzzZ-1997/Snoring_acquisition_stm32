#include "4g_uart.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "timer.h"
vu8 uart4_tx_flag;
static void _UART4_DMA_TX_Config(void);

void uart4_init(u32 bound)
{
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 	//ʹ��GPIOAʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);	//ʹ��USART4ʱ��
 
	//����4��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_UART4); 	//GPIOA0����ΪUSART4_RX
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_UART4); 	//GPIOA1����ΪUSART4_TX
	
	//USART4�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1; 	//GPIOA0��GPIOA1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 			//����
	GPIO_Init(GPIOA,&GPIO_InitStructure); 					//��ʼ��PA0��PA1

   //USART4 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;				//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;	//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;		//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(UART4, &USART_InitStructure); 				//��ʼ������4
	
	USART_Cmd(UART4, ENABLE);  								//ʹ�ܴ���4
	
#if UART4_RX_EN  //���ʹ���˽���,�������ý����ж�
//	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);			//���������ж�

	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;		//����4�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;	//��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							//����ָ���Ĳ�����ʼ��NVIC�Ĵ�����
#endif

#if EN_UART4_DMA_TX
//	����ʹ�ô��ڵ������жϣ�ֱ��ʹ��DMA��������ж�
//	/* Enable the USART1 Interrupt */
//	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;//��ռ���ȼ�
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	USART_ClearFlag(USART1, USART_FLAG_TC);				//������ڷ�����ɱ�־λ
//	USART_ITConfig(USART1, USART_IT_TC, ENABLE);		// ʹ�ܴ��ڷ�������ж�	
	
	_UART4_DMA_TX_Config();								//����4 DMA����
	USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);		//ʹ��UART4��DMA���ͽӿ�
#endif

}

#if EN_UART4_DMA_TX		//���ʹ����DMA��ʽ����

u8 UART_DMA_TX_BUF[UART_DMA_TX_LEN];	//DMA���ͻ���

static void _UART4_DMA_TX_Config(void)
{
	DMA_InitTypeDef  DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);			//DMA1ʱ��ʹ��
	
	//���֪UART4_TX��Ӧ��DMA1_Stream4,ͨ��4
	DMA_Cmd(DMA1_Stream4, DISABLE);									//�ر�DMA����
	DMA_DeInit(DMA1_Stream4);										//����DMA1����
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;  				//ͨ��ѡ��
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&UART4->DR;		//DMA�����ַ
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&UART_DMA_TX_BUF;			//DMA �洢��0��ַUART4_TX_BUF
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;			//�洢��������ģʽ
	DMA_InitStructure.DMA_BufferSize = UART4_MAX_SEND_LEN;						//���ݴ����� 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//���������ģʽ
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;			//�洢������ģʽ
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݳ���:8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	//�洢�����ݳ���:8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;					// ʹ����ͨģʽ 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;			//�е����ȼ�
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;		//�洢��ͻ�����δ���
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//����ͻ�����δ���
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);						//��ʼ��DMA Stream
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;		//��ռ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);					//����DMA������ɺ�����ж�
		
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
		if((UART4_RX_STA&0x8000)==0)		//�������δ��ɣ����ʱ���λΪ1
		{
			if(UART4_RX_STA<UART4_MAX_RECV_LEN)	//�����Խ�������
			{
				TIM_SetCounter(TIM7,0);
				if(UART4_RX_STA==0) 				//ʹ�ܶ�ʱ��7���ж� 
				{
//					TIM_SetCounter(TIM7,0);			//���������
					TIM_Cmd(TIM7,ENABLE);			//ʹ�ܶ�ʱ��7
				}
				UART4_RX_BUF[UART4_RX_STA++]=res;	//��¼���յ���ֵ	 
			}else 
			{
				TIM_Cmd(TIM7,DISABLE);				//����������ȣ�����ǰ�رճ�ʱ��ʱ��
				UART4_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
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
	DMA_SetCurrDataCounter(DMA1_Stream4, len);	//����Ҫ������ֽ���
	DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
	DMA_Cmd(DMA1_Stream4, ENABLE);				//����һ��DMA����
//	while(uart4_tx_flag==0);					//�ȴ�DMA�������
//	uart4_tx_flag = 0;							//���÷��ͱ��
}
