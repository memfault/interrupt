#include "compiler_flag_examples.h"

//! 0000807c <simple_for_loop_with_byte>:
//!    807c:       2300            movs    r3, #0
//!    807e:       461a            mov     r2, r3
//!    8080:       b2d9            uxtb    r1, r3
//!    8082:       4288            cmp     r0, r1
//!    8084:       d801            bhi.n   808a <simple_for_loop_with_byte+0xe>
//! -- Extra instruction for masking so register holds a uint8_t
//!    8086:       b2d0            uxtb    r0, r2
//!    8088:       4770            bx      lr
//!    808a:       441a            add     r2, r3
//!    808c:       3301            adds    r3, #1
//!    808e:       e7f7            b.n     8080 <simple_for_loop_with_byte+0x4>
//! Total function size: 20 bytes (10 instructions)
uint8_t simple_for_loop_with_byte(uint8_t max_value) {
  int sum = 0;
  for (uint8_t i = 0; i < max_value; i++) {
    sum += i;
  }
  return sum;
}

//! 00008090 <simple_for_loop_with_word>:
//!    8090:       2300            movs    r3, #0
//!    8092:       461a            mov     r2, r3
//!    8094:       4298            cmp     r0, r3
//!    8096:       dc01            bgt.n   809c <simple_for_loop_with_word+0xc>
//!    8098:       4610            mov     r0, r2
//!    809a:       4770            bx      lr
//!    809c:       441a            add     r2, r3
//!    809e:       3301            adds    r3, #1
//!    80a0:       e7f8            b.n     8094 <simple_for_loop_with_word+0x4>
//! Total function size: 18 bytes (9 instructions)
int simple_for_loop_with_word(uint8_t max_value) {
  int sum = 0;
  for (int i = 0; i < max_value; i++) {
    sum += i;
  }
  return sum;
}
