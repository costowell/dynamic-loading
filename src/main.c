#include "draw.h"
#include "ipc.h"
#include "modules.h"
#include "util.h"
#include <signal.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handle_sigint(int sig) {
  cursor_visible(true);
  exit(EXIT_FAILURE);
}

void setup_signal_handlers() { signal(SIGINT, handle_sigint); }

int select_module(module_t *module) {
  if (load_module(module)) {
    fprintf(stderr, "failed to load module: %s\n", module->path);
    return 1;
  }
  return draw_thread_start(module);
}

int main() {
  setup_signal_handlers();
  int module_count;
  module_t **modules = list_modules(&module_count);
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
    if (command)
      continue;

    if (command->type == COMMAND_SELECT && command->argc == 1) {
      int mode = strtol(command->argv[0], NULL, 10);
      if (mode <= 0 || mode > module_count) {
        fprintf(stderr, "invalid mode number\n");
        continue;
      }
      select_module(modules[mode - 1]);
    } else {
      fprintf(stderr, "invalid command entered\n");
    }
  }

  return EXIT_SUCCESS;
}
