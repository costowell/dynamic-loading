#include "ipc.h"
#include "modules.h"
#include "modules/module.h"
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define COLOR_COUNT 200
#define SEC_TO_MICROSEC 1000000.0f
#define MICROSECONDS_PER_FRAME (1.0f / 60.0f) * SEC_TO_MICROSEC

module_data_t *init_module_data() {
  module_data_t *data = calloc(1, sizeof(module_data_t));
  data->colors = calloc(COLOR_COUNT, sizeof(color_t));
  data->quantity = COLOR_COUNT;

  return data;
}

void cursor_visible(bool visible) {
  if (visible) {
    printf("\033[?25h");
  } else {
    printf("\033[?25l");
  }
}

void draw_loop(module_data_t *data, int (*module_update)()) {
  struct timeval frame_start, frame_end;
  double elapsed_us, sleep_us = 0;
  cursor_visible(false);
  while (true) {
    gettimeofday(&frame_start, NULL);
    if (module_update())
      return;
    for (int i = 0; i < data->quantity; ++i) {
      color_t *color = &data->colors[i];
      printf("\033[48;2;%d;%d;%dm ", color->r, color->g, color->b);
    }
    printf("\r");
    usleep(sleep_us);
    gettimeofday(&frame_end, NULL);

    elapsed_us = (frame_end.tv_sec - frame_start.tv_sec) * SEC_TO_MICROSEC;
    elapsed_us += (frame_end.tv_usec - frame_start.tv_usec);
    sleep_us += MICROSECONDS_PER_FRAME - elapsed_us;
  }
}

void handle_sigint(int sig) {
  cursor_visible(true);
  exit(EXIT_FAILURE);
}

void setup_signal_handlers() { signal(SIGINT, handle_sigint); }

int main() {
  setup_signal_handlers();
  module_t **modules = list_modules();
  if (!modules) {
    fprintf(stderr, "no modules found\n");
    return EXIT_FAILURE;
  }

  if (ipc_init()) {
    fprintf(stderr, "failed to init ipc\n");
    return EXIT_FAILURE;
  }

  while (true) {
    command_t *command = ipc_listen();
    if (command) {
      printf("%d %d\n", command->type, command->argc);
    } else {
      printf(":(\n");
    }
  }

  int module_count = 0;
  while (modules[module_count]) {
    printf("%d: %s\n", module_count, modules[module_count]->path);
    module_count++;
  }

  int sel = -1;
  while (sel < 0 || sel >= module_count) {
    printf("Select a number: ");
    scanf("%d", &sel);
  }

  module_t *module = modules[sel];
  module_data_t *data = init_module_data();
  if (load_module(module)) {
    fprintf(stderr, "failed to load module: %s\n", modules[0]->path);
    return EXIT_FAILURE;
  }

  // Init and run
  module->init(data);
  draw_loop(data, module->update);

  return EXIT_SUCCESS;
}
