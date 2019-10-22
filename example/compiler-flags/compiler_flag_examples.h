#pragma once

//! @file
//!
//! Header for functions shared across the *.c files that make up the examples

#include <stdbool.h>
#include <stdint.h>

bool float_promotion_example(float val);

typedef enum {
  kShortEnum_Ok = 0,
  kShortEnum_InternalError = 0xdead,
  kShortEnum_ExternalError = 0xfeee,
} eShortEnum;

void simple_enum_lookup_value(int value, eShortEnum *result);

//! A bad docstring that clang can identify issues with using -Wdocumentation
//!
//! @param a The first value of the summation
//! @param c The second value of the summation
//!
//! @return the sum of a & b
int simple_math_get_sum(int a, int b);

int simple_math_get_delta(int a, int b);


uint8_t simple_for_loop_with_byte(uint8_t max_value);
int simple_for_loop_with_word(uint8_t max_value);


void tentative_global_init(int initial_value);
int tentative_global_increment(void);
