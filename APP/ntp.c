#include "ntp.h"
#include "malloc.h"
#include "string.h"
#include "4g.h"
#include "rtc.h"
#include "recorder.h"
#include "wifi.h"

u16 buffer[48]={0xE3,0x0,0x06,0xEC,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x31,0x4E,0x31,0x34,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
NTP_Packet ntppack;       

//���������ж����ڣ�ʹ�û�ķ����ɭ���㹫ʽ��
//�������ڣ�0~6(������)
u8 caculateWeekday(u16 year, u8 mon, u8 day)
{
	if(mon==1 || mon==2)
	{
		mon += 12;
		year --;
	}
	return (day+2*mon+3*(mon+1)/5+year+year/4-year/100+year/400)%7;
}



//����ʱ��
//�ɹ���0����ʧ�ܣ�1��
u8 timeUpdate(void)
{
	u8 tmp1, tmp2, tmp3, tmp4;
	char timestr[20];
//	tmp1 = NTP_Req(&timestamp,500);
	tmp1 =  getTimeFromNTPServer();
	timestamp=timestamp+1;
	printf("%d\r\n",timestamp);
	if(tmp1==0)
	{
		RTC_HSE_Init(timestamp);
		printf("NTPʱ����ͬ��\r\n");
		return 0;
	}
	else
	{
		RTC_HSE_Init(1546272000);		//2019.1.1 00:00:00
	}
	if(enterAT()==0)
	{
		printf("����У��ʱ��...\r\n");
		if(send_cmd("AT+CCLK?", "+CCLK:",100,1,0, timestr)==0)
		{
			printf("time:%s\r\n",timestr);
			tmp1 = (timestr[2]-'0')*10 + (timestr[3]-'0');
			tmp2 = timestr[5]==' '?(timestr[6]-'0'):((timestr[5]-'0')*10+(timestr[6]-'0'));
			tmp3 = timestr[8]==' '?(timestr[9]-'0'):((timestr[8]-'0')*10+(timestr[9]-'0'));
			tmp4 = caculateWeekday(tmp1+2000, tmp2, tmp3)+1;			//���ڼ�����ʽ��ͬ,��1ƥ��
			//printf("%d-%d-%d %d\r\n",tmp1,tmp2,tmp3,tmp4);
			tmp4 = RTC_Set_Date(tmp1,tmp2,tmp3,tmp4);					//������0
			tmp1 = timestr[11]==' '?(timestr[12]-'0'):((timestr[11]-'0')*10+(timestr[12]-'0'));
			tmp2 = timestr[14]==' '?(timestr[15]-'0'):((timestr[14]-'0')*10+(timestr[15]-'0'));
			tmp3 = timestr[17]==' '?(timestr[18]-'0'):((timestr[17]-'0')*10+(timestr[18]-'0'));
			//printf("%d-%d-%d\r\n",tmp1,tmp2,tmp3);
			tmp4 = tmp4 && RTC_Set_Time(tmp1,tmp2,tmp3,RTC_H12_AM);
			if(tmp4==1)printf("��վ����ʱ����ͬ��\r\n");
			return !tmp4;
		}
		quitAT();
	}
	return 1;
}

