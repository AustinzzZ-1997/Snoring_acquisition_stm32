#include "timer.h"

extern vu16 UART4_RX_STA;
extern u8 i;

//定时器7中断服务程序(用于WiFi接收过程中的超时判断)		    
void TIM7_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)	//是更新中断
	{	 			   
		UART4_RX_STA |= 1<<15;							//标记接收完成
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update  );  	//清除TIM7更新中断标志    
		TIM_Cmd(TIM7, DISABLE);  						//关闭TIM7 
	}	    
}




//用于接收过程中超时判断
void TIM7_Timeout_Init(u16 arr, u16 psc)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);//TIM7时钟使能    
	
	//定时器TIM7初始化
	TIM_TimeBaseStructure.TIM_Period = arr; 					//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 					//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 	//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //TIM向上计数模式
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure); 			//根据指定的参数初始化TIMx的时间基数单位
	TIM_ITConfig(TIM7,TIM_IT_Update,ENABLE ); 					//使能指定的TIM7中断,允许更新中断
	
	TIM_Cmd(TIM7,DISABLE);//关闭定时器7
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;	//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;			//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);								//根据指定的参数初始化VIC寄存器

#ifdef MY_DEBUG
	printf("接收超时时间：%dms\r\n", 168000000/(arr+1)/(psc+1));
#endif
}



void TIM5_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5,ENABLE);  ///使能TIM3时钟
	
  TIM_TimeBaseInitStructure.TIM_Period = arr; 	//自动重装载值
	TIM_TimeBaseInitStructure.TIM_Prescaler=psc;  //定时器分频
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; //向上计数模式
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 

	TIM_TimeBaseInit(TIM5,&TIM_TimeBaseInitStructure);//初始化TIM3
	
	TIM_ITConfig(TIM5,TIM_IT_Update,ENABLE); //允许定时器3更新中断
	TIM_Cmd(TIM5,ENABLE); //使能定时器3

	NVIC_InitStructure.NVIC_IRQChannel=TIM5_IRQn; //定时器3中断	
	
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01; //抢占优先级1
	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x02; //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	
}

//定时器3中断服务函数
void TIM5_IRQHandler(void)
{

	if(TIM_GetITStatus(TIM5,TIM_IT_Update)==SET) //溢出中断
	{
		i++;
	}
	TIM_ClearITPendingBit(TIM5,TIM_IT_Update);  //清除中断标志位
}
