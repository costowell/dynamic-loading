#ifndef IPC_H
#define IPC_H

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

#define IPC_PATH "/tmp/dyno_sock"
#define BUFFER_SIZE 1024
#define MAX_ARGUMENTS 16

typedef enum _command_type { COMMAND_SELECT } command_type_t;
typedef struct _command {
  command_type_t type;
  int argc;
  char **argv;
} command_t;

int ipc_init();
command_t *ipc_listen();

#endif
