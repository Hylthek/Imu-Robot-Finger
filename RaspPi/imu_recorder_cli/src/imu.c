#include "imu.h"
#include <stdint.h>
#include "spi.h"
#include <stdio.h>
#include <unistd.h>

/*
7:5 GYRO_FS_SEL
Full scale select for gyroscope UI interface output
000: ±2000dps (default)
001: ±1000dps
010: ±500dps
011: ±250dps
100: ±125dps
101: ±62.5dps
110: ±31.25dps
111: ±15.625dps
*/

/*
GYRO_CONFIG0 3:0
Gyroscope ODR selection for UI interface output
0000: Reserved
0001: 32kHz
0010: 16kHz
0011: 8kHz
0100: 4kHz
0101: 2kHz
0110: 1kHz (default)
0111: 200Hz
1000: 100Hz
1001: 50Hz
1010: 25Hz
1011: 12.5Hz
1100: Reserved
1101: Reserved
1110: Reserved
1111: 500Hz
*/

/*
7:5 ACCEL_FS_SEL
Full scale select for accelerometer UI interface output
000: ±16g (default)
001: ±8g
010: ±4g
011: ±2g
100: Reserved
101: Reserved
110: Reserved
111: Reserved
*/

/*
ACCEL_CONFIG0 3:0
Accelerometer ODR selection for UI interface output
0000: Reserved
0001: 32kHz (LN mode)
0010: 16kHz (LN mode)
0011: 8kHz (LN mode)
0100: 4kHz (LN mode)
0101: 2kHz (LN mode)
0110: 1kHz (LN mode) (default)
0111: 200Hz (LP or LN mode)
1000: 100Hz (LP or LN mode)
1001: 50Hz (LP or LN mode)
1010: 25Hz (LP or LN mode)
1011: 12.5Hz (LP or LN mode)
1100: 6.25Hz (LP mode)
1101: 3.125Hz (LP mode)
1110: 1.5625Hz (LP mode)
1111: 500Hz (LP or LN mode)
*/

// https://download.mikroe.com/documents/datasheets/ICM-42688-P_Datasheet.pdf

// One-time writes to IMU config-type registers. NOT OPTIONAL.
void ImuInitRegisters(int file_desc)
{
  uint8_t spi_out[6], in_buf[6];

  // Perform software reset.
  spi_out[0] = kDeviceConfig;
  spi_out[1] = 0b00000001;
  spi_transfer(file_desc, spi_out, in_buf, 2);
  // Wait at least 1ms for reset to be effective.
  usleep(2000);

  // Initialize interrupts. Set pulse duration -> 8us and de-assertion duration to 0us.
  spi_out[0] = kIntConfig1;
  spi_out[1] = 0b01100000;
  spi_transfer(file_desc, spi_out, in_buf, 2);

  // Change interrupt output from "Reset done" to "UI data ready".
  spi_out[0] = kIntSource0;
  spi_out[1] = 0b00001000;
  spi_transfer(file_desc, spi_out, in_buf, 2);

  // Set dataReady interrupt to push-pull.
  spi_out[0] = kIntConfig;
  spi_out[1] = 0b00000010;
  spi_transfer(file_desc, spi_out, in_buf, 2);

  // Place gyro and accel in low noise mode.
  spi_out[0] = kPwrMgmt0;
  spi_out[1] = 0b00001111;
  spi_transfer(file_desc, spi_out, in_buf, 2);

  // RTC clock input is NOT required.
  spi_out[0] = kIntfConfig1;
  spi_out[1] = 0b10010001;
  spi_transfer(file_desc, spi_out, in_buf, 2);

  // Refer to reference comments above.
  const int kGyroAccelConfigCode = 0b00000110; // default FS, 1000hz
  spi_out[0] = kAccelConfig0;
  spi_out[1] = kGyroAccelConfigCode;
  spi_out[2] = kGyroConfig0;
  spi_out[3] = kGyroAccelConfigCode;
  spi_transfer(file_desc, spi_out, in_buf, 4);

  // Bank 1.
  spi_out[0] = kRegBankSel;
  spi_out[1] = 0b00000001; // Change from bank 0 to bank 1.
  spi_out[2] = kIntfConfig5;
  spi_out[3] = 0b00000000; // Sets pin 9 function to Default (INT2).
  spi_out[4] = kRegBankSel;
  spi_out[5] = 0b00000000; // Change from bank 1 to bank 0.
  spi_transfer(file_desc, spi_out, in_buf, 6);

  // Reading test.
  const int kWhoAmI = 0x75;
  spi_out[0] = 0x80 | kWhoAmI;
  spi_out[1] = 0x00;
  spi_transfer(file_desc, spi_out, in_buf, 2);
  if (in_buf[1] != 0x47)
    printf("Warning: WHO_AM_I register of IMU device did not return expected value of 0x47. Value: 0x%x\n", in_buf[1]);
}
