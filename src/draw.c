#include "draw.h"
#include "modules/module.h"
#include "util.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define SEC_TO_NANOSEC 1000000000
#define USEC_TO_NANOSEC 1000
#define FRAMES 60
#define NANOSECS_PER_FRAME ((SEC_TO_NANOSEC) / FRAMES)
#define COLOR_COUNT 200

static atomic_bool draw_stop;
static pthread_t draw_thread;
static module_data_t *data;

void *draw(module_t *module) {
  struct timeval frame_start, frame_end;
  struct timespec sleep_nanos = {.tv_sec = 0, .tv_nsec = 0};
  cursor_visible(false);
  while (!draw_stop) {
    gettimeofday(&frame_start, NULL);
    if (module->update()) {
      free(data);
      return NULL;
    }
    for (int i = 0; i < data->quantity; ++i) {
      color_t *color = &data->colors[i];
      printf("\033[48;2;%d;%d;%dm ", color->r, color->g, color->b);
    }
    printf("\r");
    gettimeofday(&frame_end, NULL);
    nanosleep(&sleep_nanos, NULL);

    sleep_nanos.tv_sec = frame_end.tv_sec - frame_start.tv_sec;
    sleep_nanos.tv_nsec =
        NANOSECS_PER_FRAME -
        (frame_end.tv_usec - frame_start.tv_usec) * USEC_TO_NANOSEC;
  }
  return NULL;
}

int draw_thread_start(module_t *module) {
  data = calloc(1, sizeof(module_data_t));
  data->colors = calloc(COLOR_COUNT, sizeof(color_t));
  data->quantity = COLOR_COUNT;
  draw_stop = false;

  module->init(data);
  return pthread_create(&draw_thread, NULL, (void *(*)(void *))draw, module);
}
int draw_thread_stop() {
  int ret;
  draw_stop = true;
  ret = pthread_join(draw_thread, NULL);
  if (data) {
    free(data);
    data = NULL;
  }

  if (ret) {
    fprintf(stderr, "failed to join thread, error %d\n", ret);
    free(data);
    return 1;
  }
  return 0;
}
