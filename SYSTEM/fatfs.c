#include "fatfs.h"
#include <string.h>


#define FILE_MAX_TYPE_NUM		7	//���FILE_MAX_TYPE_NUM������
#define FILE_MAX_SUBT_NUM		4	//���FILE_MAX_SUBT_NUM��С��

//�ļ������б�
u8* const FILE_TYPE_TBL[FILE_MAX_TYPE_NUM][FILE_MAX_SUBT_NUM]=
{
	{"BIN"},			//BIN�ļ�
	{"LRC"},			//LRC�ļ�
	{"NES","SMS"},		//NES/SMS�ļ�
	{"TXT","C","H"},	//�ı��ļ�
	{"WAV","MP3","APE","FLAC"},//֧�ֵ������ļ�
	{"BMP","JPG","JPEG","GIF"},//ͼƬ�ļ�
	{"AVI"},			//��Ƶ�ļ�
};

/* ------------------�����ļ���------------------------*/
UINT br,bw;					//��д����
FILINFO fileinfo;			//�ļ���Ϣ
DIR dir;  					//Ŀ¼

FATFS *diskfs[FF_VOLUMES];	//�߼����̹�����.	 
FIL *file;	  				//�ļ�1
FIL *ftemp;	  				//�ļ�2.
u8 *fatbuf;					//SD�����ݻ�����

#if !SUPPORT_MALLOC
FATFS _diskfs[FF_VOLUMES];	//�߼����̹�����.	 
FIL _file;	  				//�ļ�1
FIL _ftemp;	  				//�ļ�2.
u8 _fatbuf[512];			//SD�����ݻ�����
#endif


//����fatfs�����ڴ�
u8 fatfs_init(void)
{
	u8 i;
#if defined(__MALLOC_H)
	for(i=0; i<FF_VOLUMES; i++)
	{
		diskfs[i] = (FATFS*)mymalloc(SRAMIN, sizeof(FATFS));
		if(!diskfs[i]) break;
	}
	file=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//Ϊfile�����ڴ�
	ftemp=(FIL*)mymalloc(SRAMIN,sizeof(FIL));		//Ϊftemp�����ڴ�
	fatbuf=(u8*)mymalloc(SRAMIN,512);				//Ϊfatbuf�����ڴ�
	return (i==FF_VOLUMES&&file&&ftemp&&fatbuf)?0:1;//������һ��ʧ��,��ʧ��.
#else
	for(i=0; i<FF_VOLUMES; i++)
	{
		diskfs[i] = _diskfs+i;
	}
	file = &_file;			//Ϊfile�����ڴ�
	ftemp = &_ftemp;		//Ϊftemp�����ڴ�
	fatbuf = _fatbuf;		//Ϊfatbuf�����ڴ�
	return 0;
#endif
}

//��ȡ�ļ�����
//fname:�ļ���
//����ֵ:0XFF,��ʾ�޷�ʶ����ļ����ͱ��.
//		 ����,����λ��ʾ��������,����λ��ʾ����С��.
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

//df��disk free�����ڻ�ȡ���̵Ŀ���������Ϣ
FRESULT fatfs_df(u8 *drv,u32 *total,u32 *free)
{
	FATFS *fsobj;
	FRESULT res;
	u32 free_clust=0, free_sect=0, tol_sect=0;
	//��ȡ������Ϣ�����д�����
	res = f_getfree((const TCHAR*)drv, (DWORD*)&free_clust, &fsobj);
	if(res==0)						//����ɹ���ȡ
	{
		tol_sect = (fsobj->n_fatent-2) * (fsobj->csize);	//��������
		free_sect = free_clust * (fsobj->csize);			//����������
#if FF_MAX_SS != FF_MIN_SS							//���������С��ȷ��
		*total = tol_sect * fsobj->ssize / 1024;
		*free = free_sect * fsobj->ssize /1024;
#else												//������Сȷ����Ϊ512
		*total = tol_sect >>1;		//��λΪKB
		*free = free_sect >>1;		//��λΪKB
#endif		
	}
	return res;
}

//mv����move���ļ��ƶ�(�����ļ���)
FRESULT fatfs_mv()
{
	return FR_OK;
}

//cp����copy���ļ�����(�����ļ���)
FRESULT fatfs_cp()
{
	return FR_OK;
}

//ls,��list����ȡ��·���µ��ļ����ļ���
FRESULT fatfs_ls()
{
	return FR_OK;
}

//du����disk usage����ȡ�������ļ���С(�����ļ���)
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
						LCD_ShowNum(30+8*14,90,total>>10,5,16);	//��ʾSD�������� MB
						LCD_ShowNum(30+8*14,110,free>>10,5,16);     //��ʾSD��ʣ������ MB
				} break;
		}
		delay_ms(20);
	}
}

#endif
