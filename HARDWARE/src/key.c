#include "key.h"



//������ʼ������
void KEY_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);//ʹ��GPIOBʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 		//KEY��Ӧ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	//��ͨ����ģʽ
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	//����
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//��ʼ��GPIOB11

	
}

/*��ȡ�������
* mode:
* 	0:ֻ�ж��Ƿ��£���Ӧʱ���
*	1:�жϰ���ʱ�䳤�̣���ʵ�ֶ�״̬���(������Ȱ����ɿ��󷵻�)
*/
u8 KEY_Scan(u8 mode)
{
	u8 i = 60;
	u8 key_up = 0, key_down = 0;	//��������Ͱ��´���
	if(KEY == PRESS_LEVEL)					//����а���
	{
		delay_ms(10);	//����
		if(KEY == PRESS_LEVEL)	
		{
			if(!mode) return KEY_PRESS;
			while(i--)
			{
				delay_ms(20);
				key_down ++;		//���´�����1��������ʱ��
				if(KEY != PRESS_LEVEL)		//�����;�ɿ�
				{
					key_up ++;		//���������1
					if(key_up>3)	//�������࣬������Ѿ��������ΰ����¼�
					{
						if(key_down < 30)
							return KEY_PRESS;
						else
							return KEY_SHORT;
					}
				}
			}
			while(KEY == PRESS_LEVEL);				//�ȵ��ɿ��ŷ���
			return KEY_LONG;
		}
	}
	return 0;
}

