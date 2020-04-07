#include "test_enum.h"

const char *test_enum_to_str_v1(eTestEnum e) {
  switch (e) {
    case kTestEnum_Val1:
      return "Val1";
    case kTestEnum_Val2:
      return "Val2";
    case kTestEnum_Val3:
      return "Val3";
    case kTestEnum_Val4:
    default:
      return "Val4";
  }
}
