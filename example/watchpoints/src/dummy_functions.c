#include "hal/logging.h"

void dummy_function_1(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_2(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_3(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_4(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_5(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_6(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_7(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_8(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

void dummy_function_9(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}

__attribute__((section("ram_func")))
void dummy_function_ram(void) {
  EXAMPLE_LOG("stub function '%s' called", __func__);
}
