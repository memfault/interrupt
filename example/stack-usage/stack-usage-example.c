#include <stdio.h>

int foo_2(int c) {
  int array[4];
  array[1] = c;
  array[3] = c* c;
  return array[3] - array[1];
}

int foo(int a, int b) {
  int array[10];
  array[1] = a;
  array[9] = foo_2(b);

  return array[1] * array[9];
}

int main() {
  printf("%d\n", foo(1,2));
}
