#include <errno.h>
#include <poll.h>
#include <stdbool.h>
#include <unistd.h>

// Check if there is stuff in stdin.
bool stdin_has_data_poll(void) {
  struct pollfd pfd;
  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;
  pfd.revents = 0;

  int rv;
  do {
    rv = poll(&pfd, 1, 0); /* timeout 0 => immediate return */
  } while (rv == -1 && errno == EINTR);

  if (rv > 0) {
    return (pfd.revents & POLLIN) != 0;
  }
  return false;
}