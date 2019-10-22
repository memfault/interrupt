#include "compiler_flag_examples.h"

int g_variable;

void tentative_global_init(int initial_value) {
  g_variable = initial_value;
}

int tentative_global_increment(void) {
  g_variable++;
}
