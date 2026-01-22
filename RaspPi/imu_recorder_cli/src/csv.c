#include "csv.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

FILE* gImuCsvFd = NULL;

// Perform a safe exit that flushes the csv file.
void SafeExit() {
  if (gImuCsvFd != NULL) {
    fclose(gImuCsvFd);  // Close the file
    printf("\nFile closed.\n");
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

FILE* OpenCsv() {
  time_t now = time(NULL);
  char date[20];
  strftime(date, sizeof(date), "%Y-%m-%d_%H-%M-%S", localtime(&now));
  char kTempFileName[50];
  snprintf(kTempFileName, sizeof(kTempFileName), "data/data_%s.csv", date);
  return fopen(kTempFileName, "w");
}