/******************** (C) COPYRIGHT 2015 shjh********************

* File Name          : mal_sd.c
* Author             : Edwards
* Version            : V1.0.3
* Date               : 28/09/2015
* Description        : Medium Access Layer interface
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

/* Includes ------------------------------------------------------------------*/
#include "mal_sd.h"
//#include "avr_spi_sd.h"
#include "data_flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

#define SDConfig_OK   0
#define SDConfig_FALL 1


/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/*******************************************************************************

* Function Name  : MAL_InitConfig
* Description    : Media access layer initialize SD card(SPI)
* Input          : None
* Output         : None
* Return         : SDConfigStatus:SD Card initiation code
*******************************************************************************/

uint16_t MAL_InitConfig(void)
{
	uint16_t status = SDConfig_OK;
        
    SD_CardInfo cardinfo;

      if(!(SD_Init()))
      {
        
			if(SD_GetCardInfo(&cardinfo)==0) /*获取SD卡信息 */
			return status;//初始化成功
      
      }    

        return SDConfig_FALL;
}


/*****************************************************************************************

* Function Name  : MAL_WriteData
* Description    : Allows to write memory area specified for the given card.
* Input          : - blockAddr:Address from where data are to be write.
*                  - writeBuff:pointer to the buffer that will contain the 
*                    transferd data
*                  - numByteToWrite:The length of the data to be written
* Output         : 
* Return         : SD_ErrorStarus: SD Card Error code.
******************************************************************************************/



MAL_ErrorStarus MAL_WriteData(void * writeBuff,uint32_t blockAddr,  uint32_t numByteToWrite)

{
    uint32_t memoryOffset=0;
    
    uint32_t res =0;
  
  if((!numByteToWrite) || (numByteToWrite > (MAX_BLOCK_COUNT * BLOCK_BYTE_SIZE)))

    {
      return WRITE_COUNT_ERR;
    }
    
    if((blockAddr < 0) || (blockAddr> MAX_BLOCK_COUNT))
    {
      return WRITE_ADDRESS_ERR;
    }
    
    uint32_t  blocksCount = (numByteToWrite % 512) ? ( numByteToWrite / 512 + 1) : (numByteToWrite / 512);

    
    
    if(!blocksCount)
    {
      return WRITE_COUNT_ERR;
    }
    
    memoryOffset = blockAddr*BLOCK_BYTE_SIZE;//计算块地址
    
    
    if(blocksCount-1)//写多块;blocksCount>1
    {
        uint16_t res = SD_WriteMultiBlocks( (uint8_t * )writeBuff,  memoryOffset ,BLOCK_BYTE_SIZE, blocksCount);

        
        if ( res != SD_RESPONSE_NO_ERROR )
        {
          return WRITE_DATA_ERR;
        } 
        
       
    }
    else//写一块数据
    {
        res = SD_WriteBlock( (uint8_t * )writeBuff, memoryOffset , BLOCK_BYTE_SIZE);

         if ( res != SD_RESPONSE_NO_ERROR )
        {
          return WRITE_DATA_ERR;
        } 
      
    }
    
    return WRITE_DATA_SUCCESS;
    
}


/*****************************************************************************************

* Function Name  : MAL_ReadData
* Description    : Allows to read memory area specified for the given card.
* Input          : - blockAddr:Address from where data are to be write.
*                  - ReadBuff:pointer to the buffer that will contain the 
*                    read data
*                  - numByteToRead:The length of the data to be read
* Output         : 
* Return         : SD_ErrorStarus: SD Card Error code.
******************************************************************************************/        

        
MAL_ErrorStarus MAL_ReadData(void * readBuff,uint32_t blockAddr,  uint32_t numByteToRead)

{
   uint32_t memoryOffset=0;
   
   uint32_t res =0;
  
  if((!numByteToRead) || (numByteToRead > (MAX_BLOCK_COUNT * BLOCK_BYTE_SIZE)))
    {
      return READ_COUNT_ERR;
    }
    
    if((blockAddr < 0)||(blockAddr> MAX_BLOCK_COUNT))
    {
      return READ_ADDRESS_ERR;
    }
    
    uint8_t tempBuffer[8 * BLOCK_BYTE_SIZE];//开辟一块的缓冲区(4k数据空间)
    
    memset(tempBuffer, 0, 8 * BLOCK_BYTE_SIZE);//初始化赋值为0
             
    uint32_t  blocksCount = (numByteToRead % 512) ? ( numByteToRead / 512 + 1) : (numByteToRead / 512);//计算要读取的块数量

        
    if(!blocksCount)
    {
      return READ_COUNT_ERR;
    }
    
    memoryOffset = blockAddr*BLOCK_BYTE_SIZE;//计算块内存地址
    
    
    if(blocksCount-1)//读多块;blocksCount>1
    {
        uint16_t res = SD_ReadMultiBlocks( (uint8_t * )tempBuffer,  memoryOffset ,BLOCK_BYTE_SIZE, blocksCount);

        
         if ( res != SD_RESPONSE_NO_ERROR )
        {
          return READ_DATA_ERR;
        }        
  
    }   
    else//读一块数据 
    {  
        res = SD_ReadBlock((uint8_t * )tempBuffer,  memoryOffset , BLOCK_BYTE_SIZE); 

    
         if ( res != SD_RESPONSE_NO_ERROR )
        {
          return READ_DATA_ERR;
        }  
        
    }

    memcpy(readBuff, tempBuffer, numByteToRead);//提取需要读取的数据
   
    
    return READ_DATA_SUCCESS;
    
}


/*****************************************************************************************

* Function Name  : MAL_ReadDisk
* Description    : Allows to read memory area specified for the given card.
* Input          : - sector: Sector address from where data are to be read.
*                  - ReadBuff:Pointer to the buffer that will contain the
*                    read data
*                  - blockByteSize:The size of the block
*                  - count:The count of the block 
*				   
* Output         :
* Return         : SD_ErrorStarus: SD Card Error code.
******************************************************************************************/

MAL_ErrorStarus MAL_ReadDisk(void *readbuff, uint32_t sector, uint32_t blockByteSize, uint8_t count)
{
	uint8_t ret =0;
	
	if(!count)return PARERR_ERR;
	for(; count > 0 ; count--)
	{
		ret = data_flash_read_block((sector*BLOCK_BYTE_SIZE), blockByteSize, readbuff);
		if(ret != DF_OK)
		{
			return READ_DATA_ERR
		}
		sector++;
		readbuff+=BLOCK_BYTE_SIZE;
	}
	return READ_DATA_SUCCESS;
	
	
	
	//if(count == 1)
	//{
		//
		//if(SD_ReadBlock(readbuff, (sector*BLOCK_BYTE_SIZE), blockByteSize)==SD_RESPONSE_NO_ERROR)//读1块
		//
		//return READ_DATA_SUCCESS;
		//
	//}
	//else
	//{
		//
		//if(SD_ReadMultiBlocks(readbuff, (sector*BLOCK_BYTE_SIZE), blockByteSize, count)==SD_RESPONSE_NO_ERROR)
		//
		//return READ_DATA_SUCCESS;
//
	//}
	
	//return READ_DATA_ERR;
	
		
	
}


/*****************************************************************************************

* Function Name  : MAL_WriteDisk
* Description    : Allows to Write memory area specified for the given card.
* Input          : - sector: Sector address from where data are to be write.
*                  - WriteBuff:Pointer to the buffer that will contain the
*                    write data
*                  - blockByteSize:The size of the block
*                  - count:The count of the block
*
* Output         :
* Return         : SD_ErrorStarus: SD Card Error code.
******************************************************************************************/

MAL_ErrorStarus MAL_WriteDisk(void *writebuff, uint32_t sector, uint32_t blockByteSize, uint8_t count)
{
	uint8_t ret =0;
	
	if(!count)return PARERR_ERR;
	for(; count > 0 ; count--)
	{
		ret = data_flash_erase_block((sector*BLOCK_BYTE_SIZE), DF_BLOCK_4KB);
		if(ret != DF_ERASE_COMPLETED)
		{
			return WRITE_DATA_ERR
		}
		
		ret	= data_flash_write_block(writebuff, (sector*BLOCK_BYTE_SIZE), blockByteSize,);
		if(ret != DF_WRITE_COMPLETED)
		{
			return WRITE_DATA_ERR
		}			 
		sector++;
		writebuff+=BLOCK_BYTE_SIZE;
	}
	return WRITE_DATA_SUCCESS;
	
	
	//if(count == 1)
	//{
		//
		//if(SD_WriteBlock(writebuff, (sector*BLOCK_BYTE_SIZE), blockByteSize)==SD_RESPONSE_NO_ERROR)//读1块
		//
		//return WRITE_DATA_SUCCESS;
		//
	//}
	//else
	//{
		//
		//if(SD_WriteMultiBlocks(writebuff, (sector*BLOCK_BYTE_SIZE), blockByteSize, count)==SD_RESPONSE_NO_ERROR)
		//
		//return WRITE_DATA_SUCCESS;
//
	//}
	//
	//return WRITE_DATA_ERR;
	//
	
	
}






















/******************* (C) COPYRIGHT 2015 shjh *****END OF FILE****/










