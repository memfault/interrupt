// test/test_add.c

#include <add.h>
#include <unity.h>


void test_add(void) {
    TEST_ASSERT_EQUAL(32, add(25, 7));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_add);
    UNITY_END();

    return 0;
}

