/**
Copyright (C), Jihua Information Tech. Co., Ltd.

File name: fs.c
Author: wxj
Version: 1.0.0.02
Date: 2015/11/16 9:52:41

Description:
History:
*/

#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "fs.h"
#include "rtc/rtc.h"

#include "../Log/log.h"

 
volatile const char DiskLabel[] = { "MOTOREC"};


char isFlOpen = 0;

//static FRESULT         res;
static FIL             fl;
static FATFS           fs, * pfs;
static DWORD           clust;

//static UINT            w;//r;
//static FILINFO 		   finfo;
static DIR 			   dirs;

//static FIL * pVoiceFile = NULL;


//DiskInfo_t disk_info;

static int VoiceInfoOffset = 0;

APP_RES initFsDir(void);

/*queue is used to send XNL message*/
static volatile xQueueHandle fl_oper_queue = NULL;

/***
初始化文件操作
功能：

1，初始化SD卡及文件系统，如果不存在文件系统则格式化
2，检查磁盘标签名为“MOTOREC”----暂不要求
3，读取SD卡和文件系统信息，（包括剩余容量等，结构体名为DiskInfo_t，内容自定）
4，创建初始文件夹，文件结构如下
- root
- ----Radio.ini
- ----REC
- --------ZONE01
---------------CH01
-------------------%04d%02d%02d.DAT(年，月，日)
-------------------%04d%02d%02d.INF
-
参数：存储SD卡信息的地址
返回值；APP_RES_OK：成功，DISK_OPERATION_ERROR：SD卡初始化失败，FILE_OPERATION_ERROR：文件操作失败
*/
//

fs_err_t disk_init(void)
{
	char str[MAX_DISK_LABEL_SIZE];
	FRESULT res = 0;
	
	/*initialize disk*/
	if(MAL_InitConfig())
	{
		return disk_err;
	}
	
	//res = f_mkfs (0, 0, 4096);	
		
	if(f_mount(&fs, "/", 1) != FR_OK)
	{
		return amount_err;
	}
	
	res = f_getfree("/", &clust, &pfs);
	if(res == FR_NO_FILESYSTEM) 
	{
		/* Create a file system on the drive */
		res = f_mkfs (0, 0, 4096);
		if( f_getfree("/", &clust, &pfs) != FR_OK)
		{
			f_mount(NULL, "/", 1);	
			return no_fs;
		}
	}
	else if(res != FR_OK)
	{
		f_mount(NULL, "/", 1);
		return fs_err;
	}
	
	
	//pdisk->freecap = ((clust*(pfs->csize)/1024/1024)*512);
	
	memset(str,0,MAX_DISK_LABEL_SIZE);
	if(f_getlabel("/", str, 0) != FR_OK)
	{
		f_mount(NULL, "/", 1);
		return fs_err;
	}
		
	if(memcmp(DiskLabel, str, sizeof(DiskLabel) - 1) != 0)
	{
		if(f_setlabel("MOTOREC") != FR_OK)
		{
			f_mount(NULL, "/", 1);
			return fs_err;
		}
	}
			
	if ( f_opendir(&dirs, "/") == FR_OK)
	{
		if(f_open(&fl, "sys.ini", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK )
		{
			f_close(&fl);
		}
		else 
		{
			f_mount(NULL, "/", 1);
			return fs_err;
		}
				
		f_mkdir("/REC");			
		f_mkdir("/REC/ZONE01");			
		f_mkdir("/REC/ZONE01/CH01");		
	}
	else 
	{
		f_mount(NULL, "/", 1);
		return fs_err;
	}
	
	f_mount(NULL, "/", 1);		
	return fs_ok;
}

fs_err_t fl_write_func(void * path, S32 offset, void * buffer, U32 length)
{	
	if(f_mount(&fs, "/", 1) != FR_OK)
	{
		return amount_err;
	}
	
	FRESULT res	= f_open (&fl, path, FA_WRITE | FA_OPEN_ALWAYS);
	if(FR_NO_PATH == res)
	{
		if ( f_opendir(&dirs, "/") != FR_OK)
		{
			f_mount(NULL, "/", 1);
			return fs_err;
		}
		
		static char filename[MAX_PATH_LENGTH], path_bk[MAX_PATH_LENGTH];		
		memcpy(path_bk, path, strlen(path));
				
		char * dir = strtok(path_bk,"/");
		sprintf(filename,"//%s", dir);		
		dir = strtok(NULL,"/");
		while(TRUE)
		{
			if(NULL != dir)
			{				
				res = f_mkdir(filename);
				if((res == FR_OK) || (res == FR_EXIST))
				{					
					sprintf(filename,"%s//%s", filename, dir);
					dir = strtok(NULL,"/");					
				}
				else
				{
					f_mount(NULL, "/", 1);	
					return new_dir_err;
				}
				
			}
			else
			{
				res	= f_open (&fl, path, FA_WRITE | FA_OPEN_ALWAYS);
				if(res != FR_OK)
				{					
					f_mount(NULL, "/", 1);	
					return open_fl_err;
				}
				break;
			}
		}
	}
	else if(res != FR_OK)
	{
		f_mount(NULL, "/", 1);
		return open_fl_err;
	}
	
	if(offset == FILE_END)
	{
		f_lseek(&fl, fl.fsize);
	}
	else
	{
		f_lseek(&fl, offset);
	}
	
	UINT w;
	f_write (&fl, buffer, length, &w );

	f_close (&fl);
		
	f_mount(NULL, "/", 1);	
}



fs_err_t fl_read_func(void * path, S32 offset, void * buffer, U32 length)
{
	if(f_mount(&fs, "/", 1) != FR_OK)
		{
			return amount_err;
		}
	
		FRESULT res	= f_open (&fl, path, FA_READ | FA_OPEN_EXISTING);
		if(FR_NO_PATH == res)
		{
			if ( f_opendir(&dirs, "/") != FR_OK)
			{
				f_mount(NULL, "/", 1);
				return fs_err;
			}
		
			static char filename[MAX_PATH_LENGTH], path_bk[MAX_PATH_LENGTH];		
			memcpy(path_bk, path, strlen(path));
				
			char * dir = strtok(path_bk,"/");
			sprintf(filename,"//%s", dir);		
			dir = strtok(NULL,"/");
			while(TRUE)
			{
				if(NULL != dir)
				{				
					res = f_mkdir(filename);
					if((res == FR_OK) || (res == FR_EXIST))
					{					
						sprintf(filename,"%s//%s", filename, dir);
						dir = strtok(NULL,"/");					
					}
					else
					{
						f_mount(NULL, "/", 1);	
						return new_dir_err;
					}
				
				}
				else
				{
					res	= f_open (&fl, path, FA_READ | FA_OPEN_EXISTING);
					if(res != FR_OK)
					{					
						f_mount(NULL, "/", 1);	
						return open_fl_err;
					}
					break;
				}
			}
		}
		else if(res != FR_OK)
		{
			f_mount(NULL, "/", 1);
			return open_fl_err;
		}
	
		if(offset == FILE_END)
		{
			f_lseek(&fl, fl.fsize);
		}
		else
		{
			f_lseek(&fl, offset);
		}
	
		UINT w;
		f_read (&fl, buffer, length, &w );

		f_close (&fl);
		
		f_mount(NULL, "/", 1);		
	
	
	
	
	
}



static void fl_oper_process(void * pvParameters)
{
	fl_oper_t * fl_oper = pvPortMalloc(sizeof(fl_oper_t));
	for(;;)
	{
		if(pdTRUE ==xQueueReceive( fl_oper_queue, fl_oper, portMAX_DELAY ))
		{  
			switch(fl_oper->opcode)
			{
			case FL_WRITE:
			
				if(NULL != fl_oper->payload)
				{
					if(NULL != ((fl_write_t *)(fl_oper->payload))->buffer)
					{
						fl_write_func(((fl_write_t *)(fl_oper->payload))->path
							, ((fl_write_t *)(fl_oper->payload))->offset
							, ((fl_write_t *)(fl_oper->payload))->buffer
							, ((fl_write_t *)(fl_oper->payload))->length);
						
						vPortFree(((fl_write_t *)(fl_oper->payload))->buffer);
					}
					vPortFree(fl_oper->payload);					
				}
				break;
				
			case FL_READ:
			
				if(NULL != fl_oper->payload)
				{
					if(NULL != ((fl_read_t *)(fl_oper->payload))->buffer)
					{
						fl_read_func(((fl_read_t *)(fl_oper->payload))->path
						, ((fl_read_t *)(fl_oper->payload))->offset
						, ((fl_read_t *)(fl_oper->payload))->buffer
						, ((fl_read_t *)(fl_oper->payload))->length);
						
						vPortFree(((fl_read_t *)(fl_oper->payload))->buffer);
					}
					vPortFree(fl_oper->payload);
				}
				break;
			
			   
								
			default:
				if(NULL != fl_oper->payload)
				{
					vPortFree(fl_oper->payload);
				}
				break;
			}			
		}
		
	}
}


void fl_write(void * path, S32 offset, void * buffer, U32 length)
{
	
	fl_write_t * fl_write_ptr = pvPortMalloc(sizeof(fl_write_t));
	
	sprintf(fl_write_ptr->path, "%s", path);
	fl_write_ptr->offset = offset;
	fl_write_ptr->length = length;	
	
	fl_write_ptr->buffer = pvPortMalloc(length);
	memcpy(fl_write_ptr->buffer, buffer, length);
	
	fl_oper_t fl_oper;
	
	fl_oper.opcode = FL_WRITE;
	fl_oper.payload = fl_write_ptr;
	
	if(pdTRUE != xQueueSend( fl_oper_queue, &fl_oper, 0 ))
	{
		vPortFree(fl_write_ptr->buffer);
		vPortFree(fl_write_ptr);
		log("\n\r Wmm \n\r");//man...提升SPI_PBA时钟的频率可以有效的提升写文件的速度。
	}
	
}

void fl_read(void * path, S32 offset, void * buffer, U32 length)
{
	
	fl_read_t * fl_read_ptr = pvPortMalloc(sizeof(fl_read_t));
	
	sprintf(fl_read_ptr->path, "%s", path);
	fl_read_ptr->offset = offset;
	fl_read_ptr->length = length;
	
	fl_read_ptr->buffer = pvPortMalloc(length);
	memcpy(fl_read_ptr->buffer, buffer, length);
	
	fl_oper_t fl_oper;
	
	fl_oper.opcode = FL_READ;
	fl_oper.payload = fl_read_ptr;
	
	if(pdTRUE != xQueueSend( fl_oper_queue, &fl_oper, 0 ))
	{
		vPortFree(fl_read_ptr->buffer);
		vPortFree(fl_read_ptr);
		log("\n\r Rmm \n\r");//man...提升SPI_PBA时钟的频率可以有效的提升写文件的速度。
	}
	
}



fs_err_t fs_init(void)
{
	fs_err_t res = disk_init();
	if( fs_ok ==  res)
	{
		fl_oper_queue = xQueueCreate(20, sizeof(fl_oper_t)); //20*512bytes = 10k
		
		/*create a task for files operation*/
		xTaskCreate(
		fl_oper_process
		,  (const signed portCHAR *)"File Operations"
		,  512
		,  NULL
		,  2//1
		,  NULL
		);		
		
		return fs_ok;		
	}
	else
	{
		return res;
	}
}

APP_RES delDir(const char * dir)
{

	//char Path[MAX_FILE_NAME_SIZE];
	//memset(Path, 0, MAX_FILE_NAME_SIZE);
	//sprintf(Path, "%s//*", dir);
	
#ifdef VS2013_TEST
	
	WIN32_FIND_DATAA p;

	//myPrintf("FileList:\r\n---------\r\n");
	HANDLE h = FindFirstFileA((LPCSTR)Path, &p);

	if (p.cFileName[0] != '.')
	{
		sprintf(Path, "%s//%s", dir, p.cFileName);
		if (p.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			delDir(Path);
		else
			remove(Path);
		//myPrintf("%s\r\n", p.cFileName);
	}	
	//puts(p.cFileName);
	while (FindNextFileA(h, &p))
	{
		
		if (p.cFileName[0] != '.')
		{
			sprintf(Path, "%s//%s", dir, p.cFileName);
			if (p.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
				delDir(Path);
			else
			{
				remove(Path);
				myPrintf("delete data file,  DIR:%s \r\n", Path);
			}

			//myPrintf("%s\r\n", p.cFileName);
		}
	}
	//sprintf((char *)dir, "%s//", dir, p.cFileName);
	//int res = _unlink(dir);
	myPrintf("delete data dir,  DIR:%s:%d \r\n",  dir);

	//if ((filename[0] == 0) || (filename[0] == (char)0xFF))return END_VOICE;

	//sprintf((char *)pRec->voice.file, REC_REPLY_FILE, pRec->radio.channel.zone, pRec->radio.channel.channel, filename, "INF");

	#else
	
	#endif
	return APP_RES_OK;
}


/***
格式化磁盘
功能：	1，清除磁盘上的数据
参数：无
返回值；APP_RES_OK，成功，DISK_OPERATION_ERROR，SD卡初始化失败
*/

APP_RES fs_format(void)
{	
#ifdef VS2013_TEST
	APP_RES res = delDir("d://motorec");
#else
	
#endif	
	//system("del   /s/q  d://motorec");
	return APP_RES_OK;
}
/***
打开或创建语音文档
功能：	1，检查剩余空间，如大于设定值则创建或打开相应的文件
参数：文件名
返回值；APP_RES_OK，成功，FREE_CAP_LIMIT：剩余空间过小，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_OPEN：有打开文件
*/
//
//APP_RES fs_openOrCreateWriteVoiceFile(const char * path)
//{	
	////log(path);
	//if(f_mount(&fs, "/", 1) != FR_OK)return FILE_OPERATION_ERROR;
	//
	//res = f_open(&fl, path, FA_OPEN_ALWAYS | FA_WRITE);
	//
	//if(res == FR_OK)
	//{
		//isFlOpen = 1;
		////log("openfile ok lenght:%d",fl.fsize);
		//return APP_RES_OK;
	//}	
	//else if(res == FR_NO_PATH)
	//{
		//if ( f_opendir(&dirs, "/") != FR_OK)
		//{
			//f_mount(NULL, "/", 1);
			//return FILE_OPERATION_ERROR;
		//}		
		//
		//unsigned int zone, channel, index;
		//sscanf(path, REC_DATA_PATH, &zone, &channel, &index);
			//
		//char pathname[MAX_FILE_NAME_SIZE];
		//memset(pathname, 0, MAX_FILE_NAME_SIZE);
		//sprintf(pathname, REC_ZONE_DIR, zone);				
		//res = f_mkdir(pathname);	
		//if((res == FR_OK) || (res == FR_EXIST))
		//{
			//char chnpathname[MAX_FILE_NAME_SIZE];
			//memset(chnpathname, 0, MAX_FILE_NAME_SIZE);
			//sprintf(chnpathname, REC_CHANNEL_DIR, zone, channel);
			//res = f_mkdir(chnpathname);
			//if((res == FR_OK) || (res == FR_EXIST))
			//{				
				//res = f_open(&fl, path, FA_OPEN_ALWAYS | FA_WRITE);
				//if(res = FR_OK)
				//{
					//isFlOpen = 1;
					////log("openfile ok lenght:%d",fl.fsize);
					//return APP_RES_OK;					
				//}
			//}
			//
		//}
	//}	
	//
	//f_mount(NULL, "/", 1);
	//return FILE_OPERATION_ERROR;
//}
//

int fs_getVoiceFileLength(void)
{
#ifdef VS2013_TEST 
	fseek(pVoiceFile, 0, SEEK_END);
	return ftell(pVoiceFile);
#else
	if(isFlOpen = 1)
	return fl.fsize;
	else return 0;
#endif
}

/***
写入语音数据
功能：	1，向已打开语音文件追加写入数据
参数：1，数据，2，长度
返回值；APP_RES_OK，成功，FILE_OPERATION_ERROR：文件系统操作失败，FILE_ALREADY_CLOSE：文件已经关闭
*/

APP_RES fs_WriteVoiceFile(void * dat, int len)
{
#ifdef VS2013_TEST
	fwrite(dat, len, 1, pVoiceFile);
#else


//log("file size: %d\r\n len :%d",fl.fsize, len);
	//f_lseek(&fl,fl.fsize);
	//f_write(&fl,dat,len,&w);
	//log("w:%d",w);
	//log("file size: %d\r\n",fl.fsize);
#endif	
	return APP_RES_OK;
}
/***
存储语音信息
功能：	1，存储语音信息
参数：文件名，数据，长度
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_OPEN：有打开文件
*/

APP_RES fs_saveVoiceInfo(const char * path, void * dat, int len)
{
	
	if(f_mount(&fs, "/", 1) != FR_OK)return FILE_OPERATION_ERROR;
	
#ifdef VS2013_TEST
	pVoiceFile = fopen(path, "ab");
#else
UINT w;
	
	static APP_RES res  = APP_RES_OK;
	res = f_open ( 
	&fl ,           /* [OUT] Pointer to the file object structure */
	path, /* [IN] File name */
	FA_WRITE | FA_OPEN_ALWAYS          /* [IN] Mode flags */
	);
#endif
	if (res ==APP_RES_OK)
	{
#ifdef VS2013_TEST
		fwrite(dat, len, 1, pVoiceFile);
		fclose(pVoiceFile);
#else

f_lseek(&fl, fl.fsize);
f_write (
&fl,          /* [IN] Pointer to the file object structure */
dat, /* [IN] Pointer to the data to be written */
len,         /* [IN] Number of bytes to write */
&w          /* [OUT] Pointer to the variable to return number of bytes written */
);	
#endif	

 f_close (
&fl    /* [IN] Pointer to the file object */
);	
		//pVoiceFile = NULL;
		return APP_RES_OK;
	}
	else
f_mount(NULL, "/", 1);	
		return FILE_OPERATION_ERROR;
}
/***
查询即将播放的语音
功能：	1，按查找方式在文件中查找即将播放的语音
参数：当前语音信息（也用于存储查找结果），查找方式
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件夹操作错误，	END_VOICE：已经是最后/最前一条；NO_VOICE：没有语音
*/


//APP_RES fs_findVoiceInfo(Recording_t * pRec, ReplyType_t type)
//{
	
	//if (type == CURR)return APP_RES_OK;
//
	//char currentFileFlag = 0;
	//char dirpath[MAX_FILE_NAME_SIZE];
	//char filename[9];
	//memset(dirpath, 0, MAX_FILE_NAME_SIZE);
	//sprintf(dirpath, REC_CHANNEL_DIR, pRec->radio.channel.zone, pRec->radio.channel.channel);
	//
	//#ifdef VS2013_TEST
	//if (_access(dirpath, 0) == -1) //is Exist dir
	//#else
	//#endif
	//{
		//return NO_VOICE;
	//}
//
	//VoiceInfoOffset = VoiceInfoOffset % sizeof(Recording_t) ? (VoiceInfoOffset / sizeof(Recording_t))* sizeof(Recording_t) : VoiceInfoOffset;
//
	//char currFile[9];
	//sprintf(currFile, REC_FILE_NAME, pRec->voice.time.year, pRec->voice.time.month, pRec->voice.time.day);
//
	//if (type == PREV)
	//{
		//if (VoiceInfoOffset / sizeof(Recording_t))
		//{			
			//sprintf((char *)pRec->voice.file, REC_REPLY_FILE, pRec->radio.channel.zone, pRec->radio.channel.channel, currFile, "INF");
			//
			//VoiceInfoOffset -= sizeof(Recording_t);
			//currentFileFlag = 1;
			//goto READ_INFO;
		//}
		//
	//}
	//else if (type == NEXT)
	//{		
		////open file and get file lenght
		//sprintf((char *)pRec->voice.file, REC_REPLY_FILE, pRec->radio.channel.zone, pRec->radio.channel.channel, currFile,"INF");
		//
	//#ifdef VS2013_TEST
		//pVoiceFile = fopen((const char *)pRec->voice.file, "r"); //open file 
	//#else
	//#endif
		//
		//if (pVoiceFile == NULL) return NO_VOICE;
		//
	//#ifdef VS2013_TEST	
		//fseek(pVoiceFile, 0, SEEK_END); //f_ssek
		//int filelen = ftell(pVoiceFile);//f_ssek
		//fclose(pVoiceFile);
	//#else
		//int filelen = 0;
	//#endif
//
		//if ((VoiceInfoOffset / sizeof(Recording_t) + 1) < (filelen / sizeof(Recording_t)))
		//{
			//VoiceInfoOffset += sizeof(Recording_t);
			//currentFileFlag = 1;
			//goto READ_INFO;
		//}
	//}
	//
	//sprintf(dirpath, REC_SEARCH_INF, pRec->radio.channel.zone, pRec->radio.channel.channel);
//
	////find all files in current dir
//#ifdef VS2013_TEST
	//WIN32_FIND_DATAA p;
	//HANDLE h = FindFirstFileA((LPCSTR)dirpath, &p);
	//if (type == PREV)
	//{
		//memset(filename, 0x00, 9);
		//if (memcmp(p.cFileName, currFile, 8) < 0)
		//{
			//memcpy(filename, p.cFileName, 8);
		//}
	//}
	//else if (type == NEXT)
	//{
		//memset(filename, 0xFF, 9);
		//if (memcmp(p.cFileName, currFile, 8) > 0)
		//{
			//memcpy(filename, p.cFileName, 8);
		//}
	//}
	//else
	//{
		//memcpy(filename, p.cFileName, 8);
	//}
	//
	//filename[8] = '\0';
	//myPrintf("FileList:\r\n---------\r\n%s\r\n", p.cFileName);
	////puts(p.cFileName);
	//while (FindNextFileA(h, &p))
	//{
		//switch (type)
		//{
		//case EARLIEST: 
			//if (memcmp(p.cFileName, filename, 8) < 0)
			//{
				//memcpy(filename, p.cFileName, 8);
			//}
			//break;
		//case PREV:
			//if ((memcmp(p.cFileName, currFile, 8) < 0) && (memcmp(p.cFileName, filename, 8) > 0))
			//{
				//memcpy(filename, p.cFileName, 8);
			//}
			//break;
		//case NEXT:
			//if ((memcmp(p.cFileName, currFile, 8) > 0) && (memcmp(p.cFileName, filename, 8) < 0))
			//{
				//memcpy(filename, p.cFileName, 8);
			//}
			//break;
		//case LAST:
			//if (memcmp(p.cFileName, filename, 8) > 0)
			//{
				//memcpy(filename, p.cFileName, 8);
			//}
			//break;
		//}
		//myPrintf("%s\r\n", p.cFileName);
	//}
//#else
//#endif
//
	//if ((filename[0] == 0) || (filename[0] == (char )0xFF))return END_VOICE;
//
	//sprintf((char *)pRec->voice.file, REC_REPLY_FILE, pRec->radio.channel.zone, pRec->radio.channel.channel, filename,"INF");
//
	////read info
//READ_INFO:
//
//#ifdef VS2013_TEST
	//pVoiceFile = fopen((const char *)pRec->voice.file, "r");
//#else
//#endif	
	//if (pVoiceFile == NULL) return NO_VOICE;
//
	//if (!currentFileFlag)
	//{
		////ge toffset
		//switch (type)
		//{
		//case EARLIEST:
		//case NEXT:
			//VoiceInfoOffset = 0;
			//break;
		//case PREV:
		//case LAST:
		//#ifdef VS2013_TEST	
			//fseek(pVoiceFile, 0, SEEK_END);
			//if (ftell(pVoiceFile) >= sizeof(Recording_t))
				//VoiceInfoOffset = (ftell(pVoiceFile) / sizeof(Recording_t)-1) * sizeof(Recording_t);
		//#else
		//#endif
			//break;
		//}
	//}
	//
//#ifdef VS2013_TEST	
	//fseek(pVoiceFile, VoiceInfoOffset, SEEK_SET);
	//fread(pRec, sizeof(Recording_t), 1, pVoiceFile);
	//fclose(pVoiceFile);
//#else
//#endif
	//
	//pVoiceFile = NULL;

	//return APP_RES_OK;
//}
/***
功能：	1，打开语音文档用于读取
参数：文件名
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_OPEN：有打开文件
*/

APP_RES fs_openReadVoiceFile(const char * file)
{
	return APP_RES_OK;
}
/***
打开语音文档用于读取
功能：	1，读取一打开文件中语音数据
参数：buffer, offset, length
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_CLOSE：文件已经关闭
*/

APP_RES fs_readVoiceDat(void * buff, int offset, int len)
{
	return APP_RES_OK;
}
/***
关闭语音文档
功能：	1，关闭语音文件
参数：文件名
返回值；APP_RES_OK，成功，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_CLOSE：文件已经关闭
*/

APP_RES fs_closeVoiceFile(void)
{
#ifdef VS2013_TEST	
	fclose(pVoiceFile);
#else
	isFlOpen = 0;
	f_close(&fl);
	f_mount(NULL, "/", 1);
#endif	
	//pVoiceFile = NULL;
	return APP_RES_OK;
}
/***
删除语音文件
功能：	1，查找到磁盘上最早的语音（包括信息文件）文件，并删除
参数：DeleteType_t type:删除方式，DateTime_t * date:仅方式为DATE时有效，指删除该日期之前所有文件
返回值；APP_RES_OK，成功，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_CLOSE：文件已经关闭
*/


//APP_RES fs_DeleteVoiceFile(DeleteType_t type, DateTime_t * time)
//{
	//return APP_RES_OK;
//}

