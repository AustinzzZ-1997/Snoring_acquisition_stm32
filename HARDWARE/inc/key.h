#ifndef _KEY_H
#define _KEY_H
#include "sys.h"



//����ķ�ʽ��ͨ��ֱ�Ӳ���HAL�⺯����ʽ��ȡIO
#define KEY        	GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_10) //KEY����PB10
#define PRESS_LEVEL		0						//�������µĵ�ƽ

#define KEY_PRESS	1
#define KEY_SHORT	2
#define KEY_LONG	3


void KEY_Init(void);		//IO��ʼ��
u8 KEY_Scan(u8 mode);		//����ɨ�躯��

#endif
