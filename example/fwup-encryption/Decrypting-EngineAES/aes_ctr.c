#include "aes_ctr.h"


void aes_setup( void ) {

    uint32_t keyaddr;
    uint32_t Nonce;

    //! Enable source clock for  AES peripheral
    RCC->AHB2ENR |= RCC_AHB2ENR_AESEN;

    //! Disable the AES peripheral
    AES->CR &= ~AES_CR_EN;

    //! Select the mode CTR
    AES->CR |= AES_CR_CHMOD_1;

    //! Select the mode decryption
    AES->CR |= AES_CR_MODE_1;

    //! Select the Data type 8-bit
    AES->CR |= AES_CR_DATATYPE_1;
    
    //! Set the key, for default key size it's 128 bits 
    keyaddr = (uint32_t)(pKeyAES);

    AES->KEYR3 = __REV(*(uint32_t*)(keyaddr));
    keyaddr += 4;
    AES->KEYR2 = __REV(*(uint32_t*)(keyaddr));
    keyaddr += 4;
    AES->KEYR1 = __REV(*(uint32_t*)(keyaddr));
    keyaddr += 4;
    AES->KEYR0  = __REV(*(uint32_t*)(keyaddr));

    //! Set the Nonce InitVector/InitCounter
    Nonce = (uint32_t)(NonceAES);

    AES->IVR3 = __REV(*(uint32_t*)(Nonce));
    Nonce += 4;
    AES->IVR2 = __REV(*(uint32_t*)(Nonce));
    Nonce += 4;
    AES->IVR1 = __REV(*(uint32_t*)(Nonce));
    Nonce += 4;
    AES->IVR0 = __REV(*(uint32_t*)(Nonce));

    //! Enable the AES peripheral
    AES->CR |=  AES_CR_EN;
}

void aes_decryption( uint8_t *Input_CipherFirmware, uint16_t buf_size, uint8_t *Out_PlainFirmware ) {

    uint32_t inputaddr  = (uint32_t)Input_CipherFirmware;
    uint32_t outputaddr = (uint32_t)Out_PlainFirmware;

    for( uint32_t idx = 0; idx < buf_size; idx += 16 ){

        //! Write the ChipherText in the input register DINR
        AES->DINR = *(uint32_t*)(inputaddr);
        inputaddr += 4;
        AES->DINR = *(uint32_t*)(inputaddr);
        inputaddr += 4;
        AES->DINR = *(uint32_t*)(inputaddr);
        inputaddr += 4;
        AES->DINR = *(uint32_t*)(inputaddr);
        inputaddr += 4;

        //! Computation completed flag
        while( (AES->SR & AES_SR_CCF) == 0);

        //! Clear the completed flag
        AES->CR |= AES_CR_CCFC;

        //! Read the PainText from the output register DOUTR     
        *(uint32_t*)(outputaddr) = AES->DOUTR;
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = AES->DOUTR;
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = AES->DOUTR;
        outputaddr+=4U;
        *(uint32_t*)(outputaddr) = AES->DOUTR;
        outputaddr+=4U;
    }

}


