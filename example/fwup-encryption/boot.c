#include "aes_ctr.h"
#include "flash.h"

#define SLOT_ADDRESS_CIPHER_APP  0x08001000
#define SLOT_ADDRESS_PLAIN_APP   0x08002000
#define FLASH_CLEAN_SECTOR_VALUE 0xFFFFFFFF
#define NUMBER_BYTES_TO_PROCESS  16
#define FIRMWARE_PLAIN_APP_SIZE  2176     

int  image_validate(void);
void image_decrypt(void);
void image_install(uint8_t *PlainFirmware, uint32_t *Address);
void image_start(void);

uint8_t PlainFirmware[NUMBER_BYTES_TO_PROCESS];
uint8_t CipherFirmware[NUMBER_BYTES_TO_PROCESS];

int main(void) {

    aes_setup();

    if(image_validate()) {
        image_decrypt();
    }

    image_start();

    return 0;
}

int image_validate(void) {
    uint32_t slot_app;
    uint32_t slot_chipherApp;
    slot_app = *(uint32_t*)(SLOT_ADDRESS_PLAIN_APP);
    slot_chipherApp = *(uint32_t*)(SLOT_ADDRESS_CIPHER_APP);

    if( slot_app == FLASH_CLEAN_SECTOR_VALUE &&  slot_chipherApp != FLASH_CLEAN_SECTOR_VALUE ) {
        //! Decrypt the app and start the installation
        return 1;
    }
     
    return 0;
}

void image_decrypt(void) {

    uint32_t slot_Cipher_appAddr = SLOT_ADDRESS_CIPHER_APP;
    uint32_t slot_Plain_appAddr = SLOT_ADDRESS_PLAIN_APP;
    int8_t idx = 3;
    uint32_t word;
    int32_t bytes_remaining = FIRMWARE_PLAIN_APP_SIZE;
    
    word = *(uint32_t*)(slot_Cipher_appAddr);
    
    flash_unlock();

    //! Erase flash sector slot app
    flash_erase(2, 2);

    while(bytes_remaining) {

        //! Read 16 bytes of encrypted data
        for(size_t i=0; i<NUMBER_BYTES_TO_PROCESS; i++) {
            if(idx < 0) {
                idx = 3;
                slot_Cipher_appAddr += 4;
                word = *(uint32_t*)(slot_Cipher_appAddr);
            }
            CipherFirmware[i] = (word >> (8*idx--)) & 0xff;
        }

        //! Decrypt the 16 bytes of data firmware
        aes_decryption( CipherFirmware, NUMBER_BYTES_TO_PROCESS, PlainFirmware );

        //! Write 16 bytes of data to flash memory
        image_install( PlainFirmware, &slot_Plain_appAddr);

        bytes_remaining -= 16;
        idx = 3;
        slot_Cipher_appAddr += 4;
        word = *(uint32_t*)(slot_Cipher_appAddr);
        if(bytes_remaining <= 0 ) bytes_remaining = 0;
    }

    flash_lock();
}

void image_install(uint8_t *PlainFirmware, uint32_t *Address) {

    uint32_t wordPlain;

    //! Start Write the 16 bytes 
    wordPlain = PlainFirmware[3] | (PlainFirmware[2] << 8) | (PlainFirmware[1] << 16) | (PlainFirmware[0] << 24);    
    flash_write(*Address, wordPlain);

    *Address += 4;
    wordPlain = PlainFirmware[7] | (PlainFirmware[6] << 8) | (PlainFirmware[5] << 16) | (PlainFirmware[4] << 24);
    flash_write(*Address, wordPlain);

    *Address += 4;
    wordPlain = PlainFirmware[11] | (PlainFirmware[10] << 8) | (PlainFirmware[9] << 16) | (PlainFirmware[8] << 24);
    flash_write(*Address, wordPlain);

    *Address += 4;
    wordPlain = PlainFirmware[15] | (PlainFirmware[14] << 8) | (PlainFirmware[13] << 16) | (PlainFirmware[12] << 24);
    flash_write(*Address, wordPlain);

    *Address += 4;
}

void image_start(void) {
    uint32_t jump_address;
    typedef void(*start_app)(void);
    start_app p_jump_app;

    jump_address = *(volatile uint32_t *)(((uint32_t)SLOT_ADDRESS_PLAIN_APP + 4));
    p_jump_app   = (start_app)jump_address;

    //! Initialize loader's Stack Pointer 
    __set_MSP(*(__IO uint32_t *)(SLOT_ADDRESS_PLAIN_APP));

    //! Jump into app
    p_jump_app();
}
