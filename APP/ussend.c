#include "ussend.h"
#include "fatfs.h"
#include "malloc.h"
#include "led.h"
#include "string.h"
#include "4g_uart.h"
#include "i2s.h"
#include "http.h"
#include "crc.h"
#include "wifi.h"
#include "M8266Hostif.h"
#include "M8266WIFIDrv.h"

vu8 tx_flag;				//发送标志	(1)就绪;(0)未就绪
vu8 DMA1_tx_flag;				//发送标志	(1)就绪;(0)未就绪
unsigned char crc_buf[1000];
u16 status_flag=0;

void M8266WIFI_Tans(u8 snd_data[],u32 TEST_SEND_DATA_SIZE);

u8 us_dma_send(void)
{	
	u16 status = 0,sent;
	u8  read_flag=0;
	uint16_t crc=0xFFFF;
	u8 res;
	u8 pname[30];
	u8 check_inf[100];
	u8 rec_data[100];
	DIR recdir;
	FIL *ftemp;
	ftemp = (FIL*)mymalloc(SRAMIN, sizeof(FIL));
	res = f_opendir(&recdir,"0:/RECORDER");
	if(res==FR_OK)
	{
		while(1)
		{
			res = f_readdir(&recdir, &fileinfo);
			if(res!=FR_OK||fileinfo.fname[0]==0)		
			break;//错误了/到末尾了,退出
			res = fatfs_ftype(fileinfo.fname);
			if(res==FT_WAV)
			{	
				sprintf((char*)pname,"0:/RECORDER/%s",fileinfo.fname);
				printf("pname:%s\r\n",fileinfo.fname);
				res = f_open(ftemp,(TCHAR*)pname,FA_READ);		
				br = UART_DMA_TX_LEN;
				M8266WIFI_SetSocket();
//				delay_ms(5000);
				if(res==FR_OK)
				{
					while(1)
					{
							if(br==UART_DMA_TX_LEN&&read_flag==1)
							{
								if(f_read(ftemp,UART_DMA_TX_BUF,UART_DMA_TX_LEN,&br)==FR_OK)
								{	
//									sent=M8266WIFI_SPI_Send_Data(UART_DMA_TX_BUF,br,0,&status);
//								    sent = M8266WIFI_SPI_Send_Udp_Data(UART_DMA_TX_BUF,br,0,"115.28.24.140",7010,&status);

									sent = M8266WIFI_SPI_Send_BlockData(UART_DMA_TX_BUF, br, 5000, 0, NULL, 0, &status);
//								    printf("sent:%d\r\n",sent);
//									status_flag++;
//									if(status_flag==2)
//									{
//									 delay_ms(150);
//									 status_flag=0;
//									}
//									delay_ms(10);
//									printf("\r\n实际发出%d字节,status:%x\r\n",sent,status);
//									if(status!=0)
//									M8266WIFI_SetSocket();
//									memset(UART_DMA_TX_BUF,0,br);
//									M8266WIFI_Tans(UART_DMA_TX_BUF,br);
//									printf("br:%d\r\n",br);
//									delay_ms(20);
								}
							}
							else if(br==UART_DMA_TX_LEN&&read_flag==0)
							{
								if(f_read(ftemp,UART_DMA_TX_BUF,UART_DMA_TX_LEN,&br)==FR_OK)
								{				
									crc=calculate_crc16(crc,UART_DMA_TX_BUF,br);
								}
							}
							else if(br<UART_DMA_TX_LEN&&read_flag==1)			//br如果小于读取值，则表示到达文件尾
							{
								read_flag=0;
								break;
							}
							else if(br<UART_DMA_TX_LEN&&read_flag==0)			//br如果小于读取值，则表示到达文件尾
							{
								crc= (crc&0xff)<<8|(crc&0xff00)>>8;
								sprintf((char*)check_inf,"fileName=%s&deviceId=A2&crc=%x&token=thisispasswordicanttellu\r\n",fileinfo.fname,crc);
//								M8266WIFI_SPI_Send_Udp_Data(check_inf,strlen((char*)check_inf),0,"115.28.24.140",7010,&status);
								M8266WIFI_Tans(check_inf,strlen((char*)check_inf));
//								printf("okk\r\n");
								while(1)
								{
								if(M8266WIFI_SPI_Has_DataReceived())
								{
									M8266WIFI_SPI_RecvData(rec_data,100,50,0,&status);
									if(rec_data[9]=='t')
										{
											printf("\r\ncheck正确\r\n");
											break;
										}else	
										{
//											M8266WIFI_SPI_Send_Udp_Data(check_inf,strlen((char*)check_inf),0,"115.28.24.140",7010,&status);
											M8266WIFI_Tans(check_inf,strlen((char*)check_inf));
										}
								}
								}
								crc=0xFFFF;
								read_flag=1;
								f_lseek(ftemp,0);
								br=UART_DMA_TX_LEN;
							}
					}
					f_close(ftemp);
					printf("\r\n完成一个文件传输\r\n");
					M8266WIFI_SPI_Delete_Connection(0,&status);
				}
			}
		}
		f_closedir(&recdir);
		return 0;
	}
	return 1;
}


void DMA2_Stream7_IRQHandler(void)
{	
	if(DMA_GetITStatus(DMA2_Stream7, DMA_IT_TCIF7)==SET)
	{
		DMA_Cmd(DMA2_Stream7, DISABLE);
		DMA_ClearITPendingBit(DMA2_Stream7, DMA_IT_TCIF7);
		tx_flag = 1;
	}
}

//u32 CRC32( u8 *pBuf, u16 nSize )
//{
//  u32 index = 0;
//  CRC_ResetDR( ); //复位CRC
//  for ( index = 0; index < nSize; index++ )
//  {
//    CRC->DR = (u32) pBuf[ index ];
//  }
//  return ( CRC->DR );
//}
