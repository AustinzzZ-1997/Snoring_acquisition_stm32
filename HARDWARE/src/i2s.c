#include "i2s.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//I2S 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/5/24
//版本：V1.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
//********************************************************************************
//V1.1 20140606
//新增I2S2ext_Init、I2S2ext_RX_DMA_Init、I2S_Rec_Start和I2S_Rec_Stop等函数
////////////////////////////////////////////////////////////////////////////////// 	
 

//I2S2初始化
//参数I2S_Standard:  @ref SPI_I2S_Standard  I2S标准,
//I2S_Standard_Phillips,飞利浦标准;
//I2S_Standard_MSB,MSB对齐标准(右对齐);
//I2S_Standard_LSB,LSB对齐标准(左对齐);
//I2S_Standard_PCMShort,I2S_Standard_PCMLong:PCM标准
//参数I2S_Mode: @ref SPI_I2S_Mode  I2S_Mode_SlaveTx:从机发送;I2S_Mode_SlaveRx:从机接收;I2S_Mode_MasterTx:主机发送;I2S_Mode_MasterRx:主机接收;
//参数I2S_Clock_Polarity  @ref SPI_I2S_Clock_Polarity:  I2S_CPOL_Low,时钟低电平有效;I2S_CPOL_High,时钟高电平有效
//参数I2S_DataFormat： @ref SPI_I2S_Data_Format :数据长度,I2S_DataFormat_16b,16位标准;I2S_DataFormat_16bextended,16位扩展(frame=32bit);I2S_DataFormat_24b,24位;I2S_DataFormat_32b,32位.
void I2S2_Init(u16 I2S_Standard,u16 I2S_Mode,u16 I2S_Clock_Polarity,u16 I2S_DataFormat)
{ 
  I2S_InitTypeDef I2S_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);//使能SPI2时钟
	
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,ENABLE); //复位SPI2
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2,DISABLE);//结束复位
  
	I2S_InitStructure.I2S_Mode=I2S_Mode;//IIS模式
	I2S_InitStructure.I2S_Standard=I2S_Standard;//IIS标准
	I2S_InitStructure.I2S_DataFormat=I2S_DataFormat;//IIS数据长度
	I2S_InitStructure.I2S_MCLKOutput=I2S_MCLKOutput_Enable;//主时钟输出使能
	I2S_InitStructure.I2S_AudioFreq=REC_SAMPLE;//IIS频率设置
	I2S_InitStructure.I2S_CPOL=I2S_Clock_Polarity;//空闲状态时钟电平
	I2S_Init(SPI2,&I2S_InitStructure);//初始化IIS

	SPI_I2S_DMACmd(SPI2,SPI_I2S_DMAReq_Tx,ENABLE);//SPI2 TX DMA请求使能.
	I2S_Cmd(SPI2,ENABLE);//SPI2 I2S EN使能.	
} 
 

//I2S2ext配置
//参数I2S_Standard:  @ref SPI_I2S_Standard  I2S标准,
//I2S_Standard_Phillips,飞利浦标准;
//I2S_Standard_MSB,MSB对齐标准(右对齐);
//I2S_Standard_LSB,LSB对齐标准(左对齐);
//I2S_Standard_PCMShort,I2S_Standard_PCMLong:PCM标准
//参数I2S_Mode:  @ref SPI_I2S_Mode  I2S_Mode_SlaveTx:从机发送;I2S_Mode_SlaveRx:从机接收;
//参数I2S_Clock_Polarity   @ref SPI_I2S_Clock_Polarity:  I2S_CPOL_Low,时钟低电平有效;I2S_CPOL_High,时钟高电平有效
//参数I2S_DataFormat： @ref SPI_I2S_Data_Format :数据长度,I2S_DataFormat_16b,16位标准;I2S_DataFormat_16bextended,16位扩展(frame=32bit);I2S_DataFormat_24b,24位;I2S_DataFormat_32b,32位.

void I2S2ext_Init(u16 I2S_Standard,u16 I2S_Mode,u16 I2S_Clock_Polarity,u16 I2S_DataFormat)
{  
	I2S_InitTypeDef I2S2ext_InitStructure;
	
	I2S2ext_InitStructure.I2S_Mode=I2S_Mode^(1<<8);//IIS模式
	I2S2ext_InitStructure.I2S_Standard=I2S_Standard;//IIS标准
	I2S2ext_InitStructure.I2S_DataFormat=I2S_DataFormat;//IIS数据长度
	I2S2ext_InitStructure.I2S_MCLKOutput=I2S_MCLKOutput_Enable;//主时钟输出使能
	I2S2ext_InitStructure.I2S_AudioFreq=REC_SAMPLE;//IIS频率设置
	I2S2ext_InitStructure.I2S_CPOL=I2S_Clock_Polarity;//空闲状态时钟电平
	
	I2S_FullDuplexConfig(I2S2ext,&I2S2ext_InitStructure);//初始化I2S2ext配置
	
	SPI_I2S_DMACmd(I2S2ext,SPI_I2S_DMAReq_Rx,ENABLE);//I2S2ext RX DMA请求使能.
 
	I2S_Cmd(I2S2ext,ENABLE);		//I2S2ext I2S EN使能.
	
} 
 

//I2S2 TX DMA配置
//设置为双缓冲模式,并开启DMA传输完成中断
//buf0:M0AR地址.
//buf1:M1AR地址.
//num:每次传输数据量
void I2S2_TX_DMA_Init(u8* buf0,u8 *buf1,u16 num)
{  
	NVIC_InitTypeDef   NVIC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	
 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
	
	DMA_DeInit(DMA1_Stream4);
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE){}//等待DMA1_Stream4可配置 
		
	DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_FEIF4|DMA_IT_DMEIF4|DMA_IT_TEIF4|DMA_IT_HTIF4|DMA_IT_TCIF4);//清空DMA1_Stream4上所有中断标志

  /* 配置 DMA Stream */

	DMA_InitStructure.DMA_Channel = DMA_Channel_0;  //通道0 SPI2_TX通道 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&SPI2->DR;//外设地址为:(u32)&SPI2->DR
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)buf0;//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;//存储器到外设模式
	DMA_InitStructure.DMA_BufferSize = num;//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//存储器数据长度：16位 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// 使用循环模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//高优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //不使用FIFO模式        
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//外设突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//存储器突发单次传输
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);//初始化DMA Stream
	
	DMA_DoubleBufferModeConfig(DMA1_Stream4,(u32)buf1,DMA_Memory_0);//双缓冲模式配置
 
	DMA_DoubleBufferModeCmd(DMA1_Stream4,ENABLE);//双缓冲模式开启
 
	DMA_ITConfig(DMA1_Stream4,DMA_IT_TC,ENABLE);//开启传输完成中断
 
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置
} 
//I2S2ext RX DMA配置
//设置为双缓冲模式,并开启DMA传输完成中断
//buf0:M0AR地址.
//buf1:M1AR地址.
//num:每次传输数据量
void I2S2ext_RX_DMA_Init(u16* buf0,u16 *buf1,u16 num)
{ 	
	NVIC_InitTypeDef   NVIC_InitStructure;
	DMA_InitTypeDef  DMA_InitStructure;
	
 
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE);//DMA1时钟使能 
	
	DMA_DeInit(DMA1_Stream3);
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE){}//等待DMA1_Stream3可配置 
		
	DMA_ClearITPendingBit(DMA1_Stream3,DMA_IT_FEIF3|DMA_IT_DMEIF3|DMA_IT_TEIF3|DMA_IT_HTIF3|DMA_IT_TCIF3);//清空DMA1_Stream3上所有中断标志

  /* 配置 DMA Stream */

	DMA_InitStructure.DMA_Channel = DMA_Channel_3;  //通道3 I2S2ext_RX通道 
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&I2S2ext->DR;//外设地址为:(u32)&I2S2ext->DR>DR
	DMA_InitStructure.DMA_Memory0BaseAddr = (u32)buf0;//DMA 存储器0地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//外设到存储器模式
	DMA_InitStructure.DMA_BufferSize = num;//数据传输量 
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设数据长度:16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//存储器数据长度：16位 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;// 使用循环模式 
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//中等优先级
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable; //不使用FIFO模式        
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//外设突发单次传输
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//存储器突发单次传输
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);//初始化DMA Stream
	
	DMA_DoubleBufferModeConfig(DMA1_Stream3,(u32)buf1,DMA_Memory_0);//双缓冲模式配置
 
	DMA_DoubleBufferModeCmd(DMA1_Stream3,ENABLE);//双缓冲模式开启
 
	DMA_ITConfig(DMA1_Stream3,DMA_IT_TC,ENABLE);//开启传输完成中断
  
		
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority =0x00;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);//配置
	
	 
} 

//I2S DMA回调函数指针
void (*i2s_tx_callback)(void);	//TX回调函数
void (*i2s_rx_callback)(void);	//RX回调函数
extern vu8 dma_4g_status;
extern vu8 uart4_tx_flag;
//DMA1_Stream4中断服务函数
void DMA1_Stream4_IRQHandler(void)
{      
	if(DMA_GetITStatus(DMA1_Stream4,DMA_IT_TCIF4)==SET)////DMA1_Stream4,传输完成标志
	{ 
		DMA_ClearITPendingBit(DMA1_Stream4,DMA_IT_TCIF4);
//		if(dma_4g_status==0)
//			i2s_tx_callback();	//执行回调函数,读取数据等操作在这里面处理
//		else
			uart4_tx_flag = 1;	//4G模块DMA判断传输完成
	}   											 
} 

//DMA1_Stream3中断服务函数
void DMA1_Stream3_IRQHandler(void)
{      
	if(DMA_GetITStatus(DMA1_Stream3,DMA_IT_TCIF3)==SET)	//DMA1_Stream3,传输完成标志
	{ 
		DMA_ClearITPendingBit(DMA1_Stream3,DMA_IT_TCIF3);	//清除传输完成中断
      	i2s_rx_callback();	//执行回调函数,读取数据等操作在这里面处理  
	}  											 
} 
//I2S开始播放
void I2S_Play_Start(void)
{   	  
	DMA_Cmd(DMA1_Stream4,ENABLE);//开启DMA TX传输,开始播放 	
}
//关闭I2S播放
void I2S_Play_Stop(void)
{   	 
	DMA_Cmd(DMA1_Stream4,DISABLE);//关闭DMA,结束播放 
}
//I2S开始录音
void I2S_Rec_Start(void)
{   	    
	DMA_Cmd(DMA1_Stream3,ENABLE);//开启DMA TX传输,开始录音	
}
//关闭I2S录音
void I2S_Rec_Stop(void)
{  
	DMA_Cmd(DMA1_Stream3,DISABLE);//关闭DMA,结束录音	 
	 
}








