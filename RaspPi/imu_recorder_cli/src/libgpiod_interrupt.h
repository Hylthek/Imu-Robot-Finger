#include <gpiod.h>

// Setup.
int GpioSetup(const unsigned int line_offset);

// Check if an event occurred.
bool GpioGetEvent();