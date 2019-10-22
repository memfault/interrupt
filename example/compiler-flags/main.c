//! @file

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <stdio.h>
#include <string.h>

#include "compiler_flag_examples.h"

#define MEMFAULT_EXAMPLE_MACRO 0x4d

#if ACCEL_ENABLED
//
// accelerometer configuration code
//
#endif /* ACCEL_ENABLED */

#if defined(ACCEL_ENABLED) && ACCEL_ENABLE
// typo "ACCEL_ENABLE" (instead of ACCEL_ENABLED) will go unnoticed
#endif

typedef struct {
  uint8_t a;
  uint16_t b;
  uint64_t c;
  uint32_t d;
} my_struct;

_Static_assert(offsetof(my_struct, a) == 0, "a not at offset 0 within struct");
_Static_assert(offsetof(my_struct, b) == 2, "b not at offset 2 within struct");
_Static_assert(offsetof(my_struct, c) == 8, "c not at offset 8 within struct");
_Static_assert(offsetof(my_struct, d) == 16, "d not at offset 16 within struct");

typedef struct __attribute__((packed)) {
  uint8_t a;
  uint16_t b;
  uint64_t c;
  uint32_t d;
} my_packed_struct;

_Static_assert(offsetof(my_packed_struct, a) == 0, "a not at offset 0 within struct");
_Static_assert(offsetof(my_packed_struct, b) == 1, "b not at offset 2 within struct");
_Static_assert(offsetof(my_packed_struct, c) == 3, "c not at offset 8 within struct");
_Static_assert(offsetof(my_packed_struct, d) == 11, "d not at offset 16 within struct");

static void prv_various_memset_bugs(void) {
  #define NUM_ITEMS 10
  uint32_t buf[NUM_ITEMS];
  memset(buf, NUM_ITEMS, 0x0);
  memset(buf, 0x0, NUM_ITEMS);
}

int variable_shadow_error_example2(void) {
  int result = 4;

  for (int i = 0; i < 10; i++) {
    int result = i;
    // do something with rv
  }

  return result;
}

#define MEMFAULT_UNUSED(_x) (void)(_x)

typedef void (*FlashReadDoneCallback)(void *ctx);

static void prv_spi_flash_read_cb(void *ctx) {
  // nothing to do with ctx
}

int flash_read(FlashReadDoneCallback cb, void *ctx) {
  // perform flash read
  // [...]
  // invoke callback
  cb(ctx);
}

void print_user_provided_buffer(char *buf) {
  printf(buf);
}

void snprintf_truncation_example(int val) {
  char buf[4];
  snprintf(buf, sizeof(buf), "%d", val);
}

uint8_t conversion_error_example(uint32_t val1, uint8_t val2) {
  uint8_t step1 = val1 * val2;
  // some more operations
  int final_step = step1 / val2;
  return final_step;
}

// This is the variable that will wind up getting used in
// tentative_global.c!
int g_variable = 4;

void _start(void) { // fake-entry-point
  my_struct a[2] = {  };
  eShortEnum value;
  simple_enum_lookup_value(a[0].a, &value);

  flash_read(prv_spi_flash_read_cb, NULL);

  simple_for_loop_with_byte(value);
  simple_for_loop_with_word(value);


  float_promotion_example(7.2f);
  simple_math_get_sum(value, value + 1);

  tentative_global_init(MEMFAULT_EXAMPLE_MACRO);
  tentative_global_increment();

  prv_various_memset_bugs();
}
