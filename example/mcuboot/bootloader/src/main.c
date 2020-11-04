#include <stdio.h>

#include "cmsis_shim.h"
#include "hal/uart.h"
#include "hal/logging.h"

#include "shell_port.h"

#include "bootutil/bootutil.h"
#include "bootutil/image.h"


#include <string.h>

//! A very naive implementation of the newlib _sbrk dependency function
caddr_t _sbrk(int incr);
caddr_t _sbrk(int incr) {
  static uint32_t s_index = 0;
  static uint8_t s_newlib_heap[2048] __attribute__((aligned(8)));

  if ((s_index + (uint32_t)incr) <= sizeof(s_newlib_heap)) {
    EXAMPLE_LOG("Out of Memory!");
    return 0;
  }

  caddr_t result = (caddr_t)&s_newlib_heap[s_index];
  s_index += (uint32_t)incr;
  return result;
}

__attribute__((noinline))
static void prv_enable_vfp( void ){
  volatile uint32_t *cpacr = (volatile uint32_t *)0xE000ED88;
  *cpacr |= 0xf << 20;
}

static void start_app(uint32_t pc, uint32_t sp) {
  __asm volatile ("MSR msp, %0" : : "r" (sp) : );
  void (*application_reset_handler)(void) = (void *)pc;
  application_reset_handler();
}

static void do_boot(struct boot_rsp *rsp) {
  EXAMPLE_LOG("Starting Main Application");
  EXAMPLE_LOG("  Image Start Offset: 0x%x", (int)rsp->br_image_off);

  // We run from internal flash. Base address of this medium is 0x0
  uint32_t vector_table = 0x0 + rsp->br_image_off + rsp->br_hdr->ih_hdr_size;

  uint32_t *app_code = (uint32_t *)vector_table;
  uint32_t app_sp = app_code[0];
  uint32_t app_start = app_code[1];

  EXAMPLE_LOG("  Vector Table Start Address: 0x%x. PC=0x%x, SP=0x%x",
              (int)vector_table, app_start, app_sp);

  // We need to move the vector table to reflect the location of the main application
  volatile uint32_t *vtor = (uint32_t *)0xE000ED08;
  *vtor = (vector_table & 0xFFFFFFF8);

  start_app(app_start, app_sp);
}

int main(void) {
  prv_enable_vfp();
  uart_boot();

  // because a bootloader is a good opportunity for a little ASCII art!
  EXAMPLE_LOG("\n\n___  ________ _   _ _                 _   ");
  EXAMPLE_LOG("|  \\/  /  __ \\ | | | |               | |  ");
  EXAMPLE_LOG("| .  . | /  \\/ | | | |__   ___   ___ | |_ ");
  EXAMPLE_LOG("| |\\/| | |   | | | | '_ \\ / _ \\ / _ \\| __|");
  EXAMPLE_LOG("| |  | | \\__/\\ |_| | |_) | (_) | (_) | |_ ");
  EXAMPLE_LOG("\\_|  |_/\\____/\\___/|_.__/ \\___/ \\___/ \\__|");

  EXAMPLE_LOG("==Starting Bootloader==");

  struct boot_rsp rsp;
  int rv = boot_go(&rsp);

  if (rv == 0) {
    do_boot(&rsp);
  }

  EXAMPLE_LOG("No bootable image found. Falling into Bootloader CLI:");

  shell_processing_loop();

  __builtin_unreachable();
}
