#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef MOCK_GPIO
#define gpioInitialise() mock_gpioInitialise()
#define gpioSetMode(pin, mode) mock_gpioSetMode(pin, mode)
#define gpioWrite(pin, level) mock_gpioWrite(pin, level)
#define gpioRead(pin) mock_gpioRead(pin)
#define gpioTerminate() mock_gpioTerminate()
#define gpioSetPullUpDown(PIN, PI_PUD_UP) mock_gpioSetPullUpDown(PIN, PI_PUD_UP)
#define gpioSetAlertFunc(PIN, GpioInterruptCallback) \
  mock_gpioSetAlertFunc(PIN, GpioInterruptCallback)
#define PI_PUD_UP 0
#define PI_INPUT 0
#else
#include <pigpio.h>
#endif

// Mock implementations
int mock_gpioInitialise();
int mock_gpioSetMode(unsigned pin, unsigned mode);
int mock_gpioWrite(unsigned pin, unsigned level);
int mock_gpioRead(unsigned pin);
void mock_gpioTerminate();
int mock_gpioSetPullUpDown(unsigned pin, unsigned pud);
int mock_gpioSetAlertFunc(unsigned pin, void (*callback)(int, int, uint32_t));

extern volatile bool gIntNegEdge;
void GpioInterruptCallback(int gpio, int level, uint32_t tick);
int InitGpio();
