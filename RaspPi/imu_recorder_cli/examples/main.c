#include "libgpiod_example_read.h"
#include <stdio.h>

int main() {
  setup(25);

  printf("PIN %d: %d\n", 25, read_pin());

  return 0;
}