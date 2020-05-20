#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char **argv) {
  char *buf = malloc(100);
  strcpy(buf, "Hello World");
  printf("%s\n", buf);
  return 0;
}
