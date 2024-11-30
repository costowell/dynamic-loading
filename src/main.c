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

static int module_count;
static module_t **modules;

void handle_sigint() {
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

char *handle_ipc_command(int argc, char **argv) {
  if (argc == 0)
    return "empty command\n";

  if (!strcmp(argv[0], "select") && argc == 2) {
    long mode = strtol(argv[1], NULL, 10);
    if (mode <= 0 || mode > module_count) {
      return "invalid mode number\n";
    }
    select_module(modules[mode - 1]);
  } else if (!strcmp(argv[0], "stop")) {
    draw_thread_stop();
  } else {
    return "invalid command\n";
  }
  return "success\n";
}

int main() {
  setup_signal_handlers();

  modules = list_modules(&module_count);
  if (!modules) {
    fprintf(stderr, "no modules found\n");
    return EXIT_FAILURE;
  }

  if (ipc_init()) {
    fprintf(stderr, "failed to init ipc\n");
    return EXIT_FAILURE;
  }
  ipc_conn_t conn;
  ipc_command_t cmd;
  while (!ipc_listen(&conn)) {
    while (!ipc_conn_recv_cmd(&conn, &cmd)) {
      char *msg = handle_ipc_command(cmd.argc, cmd.argv);
      ipc_conn_send(&conn, msg);
    }
  }
  return EXIT_SUCCESS;
}
