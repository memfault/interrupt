#include "flash.h"

#define FLASH_KEY1          0x45670123U
#define FLASH_KEY2          0xCDEF89ABU  
#define FLASH_BANK_1        ((uint32_t)0x01)
#define FLASH_SR_ERRORS     (FLASH_SR_OPERR  | FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR | FLASH_SR_PGSERR  | FLASH_SR_MISERR | FLASH_SR_FASTERR | FLASH_SR_RDERR | FLASH_SR_OPTVERR | FLASH_SR_PEMPTY)
#define FLASH_ECCR_ERRORS   (FLASH_ECCR_ECCD | FLASH_ECCR_ECCD2 | FLASH_ECCR_ECCC | FLASH_ECCR_ECCC2)

bool flash_unlock(void) {

    //! Authorize the FLASH Registers access
    FLASH->KEYR = FLASH_KEY1;
    FLASH->KEYR = FLASH_KEY2;
    
    //! Verify Flash is unlocked 
    if( (FLASH->CR & FLASH_CR_LOCK) != 0U) {
      return false;
    }

    return true;
}

bool flash_lock(void) {
    //! Set the LOCK Bit to lock the FLASH Registers access
    FLASH->CR |= FLASH_CR_LOCK;
    return true;
}

static bool pvr_flash_clear_error(void) { 
    
    uint32_t error;

    while( (FLASH->SR & FLASH_SR_BSY) != 0 ) {
        //! Flash operation is in progress
        for(uint32_t i = 0; i <500; i++) {
            __asm__("nop");
        }
        return false;
    }

    //! Clear the error code 
    error = (FLASH->SR & FLASH_SR_ERRORS);

    if(error) {
        if(((error) & FLASH_ECCR_ERRORS) != 0U) { 
            SET_BIT(FLASH->ECCR, ((error) & FLASH_ECCR_ERRORS));
        }

        if(((error) & ~(FLASH_ECCR_ERRORS)) != 0U) { 
            WRITE_REG(FLASH->SR, ((error) & ~(FLASH_ECCR_ERRORS))); 
        }
    }

    //! Clear EOP End of Operation pending bit
    if(FLASH->SR & FLASH_SR_EOP) {
        WRITE_REG(FLASH->SR, ((FLASH_SR_EOP) & ~(FLASH_ECCR_ERRORS)));
    }

    return true;
}

void flash_erase(uint32_t ErasePage, uint32_t NbPages) {
    
    uint32_t page_index;

    pvr_flash_clear_error();

    //! Disable data cache
    CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCEN);

    for(page_index = ErasePage; page_index < (ErasePage + NbPages); page_index++)
    {
        CLEAR_BIT(FLASH->CR, FLASH_CR_BKER);

        //! Proceed to erase the page         
        MODIFY_REG(FLASH->CR, FLASH_CR_PNB, ((page_index & 0xFFU) << FLASH_CR_PNB_Pos));
        SET_BIT(FLASH->CR, FLASH_CR_PER);
        SET_BIT(FLASH->CR, FLASH_CR_STRT);

        pvr_flash_clear_error();

        //! If the erase operation is completed, disable the PER Bit
        CLEAR_BIT(FLASH->CR, (FLASH_CR_PER | FLASH_CR_PNB));
    }


    //! Reset data cache
    do { SET_BIT(FLASH->ACR, FLASH_ACR_ICRST);
         CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICRST);
    } while (0);

    //! Enable data cache
    SET_BIT(FLASH->ACR, FLASH_ACR_DCEN);
}

void flash_write(uint32_t Address, uint32_t Data) {

    pvr_flash_clear_error();
    
    //! Disable data cache
    CLEAR_BIT(FLASH->ACR, FLASH_ACR_DCEN);

    //! Set PG bit
    SET_BIT(FLASH->CR, FLASH_CR_PG);

    //! Program first word
    *(__IO uint32_t*)Address = (uint32_t)Data;

    __ISB();

    pvr_flash_clear_error();
    
    //! Disable the PG or FSTPG Bit
    CLEAR_BIT(FLASH->CR, FLASH_CR_PG);

    //! Reset data cache
    do { SET_BIT(FLASH->ACR, FLASH_ACR_ICRST);
         CLEAR_BIT(FLASH->ACR, FLASH_ACR_ICRST);
    } while (0);

    //! Enable data cache
    SET_BIT(FLASH->ACR, FLASH_ACR_DCEN);
}

