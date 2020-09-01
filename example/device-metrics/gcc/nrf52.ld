OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
SEARCH_DIR(.)

/**
* Memory layout for the NRF52
*/

MEMORY
{
  FLASH (rx) : ORIGIN = 0x0, LENGTH = 0xE0000
  FLASH_EVENT_STORAGE (r) : ORIGIN = 0xE0000, LENGTH = 0x2000
  RAM (rwx) :  ORIGIN = 0x20000000, LENGTH = 64000
  COREDUMP_RAM (rwx) : ORIGIN = 0x20010000, LENGTH = 8192
}

STACK_SIZE = 4096; /* 1024; */

/* Variables used by FreeRTOS-MPU. */
_Privileged_Functions_Region_Size = 32K;
_Privileged_Data_Region_Size = 512;

__FLASH_segment_start__ = ORIGIN( FLASH );
__FLASH_segment_end__ = __FLASH_segment_start__ + LENGTH( FLASH );

__privileged_functions_start__ = ORIGIN( FLASH );
__privileged_functions_end__ = __privileged_functions_start__ + _Privileged_Functions_Region_Size;

__SRAM_segment_start__ = ORIGIN( RAM );
__SRAM_segment_end__ = __SRAM_segment_start__ + LENGTH( RAM );

__privileged_data_start__ = ORIGIN( RAM );
__privileged_data_end__ = ORIGIN( RAM ) + _Privileged_Data_Region_Size;

/* Section Definitions */
SECTIONS
{

    .interrupts :
    {
        KEEP(*(.isr_vector .isr_vector.*))
        /* FIXME: Size this */

        . = 0x200;
    } > FLASH

    .got :
    {
        KEEP(*(.got .got.*))
    } > FLASH

    .note.gnu.build-id :
    {
        __start_gnu_build_id_start = .;
        KEEP(*(.note.gnu.build-id))
    } > FLASH


    /* Privileged functions - Section needs to be 32 byte aligned to satisfy MPU requirements. */
    .privileged_functions : ALIGN(32)
    {
        . = ALIGN(32);
        __privileged_functions_start__ = .;
        *(privileged_functions)
        . = ALIGN(32);
        /* End address must be the last address in the region, therefore, -1. */
        __privileged_functions_end__ = . - 1;
    } > FLASH

    /* FreeRTOS System calls - Section needs to be 32 byte aligned to satisfy MPU requirements. */
    .freertos_system_calls : ALIGN(32)
    {
        . = ALIGN(32);
        __syscalls_flash_start__ = .;
        *(freertos_system_calls)
        . = ALIGN(32);
        /* End address must be the last address in the region, therefore, -1. */
        __syscalls_flash_end__ = . - 1;
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

    .data : AT (_etext)
    {
        . = ALIGN(4);
        _sdata = .;
        /* Privileged data - It needs to be 32 byte aligned to satisfy MPU requirements. */
        . = ALIGN(32);
        __privileged_sram_start__ = .;
        *(privileged_data);
        . = ALIGN(32);
        /* End address must be the last address in the region, therefore, -1. */
        __privileged_sram_end__ = . - 1;

        *(.data .data.*)
        _edata = . ;
        . = ALIGN(4);
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
        __StackLimit = .;
        _sstack = .;
        . = . + STACK_SIZE;
        . = ALIGN(8);
        _estack = .;
    } > RAM

    . = ALIGN(4);
    _end = . ;
}
