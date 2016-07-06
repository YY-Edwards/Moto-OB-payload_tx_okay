/******************** (C) COPYRIGHT 2015 shjh**********************************

 * 文件名  ：.c
 * 描述    ：SD卡相关函数
 * 实验平台：AVR
 * 标准库  ：STM32F10x_StdPeriph_Driver V3.5.0
 * 作者    ：Edwards 
*******************************************************************************
*             -------------------------------------------------
*             |  AT32UC3B    	|     SD          Pin         |
*             -------------------------------------------------
*             | PA.24 / CS      |   ChipSelect      1         |
*             | PA.28 / MOSI   	|   DataIn          2         |
*             |               	|   GND             3 (0 V)   |
*             |               	|   VDD             4 (3.3 V) |
*             | PA.17 / SCLK   	|   Clock           5         |
*             |               	|   GND             6 (0 V)   |
*             | PA.29 / MISO  	|   DataOut         7         |
*             -------------------------------------------------
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


#include "avr_spi_sd.h"
#include <stdio.h>
//#include "GOB.h"
#include "spi/spi.h"

#define BLOCK_SIZE    512
volatile avr32_spi_t *spi;
static uint8_t flag_SDHC = 0;
uint8_t SD_flash_failure = 0;
 

/**
  * @brief  DeInitializes the SD/SD communication.
  * @param  None
  * @retval None
  */
void SD_DeInit(void)
{
  SD_LowLevel_DeInit();
}


/**-------------------------------------------------------
  * @函数名 SD_Init
  * @功能   SD卡初始化
  * @参数   无
  * @返回值 
***------------------------------------------------------*/
SD_Error SD_Init(void)
{
  uint32_t TimeOut, i = 0;
  SD_Error Status = SD_RESPONSE_NO_ERROR;

  /* 初始化SD_SPI,配置相关管脚和SPI */
  SD_LowLevel_Init(); 

  /*设置SD_SPI传输速度*/
  SD_SPI_SetSpeedLow();
  
  /*------------Put SD in SPI mode--------------*/
  TimeOut = 0;
  do
  {
	  
    /* 将SD卡的片选信号CS拉高*/
    SD_CS_HIGH();

    /*!< Rise CS and MOSI for 80 clocks cycles */
	/* 在CS位高的状态下，发送10次虚拟字节0xFF,目的是提供80个时钟*/
    for (i = 0; i <= 9; i++)
    {
      SD_WriteByte(SD_DUMMY_BYTE);
    }    
    
	Status = SD_GoIdleState();

    if(TimeOut > 6)
    {
      break;
    }
    TimeOut++;
  }while(Status);


   /*设置SD_SPI传输速度*/
  SD_SPI_SetSpeedHi();
  return (Status);
}


/**-------------------------------------------------------
  * @函数名 SD_GetCardInfo
  * @功能   返回特定卡的信息
  * @参数   
  * @返回值 
***------------------------------------------------------*/
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo)
{
  static uint32_t i = 0;
  SD_Error status = SD_RESPONSE_FAILURE;

  status = SD_GetCSDRegister(&(cardinfo->SD_csd));
  if(SD_RESPONSE_FAILURE == status)
  {
    status = SD_GetCSDRegister(&(cardinfo->SD_csd));
  }
  status = SD_GetCIDRegister(&(cardinfo->SD_cid));
  
  if (flag_SDHC = 1)
  {
	  
	  /* SDHC memory capacity = (C_SIZE+1) * 512K byte */
	  
	  cardinfo->CardCapacity = (cardinfo->SD_csd.DeviceSize + 1) ; 
	  cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
	  cardinfo->CardCapacity *= cardinfo->CardBlockSize; 
  }
  else
  {
	  
	cardinfo->CardCapacity *= (1 << (cardinfo->SD_csd.DeviceSizeMul + 2));
	cardinfo->CardBlockSize = 1 << (cardinfo->SD_csd.RdBlockLen);
	cardinfo->CardCapacity *= cardinfo->CardBlockSize;
  
  }

  if(i == 1)
  {
  
    //printf("\n\r SD CardBlockSize %d CardCapacity %d M", cardinfo->CardBlockSize, 
            //cardinfo->CardCapacity / 1024 / 1024);
  }
  i++; 
  
  /*!< Returns the reponse */
  return status;
}

/**
  * @brief  Reads a block of data from the SD.
  * @param  pBuffer: pointer to the buffer that receives the data read from the SD.
  * @param  ReadAddr: SD's internal address to read from.
  * @param  BlockSize: the SD card Data block size.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_ReadBlock(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  SD_CS_LOW();
  
if(flag_SDHC == 1)
{
	ReadAddr = ReadAddr/512;
}
  /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
  SD_SendCmd(SD_CMD_READ_SINGLE_BLOCK, ReadAddr, 0xFF);
  
  /*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
  {
    /*!< Now look for the data token to signify the start of the data */
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
      /*!< Read the SD block data : read NumByteToRead data */
      for (i = 0; i < BlockSize; i++)
      {
        /*!< Save the received data */
        *pBuffer = SD_ReadByte();
       
        /*!< Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /*!< Get CRC bytes (not really needed by us, but required by SD) */
      SD_ReadByte();
      SD_ReadByte();
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  
  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Reads multiple block of data from the SD.
  * @param  pBuffer: pointer to the buffer that receives the data read from the SD.
  * @param  ReadAddr: SD's internal address to read from.
  * @param  BlockSize: the SD card Data block size.
  * @param  NumberOfBlocks: number of blocks to be read.
  * @retval The SD Response:
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_ReadMultiBlocks(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t i = 0, Offset = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  
  /*!< SD chip select low */
  SD_CS_LOW();
  /*!< Data transfer */
  while (NumberOfBlocks--)
  {
    if(flag_SDHC == 1)
    {
        /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
        SD_SendCmd (SD_CMD_READ_SINGLE_BLOCK,(ReadAddr + Offset)/512, 0xFF);
    }
    else
    {
        /*!< Send CMD17 (SD_CMD_READ_SINGLE_BLOCK) to read one block */
        SD_SendCmd (SD_CMD_READ_SINGLE_BLOCK, ReadAddr + Offset, 0xFF);
    }
    /*!< Check if the SD acknowledged the read block command: R1 response (0x00: no errors) */
    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
      return  SD_RESPONSE_FAILURE;
    }
    /*!< Now look for the data token to signify the start of the data */
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
      /*!< Read the SD block data : read NumByteToRead data */
      for (i = 0; i < BlockSize; i++)
      {
        /*!< Read the pointed data */
        *pBuffer = SD_ReadByte();
        /*!< Point to the next location where the byte read will be saved */
        pBuffer++;
      }
      /*!< Set next read address*/
      Offset += 512;
      /*!< get CRC bytes (not really needed by us, but required by SD) */
      SD_ReadByte();
      SD_ReadByte();
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
    else
    {
      /*!< Set response value to failure */
      rvalue = SD_RESPONSE_FAILURE;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Writes a block on the SD
  * @param  pBuffer: pointer to the buffer containing the data to be written on the SD.
  * @param  WriteAddr: address to write on.
  * @param  BlockSize: the SD card Data block size.
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_WriteBlock(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  SD_CS_LOW();

  if(flag_SDHC == 1)
  {
  	WriteAddr = WriteAddr/512;
  }
  /*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write multiple block */
  SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr, 0xFF);
  
  /*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
  {
    /*!< Send a dummy byte */
    SD_WriteByte(SD_DUMMY_BYTE);

    /*!< Send the data token to signify the start of the data */
    SD_WriteByte(0xFE);

    /*!< Write the block data to SD : write count data by block */
    for (i = 0; i < BlockSize; i++)
    {
      /*!< Send the pointed byte */
      SD_WriteByte(*pBuffer);
      /*!< Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /*!< Put CRC bytes (not really needed by us, but required by SD) */
    SD_ReadByte();
    SD_ReadByte();

    /*!< Read data response */
    if (SD_GetDataResponse() == SD_DATA_OK)
    {
      rvalue = SD_RESPONSE_NO_ERROR;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Writes many blocks on the SD
  * @param  pBuffer: pointer to the buffer containing the data to be written on the SD.
  * @param  WriteAddr: address to write on.
  * @param  BlockSize: the SD card Data block size.
  * @param  NumberOfBlocks: number of blocks to be written.
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_WriteMultiBlocks(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks)
{
  uint32_t i = 0, Offset = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;

  /*!< SD chip select low */
  SD_CS_LOW();
  /*!< Data transfer */
  while (NumberOfBlocks--)
  {
    if(flag_SDHC == 1)
    {
        /* Send CMD24 (MSD_WRITE_BLOCK) to write blocks */
        SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, (WriteAddr + Offset)/512, 0xFF);
    }
    else
    {
        /*!< Send CMD24 (SD_CMD_WRITE_SINGLE_BLOCK) to write blocks */
        SD_SendCmd(SD_CMD_WRITE_SINGLE_BLOCK, WriteAddr + Offset, 0xFF);
    }
    /*!< Check if the SD acknowledged the write block command: R1 response (0x00: no errors) */
    if (SD_GetResponse(SD_RESPONSE_NO_ERROR))
    {
      return SD_RESPONSE_FAILURE;
    }
    /*!< Send dummy byte */
    SD_WriteByte(SD_DUMMY_BYTE);
    /*!< Send the data token to signify the start of the data */
    SD_WriteByte(SD_START_DATA_SINGLE_BLOCK_WRITE);
    /*!< Write the block data to SD : write count data by block */
    for (i = 0; i < BlockSize; i++)
    {
      /*!< Send the pointed byte */
      SD_WriteByte(*pBuffer);
      /*!< Point to the next location where the byte read will be saved */
      pBuffer++;
    }
    /*!< Set next write address */
    Offset += 512;
    /*!< Put CRC bytes (not really needed by us, but required by SD) */
    SD_ReadByte();
    SD_ReadByte();
    /*!< Read data response */
    if (SD_GetDataResponse() == SD_DATA_OK)
    {
      /*!< Set response value to success */
      rvalue = SD_RESPONSE_NO_ERROR;
    }
    else
    {
      /*!< Set response value to failure */
      rvalue = SD_RESPONSE_FAILURE;
    }
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);
  /*!< Returns the reponse */
  return rvalue;
}

/**
  * @brief  Read the CSD card register.
  *         Reading the contents of the CSD register in SPI mode is a simple 
  *         read-block transaction.
  * @param  SD_csd: pointer on an SCD register structure
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCSDRegister(SD_CSD* SD_csd)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  uint8_t CSD_Tab[16];
  uint32_t CardCapacity;
  static uint32_t times = 0;

  /*!< SD chip select low */
  SD_CS_LOW();
  /*!< Send CMD9 (CSD register) or CMD10(CSD register) */
  SD_SendCmd(SD_CMD_SEND_CSD, 0, 0xFF);
  /*!< Wait for response in the R1 format (0x00 is no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
  {
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
      for (i = 0; i < 16; i++)
      {
        /*!< Store CSD register value on CSD_Tab */
        CSD_Tab[i] = SD_ReadByte();
        if(0 == times)
        {
          //printf("\n\r CSD_Tab[%d] 0x%X", i, CSD_Tab[i]);
        }
      }
    }
    /*!< Get CRC bytes (not really needed by us, but required by SD) */
    SD_WriteByte(SD_DUMMY_BYTE);
    SD_WriteByte(SD_DUMMY_BYTE);
    /*!< Set response value to success */
    rvalue = SD_RESPONSE_NO_ERROR;
  }
  else
  {
     return rvalue;
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Byte 0 */
  SD_csd->CSDStruct = (CSD_Tab[0] & 0xC0) >> 6;
  SD_csd->SysSpecVersion = (CSD_Tab[0] & 0x3C) >> 2;
  SD_csd->Reserved1 = CSD_Tab[0] & 0x03;

  if(0 == times)
  {
    if(SD_csd->CSDStruct == 1)
    {
      //printf("\n\r SDHC CSD Version 2.0 Acess AS SD High Capacity.");
      flag_SDHC = 1;
    }
	else
	{
      //printf("\n\r SDSC CSD Version 1.0 Acess AS SD Standard Capacity.");	
      flag_SDHC = 0;
	}
  }

  /*!< Byte 1 */
  SD_csd->TAAC = CSD_Tab[1];

  /*!< Byte 2 */
  SD_csd->NSAC = CSD_Tab[2];

  /*!< Byte 3 */
  SD_csd->MaxBusClkFrec = CSD_Tab[3];

  /*!< Byte 4 */
  SD_csd->CardComdClasses = CSD_Tab[4] << 4;

  /*!< Byte 5 */
  SD_csd->CardComdClasses |= (CSD_Tab[5] & 0xF0) >> 4;
  SD_csd->RdBlockLen = CSD_Tab[5] & 0x0F;

  /*!< Byte 6 */
  SD_csd->PartBlockRead = (CSD_Tab[6] & 0x80) >> 7;
  SD_csd->WrBlockMisalign = (CSD_Tab[6] & 0x40) >> 6;
  SD_csd->RdBlockMisalign = (CSD_Tab[6] & 0x20) >> 5;
  SD_csd->DSRImpl = (CSD_Tab[6] & 0x10) >> 4;
  SD_csd->Reserved2 = 0; /*!< Reserved */

  if(flag_SDHC == 0)
  {
    SD_csd->DeviceSize = (CSD_Tab[6] & 0x03) << 10;
  
    /*!< Byte 7 */
    SD_csd->DeviceSize |= (CSD_Tab[7]) << 2;
  
    /*!< Byte 8 */
    SD_csd->DeviceSize |= (CSD_Tab[8] & 0xC0) >> 6;
  }
  else
  {
    SD_csd->DeviceSize = (CSD_Tab[7] & 0x3F) << 16;
    /* Byte 7 */
    SD_csd->DeviceSize += (CSD_Tab[8]) << 8;
    /* Byte 8 */
    SD_csd->DeviceSize += CSD_Tab[9] ;
  }    

  SD_csd->MaxRdCurrentVDDMin = (CSD_Tab[8] & 0x38) >> 3;
  SD_csd->MaxRdCurrentVDDMax = (CSD_Tab[8] & 0x07);

  /*!< Byte 9 */
  SD_csd->MaxWrCurrentVDDMin = (CSD_Tab[9] & 0xE0) >> 5;
  SD_csd->MaxWrCurrentVDDMax = (CSD_Tab[9] & 0x1C) >> 2;
  SD_csd->DeviceSizeMul = (CSD_Tab[9] & 0x03) << 1;
  /*!< Byte 10 */
  SD_csd->DeviceSizeMul |= (CSD_Tab[10] & 0x80) >> 7;
    
  SD_csd->EraseGrSize = (CSD_Tab[10] & 0x40) >> 6;
  SD_csd->EraseGrMul = (CSD_Tab[10] & 0x3F) << 1;

  /*!< Byte 11 */
  SD_csd->EraseGrMul |= (CSD_Tab[11] & 0x80) >> 7;
  SD_csd->WrProtectGrSize = (CSD_Tab[11] & 0x7F);

  /*!< Byte 12 */
  SD_csd->WrProtectGrEnable = (CSD_Tab[12] & 0x80) >> 7;
  SD_csd->ManDeflECC = (CSD_Tab[12] & 0x60) >> 5;
  SD_csd->WrSpeedFact = (CSD_Tab[12] & 0x1C) >> 2;
  SD_csd->MaxWrBlockLen = (CSD_Tab[12] & 0x03) << 2;

  /*!< Byte 13 */
  SD_csd->MaxWrBlockLen |= (CSD_Tab[13] & 0xC0) >> 6;
  SD_csd->WriteBlockPaPartial = (CSD_Tab[13] & 0x20) >> 5;
  SD_csd->Reserved3 = 0;
  SD_csd->ContentProtectAppli = (CSD_Tab[13] & 0x01);

  /*!< Byte 14 */
  SD_csd->FileFormatGrouop = (CSD_Tab[14] & 0x80) >> 7;
  SD_csd->CopyFlag = (CSD_Tab[14] & 0x40) >> 6;
  SD_csd->PermWrProtect = (CSD_Tab[14] & 0x20) >> 5;
  SD_csd->TempWrProtect = (CSD_Tab[14] & 0x10) >> 4;
  SD_csd->FileFormat = (CSD_Tab[14] & 0x0C) >> 2;
  SD_csd->ECC = (CSD_Tab[14] & 0x03);

  /*!< Byte 15 */
  SD_csd->CSD_CRC = (CSD_Tab[15] & 0xFE) >> 1;
  SD_csd->Reserved4 = 1;

  if(times < 3)
  {
    if(flag_SDHC ==1)
    {
	  /* SDHC memory capacity = (C_SIZE+1) * 512K byte */
      //SD_csd->DeviceSizeMul = 8 ;
      
      CardCapacity = (SD_csd->DeviceSize + 1)*512;
      
      
     // printf("\n\r SD CardBlockSize %d CardCapacity %d M.", (1<<SD_csd->RdBlockLen), 
           // CardCapacity / 1024 ); 
      
    }

    else
    {
      CardCapacity = (SD_csd->DeviceSize + 1);
      CardCapacity *= (1 << (SD_csd->DeviceSizeMul + 2));
      CardCapacity *= (1<<SD_csd->RdBlockLen);
      //printf("\n\r SD CardBlockSize %d CardCapacity %d M.", (1<<SD_csd->RdBlockLen), 
            //CardCapacity / 1024 / 1024);
    }
    
  if(times == 0)
    {
	    uint8_t str[20];
        sprintf((char *)&str[0], " SD Card Size %dM. ", CardCapacity / 1024 / 1024);
//	    LCD_DisplayStringLine(LCD_LINE_0, str);
    }
	times++;
  }
  /*!< Return the reponse */
  return rvalue;
}

/**
  * @brief  Read the CID card register.
  *         Reading the contents of the CID register in SPI mode is a simple 
  *         read-block transaction.
  * @param  SD_cid: pointer on an CID register structure
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetCIDRegister(SD_CID* SD_cid)
{
  uint32_t i = 0;
  SD_Error rvalue = SD_RESPONSE_FAILURE;
  uint8_t CID_Tab[16];
  
  /*!< SD chip select low */
  SD_CS_LOW();
  
  /*!< Send CMD10 (CID register) */
  SD_SendCmd(SD_CMD_SEND_CID, 0, 0xFF);
  
  /*!< Wait for response in the R1 format (0x00 is no errors) */
  if (!SD_GetResponse(SD_RESPONSE_NO_ERROR))
  {
    if (!SD_GetResponse(SD_START_DATA_SINGLE_BLOCK_READ))
    {
      /*!< Store CID register value on CID_Tab */
      for (i = 0; i < 16; i++)
      {
        CID_Tab[i] = SD_ReadByte();
      }
    }
    /*!< Get CRC bytes (not really needed by us, but required by SD) */
    SD_WriteByte(SD_DUMMY_BYTE);
    SD_WriteByte(SD_DUMMY_BYTE);
    /*!< Set response value to success */
    rvalue = SD_RESPONSE_NO_ERROR;
  }
  /*!< SD chip select high */
  SD_CS_HIGH();
  /*!< Send dummy byte: 8 Clock pulses of delay */
  SD_WriteByte(SD_DUMMY_BYTE);

  /*!< Byte 0 */
  SD_cid->ManufacturerID = CID_Tab[0];

  /*!< Byte 1 */
  SD_cid->OEM_AppliID = CID_Tab[1] << 8;

  /*!< Byte 2 */
  SD_cid->OEM_AppliID |= CID_Tab[2];

  /*!< Byte 3 */
  SD_cid->ProdName1 = CID_Tab[3] << 24;

  /*!< Byte 4 */
  SD_cid->ProdName1 |= CID_Tab[4] << 16;

  /*!< Byte 5 */
  SD_cid->ProdName1 |= CID_Tab[5] << 8;

  /*!< Byte 6 */
  SD_cid->ProdName1 |= CID_Tab[6];

  /*!< Byte 7 */
  SD_cid->ProdName2 = CID_Tab[7];

  /*!< Byte 8 */
  SD_cid->ProdRev = CID_Tab[8];

  /*!< Byte 9 */
  SD_cid->ProdSN = CID_Tab[9] << 24;

  /*!< Byte 10 */
  SD_cid->ProdSN |= CID_Tab[10] << 16;

  /*!< Byte 11 */
  SD_cid->ProdSN |= CID_Tab[11] << 8;

  /*!< Byte 12 */
  SD_cid->ProdSN |= CID_Tab[12];

  /*!< Byte 13 */
  SD_cid->Reserved1 |= (CID_Tab[13] & 0xF0) >> 4;
  SD_cid->ManufactDate = (CID_Tab[13] & 0x0F) << 8;

  /*!< Byte 14 */
  SD_cid->ManufactDate |= CID_Tab[14];

  /*!< Byte 15 */
  SD_cid->CID_CRC = (CID_Tab[15] & 0xFE) >> 1;
  SD_cid->Reserved2 = 1;

  /*!< Return the reponse */
  return rvalue;
}

/**
  * @brief  向SD卡发送一个命令.
  * @param  Cmd: 命令.
  * @param  Arg: 命令参数
  * @param  Crc: CRC校验值
  * @retval None
  */
void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc)
{
  uint32_t i = 0x00;
  
  uint8_t Frame[6];

  Frame[0] = (Cmd | 0x40); /*!< Construct byte 1 */
  
  Frame[1] = (uint8_t)(Arg >> 24); /*!< Construct byte 2 */
  
  Frame[2] = (uint8_t)(Arg >> 16); /*!< Construct byte 3 */
  
  Frame[3] = (uint8_t)(Arg >> 8); /*!< Construct byte 4 */
  
  Frame[4] = (uint8_t)(Arg); /*!< Construct byte 5 */
  
  Frame[5] = (Crc); /*!< Construct CRC: byte 6 */
  
  for (i = 0; i < 6; i++)
  {

	SD_WriteByte(Frame[i]); /*!< Send the Cmd bytes */

  }
}

/**
  * @brief  Get SD card data response.
  * @param  None
  * @retval The SD status: Read data response xxx0<status>1
  *         - status 010: Data accecpted
  *         - status 101: Data rejected due to a crc error
  *         - status 110: Data rejected due to a Write error.
  *         - status 111: Data rejected due to other error.
  */
uint8_t SD_GetDataResponse(void)
{
  uint32_t i = 0;
  uint8_t response, rvalue;

  while (i <= 64)
  {
    /*!< Read resonse */
    response = SD_ReadByte();
    /*!< Mask unused bits */
    response &= 0x1F;
    switch (response)
    {
      case SD_DATA_OK:
      {
        rvalue = SD_DATA_OK;
        break;
      }
      case SD_DATA_CRC_ERROR:
        return SD_DATA_CRC_ERROR;
      case SD_DATA_WRITE_ERROR:
        return SD_DATA_WRITE_ERROR;
      default:
      {
        rvalue = SD_DATA_OTHER_ERROR;
        break;
      }
    }
    /*!< Exit loop in case of data ok */
    if (rvalue == SD_DATA_OK)
      break;
    /*!< Increment loop counter */
    i++;
  }

  /*!< Wait null data */
  while (SD_ReadByte() == 0);

  /*!< Return response */
  return response;
}

/**
  * @brief  Returns the SD response.
  * @param  None
  * @retval The SD Response: 
  *         - SD_RESPONSE_FAILURE: Sequence failed
  *         - SD_RESPONSE_NO_ERROR: Sequence succeed
  */
SD_Error SD_GetResponse(uint8_t Response)
{
  uint32_t Count = 0x8FFF;

  /*!< Check if response is got or a timeout is happen */
  while ((SD_ReadByte() != Response) && Count)
  {
    Count--;
  }
  if (Count == 0)
  {
    /*!< After time out */
    return SD_RESPONSE_FAILURE;
  }
  else
  {
    /*!< Right response got */
    return SD_RESPONSE_NO_ERROR;
  }
}

/**
  * @brief  Returns the SD status.
  * @param  None
  * @retval The SD status.
  */
uint16_t SD_GetStatus(void)
{
  uint16_t Status = 0;

  /*!< SD chip select low */
  SD_CS_LOW();

  /*!< Send CMD13 (SD_SEND_STATUS) to get SD status */
  SD_SendCmd(SD_CMD_SEND_STATUS, 0, 0xFF);

  Status = SD_ReadByte();
  Status |= (uint16_t)(SD_ReadByte() << 8);

  /*!< SD chip select high */
  SD_CS_HIGH();

  /*!< Send dummy byte 0xFF */
  SD_WriteByte(SD_DUMMY_BYTE);

  return Status;
}

/**-------------------------------------------------------
  * @函数名 SD_GoIdleState
  * @功能   将SD卡置于闲置的状态
  * @参数    无
  * @返回值  
***------------------------------------------------------*/
SD_Error SD_GoIdleState(void)
{
  uint16_t TimeOut;
  uint8_t r1;
  SD_Error Status = SD_RESPONSE_NO_ERROR;
  uint16_t n2,n;

  /* 将CS拉低 */
  SD_CS_LOW();
  
  /* 发送CM0使SD卡进入SPI模式 */
  SD_SendCmd(SD_CMD_GO_IDLE_STATE, 0, 0x95);
  
  /* 等到空闲状态下的回应等于0x01 */
  if (SD_GetResponse  (SD_IN_IDLE_STATE))
  {
	//printf("\n\r 没有检测到SD卡");
    return SD_RESPONSE_FAILURE;
  }

  
  /*发送CMD8*/
  SD_SendCmd(8, 0x1AA, 0x87);

  /* 检查是否得到回应还是检测已经超时 */
  TimeOut = 200;
  while (((r1 = SD_ReadByte()) == 0xFF) && TimeOut)
  {
    TimeOut--;
  }

  if(r1 == 0x05) 
  {
    //printf("\r\n  this is STD_CAPACITY_SD_CARD!\n");
    
    TimeOut = 0;
    /*----------激活SD卡初始化的过程-----------*/
    do
    {
      /* 拉高CS*/
      SD_CS_HIGH();
      
      /* 发送虚拟字节0xFF */
      SD_WriteByte(SD_DUMMY_BYTE);
      
      /* 拉低CS */
      SD_CS_LOW();
      
      /* 发送CMD1（直到得到的回应是0x00）*/
      SD_SendCmd(SD_CMD_SEND_OP_COND, 0, 0xFF);
      /* 等待error Response等于0x00 */
      TimeOut++;
    
      if(TimeOut == 0x00F0)
      {
        break;
      }
  	
  	Status = SD_GetResponse(SD_RESPONSE_NO_ERROR);	
    }
    while (Status);
    
    /* 拉高CS */
    SD_CS_HIGH();
    
    /*发送dummy byte 0xFF */
    SD_WriteByte(SD_DUMMY_BYTE);    
  }
  else
  {
    if(r1 != 1)
    {
        //printf("\n\r SD卡r1等于%X.\n\r", r1);
    }
    //printf("\r\n 这是SDHC: HIGH_CAPACITY_SD_CARD!\n");
    r1 = 1;

    /* 从SD中读取5个字节 */
    for(n=0; n<5; n++)
    {
      SD_ReadByte();
    }
  
    /* 将CS拉高 */
    SD_CS_HIGH();
    SD_WriteByte(SD_DUMMY_BYTE);
    /* 将CS拉低 */
    SD_CS_LOW();
    SD_WriteByte(SD_DUMMY_BYTE);
    SD_WriteByte(SD_DUMMY_BYTE);
    n=0xff;
  
    do
    {
	  /* 发送CMD55 */
      SD_SendCmd(55, 0, 0xFF);
      for(n2=0; n2<0x08;n2++)
      {
       r1= SD_ReadByte();
       if(r1 !=1)
	   {
		   n=0;
	   }

      }
	   /* 发送CMD41 */
      SD_SendCmd(41, 0x40000000, 0);
  
      for(n2=0; n2<0xff;n2++)
      {
       r1= SD_ReadByte();
       if(r1 ==0)break;
      }
      n--;
    }while((r1!=0)&&(n>0));

    if(n==0)
    {
      Status =  SD_RESPONSE_FAILURE;
      //printf("\r\n 激活SDHC卡失败！\n");
    }
    else
    {
	  /* 发送CMD58 */
      SD_SendCmd(58, 0, 0);
      for(n=0;n<5;n++)
      {
      r1 = SD_ReadByte();
      }
	  flag_SDHC = 1;
      //printf("\r\n 成功激活SDHC卡！\n");
      Status =  SD_RESPONSE_NO_ERROR;
     }
  }
  
  
  /*
  if((Status != SD_RESPONSE_NO_ERROR))
  {
    //flag_SDHC = 1;
    printf("\n\r Maybe this is a SDHC(SD High Capacity cards).");
    //break;
  }
  else
  {
    //printf("\n\r SD cards inint Done.");
  }  
  */
    
  return Status;
}

/**
  * @brief  Write a byte on the SD.
  * @param  Data: byte to send.
  * @retval None
  */
uint8_t SD_WriteByte(uint8_t Data)
{
  uint8_t temp;
  
  //必须这样才能正常读写？
  spi_selectChip(spi, SD_SPI_PCS_0);
  
  /*!< Send the byte */
  spi_write(spi,  (U16)Data);
  

  /*!< Wait to receive a byte*/

  temp = spi_read(spi, (U16*)&Data);
  
  spi_unselectChip(spi, SD_SPI_PCS_0);
  
  /*!< Return the byte read from the SPI bus */ 
  return  temp;
    
}

/**
  * @brief  Read a byte from the SD.
  * @param  None
  * @retval The received byte.
  */
 
/**-------------------------------------------------------
  * @函数名 SD_ReadByte
  * @功能   从SD卡中读取一个字节
  * @参数    无
  * @返回值  The received byte
***------------------------------------------------------*/  
uint8_t SD_ReadByte(void)
{
  unsigned short *Data ;

  //必须这样才能正常读写
  spi_selectChip(spi, SD_SPI_PCS_0);
  
  /*!< Send the byte */

  spi_write(spi,  SD_DUMMY_BYTE);
  
  /*!< Return the byte read from the SPI bus */ 

   spi_read(spi, Data);

  /*!< Return the shifted data */
  
  spi_unselectChip(spi, SD_SPI_PCS_0);
  
  return (uint8_t)(*Data);//需要测试返回的的数值
}


/**-------------------------------------------------------
  * @函数名 SD_LowLevel_DeInit
  * @功能   重新初始化SD卡的相关引脚
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
void SD_LowLevel_DeInit(void)
{
 
  /*SD卡SPI失能 */
	spi_disable(spi);
  
  U16 status = 0xff;

	static const gpio_map_t SD_SPI_GPIO_MAP =
	{
			{AVR32_SPI_SCK_0_1_PIN,        AVR32_SPI_SCK_0_1_FUNCTION   },  // SPI Clock. PA17 as SCK (func C)
			{AVR32_SPI_MISO_0_2_PIN,       AVR32_SPI_MISO_0_2_FUNCTION  },  // MISO. PA28 as MISO (func C)
			{AVR32_SPI_MOSI_0_2_PIN,       AVR32_SPI_MOSI_0_2_FUNCTION  },  // MOSI. PA29 as MOSI (func C)
			{AVR32_SPI_NPCS_0_1_PIN,       AVR32_SPI_NPCS_0_1_FUNCTION  },  // Chip Select NPCS. PA24 as NPCS[0] (func B)
	};

	// SPI options.
	spi_options_t spiOptions =
	{
		.reg          = SD_SPI_FIRST_NPCS,   // PCS0
		.baudrate     = SD_SPI_MASTER_SPEED, // 12MHz
		.bits         = SD_SPI_BITS,         // 8 bit per transfer
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI.
	gpio_enable_module(SD_SPI_GPIO_MAP, sizeof(SD_SPI_GPIO_MAP) / sizeof(SD_SPI_GPIO_MAP[0]));

	spi = &AVR32_SPI;

	// Initialize as master.
	spi_initMaster(spi, &spiOptions);

	// Set selection mode: variable_ps, pcs_decode, delay.
	spi_selectionMode(spi, 0, 0, 0);

	// Enable SPI.
	spi_enable(spi);

	// Initialize data flash with SPI clock Osc0.
	if (spi_setupChipReg(spi, &spiOptions, FOSC0) != SPI_OK)
	{
		//fatal_error(FATAL_ERROR_DATA_FLASH_SPI_INIT);
		//SD_flash_failure = FATAL_ERROR_DATA_FLASH_SPI_INIT;
		return;
	}
}

/**-------------------------------------------------------
  * @函数名 SD_LowLevel_Init
  * @功能   初始化SD卡的相关引脚
  * @参数   无
  * @返回值 无
***------------------------------------------------------*/
void SD_LowLevel_Init(void)
{

 U16 status = 0xff;

	static const gpio_map_t SD_SPI_GPIO_MAP =
	{
			{AVR32_SPI_SCK_0_1_PIN,        AVR32_SPI_SCK_0_1_FUNCTION   },  // SPI Clock. PA17 as SCK (func C)
			{AVR32_SPI_MISO_0_2_PIN,       AVR32_SPI_MISO_0_2_FUNCTION  },  // MISO. PA28 as MISO (func C)
			{AVR32_SPI_MOSI_0_2_PIN,       AVR32_SPI_MOSI_0_2_FUNCTION  },  // MOSI. PA29 as MOSI (func C)
			{AVR32_SPI_NPCS_0_1_PIN,       AVR32_SPI_NPCS_0_1_FUNCTION  },  // Chip Select NPCS. PA24 as NPCS[0] (func B)
	};

	// SPI options.
	spi_options_t spiOptions =
	{
		.reg          = SD_SPI_FIRST_NPCS,   // PCS0
		.baudrate     = SD_SPI_MASTER_SPEED, // 24MHz
		.bits         = SD_SPI_BITS,         // 8 bit per transfer
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI.
	gpio_enable_module(SD_SPI_GPIO_MAP, sizeof(SD_SPI_GPIO_MAP) / sizeof(SD_SPI_GPIO_MAP[0]));

	spi = &AVR32_SPI;

	// Initialize as master.
	spi_initMaster(spi, &spiOptions);

	// Set selection mode: variable_ps, pcs_decode, delay.
	spi_selectionMode(spi, 0, 0, 0);

	// Enable SPI.
	spi_enable(spi);

	// Initialize data flash with SPI clock PBA.
	if (spi_setupChipReg(spi, &spiOptions, SPI_Clock_PBA) != SPI_OK)
	{
		return ;
	}
	else
	{
	return;
	}


	
}


void SD_SPI_SetSpeed(uint16_t SPI_BaudRatePrescaler)
{
 

	spi->csr0 = (spi->csr0 & (uint16_t)0x00FF) |SPI_BaudRatePrescaler;


 
    spi_enable(spi); /*!< SD_SPI enable */
	
 
}

void SD_SPI_SetSpeedLow(void)
{


	SD_SPI_SetSpeed(0xFF00);//baudDiv=255


	
}

void SD_SPI_SetSpeedHi(void)
{

	SD_SPI_SetSpeed(0x0100);//baudDiv=1

}


/*等待卡准备好*/
//返回值：0，准备好了；其他，错误代码
SD_Error SD_WaitReady(void)
{
	U32 t=0;
	do
	{
		if(SD_WriteByte(0XFF)==0XFF)return SD_RESPONSE_NO_ERROR;//OK
		t++;
	}while(t<0XFFFFFF);//等待
	return SD_RESPONSE_FAILURE ;
}

uint32_t SD_GetSectorCount(void)
{
	SD_CardInfo sdinfo;
	
	
	uint32_t  static temp;
	
	
	SD_GetCardInfo(&sdinfo);
	
	temp = sdinfo.CardCapacity;
	
	temp = temp*(1024/512);//扇区数量。注意:1sector=512byte
	
	
	//此处，计算大容量扇区时数值请注意不要超过32位
	//temp = temp/512;
	
	
	
	return (temp);
	
	

}





//SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr)//暂时不编写









/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */  

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
