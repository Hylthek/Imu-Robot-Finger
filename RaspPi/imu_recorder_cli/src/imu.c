#include "imu.h"

#include <stdint.h>

#include "spi.h"

// One-time writes to IMU config-type registers. NOT OPTIONAL.
void ImuInitRegisters(int file_desc) {
  // Do some one-time SPI writes.
  uint8_t spi_out[6], in_buf[6];

  // Bank 0.
  spi_out[0] = kIntConfig1;
  spi_out[1] = 0b01000000;  // Initialize interrupts. Also set interrupt pulse
                            // to 100us->8us
  spi_transfer(file_desc, spi_out, in_buf, 2);
  spi_out[0] = kIntSource0;
  spi_out[1] = 0b00001000;  // Change interrupt output from "Reset done" to "UI
                            // data ready".
  spi_transfer(file_desc, spi_out, in_buf, 2);
  spi_out[0] = kIntConfig;
  spi_out[1] = 0b00000010;  // Set dataReady interrupt to push-pull.
  spi_transfer(file_desc, spi_out, in_buf, 2);
  spi_out[0] = kPwrMgmt0;
  spi_out[1] = 0b00001111;  // Place gyro and accel in low noise mode.
  spi_transfer(file_desc, spi_out, in_buf, 2);
  spi_out[0] = kIntfConfig1;
  spi_out[1] = 0b10010001;  // RTC clock input is NOT required.
  spi_transfer(file_desc, spi_out, in_buf, 2);
  spi_out[0] = kAccelConfig0;
  spi_out[1] = 0b00000110;  // Keep FS at +-16g, increase IMU freq to 1kHz.
  spi_transfer(file_desc, spi_out, in_buf, 2);
  spi_out[0] = kGyroConfig0;
  spi_out[1] = 0b01000110;  // Change FS from +-2000dps to +-500dps.
  spi_transfer(file_desc, spi_out, in_buf, 2);

  // Bank 1.
  spi_out[0] = kRegBankSel;
  spi_out[1] = 0b00000001;  // Change from bank 0 to bank 1.
  spi_out[2] = kIntfConfig5;
  spi_out[3] = 0b00000000;  // Sets pin 9 function to Default (INT2).
  spi_out[4] = kRegBankSel;
  spi_out[5] = 0b00000000;  // Change from bank 1 to bank 0.
  spi_transfer(file_desc, spi_out, in_buf, 6);
}
