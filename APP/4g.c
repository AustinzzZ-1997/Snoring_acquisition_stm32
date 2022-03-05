#include "4g.h"
#include "timer.h"
#include "string.h"



//获取当前参数
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









//4G模块硬件初始化
static void FourG_Hardwave(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);	//使能GPIOC时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;			//普通输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//50MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			//上拉
	GPIO_Init(GPIOC, &GPIO_InitStructure);					//初始化
	GPIO_SetBits(GPIOC,GPIO_Pin_13);						//GPIOC13设置高，4G电源导通
	
	uart4_init(3000000);										//初始化串口
}

//4G模块硬件重启
u8 FourG_Reset(void)
{
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);				//RESET管脚拉低0.5秒复位
	delay_ms(1000);									//拉低1秒
	GPIO_SetBits(GPIOC,GPIO_Pin_13);
	if(send_cmd("", "[USR", 2000, 0, 0,NULL)==0)		//等待接收开机字符，20秒
		return 0;
	return 1;
}

//4G模块初始化
u8 FourG_Init(void)
{
	u8 retry =3;
	u8 res;
	FourG_Hardwave();
	TIM7_Timeout_Init(1000-1,1680-1);					//设置网络接收超时时间10ms
	while(retry--)
	{
		if(FourG_Reset()==0)							//4G模块复位
		{
//			if(enterAT()==0)
//			{
//				send_cmd("AT+UART=460800,8,1,NONE,NFC","OK",20,1,NULL);
//				uart4_init(460800);
//			}
			res = getCurrentInfo();						//获取当前参数		
			return res;
		}
	}
	return 1;
}


//检查网络设置是否已经设置正确
//0：已经设置正确
//1：其中至少有1项设置错误
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

//根据参数配置网络参数
// sock：	"SOCKA" / 	"SOCKB"
// pto:		"TCP"	/	"UDP"
// addr:	IP地址或网址（字符串）
// port：	服务器端口（0~65535）
// mode：	0（短连接），1（长连接）
u8 FourG_SetSOCK(char *sock, char *pto, char *addr, u16 port, u8 mode)
{
	char cmd[50]={0};
	if(FourG_CheckSOCK(sock, pto, addr, port, mode)==0) 
	{
		printf("无需设置\r\n");
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
				send_cmd("AT+SOCKBSL=LONG", "OK", 50, 1, 0,NULL);		//设置为长连接
				
			}	
			else
			{
				send_cmd("AT+SOCKBSL=SHORT", "OK", 50, 1, 0,NULL);	//设置为短连接
				send_cmd("AT+SHORBTO=5", "OK", 50, 1,0, NULL);		//设置短连接超时时间为5秒
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
				send_cmd("AT+SHORATO=5", "OK", 50, 1, 0,NULL);		//设置短连接超时时间为5秒
			}
			sprintf(cmd, "AT+SOCKA=%s,%s,%d",pto,addr,port);
			send_cmd(cmd, "OK", 50, 1,0, NULL);
		}
		printf("设置完成\r\n");
		send_cmd("AT+Z", "", 50, 1, 0,NULL);				//重启模块
		if(send_cmd("", "[USR", 2000, 0, 0,NULL)==0)		//等待接收开机字符，20秒
			return 0;
		//quitAT();
	}
	return 1;
}



//检查应答
char* ack_check(char *str)
{
	char *strx = 0;
	if(UART4_RX_STA&0x8000)
		UART4_RX_BUF[UART4_RX_STA&0x7fff] = '\0';
	strx = strstr((char*)UART4_RX_BUF, str);
	return strx;
}


/**
* @brief  向模块发送AT指令并验证应答
* @param  cmd：		要发送的命令字符串,如果为空字符串，则表示不需要发送，仅等待接收
*		  ack：		期待的应答结果,如果为空字符串，则表示不需要等待应答
*		  timeout：	超时时间（单位10ms)
*		  crlf:		是否带有回车换行.(1有，0没有)
*		  endptr:	指向应答结果末尾的指针。如果为NULL，则不进行操作,并且仅在成功应答的时候有效
* @retval 0：发送成功（得到期待的应答结果）
*		  1：发送失败（未含有期待的应答结果）
* 注意： 在获取endptr时，需要先检查返回值
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
				UART4_RX_STA = 0;		//如果去掉该注释，且注释return，则一直等待到超时，或收到应答才返回。（可多次接收数据）
				//return 1;					//如果接收到数据且不匹配，立刻返回（只接收一次数据）
			}
		}
		if(timeout == 0) res = 1;
	}
	return res;
}

//进入AT设置
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

//退出AT设置
u8 quitAT(void)
{
	return send_cmd("AT+ENTM", "", 100, 1,0,NULL);
}






