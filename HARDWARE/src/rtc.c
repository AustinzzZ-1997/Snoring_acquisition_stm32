#include "rtc.h"
#include <time.h>

#define JAN_1970             (2208988800U+57600U)		//1970���Ӧ��ʱ������Լ�ʱ��Ӱ��

//RTCʱ������(���ѡ����24Сʱ�ƣ���ampm��Ч)
//hour,min,sec:Сʱ,����,����
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC��������
//year,month,date:��(0~99),��(1~12),��(1~31)
//week:1-7
//����ֵ:SUCEE(1),�ɹ�
//       ERROR(0),�����ʼ��ģʽʧ�� 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;	
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}

//ѡ��HSE��ΪRTCʱ�ӣ���ϵͳ��Դ��ʧʱ���޷���֤AWU��״̬
//����ֵ��0�����óɹ�
//		  1��LSIʱ������ʧ��
//		  2, ʱ���Ѿ����ã������޸�
u8 RTC_HSE_Init(u32 timestamp)
{
	struct tm *nowtime;
	RTC_InitTypeDef RTC_InitStructure;
	u8 retry=0XFF;
	//���ʱ���������ο��ֲ�P88
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//ʹ��PWRʱ��
	PWR_BackupAccessCmd(ENABLE);							//ʹ�ܺ󱸼Ĵ�������
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050 || 1)	//����ǵ�һ�����ã������Ѿ������Ƿ��һ�����ã�
	{ 
		RCC_HSEConfig(ENABLE);			//ʹ��HSEʱ��
		while(RCC_GetFlagStatus(RCC_FLAG_HSERDY)==RESET)
		{
			delay_ms(10);
			retry --;
			if(retry==0) return 1;
		}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div8);				//����RTCʱ�ӣ�����ΪHSE
		RCC_RTCCLKCmd(ENABLE);								//ʹ��RTCʱ��
		
		//����RTCΪ1Hz��ʱ�ӣ�Fck=LSI/(PREDIV_A+1)/(PREDIV_S+1)     
		//PREDIV_AΪ�첽Ԥ��Ƶϵ����7λ����ȡֵ�ϴ󣬿��Խ��͹���
		//PREDIV_SΪͬ��Ԥ��Ƶϵ����15λ��
		//LSIʱ��Ϊ32kHz����DIV_A��ȡֵ99��DIV_Sȡֵ319
		RTC_InitStructure.RTC_AsynchPrediv = 100-1;
		RTC_InitStructure.RTC_SynchPrediv = 10000-1;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);
		timestamp -= JAN_1970;
		nowtime = localtime(&timestamp);				//��ʱ���ת����ʱ��ṹ��,
		//printf("t%d\r\n",nowtime->tm_year);				//��֪���Ĳ���100�ꡣ����
		RTC_Set_Time(nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec, RTC_H12_AM);	//24Сʱ�ƣ����һ��������Ч
		RTC_Set_Date(nowtime->tm_year%100, nowtime->tm_mon+1, nowtime->tm_mday+1, nowtime->tm_wday+1);		//time.hת����������������0��ʼ��
		
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//�ڱ��ݼĴ����б�ǳ�ʼ��
		return 0;
	}
	return 2;
}

void PrintTime(void)
{
	RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	
	printf("Date:20%02d-%02d-%02d weekday:%d",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,RTC_DateStruct.RTC_WeekDay);
	printf("Time:%02d:%02d:%02d\r\n",RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
}
