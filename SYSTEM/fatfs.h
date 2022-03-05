#ifndef __FATFS_H
#define __FATFS_H

#include "ff.h"
#include "sys.h"

#define SUPPORT_MALLOC 	1		//为配合而导入的

#if SUPPORT_MALLOC
	#include "malloc.h"
#endif

extern UINT br,bw;					//读写变量
extern FILINFO fileinfo;			//文件信息
extern DIR dir;  					//目录
extern FATFS *diskfs[FF_VOLUMES];	//逻辑磁盘工作区.
extern FIL *file;	 				//文件1
extern FIL *ftemp;	 				//文件2
extern u8 *fatbuf;					//SD卡数据缓存区


//文件类型，与FILE_TYPE_TBL文件列表相对应
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
