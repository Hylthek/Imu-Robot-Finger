#ifndef IMU_H
#define IMU_H

#include <stdint.h>

enum ImuRegs {
  // IMU registers
  // Note that some are defined only for reference as bulk reads exists.
  kPwrMgmt0 = 0x4E,
  kAccelDataX1 = 0x1F,
  kAccelDataX0 = 0x20,
  kAccelDataY1 = 0x21,
  kAccelDataY0 = 0x22,
  kAccelDataZ1 = 0x23,
  kAccelDataZ0 = 0x24,
  kGyroDataX1 = 0x25,
  kGyroDataX0 = 0x26,
  kGyroDataY1 = 0x27,
  kGyroDataY0 = 0x28,
  kGyroDataZ1 = 0x29,
  kGyroDataZ0 = 0x2A,
  kIntConfig1 = 0x64,
  kIntSource0 = 0x65,
  kIntConfig = 0x14,
  kIntfConfig1 = 0x4d,
  kRegBankSel = 0x76,
  kIntfConfig5 = 0x7b,  // Note, this is in bank 1 , not bank 0.
  kAccelConfig0 = 0x50,
  kGyroConfig0 = 0x4f,
};

typedef struct {
  double t;
  int16_t ax;
  int16_t ay;
  int16_t az;
  int16_t gx;
  int16_t gy;
  int16_t gz;
} ImuSample_t;

void ImuInitRegisters(int file_desc);

#endif  // IMU_H