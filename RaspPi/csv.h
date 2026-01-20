#ifndef CSV_H
#define CSV_H

#include <signal.h>
#include <stdio.h>

extern FILE* gImuDataCsv;

void SigIntRoutine(int signal);
void SafeExit();
void HandleSigInt();

#endif  // CSV_H