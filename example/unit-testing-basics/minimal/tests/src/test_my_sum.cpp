#include "CppUTest/TestHarness.h"

extern "C" {
  #include "my_sum.h"
}

TEST_GROUP(TestMySum) {
  void setup() {
  }

  void teardown() {
  }
};

TEST(TestMySum, Test_MySumBasic) {
  int rv = my_sum(3, 4);
  LONGS_EQUAL(7, rv);
}
