#ifndef _LED_H
#define _LED_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/9/7
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

#define LEDB 		PBout(6)		//����   	
#define LEDW 		PBout(7)	   	//�׵�
#define POWER_EN	PCout(15)		//��Դ����
#define BAT_TP		PAout(4)		//������
#define LED1     PBout(5)
#define LED0     PBout(4)
void LED_Init(void);
#endif
