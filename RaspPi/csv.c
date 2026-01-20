#include "csv.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

FILE* gImuDataCsv = NULL;

// Perform a safe exit that flushes the csv file.
void SafeExit() {
  if (gImuDataCsv != NULL) {
    fclose(gImuDataCsv);  // Close the file
    printf("File closed.\n");
  }
  printf("Exiting Program.\n");
  fflush(stdout);
  exit(0);
}

// Routine for POSIX signals.
void SigIntRoutine(int signal) {
  (void)signal;
  SafeExit();
}

void HandleSigInt() {
  struct sigaction sig_action;
  sig_action.sa_handler = SigIntRoutine;
  sig_action.sa_flags = 0;
  if (sigaction(SIGINT, &sig_action, NULL) == -1)
    perror("Failed to set SIGINT handler");
}
