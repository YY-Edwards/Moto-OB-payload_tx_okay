/******************** (C) COPYRIGHT 2015 shjh********************

* File Name          : mal_sd.h
* Author             : Edwards
* Version            : V1.0.3
* Date               : 28/09/2015
* Description        : Header for mal_sd.c file.
*******************************************************************************
*

* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE 
TIME.

* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


/* Define to prevent recursive inclusion -------------------------------------*/

#ifndef _MAL_SD_H_
#define _MAL_SD_H_

/* Includes ------------------------------------------------------------------*/
//#include <utility.h>
#include <string.h>




/* Exported types ------------------------------------------------------------*/
typedef enum
{
 
  /* Standard error defines */
  WRITE_COUNT_ERR		= (0x01),
  WRITE_ADDRESS_ERR		= (0x02),
  WRITE_DATA_ERR		= (0x03), 
  WRITE_DATA_SUCCESS	= (0x04),
  READ_COUNT_ERR		= (0x05),
  READ_ADDRESS_ERR		= (0x06),
  READ_DATA_ERR			= (0x07), 
  READ_DATA_SUCCESS		= (0x08),
  ERASE_ERR				= (0x09),
  ERASE_OK				= (0x10),
  PARERR_ERR			= (0x11),
  
 
 
} MAL_ErrorStarus;


/* Exported constants --------------------------------------------------------*/
#define MAX_BLOCK_COUNT ((unsigned long int)0x00010000)//65535
//#define BLOCK_BYTE_SIZE ((unsigned long int)0x00000200)//512
#define BLOCK_BYTE_SIZE ((unsigned long int)0x00001000)//4096


/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */
unsigned short int MAL_InitConfig(void);

MAL_ErrorStarus MAL_WriteData(void *writeBuff, unsigned long int blockAddr, unsigned long int numByteToWrite);//从指定块开始写入指定长度数据

MAL_ErrorStarus MAL_ReadData( void *readBuff, unsigned long int blockAddr, unsigned long int numByteToRead);

MAL_ErrorStarus MAL_ReadDisk(void *readBuff, unsigned long int sector, unsigned long int blockByteSize, unsigned char count);

MAL_ErrorStarus MAL_WriteDisk(void *writeBuff, unsigned long int sector, unsigned long int blockByteSize, unsigned char count);







#endif /* __MAL_SD_H */

/******************* (C) COPYRIGHT 2015 shjh *****END OF FILE****/