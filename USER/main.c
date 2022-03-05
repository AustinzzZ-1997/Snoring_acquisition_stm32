#include "sys.h"
#include "led.h"
#include "key.h"
#include "sdio_sdcard.h"
#include "malloc.h"
#include "fatfs.h"
#include "4g.h"
#include "rtc.h"
#include "ntp.h"
#include "recorder.h"
#include "wm8978.h"
#include "timer.h"
#include "usmart.h"
#include "http.h"
#include "ussend.h"
#include "exti.h"
#include "crc.h"
#include "wifi.h"
#include "ntp.h"
#include "M8266HostIf.h"
#include "M8266WIFIDrv.h"
#include "M8266WIFI_ops.h"
#include "brd_cfg.h"

void M8266WIFI_SetMod(void);
u8 M8266WIFI_SetSocket(void);


/** @addtogroup Template_Project
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
vu8 dma_4g_status = 0;
u8 i = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	u8 success=0;
	SYS_Init();
	LED_Init();
	EXTIX_Init();
	WM8978_Init();				//初始化WM8978
    my_mem_init(SRAMIN);		//初始化内部内存池
	fatfs_init();				//为fatfs相关变量申请内存  
    f_mount(diskfs[0],"0:",1); 	//挂载SD卡
	printf("启动\r\n");
	TIM5_Int_Init(5000-1,8400-1);
	printf("\r\nM8266HostIf_Init\r\n");
	M8266HostIf_Init();
	printf("\r\nM8266WIFI_Module_Init_Via_SPI\r\n");
	success = M8266WIFI_Module_Init_Via_SPI();
	if(success)
	{
		printf("\r\n初始化成功！！\r\n");
	}	
	else // If M8266WIFI module initialisation failed, two led constantly flash in 2Hz
	{
		  while(1){}
	}
//	uart4_init(2000000);		//初始化串口
//	wifi_Init();
//	while(retry--)
//		{
//	     if(timeUpdate()==0)
//		 {
//		  break;
//		 }
//		}
//	while(retry--)
//		{
//			if(wifi_settansper()==0)
//			{
//			printf("TCP连接成功！！");
//			break;
//			}
//		}
//		if(retry==0)printf("TCP连接失败");
//	printf(" 开始传输\r\n");
	printf("\r\nM8266WIFI_SetMod\r\n");
	M8266WIFI_SetMod();
	
while (1)
	{	
//		M8266WIFI_Tans(UART_DMA_TX_BUF,5840);
//			if(tanspar_flag==0)
//			{	
//				us_dma_send();
//				tanspar_flag=2;
//			}
			if(tanspar_flag==0)
			{
				wav_recorder();
//				tanspar_flag=1;
				delay_ms(5);
			}
	}
}

