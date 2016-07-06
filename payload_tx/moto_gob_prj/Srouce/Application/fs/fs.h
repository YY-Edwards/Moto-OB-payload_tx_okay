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
��ʼ���ļ�����
���ܣ�

1����ʼ��SD�����ļ�ϵͳ������������ļ�ϵͳ���ʽ��
2�������̱�ǩ��Ϊ��MOTOREC��----�ݲ�Ҫ��
3����ȡSD�����ļ�ϵͳ��Ϣ��������ʣ�������ȣ��ṹ����ΪDiskInfo_t�������Զ���
4��������ʼ�ļ��У��ļ��ṹ����
- root
- ----Radio.ini
- ----REC
- --------ZONE01
---------------CH01
-------------------%04d%02d%02d.DAT(�꣬�£���)
-------------------%04d%02d%02d.INF
-
�������洢SD����Ϣ�ĵ�ַ
����ֵ��APP_RES_OK���ɹ���DISK_OPERATION_ERROR��SD����ʼ��ʧ�ܣ�FILE_OPERATION_ERROR���ļ�����ʧ��
*/

fs_err_t fs_init( void );
/***
��ʽ������
���ܣ�	1����������ϵ�����
��������
����ֵ��APP_RES_OK���ɹ���DISK_OPERATION_ERROR��SD����ʼ��ʧ��
*/

APP_RES fs_format(void);
/***
�򿪻򴴽������ĵ�
���ܣ�	1�����ʣ��ռ䣬������趨ֵ�򴴽������Ӧ���ļ�
�������ļ���
����ֵ��APP_RES_OK���ɹ���FREE_CAP_LIMIT��ʣ��ռ��С��FILE_OPERATION_ERROR���ļ�ϵͳ����ʧ��,FILE_ALREADY_OPEN���д��ļ�
*/

APP_RES fs_openOrCreateWriteVoiceFile(const char *);

int fs_getVoiceFileLength(void);

/***
д����������
���ܣ�	1�����Ѵ������ļ�׷��д������
������1�����ݣ�2������
����ֵ��APP_RES_OK���ɹ���FILE_OPERATION_ERROR���ļ�ϵͳ����ʧ�ܣ�FILE_ALREADY_CLOSE���ļ��Ѿ��ر�
*/

APP_RES fs_WriteVoiceFile(void *, int);
/***
�洢������Ϣ
���ܣ�	1���洢������Ϣ
�������ļ��������ݣ�����
����ֵ��APP_RES_OK���ɹ�����FILE_OPERATION_ERROR���ļ�ϵͳ����ʧ��,FILE_ALREADY_OPEN���д��ļ�
*/

APP_RES fs_saveVoiceInfo(const char *, void *, int);
/***
��ѯ�������ŵ�����
���ܣ�	1�������ҷ�ʽ���ļ��в��Ҽ������ŵ�����
��������ǰ������Ϣ��Ҳ���ڴ洢���ҽ���������ҷ�ʽ
����ֵ��APP_RES_OK���ɹ�����FILE_OPERATION_ERROR���ļ��в�������	END_VOICE���Ѿ������/��ǰһ����NO_VOICE��û������
*/


//APP_RES fs_findVoiceInfo(Recording_t *, ReplyType_t);
/***
���ܣ�	1���������ĵ����ڶ�ȡ
�������ļ���
����ֵ��APP_RES_OK���ɹ�����FILE_OPERATION_ERROR���ļ�ϵͳ����ʧ��,FILE_ALREADY_OPEN���д��ļ�
*/

APP_RES fs_openReadVoiceFile(const char *);
/***
�������ĵ����ڶ�ȡ
���ܣ�	1����ȡһ���ļ�����������
������buffer, offset, length
����ֵ��APP_RES_OK���ɹ�����FILE_OPERATION_ERROR���ļ�ϵͳ����ʧ��,FILE_ALREADY_CLOSE���ļ��Ѿ��ر�
*/

APP_RES fs_readVoiceDat(void *, int, int);
/***
�ر������ĵ�
���ܣ�	1���ر������ļ�
�������ļ���
����ֵ��APP_RES_OK���ɹ���FILE_OPERATION_ERROR���ļ�ϵͳ����ʧ��,FILE_ALREADY_CLOSE���ļ��Ѿ��ر�
*/

APP_RES fs_closeVoiceFile(void);
/***
ɾ�������ļ�
���ܣ�	1�����ҵ������������������������Ϣ�ļ����ļ�����ɾ��
������DeleteType_t type:ɾ����ʽ��DateTime_t * date:����ʽΪDATEʱ��Ч��ָɾ��������֮ǰ�����ļ�
����ֵ��APP_RES_OK���ɹ���FILE_OPERATION_ERROR���ļ�ϵͳ����ʧ��,FILE_ALREADY_CLOSE���ļ��Ѿ��ر�
*/


//APP_RES fs_DeleteVoiceFile(DeleteType_t, DateTime_t *);

void fl_write(void * path, S32 offset, void * buffer, U32 length);

#endif /* MYFILE_H_ */