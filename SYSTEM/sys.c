#include "sys.h"
#include "rtc.h"


RCC_ClocksTypeDef RCC_Clocks;
//static __IO uint32_t uwTimingDelay;
static uint16_t fac_us = 0;			//us��ʱ������,��SYS_Init�н��г�ʼ��
static void uart_init(u32 bound);
	
void SYS_Init(void)
{
/*
	ϵͳ����ʱ���Ѿ���system_stm32f4xx.c�ļ������ã�ϵͳʱ��Ϊ168Mhz��I2Sʱ��Ϊ192Khz
	��������systickʱ�ӣ��Լ����ô��ڴ�ӡ
*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//�����ж���2
	RCC_GetClocksFreq(&RCC_Clocks);						//��ȡϵͳ��ʱ��
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK); 
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);	//����systickΪ1ms�ж�
	fac_us = RCC_Clocks.HCLK_Frequency / 1000000;		//����usʱ�ӱ�����
	
	uart_init(921600);									//����1����
	//RTC_LSI_Init(1559198510);							//RTC����(��������ֵ��ʱ���ǲ�׼��)
	
	printf("ϵͳƵ�ʣ�%dHz\r\n",RCC_Clocks.SYSCLK_Frequency);
}

/* ****************** ��ʱ��� ********************
*/

//��ʱn΢��
void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;				//��ȡ����ֵ
	ticks = nus * fac_us;
	told = SysTick->VAL;							//��ȡ��ǰ����ֵ
	while(1)
	{
		tnow = SysTick->VAL;
		if(tnow != told)
		{
			if(tnow < told)
				tcnt += told-tnow;
			else
				tcnt += reload- tnow + told;
			told = tnow;
			if(tcnt >= ticks) break;
		}
	}
}


/* ��ʱn���� */
void delay_ms(uint16_t nms)
{
	while( nms-- ) delay_us(1000);
}


///**
//  * @brief  Inserts a delay time.
//  * @param  nTime: specifies the delay time length, in milliseconds.
//  * @retval None
//  */
//void Delay(__IO uint32_t nTime)
//{ 
//  uwTimingDelay = nTime;

//  while(uwTimingDelay != 0);
//}

///**
//  * @brief  Decrements the TimingDelay variable.
//  * @param  None
//  * @retval None
//  */
//void TimingDelay_Decrement(void)
//{
//  if (uwTimingDelay != 0x00)
//  { 
//    uwTimingDelay--;
//  }
//}

/* **************** ���ڴ�ӡ��� *********************
*/
#ifndef __MICROLIB
#pragma import(__use_no_semihosting)
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
	USART1->DR=(u8)ch;      
	return ch;
}

#endif


//��ʼ��IO ����1 
//bound:������
static void uart_init(u32 bound)
{
   //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 		//ʹ��GPIOAʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);		//ʹ��USART1ʱ��
 
	//����1��Ӧ���Ÿ���ӳ��
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 	//GPIOA9����ΪUSART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); 	//GPIOA10����ΪUSART1
	
	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; 	//GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 				//���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 				//����
	GPIO_Init(GPIOA,&GPIO_InitStructure); 						//��ʼ��PA9��PA10

   //USART1 ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;					//����������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;			//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_Init(USART1, &USART_InitStructure); 					//��ʼ������1
	USART_Cmd(USART1, ENABLE);  								//ʹ�ܴ���1 
	
#if EN_USART1_RX   //���ʹ���˽���,�������ý����ж�
//	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);				//���������ж�
	
	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;		//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;			//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);								//����ָ���Ĳ�����ʼ��NVIC�Ĵ�����
#endif
	
}

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	

void USART1_IRQHandler(void)                				//����1�жϷ������
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 										//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  	//�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART1);						//(USART1->DR);		//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)						//����δ���
		{
			if(USART_RX_STA&0x4000)							//���յ���0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;				//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;					//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}   		 
  } 
#if SYSTEM_SUPPORT_OS 	//���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();  											 
#endif
}
#endif



//ʹ��V6������(clang)
#if defined(__clang__)
//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
void __attribute__((noinline)) WFI_SET(void)
{
    __asm__("wfi");
}

//�ر������ж�(���ǲ�����fault��NMI�ж�)   
void __attribute__((noinline)) INTX_DISABLE(void)
{
    __asm__("cpsid i \t\n"
            "bx lr");
}

//���������ж�
void __attribute__((noinline)) INTX_ENABLE(void)
{
    __asm__("cpsie i \t\n"
            "bx lr");
}

//����ջ����ַ
//addr:ջ����ַ
void __attribute__((noinline)) MSR_MSP(u32 addr) 
{
    __asm__("msr msp, r0 \t\n"
            "bx r14");
}
#elif defined (__CC_ARM)    //ʹ��V5������(ARMCC)

//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//�ر������ж�(���ǲ�����fault��NMI�ж�)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//���������ж�
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
#endif
