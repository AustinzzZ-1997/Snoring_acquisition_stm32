#include "wifi.h"
#include "timer.h"
#include "string.h"
#include "stdlib.h"
u32 timestamp;

void U4Putchar(char data)
{
	while((UART4->SR&0X40)==0){}
  UART4->DR=data;
}

u8* ESP8266_CheckCmd(char* str)// Chack string
{
  char *strx=0;
  if(UART4_RX_STA&0x8000)
  {
    UART4_RX_BUF[UART4_RX_STA&0x7fff]=0;
    strx=strstr((const char*)UART4_RX_BUF,(const char*)str); 
  }
  return (u8*)strx;
}

u8 ESP8266_SendCmd(char *cmd,char *ack,u16 waittime)// cmd =1 
{
  u8 res=0;
  u4_printf("%s\r\n",cmd);
  if(ack&&waittime)
  {
    while(--waittime)
    {
      delay_ms(10);
      if(UART4_RX_STA&0X8000)
      {
        if(NULL != ESP8266_CheckCmd(ack))
        {
		  printf("%s\r\n",(const char*)UART4_RX_BUF);
		  UART4_RX_STA=0;
          res=0;
          break;
        }
        else res=1;
        UART4_RX_STA=0;
      }
    }
    if(waittime==0) res=1;
  }
  return res;
}

void wifi_Init(void)
{
	TIM7_Timeout_Init(1000-1,1680-1);					//设置网络接收超时时间10ms
	u8 state=0;
  
	UART4_RX_STA = 0;
	memset(UART4_RX_BUF,0,sizeof(UART4_RX_BUF));
	state = ESP8266_SendCmd("AT+CWMODE=1","OK",2000);   //Set WiFi mode 3 SoftAP+Station mode
	if(!state) printf("ModeSet...OK\n"); else printf("ModeSet...Error\n");
	state = ESP8266_SendCmd("AT+RST","OK",20);
	if(!state) printf("Reset...OK\n"); else printf("Reset...Error\n");
	delay_ms(5000);   // need it more maybe?
	//  state = ESP8266_SendCmd("AT+CWJAP=\"ChinaNet-xDVy\",\"yszekdsq\"","OK",10000);  // Connect router
	state = ESP8266_SendCmd("AT+CWJAP=\"jwj\",\"123456789\"","OK",10000);  // Connect router
	if(!state) printf("RouterConn...OK\n"); else printf("RouterConn..Error\n");
	state = ESP8266_SendCmd("AT+CIPMUX=0","OK",300);
	if(!state) printf("ConnSet...OK\n"); else printf("ConnSet..Error\n");
}

u8 getTimeFromNTPServer(void)
{
  u8 state=0;
  u16 t;
  u8 packetBuffer[48];
  u32 timeOut=0xffffff;
  u8 i;
  char S[10];
  memset(S,0,sizeof(S));
	
  state = ESP8266_SendCmd("AT+CIPSTART=\"UDP\",\"3.cn.pool.ntp.org\",123","CONNECT",100);
  if(!state) printf("CONNECT...ok\n");
while(1)
{
  UART4_RX_STA = 0;
  memset(UART4_RX_BUF,0,sizeof(UART4_RX_BUF));
  delay_ms(100);
  memset(packetBuffer,0,sizeof(packetBuffer));
  state = ESP8266_SendCmd("AT+CIPSEND=48",">",100);
  if(!state) printf(">>>...ok\n");else printf(">>>...fail\n");
  packetBuffer[0] = 0xe3;  // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  packetBuffer[12] = 0x31; 
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 0x31;
  packetBuffer[15] = 0x34;
  UART4_RX_STA = 0;
  memset(UART4_RX_BUF,0,sizeof(UART4_RX_BUF));
  
  for(i=0;i<48;i++)
  {
    U4Putchar(packetBuffer[i]);
  }
  
  while(timeOut--)
  {
    if(UART4_RX_STA&0x8000)
    {
		for(t=0;t<(UART4_RX_STA&0x7fff);t++)
		{
//			printf("%c",UART4_RX_BUF[t]);
			if(UART4_RX_BUF[t]=='+')
			{   
				timestamp = (u8)UART4_RX_BUF[t+48];
				timestamp <<= 8;
				timestamp |= (u8)UART4_RX_BUF[t+49];
				timestamp <<= 8;
				timestamp |= (u8)UART4_RX_BUF[t+50];
				timestamp <<= 8;
				timestamp |= (u8)UART4_RX_BUF[t+51];
//              timestamp-=2208988800;
				printf("获取到的时间戳(10进制): %d\r\n",timestamp);
				UART4_RX_STA=0;
				return 0;
			}	
		}
		UART4_RX_STA=0;
    }
  }
  
}
printf("\r\nno\r\n");
return 1;

}


u8 wifi_settansper(void)
{
  u8 state=0;
  state = ESP8266_SendCmd("AT+CIPSTART=\"TCP\",\"47.105.175.47\",8085","OK",100);
  if(!state) 
	{
		printf("TCP CONNECT...ok\n");
		state = ESP8266_SendCmd("AT+CIPMODE=1","OK",100);
		if(!state)
		{	printf("TCP透传模式...ok\n");
			 state = ESP8266_SendCmd("AT+CIPSEND",">",100);
			  if(!state) 
			  {		printf("TCP传输准备...ok\n");
					return 0;
			  }else printf("TCP传输准备...fail\n");
		  }  else printf("TCP透传模式...fail\n");
	 }else printf("TCP CONNECT...fail\n");
return 0;
}

void wifi_esctansper(void)
{
	u8 state1=0;
	u8 try=3;
	while(try--)
	{
	u4_printf("%s","+++");
	delay_ms(2000);
	state1 = ESP8266_SendCmd("AT+CIPMODE=0","OK",200);
	if(!state1) 
	{	
		printf("退出透传模式...OK\n");
		state1 = ESP8266_SendCmd("AT+CIPCLOSE","OK",200);
			if(!state1) 
			{
				printf("透传关闭...ok\n");
				break;
			}else printf("透传关闭...fail\n");
	}else printf("退出透传模式...fail\n");
}
}


