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

static size_t module_count;
static module_t *modules;
static module_t *selected_module = NULL;

int cleanup_selected_module() {
  if (selected_module && unload_module(selected_module)) {
    fprintf(stderr, "failed to unload module: %s\n", selected_module->path);
    return 1;
  }
  selected_module = NULL;
  return 0;
}
void cleanup() {
  cursor_visible(true);
  draw_thread_stop();
  if (ipc_terminate())
    fprintf(stderr, "failed to safely shutdown IPC... exiting anyways\n");
  if (draw_thread_stop())
    fprintf(stderr,
            "failed to safely shutdown draw thread... exiting anyways\n");
  if (cleanup_selected_module())
    fprintf(stderr,
            "failed to safely unload current module... exiting anyways\n");
  for (size_t i = 0; i < module_count; ++i)
    free(modules[i].path);
  free(modules);
}

void handle_sigint() {
  cleanup();
  exit(EXIT_FAILURE);
}

void setup_signal_handlers() { signal(SIGINT, handle_sigint); }

int select_module(module_t *module) {
  draw_thread_stop();
  if (cleanup_selected_module())
    return 2;
  if (load_module(module)) {
    fprintf(stderr, "failed to load module: %s\n", module->path);
    return 1;
  }
  selected_module = module;
  return draw_thread_start(module);
}

char *handle_ipc_command(int argc, char **argv) {
  if (argc == 0)
    return "empty command\n";

  if (!strcmp(argv[0], "select") && argc == 2) {
    unsigned long mode = strtoul(argv[1], NULL, 10);
    if (mode == 0 || mode > module_count) {
      return "invalid mode number\n";
    }
    select_module(&modules[mode - 1]);
  } else if (!strcmp(argv[0], "stop")) {
    draw_thread_stop();
    cleanup_selected_module();
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
  bool exit = false;
  while (!exit && !ipc_listen(&conn)) {
    while (!exit && !ipc_conn_recv_cmd(&conn, &cmd)) {
      exit = cmd.argc == 0;
      char *msg = handle_ipc_command(cmd.argc, cmd.argv);
      ipc_conn_send(&conn, msg);
      for (int i = 0; i < cmd.argc; ++i)
        free(cmd.argv[i]);
      free(cmd.argv);
    }
    ipc_conn_close(&conn);
  }

  cleanup();
  return EXIT_SUCCESS;
}
