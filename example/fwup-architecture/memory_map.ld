MEMORY
{
  sharedram (rwx): ORIGIN = 0x20000000, LENGTH = 256
  ram (rwx)      : ORIGIN = 0x20000100, LENGTH = 128K - LENGTH(sharedram)
  bootrom (rx)   : ORIGIN = 0x08000000, LENGTH = 16K
  slot1rom (rx)  : ORIGIN = 0x08004000, LENGTH = 112K
  slot2rom (rx)  : ORIGIN = 0x08020000, LENGTH = 896K
  ccm (rwx)      : ORIGIN = 0x10000000, LENGTH = 64K
}

SECTIONS
{
    .shared_memory (NOLOAD) : {
        KEEP(*(.shared_memory))
    } >sharedram
}

__sharedram_start__ = ORIGIN(sharedram);
__sharedram_size__ = LENGTH(sharedram);
__bootrom_start__ = ORIGIN(bootrom);
__bootrom_size__ = LENGTH(bootrom);
__slot1rom_start__ = ORIGIN(slot1rom);
__slot1rom_size__ = LENGTH(slot1rom);
__slot2rom_start__ = ORIGIN(slot2rom);
__slot2rom_size__ = LENGTH(slot2rom);

