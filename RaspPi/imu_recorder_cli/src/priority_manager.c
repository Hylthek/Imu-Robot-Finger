#include <sched.h>
#include <stdio.h>

int SetMaxPriority() {
  struct sched_param param;
  param.sched_priority = 99;  // Maximum priority
  if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
    perror("Failed to set real-time scheduler");
    return 1;
  }
  return 0;
}
