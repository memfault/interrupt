/* Run the boot image. */



#include "flash_map_backend/flash_map_backend.h"
#include "sysflash/sysflash.h"
#include "os/os_malloc.h"

// libc deps

#include <sys/stat.h>

int _close(int file) {
  return -1;
}

int _fstat(int file, struct stat *st) {
  st->st_mode = S_IFCHR;

  return 0;
}

int _isatty(int file) {
  return 1;
}

int _lseek(int file, int ptr, int dir) {
  return 0;
}

void _exit(int status) {
  __asm("BKPT #0");
  __builtin_unreachable();
}

void _kill(int pid, int sig) {
  return;
}

int _getpid(void) {
  return -1;
}

int _write (int file, char * ptr, int len) {
  return -1;
}

int _read (int file, char * ptr, int len) {
  return -1;
}


void *os_malloc(size_t size) {
  return malloc(size);
}

void os_free(void *ptr) {
  free(ptr);
}

int flash_area_open(uint8_t id, const struct flash_area **areap) {
  return 0;

}

void flash_area_close(const struct flash_area *area) {

}

int flash_area_read(const struct flash_area *area, uint32_t off, void *dst,
                    uint32_t len) {
  return 0;
}

int flash_area_write(const struct flash_area *area, uint32_t off, const void *src,
                     uint32_t len) {
  return 0;
}

int flash_area_erase(const struct flash_area *area, uint32_t off, uint32_t len) {
  return 0;
}

uint8_t flash_area_align(const struct flash_area *area) {
  return 0;
}

uint8_t flash_area_erased_val(const struct flash_area *area) {
  return 0;
}

int flash_area_get_sectors(int fa_id, uint32_t *count,
                           struct flash_sector *sectors) {
  return 0;
}

int flash_area_id_from_multi_image_slot(int image_index, int slot)
{
    switch (slot) {
    case 0: return FLASH_AREA_IMAGE_PRIMARY(image_index);
#if !defined(CONFIG_SINGLE_APPLICATION_SLOT)
    case 1: return FLASH_AREA_IMAGE_SECONDARY(image_index);
#if !defined(CONFIG_BOOT_SWAP_USING_MOVE)
    case 2: return FLASH_AREA_IMAGE_SCRATCH;
#endif
#endif
    }

    return -1; /* flash_area_open will fail on that */
}

int flash_area_id_from_image_slot(int slot)
{
    return flash_area_id_from_multi_image_slot(0, slot);
}


int default_CSPRNG(uint8_t *dest, unsigned int size) {
  __asm("bkpt 22");
  return -1;
}
