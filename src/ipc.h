#ifndef IPC_H
#define IPC_H

#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

#define IPC_PATH "/tmp/dyno_sock"
#define BUFFER_SIZE 1024
#define MAX_ARGUMENTS 16

typedef struct _ipc_command {
  int argc;
  char **argv;
} ipc_command_t;

typedef struct _ipc_conn_t {
  int socket;
} ipc_conn_t;

int ipc_init();
int ipc_listen(ipc_conn_t *);
int ipc_conn_recv_cmd(ipc_conn_t *, ipc_command_t *);
int ipc_conn_send(ipc_conn_t *, char *);
int ipc_conn_close(ipc_conn_t *);

#endif
