#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

int main()
{
  int count = 0;
  while (true)
  {
    printf("1000000,2000000,3000000,4000000,5000000,6000000,7000000,%d\n", count);
    count++;
    const int rate_hz = 1000;
    usleep(1000000 / rate_hz);
  }
}