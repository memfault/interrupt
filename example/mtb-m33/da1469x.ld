OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)

/**
* Memory layout for the NRF52
*/

MEMORY
{
  RAM_VEC (rx): ORIGIN = 0x20000000, LENGTH = 0x200
  RAM_FLASH (rx) : ORIGIN = (0x20000000 + 0x200), LENGTH = (32 * 1024)
  RAM (rwx) : ORIGIN = (0x20000000 + 0x200 + 32*1024), LENGTH = ((512 * 1024) - 0x200 - 32*1024 - 8192)
  MTB (rwx) : ORIGIN = 0x7e000, LENGTH = 8192
}

STACK_SIZE = 0x1000;
_estack = 0x20040000;

/* Section Definitions */
SECTIONS
{
    .interrupts :
    {
        KEEP(*(.isr_vector .isr_vector.*))
        /* FIXME: Size this */
        . = 0x200;
    } > RAM_VEC

    .got :
    {
        KEEP(*(.got .got.*))
    } > RAM_FLASH

    .text :
    {
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
    } > RAM_FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > RAM_FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > RAM_FLASH

    . = ALIGN(4);
    _etext = .;
    _sidata = .;

    .data : AT (_etext)
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data .data.*)
        . = ALIGN(4);
        _edata = . ;
    } > RAM

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        _sbss = . ;
        __bss_start__ = _sbss;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = . ;
        __bss_end__ = _ebss;
    } > RAM

    .heap (COPY):
    {
        . = ALIGN(4);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        PROVIDE ( __end__ = . );

        *(.heap*)
        __HeapLimit = .;
        . = ALIGN(4);
    } > RAM

    /* stack section */
    .stack (NOLOAD):
    {
        . = ALIGN(8);
        _sstack = .;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        _estack = .;
    } > RAM

    . = ALIGN(4);
    _end = . ;
}
