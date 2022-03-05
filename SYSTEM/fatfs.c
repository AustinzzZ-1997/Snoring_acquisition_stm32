#include "fatfs.h"
#include <string.h>


#define FILE_MAX_TYPE_NUM		7	//最多FILE_MAX_TYPE_NUM个大类
#define FILE_MAX_SUBT_NUM		4	//最多FILE_MAX_SUBT_NUM个小类

//文件类型列表
u8* const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
	{"BIN"},			//BIN文件
	{"LRC"},			//LRC文件
	{"NES","SMS"},		//NES/SMS文件
	{"TXT","C","H"},	//文本文件
	{"WAV","MP3","APE","FLAC"},//支持的音乐文件
	{"BMP","JPG","JPEG","GIF"},//图片文件
	{"AVI"},			//视频文件
};

/* ------------------公共文件区------------------------*/
UINT br,bw;					//读写变量
FILINFO fileinfo;			//文件信息
DIR dir;  					//目录

FATFS *diskfs[FF_VOLUMES];	//逻辑磁盘工作区.	 
FIL *file;	  				//文件1
FIL *ftemp;	  				//文件2.
u8 *fatbuf;					//SD卡数据缓存区

#if !SUPPORT_MALLOC
FATFS _diskfs[FF_VOLUMES];	//逻辑磁盘工作区.	 
FIL _file;	  				//文件1
FIL _ftemp;	  				//文件2.
u8 _fatbuf[512];			//SD卡数据缓存区
#endif


//申请fatfs所需内存
u8 fatfs_init(void)
{
	u8 i;
#if defined(__MALLOC_H)
	for(i=0; i<FF_VOLUMES; i++)
	{
		diskfs[i] = (FATFS*)mymalloc(SRAMIN, sizeof(FATFS));
		if(!diskfs[i]) break;
	}
	file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为file申请内存
	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//为ftemp申请内存
	fatbuf=(u8*)mymalloc(SRAMIN,512);				//为fatbuf申请内存
	return (i==FF_VOLUMES&&file&&ftemp&&fatbuf)?0:1;//申请有一个失败,即失败.
#else
	for(i=0; i<FF_VOLUMES; i++)
	{
		diskfs[i] = _diskfs+i;
	}
	file = &_file;			//为file申请内存
	ftemp = &_ftemp;		//为ftemp申请内存
	fatbuf = _fatbuf;		//为fatbuf申请内存
	return 0;
#endif
}

//获取文件类型
//fname:文件名
//返回值:0XFF,表示无法识别的文件类型编号.
//		 其他,高四位表示所属大类,低四位表示所属小类.
FT_Type fatfs_ftype(char *fname)
{
	u16 len;
	u8 i;
	char *p;
	len = strlen(fname);
	if(len>250)return FT_ERROR;
	p = strrchr((char*)(fname+len-5),'.');
	if(!p||fname+len-1==p)return FT_ERROR;
	for(i=0; i<FILE_MAX_TYPE_NUM; i++)
	{
		for(len=0; len<FILE_MAX_SUBT_NUM; len++)
		{
			if(!FILE_TYPE_TBL[i][len])break;
			if(0==strcasecmp((const char*)FILE_TYPE_TBL[i][len],p+1))
			{
				return (FT_Type)((i<<4)|len);
			}
		}
	}
	return FT_ERROR;
}

//df即disk free，用于获取磁盘的空闲容量信息
FRESULT fatfs_df(u8 *drv,u32 *total,u32 *free)
{
	FATFS *fsobj;
	FRESULT res;
	u32 free_clust=0, free_sect=0, tol_sect=0;
	//获取磁盘信息及空闲簇数量
	res = f_getfree((const TCHAR*)drv, (DWORD*)&free_clust, &fsobj);
	if(res==0)						//如果成功获取
	{
		tol_sect = (fsobj->n_fatent-2) * (fsobj->csize);	//总扇区数
		free_sect = free_clust * (fsobj->csize);			//空闲扇区数
#if FF_MAX_SS != FF_MIN_SS							//如果扇区大小不确定
		*total = tol_sect * fsobj->ssize / 1024;
		*free = free_sect * fsobj->ssize /1024;
#else												//扇区大小确定，为512
		*total = tol_sect >>1;		//单位为KB
		*free = free_sect >>1;		//单位为KB
#endif		
	}
	return res;
}

//mv，即move，文件移动(包括文件夹)
FRESULT fatfs_mv()
{
	return FR_OK;
}

//cp，即copy，文件复制(包括文件夹)
FRESULT fatfs_cp()
{
	return FR_OK;
}

//ls,即list，获取该路径下的文件及文件夹
FRESULT fatfs_ls()
{
	return FR_OK;
}

//du，即disk usage，获取磁盘中文件大小(包括文件夹)
FRESULT fatfs_du()
{
	return FR_OK;
}


#if TEST_MODE
#include "lcd.h"
#include "key.h"
#include "led.h"

void FATFS_TEST(void)
{
	u32 total, free;
	u8 key = 0;
	while(1)
	{
		key = KEY_Scan(0);
		switch(key)
		{
			case KEY1_PRES:
				if(fatfs_df("0:", &total, &free))
				{
					LCD_ShowString(30,30,200,16,16,"SD Card Fatfs Error!");
					while(1)
					{
						LED_Toggle(1);
						delay_ms(200);
					}
				}
				else
				{
						LCD_ShowString(30,70,200,16,16,"FATFS OK!");	 
						LCD_ShowString(30,90,200,16,16,"SD Total Size:     MB");	 
						LCD_ShowString(30,110,200,16,16,"SD  Free Size:     MB"); 
						LCD_ShowNum(30+8*14,90,total>>10,5,16);	//显示SD卡总容量 MB
						LCD_ShowNum(30+8*14,110,free>>10,5,16);     //显示SD卡剩余容量 MB
				} break;
		}
		delay_ms(20);
	}
}

#endif
