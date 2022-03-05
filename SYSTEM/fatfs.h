#ifndef __FATFS_H
#define __FATFS_H

#include "ff.h"
#include "sys.h"

#define SUPPORT_MALLOC 	1		//Ϊ��϶������

#if SUPPORT_MALLOC
	#include "malloc.h"
#endif

extern UINT br,bw;					//��д����
extern FILINFO fileinfo;			//�ļ���Ϣ
extern DIR dir;  					//Ŀ¼
extern FATFS *diskfs[FF_VOLUMES];	//�߼����̹�����.
extern FIL *file;	 				//�ļ�1
extern FIL *ftemp;	 				//�ļ�2
extern u8 *fatbuf;					//SD�����ݻ�����


//�ļ����ͣ���FILE_TYPE_TBL�ļ��б����Ӧ
typedef enum 
{
	FT_BIN 	= 0X00,
	
	FT_LRC 	= 0X10,
	
	FT_NES 	= 0X20,
	FT_SMS 	= 0X21,
	
	FT_TXT 	= 0X30,
	FT_C 	= 0X31,
	FT_H	= 0X32,
	
	FT_WAV	= 0X40,
	FT_MP3	= 0X41,
	FT_APE	= 0X42,
	FT_FLAC	= 0X43,
	
	FT_BMP	= 0X50,
	FT_JPG 	= 0X51,
	FT_JPEG = 0X52,
	FT_GIF	= 0X53,
	
	FT_AVI	= 0X60,
	
	FT_ERROR = 0XFF,
} FT_Type;


extern u8 fatfs_init(void);
extern FT_Type fatfs_ftype(char *fname);
extern FRESULT fatfs_df(u8 *drv,u32 *total,u32 *free);

#endif
