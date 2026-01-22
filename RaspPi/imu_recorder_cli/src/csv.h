#pragma once

#include <signal.h>
#include <stdio.h>

extern FILE* gImuCsvFd;

void SigIntRoutine(int signal);
void SafeExit();
void HandleSigInt();
FILE* OpenCsv();
