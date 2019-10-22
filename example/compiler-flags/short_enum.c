#include "compiler_flag_examples.h"

void simple_enum_lookup_value(int value, eShortEnum *result) {
  if (value < kShortEnum_InternalError) {
    *result = kShortEnum_Ok;
  } else if (value < kShortEnum_ExternalError) {
    *result = kShortEnum_InternalError;
  } else {
    *result = kShortEnum_ExternalError;
  }
}
