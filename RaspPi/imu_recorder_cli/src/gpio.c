#include "gpio.h"

#include <stdbool.h>
#include <stdint.h>

int mock_gpio_states[32] = {0};  // Mock 32 GPIO pins

int mock_gpioInitialise() {
  printf("[MOCK] GPIO Initialized\n");
  return 0;
}

int mock_gpioSetMode(unsigned pin, unsigned mode) {
  printf("[MOCK] Set GPIO %d to mode %d\n", pin, mode);
  return 0;
}

int mock_gpioWrite(unsigned pin, unsigned level) {
  printf("[MOCK] Write GPIO %d = %d\n", pin, level);
  mock_gpio_states[pin] = level;
  return 0;
}

int mock_gpioRead(unsigned pin) {
  printf("[MOCK] Read GPIO %d = %d\n", pin, mock_gpio_states[pin]);
  return mock_gpio_states[pin];
}

void mock_gpioTerminate() { printf("[MOCK] GPIO Terminated\n"); }

int mock_gpioSetPullUpDown(unsigned pin, unsigned pud) {
  printf("[MOCK] Set GPIO %d pull-up/down to %d\n", pin, pud);
  return 0;
}

int mock_gpioSetAlertFunc(unsigned pin, void (*callback)(int, int, uint32_t)) {
  printf("[MOCK] Set alert function for GPIO %d\n", pin);
  // In a real mock, you might want to simulate an interrupt here.
  return 0;
}

// The threadsafe variable and its routine.
// Set by callback, reset by main.
volatile bool gIntNegEdge = false;
void GpioInterruptCallback(int gpio, int level, uint32_t tick) {
  if (gpio == 25 && level == false) gIntNegEdge = true;
}

int InitGpio() {
  if (gpioInitialise() < 0) {
    perror("pigpio failed");
    return 1;
  }
  const unsigned PIN = 25;
  gpioSetMode(PIN, PI_INPUT);
  gpioSetPullUpDown(PIN, PI_PUD_UP);
  gpioSetAlertFunc(PIN, GpioInterruptCallback);
  return 0;
}
