/********************************************************************
 * test_m8266wifi.c
 * .Description
 *     Source file of M8266WIFI testing application 
 * .Copyright(c) Anylinkin Technology 2015.5-
 *     IoT@anylinkin.com
 *     http://www.anylinkin.com
 *     http://anylinkin.taobao.com
 *  Author
 *     wzuo
 *  Date
 *  Version
 ********************************************************************/
 
#include "stdio.h"
#include "brd_cfg.h"
#include "M8266WIFIDrv.h"
#include "M8266HostIf.h"
#include "M8266WIFI_ops.h"
#include "led.h"

void M8266WIFI_SetMod(void)
{
	 u16 status = 0;
	 u8  link_no=0;
   ////	 Macro for Test Type(Chinese：测试类型宏定义)
	 #define TEST_M8266WIFI_TYPE    1	     //           1 = Repeative Sending, 2 = Repeative Reception, 3 = Echo  4 = multi-clients transimission test
	                                       // (Chinese: 1=模组向外不停地发送数据 2=模组不停地接收数据 3= 模组将接收到的数据发送给发送方 4=多客户端测试) 
   ////	 Macros for Socket Connection Type (Chinese：套接字类型的宏定义) 
	 #define TEST_CONNECTION_TYPE   1	    //           0=WIFI module as UDP, 1=WIFI module as TCP Client, 2=WIFI module as TCP Server
	                                      // (Chinese: 0=WIFI模组做UDP, 1=WIFI模组做TCP客户端, 2=WIFI模组做TCP服务器
   ////	 Macros for Soket ip:port pairs  (Chinese：套接字的本地端口和目标地址目标端口的宏定义) 
   //local port	(Chinese：套接字的本地端口)
#if (TEST_CONNECTION_TYPE==1)         //// if module as TCP Client (Chinese:如果模组作为TCP客户端)
	 #define TEST_LOCAL_PORT  			0			//           local port=0 will generate a random local port each time fo connection. To avoid the rejection by TCP server due to repeative connection with the same ip:port
   	                                    // (Chinese: 当local port传递的参数为0时，本地端口会随机产生。这一点对于模组做客户端反复连接服务器时很有用。因为随机产生的端口每次会不一样，从而避免连续两次采用同样的地址和端口链接时被服务器拒绝。
#elif (TEST_CONNECTION_TYPE==0) || (TEST_CONNECTION_TYPE==2) //// if module as UDP or TCP Server (Chinese:如果模组作为UDP或TCP服务器)
   #define TEST_LOCAL_PORT  			4321  //           a local port should be specified //(Chinese:如果模组作为UDP或TCP服务器，则需要指定该套接字的本地端口)
#else
#error WRONG TEST_CONNECTION_TYPE defined !
#endif                                  // (Chinese: 如果模组作为TCP服务器或UDP，那么必须指定本地端口
   //local port	(Chinese：套接字的目标地址和目标端口)
#if (TEST_CONNECTION_TYPE==0)        //// if module as UDP (Chinese:如果模组作为UDP，则可以指定目标地址和端口，也可以随便填充，在发送数据时再设置或更改)
   #define TEST_REMOTE_ADDR    		"115.28.24.140"
   #define TEST_REMOTE_PORT  	    7010
	//local port	(Chinese：套接字的目标地址和目标端口)
#elif (TEST_CONNECTION_TYPE==1)      //// if module as TCP Client (Chinese:如果模组作为TCP客户端，当然必须指定目标地址和目标端口，即模组所要去连接的TCP服务器的地址和端口)
   #define TEST_REMOTE_ADDR    	 	"47.105.175.47"  //"192.168.1.3"// "www.baidu.com"
   #define TEST_REMOTE_PORT  	    8085//137 //8085          //8085	// 80
#endif			 

///////
//step 0: config tcp windows number (Chinese: 步骤0：如果是TCP类型的套接字，可以配置调整窗口参数）
#if ( 1 && ((TEST_CONNECTION_TYPE==1)||(TEST_CONNECTION_TYPE==2)) ) //If you hope to change TCP Windows, please change '0' to '1' in the #if clause before setup the connection
																																		//(Chinese: 如果想改变套接字的窗口数，可以将#if语句中的0改成1，这个配置需要在创建套接字之前执行)
// u8 M8266WIFI_SPI_Config_Tcp_Window_num(u8 link_no, u8 tcp_wnd_num, u16* status)
  if(M8266WIFI_SPI_Config_Tcp_Window_num(link_no, 4, &status)==0)
  {
		 while(1)
		 {
			 printf("\r\n配置调整窗口参数失败！\r\n");
			 delay_ms(1000);
	   }
	}

#endif
}

u8 M8266WIFI_SetSocket(void)
{
	 u16 status = 0;
	 u8  link_no=0;
///////	
//step 1: setup connection (Chinese: 步骤1：创建套接字连接）

  //  u8 M8266WIFI_SPI_Setup_Connection(u8 tcp_udp, u16 local_port, char remote_addr, u16 remote_port, u8 link_no, u8 timeout_in_s, u16* status);
	if(M8266WIFI_SPI_Setup_Connection(TEST_CONNECTION_TYPE, TEST_LOCAL_PORT, TEST_REMOTE_ADDR, TEST_REMOTE_PORT, link_no, 20, &status)==0)
	{		
		 while(1)
		 {
			 printf("\r\n创建套接字失败！\r\n");
			 delay_ms(1000);
		 }
	}
	printf("\r\n创建套接字成功！\r\n");
	return 0;
}

void M8266WIFI_Tans(u8 snd_data[],u32 TEST_SEND_DATA_SIZE)
{
		u16 status = 0;
		u8  link_no=0;                      
		volatile u32 sent = 0;
		volatile u8 debug_point;
		link_no=0;

		sent = M8266WIFI_SPI_Send_BlockData(snd_data, (u32)TEST_SEND_DATA_SIZE, 5000, link_no, NULL, 0, &status); // !! Kind noted: For those platform compilation by TI/IAR, such MSP430, K60, TMS28335 etc, PLEASE add (u32) to mandatorily convert a const to u32, or, the parameter transmission will be 16-bit and bring about function calling failure
		if( (sent==TEST_SEND_DATA_SIZE) && ((status&0xFF)==0x00) ) //Send successfully 
		{
//			printf("\r\n成功发出%d字节\r\n",sent);
		}
		else if( (status&0xFF) == 0x1E)			       // 0x1E = too many errors ecountered during sending and can not fixed, or transsmission blocked heavily(Chinese: 发送阶段遇到太多的错误或阻塞了，可以考虑加大max_loops)
		{
			debug_point = 1;
//			printf("\r\n增加max_loops的值\r\n");
			//add some process here (Chinese: 可以在此处加一些处理，比如增加max_loops的值)
		}
		else if(  ((status&0xFF) == 0x14)			 // 0x14 = connection of link_no not present (Chinese: 该套接字不存在)
		 || ((status&0xFF) == 0x15) )    // 0x15 = connection of link_no closed(Chinese: 该套接字已经关闭或断开)			
		{
			 debug_point = 2;
//			printf("\r\n需要重建建立套接字\r\n");
			 //need to re-establish the socket connection (Chinese: 需要重建建立套接字)
		}
		else if( (status&0xFF) == 0x18 )        // 0x18 = TCP server in listening states and no tcp clients have connected. (Chinese: 这个TCP服务器还没有客户端连接着它)
		{
			 debug_point = 3;
			 M8266HostIf_delay_us(99);
		}
		else
		{
			 debug_point = 4;
		M8266HostIf_delay_us(101);
		}			
} // end of M8266WIFI_Test
