#include "rtc.h"
#include <time.h>

#define JAN_1970             (2208988800U+57600U)		//1970年对应的时间戳，以及时区影响

//RTC时间设置(如果选择了24小时制，则ampm无效)
//hour,min,sec:小时,分钟,秒钟
//ampm:@RTC_AM_PM_Definitions  :RTC_H12_AM/RTC_H12_PM
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Time(u8 hour,u8 min,u8 sec,u8 ampm)
{
	RTC_TimeTypeDef RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours=hour;
	RTC_TimeTypeInitStructure.RTC_Minutes=min;
	RTC_TimeTypeInitStructure.RTC_Seconds=sec;
	RTC_TimeTypeInitStructure.RTC_H12=ampm;
	
	return RTC_SetTime(RTC_Format_BIN,&RTC_TimeTypeInitStructure);
	
}
//RTC日期设置
//year,month,date:年(0~99),月(1~12),日(1~31)
//week:1-7
//返回值:SUCEE(1),成功
//       ERROR(0),进入初始化模式失败 
ErrorStatus RTC_Set_Date(u8 year,u8 month,u8 date,u8 week)
{
	
	RTC_DateTypeDef RTC_DateTypeInitStructure;
	RTC_DateTypeInitStructure.RTC_Date=date;
	RTC_DateTypeInitStructure.RTC_Month=month;
	RTC_DateTypeInitStructure.RTC_WeekDay=week;	
	RTC_DateTypeInitStructure.RTC_Year=year;
	return RTC_SetDate(RTC_Format_BIN,&RTC_DateTypeInitStructure);
}

//选择HSE作为RTC时钟，在系统电源丢失时将无法保证AWU的状态
//返回值：0，设置成功
//		  1，LSI时间配置失败
//		  2, 时间已经配置，无需修改
u8 RTC_HSE_Init(u32 timestamp)
{
	struct tm *nowtime;
	RTC_InitTypeDef RTC_InitStructure;
	u8 retry=0XFF;
	//访问备份域，详见参考手册P88
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);							//使能后备寄存器访问
	
	if(RTC_ReadBackupRegister(RTC_BKP_DR0)!=0x5050 || 1)	//如果是第一次配置（这里已经忽略是否第一次配置）
	{ 
		RCC_HSEConfig(ENABLE);			//使能HSE时钟
		while(RCC_GetFlagStatus(RCC_FLAG_HSERDY)==RESET)
		{
			delay_ms(10);
			retry --;
			if(retry==0) return 1;
		}
		RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div8);				//设置RTC时钟，设置为HSE
		RCC_RTCCLKCmd(ENABLE);								//使能RTC时钟
		
		//配置RTC为1Hz的时钟，Fck=LSI/(PREDIV_A+1)/(PREDIV_S+1)     
		//PREDIV_A为异步预分频系数（7位），取值较大，可以降低功耗
		//PREDIV_S为同步预分频系数（15位）
		//LSI时钟为32kHz，故DIV_A可取值99，DIV_S取值319
		RTC_InitStructure.RTC_AsynchPrediv = 100-1;
		RTC_InitStructure.RTC_SynchPrediv = 10000-1;
		RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
		RTC_Init(&RTC_InitStructure);
		timestamp -= JAN_1970;
		nowtime = localtime(&timestamp);				//将时间戳转换成时间结构体,
		//printf("t%d\r\n",nowtime->tm_year);				//不知道哪差了100年。。。
		RTC_Set_Time(nowtime->tm_hour, nowtime->tm_min, nowtime->tm_sec, RTC_H12_AM);	//24小时制，最后一个参数无效
		RTC_Set_Date(nowtime->tm_year%100, nowtime->tm_mon+1, nowtime->tm_mday+1, nowtime->tm_wday+1);		//time.h转换而来的日期是以0开始的
		
		RTC_WriteBackupRegister(RTC_BKP_DR0,0x5050);	//在备份寄存器中标记初始化
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
