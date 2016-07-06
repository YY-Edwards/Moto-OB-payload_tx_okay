/*
 * myfile.h
 *
 * Created: 2015/11/16 9:50:28
 *  Author: JHDEV2
 */ 


#ifndef MYFILE_H_
#define MYFILE_H_

//#include "motorec.h"
#include "mal_sd.h"
//#include "rtc/rtc.h"
#include "ff.h"


#define REC_DATA_PATH	"/REC/ZONE%02d/CH%02d/%08d.DAT" //"D://motorec//REC//ZONE%02d//CH%02d//%04d%02d%02d.DAT"
#define REC_INFO_PATH	"/REC/ZONE%02d/CH%02d/%08d.INF" //"D://motorec//REC//ZONE%02d//CH%02d//%04d%02d%02d.INF"

#define REC_ZONE_DIR	"/REC/ZONE%02d"
#define REC_CHANNEL_DIR	"/REC/ZONE%02d/CH%02d"

#define REC_SEARCH_INF  "D://motorec//REC//ZONE%02d//CH%02d//*.INF"

#define REC_REPLY_FILE	"D://motorec//REC//ZONE%02d//CH%02d//%s.%s"

#define REC_FILE_NAME	"%04d%02d%02d"

#define FL_WRITE 0x4000
#define FL_READ  0x4001
#define FL_REMOVE 0x4002 

#define FILE_END	(-1)
#define FILE_BEGIN  (0)

typedef enum {
	APP_RES_OK = 0,		/* 0: Successful */
	XNL_ERROR,
	DISK_OPERATION_ERROR,
	REC_BUFFER_OVER,
	FILE_OPERATION_ERROR,
	ADDRESS_ERROR,
	END_VOICE,
	NO_VOICE,
	//RES_ERROR,		/* 1: R/W Error */
	//RES_WRPRT,		/* 2: Write Protected */
	//RES_NOTRDY,		/* 3: Not Ready */
	//RES_PARERR	,	/* 4: Invalid Parameter */
	UNDEFINED_DS_COMMAND,
	UNKOWN,
	RTC_ERROR,
} APP_RES;

typedef enum
{
	fs_ok = 0,
	disk_err,
	amount_err,
	open_fl_err,
	new_dir_err,
	no_fs,
	fs_err,
}fs_err_t;

#define MAX_PATH_LENGTH 64

typedef struct 
{
	char path[MAX_PATH_LENGTH];
	U32 offset;
	U32 length;
	U8 * buffer;
}fl_write_t;

typedef struct  
{
	U16 opcode;
	U8 * payload;
}fl_oper_t;

#define MAX_DISK_LABEL_SIZE 12
typedef struct
{
	char label[MAX_DISK_LABEL_SIZE];
	int freecap;
	
}DiskInfo_t;


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

fs_err_t fs_init( void );
/***
格式化磁盘
功能：	1，清除磁盘上的数据
参数：无
返回值；APP_RES_OK，成功，DISK_OPERATION_ERROR，SD卡初始化失败
*/

APP_RES fs_format(void);
/***
打开或创建语音文档
功能：	1，检查剩余空间，如大于设定值则创建或打开相应的文件
参数：文件名
返回值；APP_RES_OK，成功，FREE_CAP_LIMIT：剩余空间过小，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_OPEN：有打开文件
*/

APP_RES fs_openOrCreateWriteVoiceFile(const char *);

int fs_getVoiceFileLength(void);

/***
写入语音数据
功能：	1，向已打开语音文件追加写入数据
参数：1，数据，2，长度
返回值；APP_RES_OK，成功，FILE_OPERATION_ERROR：文件系统操作失败，FILE_ALREADY_CLOSE：文件已经关闭
*/

APP_RES fs_WriteVoiceFile(void *, int);
/***
存储语音信息
功能：	1，存储语音信息
参数：文件名，数据，长度
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_OPEN：有打开文件
*/

APP_RES fs_saveVoiceInfo(const char *, void *, int);
/***
查询即将播放的语音
功能：	1，按查找方式在文件中查找即将播放的语音
参数：当前语音信息（也用于存储查找结果），查找方式
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件夹操作错误，	END_VOICE：已经是最后/最前一条；NO_VOICE：没有语音
*/


//APP_RES fs_findVoiceInfo(Recording_t *, ReplyType_t);
/***
功能：	1，打开语音文档用于读取
参数：文件名
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_OPEN：有打开文件
*/

APP_RES fs_openReadVoiceFile(const char *);
/***
打开语音文档用于读取
功能：	1，读取一打开文件中语音数据
参数：buffer, offset, length
返回值；APP_RES_OK，成功，，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_CLOSE：文件已经关闭
*/

APP_RES fs_readVoiceDat(void *, int, int);
/***
关闭语音文档
功能：	1，关闭语音文件
参数：文件名
返回值；APP_RES_OK，成功，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_CLOSE：文件已经关闭
*/

APP_RES fs_closeVoiceFile(void);
/***
删除语音文件
功能：	1，查找到磁盘上最早的语音（包括信息文件）文件，并删除
参数：DeleteType_t type:删除方式，DateTime_t * date:仅方式为DATE时有效，指删除该日期之前所有文件
返回值；APP_RES_OK，成功，FILE_OPERATION_ERROR：文件系统操作失败,FILE_ALREADY_CLOSE：文件已经关闭
*/


//APP_RES fs_DeleteVoiceFile(DeleteType_t, DateTime_t *);

void fl_write(void * path, S32 offset, void * buffer, U32 length);

#endif /* MYFILE_H_ */