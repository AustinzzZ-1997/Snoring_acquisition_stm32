#ifndef __RECORDER_H
#define __RECORDER_H

#include "sys.h"

#if defined(FRAME_SIZE)
	#define I2S_RX_DMA_BUF_SIZE    	FRAME_SIZE		//定义RX DMA 数组大小
#else
	#define I2S_RX_DMA_BUF_SIZE    	4096			//定义RX DMA 数组大小
#endif

//RIFF块 12字节
typedef __packed struct
{
    u32 ChunkID;		   	//chunk id;这里固定为"RIFF",即0X46464952
    u32 ChunkSize ;		   	//集合大小;文件总大小-8
    u32 Format;	   			//格式;WAVE,即0X45564157
}ChunkRIFF ;
//fmt块 24字节
typedef __packed struct
{
    u32 ChunkID;		   	//chunk id;这里固定为"fmt ",即0X20746D66
    u32 ChunkSize ;		   	//子集合大小(不包括ID和Size);这里为:20.
    u16 AudioFormat;	  	//音频格式;0X01,表示线性PCM;0X11表示IMA ADPCM
	u16 NumOfChannels;		//通道数量;1,表示单声道;2,表示双声道;
	u32 SampleRate;			//采样率;0X1F40,表示8Khz
	u32 ByteRate;			//字节速率; 
	u16 BlockAlign;			//块对齐(字节); 
	u16 BitsPerSample;		//单个采样数据大小;4位ADPCM,设置为4
//	u16 ByteExtraData;		//附加的数据字节;2个; 线性PCM,没有这个参数
}ChunkFMT;	   
//fact块 12字节
typedef __packed struct 
{
    u32 ChunkID;		   	//chunk id;这里固定为"fact",即0X74636166;
    u32 ChunkSize ;		   	//子集合大小(不包括ID和Size);这里为:4.
    u32 NumOfSamples;	  	//采样的数量; 
}ChunkFACT;
//LIST块 8字节
typedef __packed struct 
{
    u32 ChunkID;		   	//chunk id;这里固定为"LIST",即0X74636166;
    u32 ChunkSize ;		   	//子集合大小(不包括ID和Size);这里为:4. 
}ChunkLIST;

//data块 8字节
typedef __packed struct 
{
    u32 ChunkID;		   	//chunk id;这里固定为"data",即0X5453494C
    u32 ChunkSize ;		   	//子集合大小(不包括ID和Size) 
}ChunkDATA;

//wav头
typedef __packed struct
{ 
	ChunkRIFF riff;	//riff块
	ChunkFMT fmt;  	//fmt块
//	ChunkFACT fact;	//fact块 线性PCM,没有这个结构体	 
	ChunkDATA data;	//data块		 
}__WaveHeader; 


extern void wav_recorder(void);
extern void recoder_new_pathname1(void);
#endif












