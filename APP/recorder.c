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
u8 i2srecbuf_sta=0;	//DMA���ݻ���״̬
FIL* f_rec=0;		//¼���ļ�	
u32 wavsize;		//wav���ݴ�С(�ֽ���,�������ļ�ͷ!!)
u8 rec_sta=0;		//¼��״̬��0x80��ʾ��ʼ¼����0��ʾֹͣ¼��				
				
//¼�� I2S_DMA�����жϷ�����.���ж�����д������
void rec_i2s_dma_rx_callback(void) 
{    
	u16 *pbuf;
	u8 res;
	u16 i,j;
	if(rec_sta==0X80)//¼��ģʽ
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

const u16 i2splaybuf[2]={0X0000,0X0000};//2��16λ����,����¼��ʱI2S Master����.ѭ������0.
//����PCM ¼��ģʽ 		  
void recoder_enter_rec_mode(void)
{
	WM8978_ADDA_Cfg(0,1);		//����ADC
	WM8978_Input_Cfg(0,1,0);	//��������ͨ��(Line)
	WM8978_Output_Cfg(0,1);		//����BYPASS��� 
	WM8978_MIC_Gain(46);		//MIC�������� 
	WM8978_SPKvol_Set(0);		//�ر�����.
	WM8978_I2S_Cfg(2,0);		//�����ֱ�׼,16λ���ݳ���
	I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16b);			//�����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ֡���� 
	I2S2ext_Init(I2S_Standard_Phillips,I2S_Mode_SlaveRx,I2S_CPOL_Low,I2S_DataFormat_16b);		//�����ֱ�׼,�ӻ�����,ʱ�ӵ͵�ƽ��Ч,16λ֡����	
	I2S2ext_RX_DMA_Init(recbuf1,recbuf2,I2S_RX_DMA_BUF_SIZE/2); 		//����RX DMA
  	i2s_rx_callback=rec_i2s_dma_rx_callback;								//�ص�����ָwav_i2s_dma_callback
	I2S2_TX_DMA_Init((u8*)&i2splaybuf[0],(u8*)&i2splaybuf[1],1); 		//����TX DMA,����ʹ��ȫ˫��ģʽ��������Ϊ��ģʽ����
	DMA1_Stream4->CR&=~(1<<4);	//�رմ�������ж�(���ﲻ���ж�������)
	I2S_Play_Start();	//��ʼI2S���ݷ���(����)							//��Ҫ
	I2S_Rec_Start(); 	//��ʼI2S���ݽ���(�ӻ�)
}  

//��ʼ��WAVͷ.
void recoder_wav_init(__WaveHeader* wavhead) //��ʼ��WAVͷ
{
	wavhead->riff.ChunkID=0X46464952;	//"RIFF"
	wavhead->riff.ChunkSize=0;			//��δȷ��,�����Ҫ����
	wavhead->riff.Format=0X45564157; 	//"WAVE"
	wavhead->fmt.ChunkID=0X20746D66; 	//"fmt "
	wavhead->fmt.ChunkSize=16; 			//��СΪ16���ֽ�
	wavhead->fmt.AudioFormat=0X01; 		//0X01,��ʾPCM;0X01,��ʾIMA ADPCM
 	wavhead->fmt.NumOfChannels=1;		////������
 	wavhead->fmt.SampleRate=REC_SAMPLE;	//16Khz������ ��������
 	wavhead->fmt.ByteRate=wavhead->fmt.SampleRate*2;////�ֽ�����=������*ͨ����1*(ADCλ��/8)
 	wavhead->fmt.BlockAlign=2;			////���С=ͨ����1*(ADCλ��/8)
 	wavhead->fmt.BitsPerSample=16;		//16λPCM
   	wavhead->data.ChunkID=0X61746164;	//"data"
 	wavhead->data.ChunkSize=0;			//���ݴ�С,����Ҫ����  
} 						    
 	

//ͨ��ʱ���ȡ�ļ���
//������SD������,��֧��FLASH DISK����
//��ϳ�:����"0:RECORDER/REC20120321210633.wav"���ļ���
void recoder_new_pathname(char *pname)
{	 
	RTC_TimeTypeDef RTC_TimeStruct;
    RTC_DateTypeDef RTC_DateStruct;
	
	RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
	RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
	
	sprintf((char*)pname,"0:/RECORDER/%02d%02d%02d_%02d%02d%02d.wav",RTC_DateStruct.RTC_Year,RTC_DateStruct.RTC_Month,RTC_DateStruct.RTC_Date,
	RTC_TimeStruct.RTC_Hours,RTC_TimeStruct.RTC_Minutes,RTC_TimeStruct.RTC_Seconds);
}     

//WAV¼�� 
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
//	while(f_opendir(&recdir,"0:/RECORDER"))//��¼���ļ���
// 	{
//		printf("RECORDER�ļ��д���!\r\n");  
//		delay_ms(500);				  
//		f_mkdir("0:/RECORDER");				//������Ŀ¼   
//	}
	recbuf1=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�1����
	recbuf2=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�2����  
//	f_rec=(FIL *)mymalloc(SRAMIN,sizeof(FIL));		//����FIL�ֽڵ��ڴ�����
//	wavhead=(__WaveHeader*)mymalloc(SRAMIN,sizeof(__WaveHeader));//����__WaveHeader�ֽڵ��ڴ����� 
//	pname=(char*)mymalloc(SRAMIN,30);						//����30���ֽ��ڴ�,����"0:RECORDER/REC00001.wav" 
	if(!recbuf1||!recbuf2)	rval=1;		//�����ڴ����
	if(rval == 0)
	{
		recoder_enter_rec_mode();				//����¼��ģʽ,��ʱ��������������ͷ�ɼ�������Ƶ 
		
//		recoder_new_pathname(pname);			//�õ��µ�����
//		recoder_wav_init(wavhead);				//��ʼ��wav����
//		res=f_open(f_rec,(const TCHAR*)pname, FA_CREATE_ALWAYS | FA_WRITE);
//		if(res)			//�ļ�����ʧ��
//		{
//			rec_sta=0;	//�����ļ�ʧ��,����¼��
//			rval=1;		//��ʾ�Ƿ����SD��
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
						printf("\r\ncheck��ȷ\r\n");
						break;
					}else	
					{
//					    M8266WIFI_SPI_Send_Udp_Data(check_inf,strlen((char*)check_inf),0,"115.28.24.140",7010,&status);
						M8266WIFI_Tans(check_inf,strlen((char*)check_inf));
					}
			}
			}
			
			rec_sta = 0x80;	//��ʼ¼��
			printf("��ʼ¼��������\r\n");			
//		}
	
	while(rval==0)
	{	
		if(wavsize>=(1024*1024*1))//¼���ļ�����300M���Զ����棬���½��ļ�����¼��
			{
				rec_sta=0;	//�ر�¼��
//				printf("File Size:%d\r\n",(u32)f_size(f_rec));
//				printf("�ļ���:%s\r\n",pname);
//				wavhead->riff.ChunkSize=wavsize+36;		//�����ļ��Ĵ�С-8;.
//				wavhead->data.ChunkSize=wavsize;		//���ݴ�С
//				f_lseek(f_rec,0);						//ƫ�Ƶ��ļ�ͷ.
//				f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//д��ͷ����
//				f_close(f_rec);
//				if(tanspar_flag==1)
				M8266WIFI_SPI_Delete_Connection(0,NULL);
				wavsize=0;
				break;    
//				else
//				{
				
//				recoder_new_pathname(pname);			//�õ��µ�����
//				recoder_wav_init(wavhead);				//��ʼ��wav����
//				res=f_open(f_rec,(const TCHAR*)pname, FA_CREATE_ALWAYS | FA_WRITE);
//				rec_sta|=0X80;	//��ʼ¼��	
//				}
			}
		}
	}
	myfree(SRAMIN,recbuf1);		//�ͷ��ڴ�
	myfree(SRAMIN,recbuf2);		//�ͷ��ڴ�  
//	myfree(SRAMIN,f_rec);		//�ͷ��ڴ�
//	myfree(SRAMIN,wavhead);		//�ͷ��ڴ�  
//	myfree(SRAMIN,pname);		//�ͷ��ڴ�
	delay_ms(2000);
}


