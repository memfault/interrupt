MEMORY
{
  ram (rwx)      : ORIGIN = 0x20000000, LENGTH = 128K
  bootrom (rx)   : ORIGIN = 0x08000000, LENGTH = 128K /* 16K */
  updaterom (rx) : ORIGIN = 0x08004000, LENGTH = 0K /* 112K */
  approm (rx)    : ORIGIN = 0x08020000, LENGTH = 896K
  ccm (rwx)      : ORIGIN = 0x10000000, LENGTH = 64K
}

__bootrom_start__ = ORIGIN(bootrom);
__bootrom_size__ = LENGTH(bootrom);
__updaterom_start__ = ORIGIN(updaterom);
__updaterom_size__ = LENGTH(updaterom);
__approm_start__ = ORIGIN(approm);
__approm_size__ = LENGTH(approm);

