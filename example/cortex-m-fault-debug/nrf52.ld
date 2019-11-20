OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)

/**
* Memory layout for the NRF52
*/

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 1M
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x40000
}

STACK_SIZE = 0x400;

/* Section Definitions */
SECTIONS
{
    .interrupts :
    {
        KEEP(*(.isr_vector .isr_vector.*))
        /* FIXME: Size this */
    } > FLASH

    .got :
    {
        KEEP(*(.got .got.*))
    } > FLASH

    .text :
    {
        *(.text .text.* .gnu.linkonce.t.*)
        *(.glue_7t) *(.glue_7)
        *(.rodata .rodata* .gnu.linkonce.r.*)
    } > FLASH

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH

    . = ALIGN(4);
    _etext = .;
    _sidata = .;

    .start_of_ram (NOLOAD):
    {
        _start_of_ram = .;
    } > RAM

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

    /* place the stack at beginning of RAM */
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
