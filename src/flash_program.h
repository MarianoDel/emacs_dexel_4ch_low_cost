//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32G030
// ##
// #### FLASH_PROGRAM.H #######################
//---------------------------------------------

// Define to prevent recursive inclusion ---------------------------------------
#ifndef _FLASH_PROGRAM_H_
#define _FLASH_PROGRAM_H_

#include "stm32g0xx.h"

// Configurations Defines ------------------------------------------------------
//- Where to Backup ------------------------------------------------------------
// #define FLASH_PAGE_FOR_BKP    PAGE15
// #define FLASH_ADDRESS_FOR_BKP    PAGE15_ADDR
#define FLASH_PAGE_FOR_BKP    PAGE31
#define FLASH_ADDRESS_FOR_BKP    PAGE31_ADDR


// Define the STM32F10x FLASH Page Size depending on the used STM32 device
// si es mayor a 128K la pagina es de 2KB, sino 1KB

#define FLASH_PAGE_SIZE        2048
#define FLASH_PAGE_SIZE_DIV2    1024
#define FLASH_PAGE_SIZE_DIV4    512

#define PAGE15    (15U)
#define PAGE31    (31U)
#if (FLASH_PAGE_SIZE == 2048)
#define PAGE15_ADDR    ((uint32_t)0x08007800)
#define PAGE31_ADDR    ((uint32_t)0x0800F800)
#endif
#if (FLASH_PAGE_SIZE == 1024)
#define PAGE30_ADDR    ((uint32_t)0x08007800)
#define PAGE31_ADDR    ((uint32_t)0x08007C00)
#define PAGE62_ADDR    ((uint32_t)0x0800F800)
#define PAGE63_ADDR    ((uint32_t)0x0800FC00)
#endif

//de libreria st las tiene #include "stm32f0xx_flash.h"
/**
  * @brief  FLASH Status
  */
typedef enum {
    FLASH_BUSY = 1,
    FLASH_ERROR_WRP,
    FLASH_ERROR_PROGRAM,
    FLASH_COMPLETE,
    FLASH_TIMEOUT
      
} FLASH_Status;

/** @defgroup FLASH_Timeout_definition
  * @{
  */
#define FLASH_ER_PRG_TIMEOUT         ((uint32_t)0x000B0000)

/** @defgroup FLASH_Flags
  * @{
  */

#define FLASH_FLAG_BSY                 FLASH_SR_BSY1     /*!< FLASH Busy flag */
#define FLASH_FLAG_PGERR               FLASH_SR_PGSERR   /*!< FLASH Programming error flag */
#define FLASH_FLAG_WRPERR              FLASH_SR_WRPERR  /*!< FLASH Write protected error flag */
#define FLASH_FLAG_EOP                 FLASH_SR_EOP     /*!< FLASH End of Programming flag */

/******************  FLASH Keys  **********************************************/
#define FLASH_FKEY1        ((uint32_t)0x45670123)        /*!< Flash program erase key1 */
#define FLASH_FKEY2        ((uint32_t)0xCDEF89AB)        /*!< Flash program erase key2: used with FLASH_PEKEY1
                                                              to unlock the write access to the FPEC. */

#define IS_FLASH_CLEAR_FLAG(FLAG) ((((FLAG) & (uint32_t)0xFFFFFFCB) == 0x00000000) && ((FLAG) != 0x00000000))

#define IS_FLASH_GET_FLAG(FLAG)  (((FLAG) == FLASH_FLAG_BSY) || ((FLAG) == FLASH_FLAG_PGERR) || \
                                  ((FLAG) == FLASH_FLAG_WRPERR) || ((FLAG) == FLASH_FLAG_EOP))


// Module exported Functions ---------------------------------------------------
FLASH_Status Flash_ErasePage (uint8_t Page_Num, uint8_t lock_after_finish);
FLASH_Status Flash_WriteConfigurations (uint32_t * Address, uint32_t size);

#endif    /* _FLASH_PROGRAM_H_ */

//--- end of file ---//

