#include "../../module.h"
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

int run(module_data_t *data) {
  while (true) {
    for (size_t i = 0; i < data->bufsize; ++i) {
      usleep(10000);
      pthread_mutex_lock(&data->mutex);
      data->buf[i] = '0';
      pthread_mutex_unlock(&data->mutex);
    }
    for (size_t i = data->bufsize - 1; i > 0; --i) {
      usleep(10000);
      pthread_mutex_lock(&data->mutex);
      data->buf[i] = '1';
      pthread_mutex_unlock(&data->mutex);
    }
  }
  return 0;
}
