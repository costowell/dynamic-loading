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

static module_array_t *modules;
static module_t *selected_module = NULL;
static pthread_mutex_t selected_module_mutex;

int deselect_module() {
  if (!selected_module)
    return 0;
  if (draw_thread_stop())
    return 1;
  if (unload_module(selected_module))
    return 2;
  selected_module = NULL;
  return 0;
}

void cleanup() {
  int ret = 0;
  cursor_visible(true);
  if (ipc_terminate())
    fprintf(stderr, "failed to safely shutdown IPC... exiting anyways\n");
  pthread_mutex_lock(&selected_module_mutex);
  if ((ret = deselect_module()))
    fprintf(stderr, "failed to deselect module (%d)... exiting anyways\n", ret);
  pthread_mutex_unlock(&selected_module_mutex);
  free_module_array(modules);
  free(modules);
}

void handle_sigint() {
  cleanup();
  exit(EXIT_FAILURE);
}

void setup_signal_handlers() { signal(SIGINT, handle_sigint); }

int select_module(module_t *module) {
  deselect_module();
  if (load_module(module)) {
    fprintf(stderr, "failed to load module: %s\n", module->path);
    return 1;
  }
  selected_module = module;
  draw_thread_start(module);
  return 0;
}

char *handle_ipc_command(ipc_command_t *cmd) {
  int argc = cmd->argc;
  char **argv = cmd->argv;
  if (argc == 0)
    return "empty command\n";

  if (!strcmp(argv[0], "select") && argc == 2) {
    unsigned long mode = strtoul(argv[1], NULL, 10);
    if (mode == 0 || mode > modules->count) {
      return "invalid mode number\n";
    }
    pthread_mutex_lock(&selected_module_mutex);
    select_module(&modules->modules[mode - 1]);
    pthread_mutex_unlock(&selected_module_mutex);
  } else if (!strcmp(argv[0], "stop")) {
    pthread_mutex_lock(&selected_module_mutex);
    deselect_module();
    pthread_mutex_unlock(&selected_module_mutex);
  } else {
    return "invalid command\n";
  }
  return "success\n";
}

void handle_ipc_conn(ipc_conn_t *conn) {
  printf("Connection recived!\n");
  ipc_command_t cmd;
  while (!ipc_conn_recv_cmd(conn, &cmd)) {
    char *msg = handle_ipc_command(&cmd);
    ipc_conn_send(conn, msg);
    free_command(&cmd);
  }
  ipc_conn_close(conn);
  free(conn);
}

void *handle_ipc_conn_wrapper(void *data) {
  handle_ipc_conn((ipc_conn_t *)data);
  return NULL;
}

int main() {
  setup_signal_handlers();

  pthread_mutex_init(&selected_module_mutex, NULL);

  modules = list_modules();
  if (modules->count == 0) {
    fprintf(stderr, "no modules found\n");
    return EXIT_FAILURE;
  }

  if (ipc_init()) {
    fprintf(stderr, "failed to init ipc\n");
    return EXIT_FAILURE;
  }

  ipc_conn_t conn, *thread_conn;
  while (!ipc_listen(&conn)) {
    pthread_t handle_thread;
    thread_conn = calloc(1, sizeof(ipc_conn_t));
    *thread_conn = conn;
    pthread_create(&handle_thread, NULL, handle_ipc_conn_wrapper, thread_conn);
    pthread_detach(handle_thread);
  }

  cleanup();
  return EXIT_SUCCESS;
}
