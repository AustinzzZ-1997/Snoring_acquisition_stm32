#include "key.h"



//按键初始化函数
void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//使能GPIOB时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 		//KEY对应引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	//普通输入模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	//上拉
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//初始化GPIOB11

	
}

/*获取按键情况
* mode:
* 	0:只判断是否按下，反应时间快
*	1:判断按下时间长短，可实现多状态检测(长按需等按键松开后返回)
*/
u8 KEY_Scan(u8 mode)
{
	u8 i = 60;
	u8 key_up = 0, key_down = 0;	//弹起次数和按下次数
	if(KEY == PRESS_LEVEL)					//如果有按下
	{
		delay_ms(10);	//消抖
		if(KEY == PRESS_LEVEL)	
		{
			if(!mode) return KEY_PRESS;
			while(i--)
			{
				delay_ms(20);
				key_down ++;		//按下次数加1，代表按下时间
				if(KEY != PRESS_LEVEL)		//如果中途松开
				{
					key_up ++;		//弹起次数加1
					if(key_up>3)	//次数过多，则代表已经结束本次按键事件
					{
						if(key_down < 30)
							return KEY_PRESS;
						else
							return KEY_SHORT;
					}
				}
			}
			while(KEY == PRESS_LEVEL);				//等到松开才返回
			return KEY_LONG;
		}
	}
	return 0;
}

