#include "4g.h"
#include "timer.h"
#include "string.h"



//��ȡ��ǰ����
u8 getCurrentInfo(void)
{
	if(enterAT()==0)
	{
		send_cmd("AT+E?","+E:",20,1,0,NULL);
		send_cmd("AT+WKMOD?","+WKMOD:",20,1,0,NULL);
		send_cmd("AT+CMDPW?","+CMDPW:",20,1,0,NULL);
		send_cmd("AT+RSTIM?","+RSTIM:",20,1,0,NULL);
		send_cmd("AT+SN?","+SN:",20,1,0,NULL);
		send_cmd("AT+ICCID?","+ICCID:",20,1,0,NULL);
		send_cmd("AT+IMEI?","+IMEI:",20,1,0,NULL);
		send_cmd("AT+UARTFT?","+UARTFT:",20,1,0,NULL);
		send_cmd("AT+UARTFL?","+UARTFL:",20,1,0,NULL);
		send_cmd("AT+APN?","+APN:",20,1,0,NULL);
		send_cmd("AT+SOCKA?","+SOCKA:",20,1,0,NULL);
		send_cmd("AT+SOCKB?","+SOCKB:",20,1,0,NULL);
		send_cmd("AT+SOCKAEN?","+SOCKAEN:",20,1,0,NULL);
		send_cmd("AT+SOCKBEN?","+SOCKBEN:",20,1,0,NULL);
		send_cmd("AT+SOCKASL?","+SOCKASL:",20,1,0,NULL);
		send_cmd("AT+SOCKBSL?","+SOCKBSL:",20,1,0,NULL);
		send_cmd("AT+SOCKALK?","+SOCKALK:",20,1,0,NULL);
		send_cmd("AT+SOCKBLK?","+SOCKBLK:",20,1,0,NULL);
		send_cmd("AT+SOCKATO?","+SOCKATO:",20,1,0,NULL);
		send_cmd("AT+SOCKBTO?","+SOCKBTO:",20,1,0,NULL);
		send_cmd("AT+SOCKIND?","+SOCKIND:",20,1,0,NULL);
		send_cmd("AT+SOCKRSTIM?","+SOCKRSTIM:",20,1,0,NULL);
		send_cmd("AT+REGEN?","+REGEN:",20,1,0,NULL);
		send_cmd("AT+REGTP?","+REGTP:",20,1,0,NULL);
		send_cmd("AT+REGDT?","+REGDT:",20,1,0,NULL);
		send_cmd("AT+REGSND?","+REGSND:",20,1,0,NULL);
		send_cmd("AT+CLOUD?","+CLOUD:",20,1,0,NULL);
		send_cmd("AT+ID?","+ID:",20,1,0,NULL);
		send_cmd("AT+HEARTEN?","+HEARTEN:",20,1,0,NULL);
		send_cmd("AT+HEARTDT?","+HEARTDT:",20,1,0,NULL);
		send_cmd("AT+HEARTSND?","+HEARTSND:",20,1,0,NULL);
		send_cmd("AT+HEARTTM?","+HEARTTM:",20,1,0,NULL);
		send_cmd("AT+HTPTP?","+HTPTP:",20,1,0,NULL);
		send_cmd("AT+HTPURL?","+HTPURL:",20,1,0,NULL);
		send_cmd("AT+HTPSV?","+HTPSV:",20,1,0,NULL);
		send_cmd("AT+HTPHD?","+HTPHD:",20,1,0,NULL);
		send_cmd("AT+HTPTO?","+HTPTO:",20,1,0,NULL);
		send_cmd("AT+HTPFLT?","+HTPFLT:",20,1,0,NULL);
		quitAT();
		return 0;
	}
	return 1;
}









//4Gģ��Ӳ����ʼ��
static void FourG_Hardwave(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	//ʹ��GPIOCʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//��ͨ���ģʽ
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//����
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//��ʼ��
	GPIO_SetBits(GPIOC,GPIO_Pin_13);						//GPIOC13���øߣ�4G��Դ��ͨ
	
	uart4_init(3000000);										//��ʼ������
}

//4Gģ��Ӳ������
u8 FourG_Reset(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);				//RESET�ܽ�����0.5�븴λ
	delay_ms(1000);									//����1��
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
	if(send_cmd("", "[USR", 2000, 0, 0,NULL)==0)		//�ȴ����տ����ַ���20��
		return 0;
	return 1;
}

//4Gģ���ʼ��
u8 FourG_Init(void)
{
	u8 retry =3;
	u8 res;
	FourG_Hardwave();
	TIM7_Timeout_Init(1000-1,1680-1);					//����������ճ�ʱʱ��10ms
	while(retry--)
	{
		if(FourG_Reset()==0)							//4Gģ�鸴λ
		{
//			if(enterAT()==0)
//			{
//				send_cmd("AT+UART=460800,8,1,NONE,NFC","OK",20,1,NULL);
//				uart4_init(460800);
//			}
			res = getCurrentInfo();						//��ȡ��ǰ����		
			return res;
		}
	}
	return 1;
}


//������������Ƿ��Ѿ�������ȷ
//0���Ѿ�������ȷ
//1������������1�����ô���
u8 FourG_CheckSOCK(char *sock, char *pto, char *addr, u16 port, u8 mode)
{
	u8 res = 0;
	char cmd[50]={0};
	sprintf(cmd, "%s,%s,%d",pto,addr,port);
	if(enterAT()==0)
	{
		res = res || send_cmd("AT+WKMOD", "NET", 50, 1,0, NULL);
		if(strcmp(sock,"SOCKB")==0)
		{
			res = res || send_cmd("AT+SOCKB", cmd, 50, 1,0, NULL);
			res = res || send_cmd("AT+SOCKBEN", "ON", 50, 1,0, NULL);
			if(mode)
				res = res || send_cmd("AT+SOCKBSL", "LONG", 50, 1,0, NULL);
			else
				res = res || send_cmd("AT+SOCKBSL", "SHORT", 50, 1, 0,NULL);
		}
		else
		{
			res = res || send_cmd("AT+SOCKA", cmd, 50, 1, 0,NULL);
			res = res || send_cmd("AT+SOCKAEN", "ON", 50, 1, 0,NULL);
			if(mode)
				res = res || send_cmd("AT+SOCKASL", "LONG", 50, 1, 0,NULL);
			else
				res = res || send_cmd("AT+SOCKASL", "SHORT", 50, 1, 0,NULL);
		}
		quitAT();
		return res;
	}
	return 1;
}

//���ݲ��������������
// sock��	"SOCKA" / 	"SOCKB"
// pto:		"TCP"	/	"UDP"
// addr:	IP��ַ����ַ���ַ�����
// port��	�������˿ڣ�0~65535��
// mode��	0�������ӣ���1�������ӣ�
u8 FourG_SetSOCK(char *sock, char *pto, char *addr, u16 port, u8 mode)
{
	char cmd[50]={0};
	if(FourG_CheckSOCK(sock, pto, addr, port, mode)==0) 
	{
		printf("��������\r\n");
		return 0;
	}
	if(enterAT()==0)
	{
		
		send_cmd("AT+WKMOD=NET", "OK", 50, 1, 0,NULL);
		if(strcmp(sock,"SOCKB")==0)
		{
			send_cmd("AT+SOCKBEN=ON", "OK", 50, 1, 0,NULL);
			if(mode)
			{
				send_cmd("AT+SOCKBSL=LONG", "OK", 50, 1, 0,NULL);		//����Ϊ������
				
			}	
			else
			{
				send_cmd("AT+SOCKBSL=SHORT", "OK", 50, 1, 0,NULL);	//����Ϊ������
				send_cmd("AT+SHORBTO=5", "OK", 50, 1,0, NULL);		//���ö����ӳ�ʱʱ��Ϊ5��
			}
			sprintf(cmd, "AT+SOCKB=%s,%s,%d",pto,addr,port);
			send_cmd(cmd, "OK", 50, 1, 0,NULL);
		}
		else
		{
			send_cmd("AT+SOCKAEN=ON", "OK", 50, 1,0, NULL);
			if(mode)
			{
				send_cmd("AT+SOCKASL=LONG", "OK", 50, 1,0, NULL);
			}	
			else
			{
				send_cmd("AT+SOCKASL=SHORT", "OK", 50, 1, 0,NULL);
				send_cmd("AT+SHORATO=5", "OK", 50, 1, 0,NULL);		//���ö����ӳ�ʱʱ��Ϊ5��
			}
			sprintf(cmd, "AT+SOCKA=%s,%s,%d",pto,addr,port);
			send_cmd(cmd, "OK", 50, 1,0, NULL);
		}
		printf("�������\r\n");
		send_cmd("AT+Z", "", 50, 1, 0,NULL);				//����ģ��
		if(send_cmd("", "[USR", 2000, 0, 0,NULL)==0)		//�ȴ����տ����ַ���20��
			return 0;
		//quitAT();
	}
	return 1;
}



//���Ӧ��
char* ack_check(char *str)
{
	char *strx = 0;
	if(UART4_RX_STA&0x8000)
		UART4_RX_BUF[UART4_RX_STA&0x7fff] = '\0';
	strx = strstr((char*)UART4_RX_BUF, str);
	return strx;
}


/**
* @brief  ��ģ�鷢��ATָ���֤Ӧ��
* @param  cmd��		Ҫ���͵������ַ���,���Ϊ���ַ��������ʾ����Ҫ���ͣ����ȴ�����
*		  ack��		�ڴ���Ӧ����,���Ϊ���ַ��������ʾ����Ҫ�ȴ�Ӧ��
*		  timeout��	��ʱʱ�䣨��λ10ms)
*		  crlf:		�Ƿ���лس�����.(1�У�0û��)
*		  endptr:	ָ��Ӧ����ĩβ��ָ�롣���ΪNULL���򲻽��в���,���ҽ��ڳɹ�Ӧ���ʱ����Ч
* @retval 0�����ͳɹ����õ��ڴ���Ӧ������
*		  1������ʧ�ܣ�δ�����ڴ���Ӧ������
* ע�⣺ �ڻ�ȡendptrʱ����Ҫ�ȼ�鷵��ֵ
*
  */
u8 send_cmd(char *cmd, char *ack, u16 timeout, u8 crlf, u8 hex, char *endptr)
{
	u8 res = 0;
	char *strx = 0;
	UART4_RX_STA = 0;
	if(cmd==NULL || ack==NULL)
		return 1;
	if(strlen(cmd)!=0)
	{
		if(crlf&&hex==0)
			u4_printf("%s\r\n", cmd);
		else if(crlf==0&&hex==0)
			u4_printf("%s", cmd);
		else if(crlf==1&&hex==1)
			u4_printf("%s\r\n", cmd);
		else if(crlf==1&&hex==0)
			u4_printf("%s", cmd);
	}
	if(strlen(ack)!=0 && timeout)
	{
		while(--timeout)
		{
			delay_ms(10);
			if(UART4_RX_STA & 0x8000)
			{
				strx = 0;
				UART4_RX_BUF[UART4_RX_STA&0x7fff] = '\0';
				strx = strstr((char*)UART4_RX_BUF, (char*)ack);
				if(strx)
				{
					//printf("CMD:%s\r\n", cmd);
					printf("CMD:%s\r\nRes:%s\r\n", cmd, UART4_RX_BUF);
					if(endptr!=NULL)
						strcpy(endptr,strx+strlen(ack));
//					UART4_RX_STA=0;
					return 0;
				}
				UART4_RX_STA = 0;		//���ȥ����ע�ͣ���ע��return����һֱ�ȴ�����ʱ�����յ�Ӧ��ŷ��ء����ɶ�ν������ݣ�
				//return 1;					//������յ������Ҳ�ƥ�䣬���̷��أ�ֻ����һ�����ݣ�
			}
		}
		if(timeout == 0) res = 1;
	}
	return res;
}

//����AT����
u8 enterAT(void)
{
	u8 retry = 3;
	if(send_cmd("AT+VER?", "+VER:", 100, 1, 0,NULL)==0)
		return 0;
	while(retry--)
	{
		delay_ms(2000);
		if(send_cmd("+++", "a", 100, 0, 0,NULL)==0)
		{
			if(send_cmd("a", "+ok", 100, 0, 0,NULL)==0)
					return 0;
		}
	}
	return 1;
}

u8 enterATT(void)
{
	u8 retry = 3;
	while(retry--)
	{
		delay_ms(2000);
		if(send_cmd("+++", "a", 100, 0, 0,NULL)==0)
		{
			if(send_cmd("a", "+ok", 100, 0, 0,NULL)==0)
					return 0;
		}
	}
	return 1;
}

//�˳�AT����
u8 quitAT(void)
{
	return send_cmd("AT+ENTM", "", 100, 1,0,NULL);
}






