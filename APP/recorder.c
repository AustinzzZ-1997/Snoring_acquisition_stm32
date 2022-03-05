#include "recorder.h"
#include "key.h"
#include "fatfs.h"
#include "wm8978.h"
#include "i2s.h"
#include "led.h"
#include "stdio.h"
#include "string.h"  
#include "exti.h"
#include "wifi.h"
#include "M8266Hostif.h"
#include "M8266WIFIDrv.h"

u16 *recbuf1;
u16 *recbuf2; 
u8 i2srecbuf_sta=0;	//DMA数据缓存状态
FIL* f_rec=0;		//录音文件	
u32 wavsize;		//wav数据大小(字节数,不包括文件头!!)
u8 rec_sta=0;		//录音状态，0x80表示开始录音，0表示停止录音				
				
//录音 I2S_DMA接收中断服务函数.在中断里面写入数据
void rec_i2s_dma_rx_callback(void) 
{    
	u16 *pbuf;
	u8 res;
	u16 i,j;
	if(rec_sta==0X80)//录音模式
	{  
		if(DMA1_Stream3->CR&(1<<19))
		pbuf = recbuf1;
		else
		pbuf = recbuf2;
		for(i=0,j=0;i<I2S_RX_DMA_BUF_SIZE/2;i++,j+=2)
		{
			pbuf[i]=pbuf[j];
		}
//		res = f_write(f_rec,pbuf,I2S_RX_DMA_BUF_SIZE/2,(UINT*)&bw);
		M8266WIFI_SPI_Send_BlockData((u8 *)pbuf, I2S_RX_DMA_BUF_SIZE/2, 5000, 0, NULL, 0, NULL);
		if(res)
		{
			printf("write error:%d\r\n",res);
		}
		else
		{
			wavsize+=I2S_RX_DMA_BUF_SIZE/2;
			printf("%d\r\n",wavsize);
		}
	}	
}

const u16 i2splaybuf[2]={0X0000,0X0000};//2个16位数据,用于录音时I2S Master发送.循环发送0.
//进入PCM 录音模式 		  
void recoder_enter_rec_mode(void)
{
	WM8978_ADDA_Cfg(0,1);		//开启ADC
	WM8978_Input_Cfg(0,1,0);	//开启输入通道(Line)
	WM8978_Output_Cfg(0,1);		//开启BYPASS输出 
	WM8978_MIC_Gain(46);		//MIC增益设置 
	WM8978_SPKvol_Set(0);		//关闭喇叭.
	WM8978_I2S_Cfg(2,0);		//飞利浦标准,16位数据长度
	I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16b);			//飞利浦标准,主机发送,时钟低电平有效,16位帧长度 
	I2S2ext_Init(I2S_Standard_Phillips,I2S_Mode_SlaveRx,I2S_CPOL_Low,I2S_DataFormat_16b);		//飞利浦标准,从机接收,时钟低电平有效,16位帧长度	
	I2S2ext_RX_DMA_Init(recbuf1,recbuf2,I2S_RX_DMA_BUF_SIZE/2); 		//配置RX DMA
  	i2s_rx_callback=rec_i2s_dma_rx_callback;								//回调函数指wav_i2s_dma_callback
	I2S2_TX_DMA_Init((u8*)&i2splaybuf[0],(u8*)&i2splaybuf[1],1); 		//配置TX DMA,由于使用全双工模式，且配置为主模式发送
	DMA1_Stream4->CR&=~(1<<4);	//关闭传输完成中断(这里不用中断送数据)
	I2S_Play_Start();	//开始I2S数据发送(主机)							//需要
	I2S_Rec_Start(); 	//开始I2S数据接收(从机)
}  

//初始化WAV头.
void recoder_wav_init(__WaveHeader* wavhead) //初始化WAV头
{
	wavhead->riff.ChunkID=0X46464952;	//"RIFF"
	wavhead->riff.ChunkSize=0;			//还未确定,最后需要计算
	wavhead->riff.Format=0X45564157; 	//"WAVE"
	wavhead->fmt.ChunkID=0X20746D66; 	//"fmt "
	wavhead->fmt.ChunkSize=16; 			//大小为16个字节
	wavhead->fmt.AudioFormat=0X01; 		//0X01,表示PCM;0X01,表示IMA ADPCM
 	wavhead->fmt.NumOfChannels=1;		////单声道
 	wavhead->fmt.SampleRate=REC_SAMPLE;	//16Khz采样率 采样速率
 	wavhead->fmt.ByteRate=wavhead->fmt.SampleRate*2;////字节速率=采样率*通道数1*(ADC位数/8)
 	wavhead->fmt.BlockAlign=2;			////块大小=通道数1*(ADC位数/8)
 	wavhead->fmt.BitsPerSample=16;		//16位PCM
   	wavhead->data.ChunkID=0X61746164;	//"data"
 	wavhead->data.ChunkSize=0;			//数据大小,还需要计算  
} 						    
 	

//通过时间获取文件名
//仅限在SD卡保存,不支持FLASH DISK保存
//组合成:形如"0:RECORDER/REC20120321210633.wav"的文件名
void recoder_new_pathname(char *pname)
{	 
	RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	
	sprintf((char*)pname,"0:/RECORDER/%02d%02d%02d_%02d%02d%02d.wav",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,
	RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
}     

//WAV录音 
void wav_recorder(void)
{ 
	u8 res;
	u8 rval = 0;
	u8 check_inf[100];
	u8 rec_data[100];
//	__WaveHeader *wavhead = 0;
//	DIR recdir;
//	char *pname = 0;
//	vu8 dma_4g_status;
//	while(f_opendir(&recdir,"0:/RECORDER"))//打开录音文件夹
// 	{
//		printf("RECORDER文件夹错误!\r\n");  
//		delay_ms(500);				  
//		f_mkdir("0:/RECORDER");				//创建该目录   
//	}
	recbuf1=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存1申请
	recbuf2=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存2申请  
//	f_rec=(FIL *)mymalloc(SRAMIN,sizeof(FIL));		//开辟FIL字节的内存区域
//	wavhead=(__WaveHeader*)mymalloc(SRAMIN,sizeof(__WaveHeader));//开辟__WaveHeader字节的内存区域 
//	pname=(char*)mymalloc(SRAMIN,30);						//申请30个字节内存,类似"0:RECORDER/REC00001.wav" 
	if(!recbuf1||!recbuf2)	rval=1;		//申请内存出错
	if(rval == 0)
	{
		recoder_enter_rec_mode();				//进入录音模式,此时耳机可以听到咪头采集到的音频 
		
//		recoder_new_pathname(pname);			//得到新的名字
//		recoder_wav_init(wavhead);				//初始化wav数据
//		res=f_open(f_rec,(const TCHAR*)pname, FA_CREATE_ALWAYS | FA_WRITE);
//		if(res)			//文件创建失败
//		{
//			rec_sta=0;	//创建文件失败,不能录音
//			rval=1;		//提示是否存在SD卡
//			printf("Create wav file error!\r\n");
//		}else 
//		{

			M8266WIFI_SetSocket();
			
		
			sprintf((char*)check_inf,"fileName=%s&deviceId=A2&crc=%x&token=thisispasswordicanttellu\r\n","testinhome.wav",0xabcd);
			M8266WIFI_Tans(check_inf,strlen((char*)check_inf));
			while(1)
			{
			if(M8266WIFI_SPI_Has_DataReceived())
			{
				M8266WIFI_SPI_RecvData(rec_data,100,50,0,NULL);
				if(rec_data[9]=='t')
					{
						printf("\r\ncheck正确\r\n");
						break;
					}else	
					{
//					    M8266WIFI_SPI_Send_Udp_Data(check_inf,strlen((char*)check_inf),0,"115.28.24.140",7010,&status);
						M8266WIFI_Tans(check_inf,strlen((char*)check_inf));
					}
			}
			}
			
			rec_sta = 0x80;	//开始录音
			printf("开始录音。。。\r\n");			
//		}
	
	while(rval==0)
	{	
		if(wavsize>=(1024*1024*1))//录音文件超过300M，自动保存，并新建文件继续录音
			{
				rec_sta=0;	//关闭录音
//				printf("File Size:%d\r\n",(u32)f_size(f_rec));
//				printf("文件名:%s\r\n",pname);
//				wavhead->riff.ChunkSize=wavsize+36;		//整个文件的大小-8;.
//				wavhead->data.ChunkSize=wavsize;		//数据大小
//				f_lseek(f_rec,0);						//偏移到文件头.
//				f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//写入头数据
//				f_close(f_rec);
//				if(tanspar_flag==1)
				M8266WIFI_SPI_Delete_Connection(0,NULL);
				wavsize=0;
				break;    
//				else
//				{
				
//				recoder_new_pathname(pname);			//得到新的名字
//				recoder_wav_init(wavhead);				//初始化wav数据
//				res=f_open(f_rec,(const TCHAR*)pname, FA_CREATE_ALWAYS | FA_WRITE);
//				rec_sta|=0X80;	//开始录音	
//				}
			}
		}
	}
	myfree(SRAMIN,recbuf1);		//释放内存
	myfree(SRAMIN,recbuf2);		//释放内存  
//	myfree(SRAMIN,f_rec);		//释放内存
//	myfree(SRAMIN,wavhead);		//释放内存  
//	myfree(SRAMIN,pname);		//释放内存
	delay_ms(2000);
}


