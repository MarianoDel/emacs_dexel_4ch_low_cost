//---------------------------------------------
// ##
// ## @Author: Med
// ## @Editor: Emacs - ggtags
// ## @TAGS:   Global
// ## @CPU:    STM32G030
// ##
// #### FLASH_PROGRAM.C #######################
//---------------------------------------------

#include "flash_program.h"
#include "stm32g0xx.h"



// Externals -------------------------------------------------------------------


// Globals ---------------------------------------------------------------------


// Private Types Constants and Macros ------------------------------------------


// Module Private Functions ----------------------------------------------------
// FLASH_Status Flash_ErasePage (uint8_t Page_Num, uint8_t lock_after_finish);
void Flash_Unlock (void);
void Flash_Lock (void);
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout);
FLASH_Status FLASH_GetStatus(void);
FLASH_Status FLASH_ProgramWord(uint32_t Address, uint32_t Data);
FLASH_Status Flash_WriteFlash (uint32_t dest_addr, uint32_t * origin_addr, uint8_t lock, uint8_t len8);
static void FLASH_Program_DoubleWord(uint32_t Address, uint64_t Data);

// Module functions ------------------------------------------------------------
FLASH_Status Flash_WriteConfigurations (uint32_t * Address, uint32_t size)
{
    FLASH_Status status = FLASH_COMPLETE;

    if ((size % 8) != 0)
        return FLASH_ERROR_PROGRAM; 

    if (size > 2048)
        return FLASH_ERROR_PROGRAM; 

    
    status = Flash_ErasePage (FLASH_PAGE_FOR_BKP, 0);
    if (status != FLASH_COMPLETE)
        return status;

    unsigned char size8 = size >> 3;
    status = Flash_WriteFlash (FLASH_ADDRESS_FOR_BKP, Address, 1, size8);

    return status;
}


FLASH_Status Flash_WriteFlash (uint32_t dest_addr, uint32_t * origin_addr, uint8_t lock, uint8_t len8)
{
    FLASH_Status status = FLASH_COMPLETE;
    uint64_t data64 = 0;
    uint64_t * odata;

    FLASH->CR |= FLASH_CR_PG;
    
    status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    if (status != FLASH_COMPLETE)
        return status;

    
    odata = (uint64_t *) origin_addr;
    for (unsigned short i = 0; i < len8; i++)
    {
        data64 = *(odata + i);
        FLASH_Program_DoubleWord((dest_addr + (i << 3)), data64);
        
        status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
        if (status != FLASH_COMPLETE)
            return status;
        
    }

    FLASH->CR &= (~FLASH_CR_PG);

    if (lock)
        Flash_Lock();

    status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);
    return status;
    
    // //verifico memoria
    // for (i = 0; i < len_in_4; i++)
    // {
    //     if (*(p_verif + i) != *(p + i))
    //         return FAILED;
    // }
    // return PASSED;
}


FLASH_Status Flash_ErasePage(uint8_t Page_Num, uint8_t lock)
{
    FLASH_Status status = FLASH_COMPLETE;
    
    Flash_Unlock();

    status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);

    if(status == FLASH_COMPLETE)
    {
        /* If the previous operation is completed, proceed to erase the page */
        FLASH->CR |= FLASH_CR_PER;
        FLASH->CR |= (Page_Num << FLASH_CR_PNB_Pos);
        FLASH->CR |= FLASH_CR_STRT;

        /* Wait for last operation to be completed */
        status = FLASH_WaitForLastOperation(FLASH_ER_PRG_TIMEOUT);

        /* Disable the PER Bit */
        FLASH->CR &= (~FLASH_CR_PER);

        if (lock)
            Flash_Lock();
        
    }

    return status;
}


void Flash_Unlock(void)
{
    if((FLASH->CR & FLASH_CR_LOCK) != RESET)
    {
        /* Unlocking the program memory access */
        FLASH->KEYR = FLASH_FKEY1;
        FLASH->KEYR = FLASH_FKEY2;
    }
}


void Flash_Lock(void)
{
    /* Set the LOCK Bit to lock the FLASH control register and program memory access */
    FLASH->CR |= FLASH_CR_LOCK;
}


// /**
//   * @brief  Waits for a FLASH operation to complete or a TIMEOUT to occur.
//   * @param  Timeout: FLASH programming Timeout
//   * @retval FLASH Status: The returned value can be: FLASH_BUSY,
//   *         FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP, FLASH_COMPLETE or FLASH_TIMEOUT.
//   */
FLASH_Status FLASH_WaitForLastOperation(uint32_t Timeout)
{
  FLASH_Status status = FLASH_COMPLETE;

  /* Check for the FLASH Status */
  status = FLASH_GetStatus();

  /* Wait for a FLASH operation to complete or a TIMEOUT to occur */
  while((status == FLASH_BUSY) && (Timeout != 0x00))
  {
    status = FLASH_GetStatus();
    Timeout--;
  }

  if(Timeout == 0x00 )
  {
    status = FLASH_TIMEOUT;
  }
  /* Return the operation status */
  return status;
}

/**
  * @brief  Returns the FLASH Status.
  * @param  None
  * @retval FLASH Status: The returned value can be:
  *         FLASH_BUSY, FLASH_ERROR_PROGRAM, FLASH_ERROR_WRP or FLASH_COMPLETE.
  */
FLASH_Status FLASH_GetStatus(void)
{
  FLASH_Status FLASHstatus = FLASH_COMPLETE;

  if((FLASH->SR & FLASH_FLAG_BSY) == FLASH_FLAG_BSY)
  {
    FLASHstatus = FLASH_BUSY;
  }
  else
  {
    if((FLASH->SR & (uint32_t)FLASH_FLAG_WRPERR)!= (uint32_t)0x00)
    {
      FLASHstatus = FLASH_ERROR_WRP;
    }
    else
    {
      if((FLASH->SR & (uint32_t)(FLASH_FLAG_PGERR)) != (uint32_t)0x00)
      {
        FLASHstatus = FLASH_ERROR_PROGRAM;
      }
      else
      {
        FLASHstatus = FLASH_COMPLETE;
      }
    }
  }
  /* Return the FLASH Status */
  return FLASHstatus;
}


/**
  * @brief  Program double-word (64-bit) at a specified address.
  * @param  Address Specifies the address to be programmed.
  * @param  Data Specifies the data to be programmed.
  * @retval None
  */
static void FLASH_Program_DoubleWord(uint32_t Address, uint64_t Data)
{
  /* Set PG bit */
  SET_BIT(FLASH->CR, FLASH_CR_PG);

  /* Program first word */
  *(uint32_t *)Address = (uint32_t)Data;

  /* Barrier to ensure programming is performed in 2 steps, in right order
    (independently of compiler optimization behavior) */
  __ISB();

  /* Program second word */
  *(uint32_t *)(Address + 4U) = (uint32_t)(Data >> 32U);
}

//--- end of file ---//
