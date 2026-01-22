#include "csv.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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
  // Get formatted date and time.
  time_t now = time(NULL);
  char date_str[20];
  strftime(date_str, sizeof(date_str), "%Y-%m-%d_%H-%M-%S", localtime(&now));

  // Check for proper recording directory.
  // This is a possible termination point.
  const char recording_dir_name[50] = "imu_recordings_dir";
  if (access(recording_dir_name, F_OK) == -1) {
    printf("ERROR: There is no dir called \"%s/\"\n", recording_dir_name);
    exit(1);
  }

  // Concatenate to final file name.
  char file_name[200];
  snprintf(file_name, sizeof(file_name), "%s/recording_%s.csv", recording_dir_name, date_str);

  // Open file and return fd.
  return fopen(file_name, "w");
}