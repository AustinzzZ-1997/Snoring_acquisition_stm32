/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "sdio_sdcard.h"

/* Definitions of physical drive number for each drive */
//#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
//#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
//#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */
#define DEV_SD		0	/* SD card */	
	

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	//DSTATUS stat;

	switch (pdrv) {
	case DEV_SD :
		// translate the reslut code here
		if(SD_GetState()==SD_CARD_TRANSFER)		//传输完成
		return RES_OK;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	switch (pdrv) {
	case DEV_SD :
		// translate the reslut code here
		if(SD_Init()== 0)
			return RES_OK;
	}
	return STA_NOINIT;
}



//#if SD_DMA_MODE
//	#define SD_ReadDisk(a,b,c)	SD_ReadBlocks_DMA(a,b,512,c)
//	#define SD_WriteDisk(a,b,c)	SD_WriteBlocks_DMA(a,b,512,c)
//#endif

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	int res;
	u8 retry = 5;
	if(!count) return RES_PARERR;	//count不能等于0，否则返回参数错误
	
	switch (pdrv) {
	case DEV_SD :
		res = SD_ReadDisk(buff, sector, count);
		while(res && retry)
		{
			SD_Init();
			res = SD_ReadDisk(buff, sector, count);
			retry --;
		}
		return RES_OK;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	int res;

	if(!count) return RES_PARERR;	//count不能等于0，否则返回参数错误
	
	switch (pdrv) {
	case DEV_SD :
		// translate the arguments here
		res = SD_WriteDisk((u8*)buff, sector, count);
		while(res)
		{
			SD_Init();
			res = SD_WriteDisk((u8*)buff, sector, count);
		}
		return RES_OK;
	}
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch (pdrv) {
	case DEV_SD :
		switch(cmd)
	    {
		    case CTRL_SYNC:
				return RES_OK;	 
		    case GET_SECTOR_SIZE:
				*(DWORD*)buff = 512; 
		        return RES_OK;
		    case GET_BLOCK_SIZE:
				*(WORD*)buff = SDCardInfo.CardBlockSize;
		        return RES_OK;	 
		    case GET_SECTOR_COUNT:
				*(DWORD*)buff = SDCardInfo.CardCapacity/512;
		        return RES_OK;
		}
	}
	return RES_PARERR;
}

