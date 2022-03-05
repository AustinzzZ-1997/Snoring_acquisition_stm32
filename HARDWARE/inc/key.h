#ifndef _KEY_H
#define _KEY_H
#include "sys.h"



//下面的方式是通过直接操作HAL库函数方式读取IO
#define KEY        	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) //KEY按键PB10
#define PRESS_LEVEL		0						//按键按下的电平

#define KEY_PRESS	1
#define KEY_SHORT	2
#define KEY_LONG	3


void KEY_Init(void);		//IO初始化
u8 KEY_Scan(u8 mode);		//按键扫描函数

#endif
