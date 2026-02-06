// SPDX-License-Identifier: GPL-2.0-or-later
// SPDX-FileCopyrightText: 2023 Kent Gibson <warthog618@gmail.com>

/* Minimal example of watching for rising edges on a single line. */

#include "libgpiod_interrupt.h"

#include <errno.h>
#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Request a line as input with edge detection. */
static struct gpiod_line_request* request_input_line(const char* chip_path,
                                                     unsigned int offset,
                                                     const char* consumer) {
  struct gpiod_request_config* req_cfg = NULL;
  struct gpiod_line_request* request = NULL;
  struct gpiod_line_settings* settings;
  struct gpiod_line_config* line_cfg;
  struct gpiod_chip* chip;
  int ret;

  chip = gpiod_chip_open(chip_path);
  if (!chip)
    return NULL;

  settings = gpiod_line_settings_new();
  if (!settings)
    goto close_chip;

  gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
  gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_RISING);

  line_cfg = gpiod_line_config_new();
  if (!line_cfg)
    goto free_settings;

  ret = gpiod_line_config_add_line_settings(line_cfg, &offset, 1,
                                            settings);
  if (ret)
    goto free_line_config;

  if (consumer) {
    req_cfg = gpiod_request_config_new();
    if (!req_cfg)
      goto free_line_config;

    gpiod_request_config_set_consumer(req_cfg, consumer);
  }

  request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

  gpiod_request_config_free(req_cfg);

free_line_config:
  gpiod_line_config_free(line_cfg);

free_settings:
  gpiod_line_settings_free(settings);

close_chip:
  gpiod_chip_close(chip);

  return request;
}

struct gpiod_line_request* request;
struct gpiod_edge_event_buffer* event_buffer;
const int event_buf_size = 1;
int GpioSetup(const unsigned int line_offset) {
  /* Example configuration - customize to suit your situation. */
  static const char* const chip_path = "/dev/gpiochip0";

  request = request_input_line(chip_path, line_offset, "watch-line-value");
  if (!request) {
    fprintf(stderr, "failed to request line: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }

  /*
   * A larger buffer is an optimisation for reading bursts of events from
   * the kernel, but that is not necessary in this case, so 1 is fine.
   */
  event_buffer = gpiod_edge_event_buffer_new(event_buf_size);
  if (!event_buffer) {
    fprintf(stderr, "failed to create event buffer: %s\n",
            strerror(errno));
    return EXIT_FAILURE;
  }

  return 0;
}

bool GpioGetEvent() {
  const int64_t timeout = 0;
  int ret = gpiod_line_request_wait_edge_events(request, timeout);
  // int ret = gpiod_line_request_read_edge_events(request, event_buffer, event_buf_size);
  if (ret == -1) {
    fprintf(stderr, "error reading edge events: %s\n", strerror(errno));
    return EXIT_FAILURE;
  }
  // If no events i.e. timed out.
  if (ret == 0)
    return false;
  // If event detected.
  if (ret == 1) {
    struct gpiod_edge_event* event;
    // Pop event.
    const unsigned long idx = 0;
    event = gpiod_edge_event_buffer_get_event(event_buffer, idx);
    return true;
  }
}