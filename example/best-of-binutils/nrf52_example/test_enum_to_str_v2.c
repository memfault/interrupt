#include "test_enum.h"

const char *test_enum_to_str_v2(eTestEnum e) {
  if (e == kTestEnum_Val1) {
    return "Val1";
  } else if (e == kTestEnum_Val2) {
    return "Val2";
  } else if (e == kTestEnum_Val3) {
    return "Val3";
  } else if (e == kTestEnum_Val4) {
    return "Val4";
  }
  return "Val4";
}
