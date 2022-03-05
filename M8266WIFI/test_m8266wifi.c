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
   ////	 Macro for Test Type(Chinese���������ͺ궨��)
	 #define TEST_M8266WIFI_TYPE    1	     //           1 = Repeative Sending, 2 = Repeative Reception, 3 = Echo  4 = multi-clients transimission test
	                                       // (Chinese: 1=ģ�����ⲻͣ�ط������� 2=ģ�鲻ͣ�ؽ������� 3= ģ�齫���յ������ݷ��͸����ͷ� 4=��ͻ��˲���) 
   ////	 Macros for Socket Connection Type (Chinese���׽������͵ĺ궨��) 
	 #define TEST_CONNECTION_TYPE   1	    //           0=WIFI module as UDP, 1=WIFI module as TCP Client, 2=WIFI module as TCP Server
	                                      // (Chinese: 0=WIFIģ����UDP, 1=WIFIģ����TCP�ͻ���, 2=WIFIģ����TCP������
   ////	 Macros for Soket ip:port pairs  (Chinese���׽��ֵı��ض˿ں�Ŀ���ַĿ��˿ڵĺ궨��) 
   //local port	(Chinese���׽��ֵı��ض˿�)
#if (TEST_CONNECTION_TYPE==1)         //// if module as TCP Client (Chinese:���ģ����ΪTCP�ͻ���)
	 #define TEST_LOCAL_PORT  			0			//           local port=0 will generate a random local port each time fo connection. To avoid the rejection by TCP server due to repeative connection with the same ip:port
   	                                    // (Chinese: ��local port���ݵĲ���Ϊ0ʱ�����ض˿ڻ������������һ�����ģ�����ͻ��˷������ӷ�����ʱ�����á���Ϊ��������Ķ˿�ÿ�λ᲻һ�����Ӷ������������β���ͬ���ĵ�ַ�Ͷ˿�����ʱ���������ܾ���
#elif (TEST_CONNECTION_TYPE==0) || (TEST_CONNECTION_TYPE==2) //// if module as UDP or TCP Server (Chinese:���ģ����ΪUDP��TCP������)
   #define TEST_LOCAL_PORT  			4321  //           a local port should be specified //(Chinese:���ģ����ΪUDP��TCP������������Ҫָ�����׽��ֵı��ض˿�)
#else
#error WRONG TEST_CONNECTION_TYPE defined !
#endif                                  // (Chinese: ���ģ����ΪTCP��������UDP����ô����ָ�����ض˿�
   //local port	(Chinese���׽��ֵ�Ŀ���ַ��Ŀ��˿�)
#if (TEST_CONNECTION_TYPE==0)        //// if module as UDP (Chinese:���ģ����ΪUDP�������ָ��Ŀ���ַ�Ͷ˿ڣ�Ҳ���������䣬�ڷ�������ʱ�����û����)
   #define TEST_REMOTE_ADDR    		"115.28.24.140"
   #define TEST_REMOTE_PORT  	    7010
	//local port	(Chinese���׽��ֵ�Ŀ���ַ��Ŀ��˿�)
#elif (TEST_CONNECTION_TYPE==1)      //// if module as TCP Client (Chinese:���ģ����ΪTCP�ͻ��ˣ���Ȼ����ָ��Ŀ���ַ��Ŀ��˿ڣ���ģ����Ҫȥ���ӵ�TCP�������ĵ�ַ�Ͷ˿�)
   #define TEST_REMOTE_ADDR    	 	"47.105.175.47"  //"192.168.1.3"// "www.baidu.com"
   #define TEST_REMOTE_PORT  	    8085//137 //8085          //8085	// 80
#endif			 

///////
//step 0: config tcp windows number (Chinese: ����0�������TCP���͵��׽��֣��������õ������ڲ�����
#if ( 1 && ((TEST_CONNECTION_TYPE==1)||(TEST_CONNECTION_TYPE==2)) ) //If you hope to change TCP Windows, please change '0' to '1' in the #if clause before setup the connection
																																		//(Chinese: �����ı��׽��ֵĴ����������Խ�#if����е�0�ĳ�1�����������Ҫ�ڴ����׽���֮ǰִ��)
// u8 M8266WIFI_SPI_Config_Tcp_Window_num(u8 link_no, u8 tcp_wnd_num, u16* status)
  if(M8266WIFI_SPI_Config_Tcp_Window_num(link_no, 4, &status)==0)
  {
		 while(1)
		 {
			 printf("\r\n���õ������ڲ���ʧ�ܣ�\r\n");
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
//step 1: setup connection (Chinese: ����1�������׽������ӣ�

  //  u8 M8266WIFI_SPI_Setup_Connection(u8 tcp_udp, u16 local_port, char remote_addr, u16 remote_port, u8 link_no, u8 timeout_in_s, u16* status);
	if(M8266WIFI_SPI_Setup_Connection(TEST_CONNECTION_TYPE, TEST_LOCAL_PORT, TEST_REMOTE_ADDR, TEST_REMOTE_PORT, link_no, 20, &status)==0)
	{		
		 while(1)
		 {
			 printf("\r\n�����׽���ʧ�ܣ�\r\n");
			 delay_ms(1000);
		 }
	}
	printf("\r\n�����׽��ֳɹ���\r\n");
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
//			printf("\r\n�ɹ�����%d�ֽ�\r\n",sent);
		}
		else if( (status&0xFF) == 0x1E)			       // 0x1E = too many errors ecountered during sending and can not fixed, or transsmission blocked heavily(Chinese: ���ͽ׶�����̫��Ĵ���������ˣ����Կ��ǼӴ�max_loops)
		{
			debug_point = 1;
//			printf("\r\n����max_loops��ֵ\r\n");
			//add some process here (Chinese: �����ڴ˴���һЩ������������max_loops��ֵ)
		}
		else if(  ((status&0xFF) == 0x14)			 // 0x14 = connection of link_no not present (Chinese: ���׽��ֲ�����)
		 || ((status&0xFF) == 0x15) )    // 0x15 = connection of link_no closed(Chinese: ���׽����Ѿ��رջ�Ͽ�)			
		{
			 debug_point = 2;
//			printf("\r\n��Ҫ�ؽ������׽���\r\n");
			 //need to re-establish the socket connection (Chinese: ��Ҫ�ؽ������׽���)
		}
		else if( (status&0xFF) == 0x18 )        // 0x18 = TCP server in listening states and no tcp clients have connected. (Chinese: ���TCP��������û�пͻ�����������)
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
