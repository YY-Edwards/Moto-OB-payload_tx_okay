/******************** (C) COPYRIGHT 2013 www.armjishu.com  ********************
 * 文件名  ：.h
 * 描述    ：
 * 实验平台：STM32神舟开发板
 * 标准库  ：STM32F10x_StdPeriph_Driver V3.5.0
 * 作者    ：www.armjishu.com 
*******************************************************************************/

#ifndef __AVR_SPI_SD_H
#define __AVR_SPI_SD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
//#include "stm3210c_eval.h"
#include "spi/spi.h"
#include "gpio/gpio.h"
//#include "utility.h"
//#include "evk1101.h"




  

/** @addtogroup Utilities
  * @{
  */
  
/** @addtogroup AVR
  * @{
  */ 

/** @addtogroup Common
  * @{
  */
  
/** @addtogroup AVR_SPI_SD
  * @{
  */  

/** @defgroup AVR_SPI_SD_Exported_Types
  * @{
  */ 

typedef enum
{
/**
  * @brief  SD reponses and error flags
  */
  SD_RESPONSE_NO_ERROR      = (0x00),
  SD_IN_IDLE_STATE          = (0x01),
  SD_ERASE_RESET            = (0x02),
  SD_ILLEGAL_COMMAND        = (0x04),
  SD_COM_CRC_ERROR          = (0x08),
  SD_ERASE_SEQUENCE_ERROR   = (0x10),
  SD_ADDRESS_ERROR          = (0x20),
  SD_PARAMETER_ERROR        = (0x40),
  SD_RESPONSE_FAILURE       = (0xFF),

/**
  * @brief  Data response error
  */
  SD_DATA_OK                = (0x05),
  SD_DATA_CRC_ERROR         = (0x0B),
  SD_DATA_WRITE_ERROR       = (0x0D),
  SD_DATA_OTHER_ERROR       = (0xFF)
} SD_Error;

/** 
  * @brief  Card Specific Data: CSD Register   
  */ 
typedef struct
{
  volatile uint8_t  CSDStruct;            /*!< CSD structure */
  volatile uint8_t  SysSpecVersion;       /*!< System specification version */
  volatile uint8_t  Reserved1;            /*!< Reserved */
  volatile uint8_t  TAAC;                 /*!< Data read access-time 1 */
  volatile uint8_t  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
  volatile uint8_t  MaxBusClkFrec;        /*!< Max. bus clock frequency */
  volatile uint16_t CardComdClasses;      /*!< Card command classes */
  volatile uint8_t  RdBlockLen;           /*!< Max. read data block length */
  volatile uint8_t  PartBlockRead;        /*!< Partial blocks for read allowed */
  volatile uint8_t  WrBlockMisalign;      /*!< Write block misalignment */
  volatile uint8_t  RdBlockMisalign;      /*!< Read block misalignment */
  volatile uint8_t  DSRImpl;              /*!< DSR implemented */
  volatile uint8_t  Reserved2;            /*!< Reserved */
  volatile uint32_t DeviceSize;           /*!< Device Size */
  volatile uint8_t  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
  volatile uint8_t  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
  volatile uint8_t  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
  volatile uint8_t  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
  volatile uint8_t  DeviceSizeMul;        /*!< Device size multiplier */
  volatile uint8_t  EraseGrSize;          /*!< Erase group size */
  volatile uint8_t  EraseGrMul;           /*!< Erase group size multiplier */
  volatile uint8_t  WrProtectGrSize;      /*!< Write protect group size */
  volatile uint8_t  WrProtectGrEnable;    /*!< Write protect group enable */
  volatile uint8_t  ManDeflECC;           /*!< Manufacturer default ECC */
  volatile uint8_t  WrSpeedFact;          /*!< Write speed factor */
  volatile uint8_t  MaxWrBlockLen;        /*!< Max. write data block length */
  volatile uint8_t  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
  volatile uint8_t  Reserved3;            /*!< Reserded */
  volatile uint8_t  ContentProtectAppli;  /*!< Content protection application */
  volatile uint8_t  FileFormatGrouop;     /*!< File format group */
  volatile uint8_t  CopyFlag;             /*!< Copy flag (OTP) */
  volatile uint8_t  PermWrProtect;        /*!< Permanent write protection */
  volatile uint8_t  TempWrProtect;        /*!< Temporary write protection */
  volatile uint8_t  FileFormat;           /*!< File Format */
  volatile uint8_t  ECC;                  /*!< ECC code */
  volatile uint8_t  CSD_CRC;              /*!< CSD CRC */
  volatile uint8_t  Reserved4;            /*!< always 1*/
} SD_CSD;

/** 
  * @brief  Card Identification Data: CID Register   
  */
typedef struct
{
  volatile uint8_t  ManufacturerID;       /*!< ManufacturerID */
  volatile uint16_t OEM_AppliID;          /*!< OEM/Application ID */
  volatile uint32_t ProdName1;            /*!< Product Name part1 */
  volatile uint8_t  ProdName2;            /*!< Product Name part2*/
  volatile uint8_t  ProdRev;              /*!< Product Revision */
  volatile uint32_t ProdSN;               /*!< Product Serial Number */
  volatile uint8_t  Reserved1;            /*!< Reserved1 */
  volatile uint16_t ManufactDate;         /*!< Manufacturing Date */
  volatile uint8_t  CID_CRC;              /*!< CID CRC */
  volatile uint8_t  Reserved2;            /*!< always 1 */
} SD_CID;

/** 
  * @brief SD Card information 
  */
typedef struct
{
  SD_CSD SD_csd;
  SD_CID SD_cid;
  uint32_t CardCapacity;  /*!< Card Capacity */
  uint32_t CardBlockSize; /*!< Card Block Size */
} SD_CardInfo;

/**
  * @}
  */
  
/** @defgroup AVR_SPI_SD_Exported_Constants
  * @{
  */ 


#define SD_SPI_FIRST_NPCS 0
#define SD_SPI_MASTER_SPEED 24000000
#define SD_SPI_BITS 8
#define SD_SPI_PCS_0 0
#define SPI_Clock_PBA 24000000




#define RESET 0

#define SET   !RESET


    
/**
  * @brief  Block Size
  */
#define SD_BLOCK_SIZE    0x200

/**
  * @brief  Dummy byte
  */
#define SD_DUMMY_BYTE   0xFF

/**
  * @brief  Start Data tokens:
  *         Tokens (necessary because at nop/idle (and CS active) only 0xff is 
  *         on the data/command line)  
  */
#define SD_START_DATA_SINGLE_BLOCK_READ    0xFE  /*!< Data token start byte, Start Single Block Read */
#define SD_START_DATA_MULTIPLE_BLOCK_READ  0xFE  /*!< Data token start byte, Start Multiple Block Read */
#define SD_START_DATA_SINGLE_BLOCK_WRITE   0xFE  /*!< Data token start byte, Start Single Block Write */
#define SD_START_DATA_MULTIPLE_BLOCK_WRITE 0xFD  /*!< Data token start byte, Start Multiple Block Write */
#define SD_STOP_DATA_MULTIPLE_BLOCK_WRITE  0xFD  /*!< Data toke stop byte, Stop Multiple Block Write */

/**
  * @brief  SD detection on its memory slot
  */
#define SD_PRESENT        ((uint8_t)0x01)
#define SD_NOT_PRESENT    ((uint8_t)0x00)


/**
  * @brief  Commands: CMDxx = CMD-number | 0x40
  */
#define SD_CMD_GO_IDLE_STATE          0   /*!< CMD0 = 0x40 */
#define SD_CMD_SEND_OP_COND           1   /*!< CMD1 = 0x41 */
#define SD_CMD_SEND_CSD               9   /*!< CMD9 = 0x49 */
#define SD_CMD_SEND_CID               10  /*!< CMD10 = 0x4A */
#define SD_CMD_STOP_TRANSMISSION      12  /*!< CMD12 = 0x4C */
#define SD_CMD_SEND_STATUS            13  /*!< CMD13 = 0x4D */
#define SD_CMD_SET_BLOCKLEN           16  /*!< CMD16 = 0x50 */
#define SD_CMD_READ_SINGLE_BLOCK      17  /*!< CMD17 = 0x51 */
#define SD_CMD_READ_MULT_BLOCK        18  /*!< CMD18 = 0x52 */
#define SD_CMD_SET_BLOCK_COUNT        23  /*!< CMD23 = 0x57 */
#define SD_CMD_WRITE_SINGLE_BLOCK     24  /*!< CMD24 = 0x58 */
#define SD_CMD_WRITE_MULT_BLOCK       25  /*!< CMD25 = 0x59 */
#define SD_CMD_PROG_CSD               27  /*!< CMD27 = 0x5B */
#define SD_CMD_SET_WRITE_PROT         28  /*!< CMD28 = 0x5C */
#define SD_CMD_CLR_WRITE_PROT         29  /*!< CMD29 = 0x5D */
#define SD_CMD_SEND_WRITE_PROT        30  /*!< CMD30 = 0x5E */
#define SD_CMD_SD_ERASE_GRP_START     32  /*!< CMD32 = 0x60 */
#define SD_CMD_SD_ERASE_GRP_END       33  /*!< CMD33 = 0x61 */
#define SD_CMD_UNTAG_SECTOR           34  /*!< CMD34 = 0x62 */
#define SD_CMD_ERASE_GRP_START        35  /*!< CMD35 = 0x63 */
#define SD_CMD_ERASE_GRP_END          36  /*!< CMD36 = 0x64 */
#define SD_CMD_UNTAG_ERASE_GROUP      37  /*!< CMD37 = 0x65 */
#define SD_CMD_ERASE                  38  /*!< CMD38 = 0x66 */

/**
  * @}
  */ 
  
/** @defgroup AVR_SPI_SD_Exported_Macros
  * @{
  */
/** 
  * @brief  Select SD Card: ChipSelect pin low   
  */  
#define SD_CS_LOW()     gpio_clr_gpio_pin(AVR32_SPI_NPCS_0_1_PIN)
/** 
  * @brief  Deselect SD Card: ChipSelect pin high   
  */
#define SD_CS_HIGH()    gpio_set_gpio_pin(AVR32_SPI_NPCS_0_1_PIN)
/**
  * @}
  */ 

/** @defgroup AVR_SPI_SD_Exported_Functions
  * @{
  */ 
void SD_DeInit(void);  
SD_Error SD_Init(void);
//uint8_t SD_Detect(void);
SD_Error SD_GetCardInfo(SD_CardInfo *cardinfo);
SD_Error SD_ReadBlock(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize);
SD_Error SD_ReadMultiBlocks(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SD_WriteBlock(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize);
SD_Error SD_WriteMultiBlocks(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SD_Error SD_GetCSDRegister(SD_CSD* SD_csd);
SD_Error SD_GetCIDRegister(SD_CID* SD_cid);
SD_Error SD_WaitReady(void);
//SD_Error SD_Erase(uint32_t startaddr, uint32_t endaddr);





void SD_SendCmd(uint8_t Cmd, uint32_t Arg, uint8_t Crc);
SD_Error SD_GetResponse(uint8_t Response);
uint8_t SD_GetDataResponse(void);
SD_Error SD_GoIdleState(void);
uint16_t SD_GetStatus(void);
uint32_t SD_GetSectorCount(void);

uint8_t SD_WriteByte(uint8_t byte);
uint8_t SD_ReadByte(void);

void SD_LowLevel_DeInit(void);
void SD_LowLevel_Init(void);
void SD_SPI_SetSpeed(uint16_t SPI_BaudRatePrescaler);
void SD_SPI_SetSpeedLow(void);
void SD_SPI_SetSpeedHi(void);


#ifdef __cplusplus
}
#endif

#endif /* __AVR_SPI_SD_H */


/******************* (C) COPYRIGHT 2015 STMicroelectronics *****END OF FILE****/
