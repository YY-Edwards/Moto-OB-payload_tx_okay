/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
//#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */
#include "fs/mal_sd.h"		/* Example: MMC/SDC contorl */
//#include "fs/avr_spi_sd.h"
#include "rtc/rtc.h"




/* Definitions of physical drive number for each drive */
#define ATA		0	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	//DSTATUS stat;
	//int result;

	//switch (pdrv) {
	//case ATA :
		//result = ATA_disk_status();

		// translate the reslut code here

		//return stat;

	//case MMC :
		//result = MMC_disk_status();

		// translate the reslut code here

		//return stat;

	//case USB :
		//result = USB_disk_status();

		// translate the reslut code here

		//return stat;
	//}
	//return STA_NOINIT;


	return RES_OK;

	
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = 0;
	//int result;

	//switch (pdrv) {
	//case ATA :
		//result = ATA_disk_initialize();

		// translate the reslut code here

		//return stat;

	//case MMC :
		//result = MMC_disk_initialize();

		// translate the reslut code here

		//return stat;

	//case USB :
		//result = USB_disk_initialize();

		// translate the reslut code here

		//return stat;
	//}
	//return STA_NOINIT;

	return stat;
	
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{

/****************************************
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		result = ATA_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
		// translate the arguments here

		result = MMC_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case USB :
		// translate the arguments here

		result = USB_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;


	***********************************/

	

	if(MAL_ReadDisk(buff, sector, BLOCK_BYTE_SIZE, count)==READ_DATA_SUCCESS)
   		return RES_OK;

	else

   		return RES_ERROR;




}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{

/******************************
	DRESULT res;
	int result;

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		result = ATA_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
		// translate the arguments here

		result = MMC_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case USB :
		// translate the arguments here

		result = USB_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;
	}

	return RES_PARERR;


	*******************************/

	
	 if(MAL_WriteDisk(buff, sector, BLOCK_BYTE_SIZE, count)==WRITE_DATA_SUCCESS)
	 return RES_OK;
	 
	 else
	 
	 return RES_ERROR;
	



}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	
	DRESULT res = RES_OK ;
	
	switch (cmd)
	{
		
		case CTRL_SYNC: 
	
			/* 拉低CS */
			//SD_CS_LOW();
	 
			//if (SD_WaitReady()== SD_RESPONSE_NO_ERROR)
			//res = RES_OK;
	 
			//else res = RES_ERROR;
	 
			/* 拉高CS */
			//SD_CS_HIGH();
			res = RES_OK;
	  
			break;
	  
		case GET_SECTOR_SIZE:
		
			*(WORD*)buff = 4096;//512;//根据AT25DF641的最小擦除单位为基准
		
			res = RES_OK;
		
			break;	
		
		case GET_BLOCK_SIZE:
		
			*(WORD*)buff = 4096;
		
			res = RES_OK;
		
			break;
		
		case GET_SECTOR_COUNT:
		
			*(DWORD*)buff = 2048;//SD_GetSectorCount();8192x1024/4096byte = 2048
			//*(DWORD*)buff = 1024;
		
			res = RES_OK;
		
			break;
		
		default:
		
			res = RES_PARERR;
		
			break;	
	  
		}
	
	return res;

	
}
#endif

DWORD get_fattime(void)
{
	DWORD current_time = 0;
	date_time_t *t = now();

	current_time = (t->year - 1980) << 25;
	current_time += t->month << 21;
	current_time += t->day << 16;
	current_time += t->hour << 11;
	current_time += t->minute << 5;
	current_time += t->second / 2;
	
	return current_time;
}