#include "led.h" 
	    
//IO初始化
void LED_Init(void)
{   
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	//LED初始化设置
	//LED0(PB4)/LED1(PB5)/LEDB(PB6)/LEDW(PB7)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//初始化
	GPIO_SetBits(GPIOB,GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);//设置高，灯灭
	
	//充电控制IO初始化设置
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//使能GPIOA时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//开漏输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//悬空
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//初始化
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);					//GPIOA4设置低，置低充电，悬空关断
	
	//电源
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;	//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//初始化
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);//GPIOB11设置低，电源导通
}
