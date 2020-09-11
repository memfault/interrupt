OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)

/**
* Memory layout for the NRF52
*/

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 0xFFF9C
  FLASH_END (rx): ORIGIN = 0xFFF9C, LENGTH = 100
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 0x40000
}

STACK_SIZE = 0x2000;
_estack = 0x20040000;

/* Section Definitions */
SECTIONS
{
    .interrupts :
    {
        KEEP(*(.isr_vector .isr_vector.*))

        /* FIXME: Size Appropriately. For now just leave buffer room
        ** so dummy_funcs always wind up at the same address
        ** and the offsets don't deviate from what we use in the blog post writeup
        */
        . = 0x240;
    } > FLASH

    .dummy_funcs :
    {

       *(.text.dummy_function_*)
    }

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


    /* stack section */
    .stack (NOLOAD):
    {
        . = ALIGN(8);
        _sstack = .;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        _estack = .;
    } > RAM

    .data : AT (_etext)
    {
        . = ALIGN(4);
        _sdata = .;
        *(ram_func )
        *(.data .data.*)
        . = ALIGN(4);
        _edata = . ;
    } > RAM

    /* .bss section which is used for uninitialized data */
    .bss (NOLOAD) :
    {
        _sbss = . ;
        __bss_start__ = _sbss;
        *(.bss.s_graphics_buf)
        *(.bss.s_registered_handler)
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

    . = ALIGN(4);
    _end = . ;
}
