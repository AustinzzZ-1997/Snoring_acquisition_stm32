#include "sys.h"
#include "rtc.h"


RCC_ClocksTypeDef RCC_Clocks;
//static __IO uint32_t uwTimingDelay;
static uint16_t fac_us = 0;			//us延时倍乘数,在SYS_Init中进行初始化
static void uart_init(u32 bound);
	
void SYS_Init(void)
{
/*
	系统整体时钟已经在system_stm32f4xx.c文件中配置，系统时钟为168Mhz，I2S时钟为192Khz
	这里配置systick时钟，以及配置串口打印
*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//设置中断组2
	RCC_GetClocksFreq(&RCC_Clocks);						//获取系统的时钟
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK); 
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);	//配置systick为1ms中断
	fac_us = RCC_Clocks.HCLK_Frequency / 1000000;		//设置us时延倍乘数
	
	uart_init(921600);									//串口1配置
	//RTC_LSI_Init(1559198510);							//RTC配置(随便给的数值，时间是不准的)
	
	printf("系统频率：%dHz\r\n",RCC_Clocks.SYSCLK_Frequency);
}

/* ****************** 延时相关 ********************
*/

//延时n微秒
void delay_us(uint32_t nus)
{
	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;				//获取重载值
	ticks = nus * fac_us;
	told = SysTick->VAL;							//获取当前计数值
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


/* 延时n毫秒 */
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

/* **************** 串口打印相关 *********************
*/
#ifndef __MICROLIB
#pragma import(__use_no_semihosting)
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR=(u8)ch;      
	return ch;
}

#endif


//初始化IO 串口1 
//bound:波特率
static void uart_init(u32 bound)
{
   //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); 		//使能GPIOA时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);		//使能USART1时钟
 
	//串口1对应引脚复用映射
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 	//GPIOA9复用为USART1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1); 	//GPIOA10复用为USART1
	
	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; 	//GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;				//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 				//推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 				//上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); 						//初始化PA9，PA10

   //USART1 初始化设置
	USART_InitStructure.USART_BaudRate = bound;					//波特率设置
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;			//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_Init(USART1, &USART_InitStructure); 					//初始化串口1
	USART_Cmd(USART1, ENABLE);  								//使能串口1 
	
#if EN_USART1_RX   //如果使能了接收,则需配置接收中断
//	USART_ClearFlag(USART1, USART_FLAG_TC);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);				//开启接收中断
	
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;			//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;		//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;			//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);								//根据指定的参数初始化NVIC寄存器、
#endif
	
}

#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	

void USART1_IRQHandler(void)                				//串口1中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS 										//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntEnter();    
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  	//接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);						//(USART1->DR);		//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)						//接收未完成
		{
			if(USART_RX_STA&0x4000)							//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;				//接收错误,重新开始
				else USART_RX_STA|=0x8000;					//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  } 
#if SYSTEM_SUPPORT_OS 	//如果SYSTEM_SUPPORT_OS为真，则需要支持OS.
	OSIntExit();  											 
#endif
}
#endif



//使用V6编译器(clang)
#if defined(__clang__)
//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
void __attribute__((noinline)) WFI_SET(void)
{
    __asm__("wfi");
}

//关闭所有中断(但是不包括fault和NMI中断)   
void __attribute__((noinline)) INTX_DISABLE(void)
{
    __asm__("cpsid i \t\n"
            "bx lr");
}

//开启所有中断
void __attribute__((noinline)) INTX_ENABLE(void)
{
    __asm__("cpsie i \t\n"
            "bx lr");
}

//设置栈顶地址
//addr:栈顶地址
void __attribute__((noinline)) MSR_MSP(u32 addr) 
{
    __asm__("msr msp, r0 \t\n"
            "bx r14");
}
#elif defined (__CC_ARM)    //使用V5编译器(ARMCC)

//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//关闭所有中断(但是不包括fault和NMI中断)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//开启所有中断
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
#endif
