#include "led.h" 
	    
//IO��ʼ��
void LED_Init(void)
{   
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	//LED��ʼ������
	//LED0(PB4)/LED1(PB5)/LEDB(PB6)/LEDW(PB7)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//��ʼ��
	GPIO_SetBits(GPIOB,GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);//���øߣ�����
	
	//������IO��ʼ������
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOAʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//��©���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//����
	GPIO_Init(GPIOA, &GPIO_InitStructure);				//��ʼ��
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);					//GPIOA4���õͣ��õͳ�磬���չض�
	
	//��Դ
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;	//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//��ʼ��
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);//GPIOB11���õͣ���Դ��ͨ
}
