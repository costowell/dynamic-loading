#include "ipc.h"
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

static int sockfd = -1;

int ipc_init() {
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1) {
    perror("socket");
    return 1;
  }

  struct sockaddr_un address;
  address.sun_family = AF_UNIX,
  strncpy(address.sun_path, IPC_PATH, sizeof(address.sun_path) / sizeof(char));

  unlink(address.sun_path);

  if (bind(sockfd, (struct sockaddr *)(&address), sizeof(address)) == -1) {
    perror("bind");
    return 1;
  }
  if (listen(sockfd, 20) == -1) {
    perror("listen");
    return 1;
  }

  return 0;
}

int parse_command(char *buf, ipc_command_t *cmd) {
  char *token, **argv, *delims = " \n";
  int argc = 0;

  argv = calloc(MAX_ARGUMENTS, sizeof(char *));

  token = strtok(buf, delims);
  while (token) {
    if (argc == MAX_ARGUMENTS) {
      free(argv);
      return 1;
    }
    argv[argc++] = token;
    token = strtok(NULL, delims);
  }

  cmd->argc = argc;
  cmd->argv = argv;
  return 0;
}

char *errtostr(int num) {
  switch (num) {
  case 1:
    return "maximum number of arguments reached";
  default:
    return "unknown error";
  }
}

int ipc_listen(ipc_conn_t *conn) {
  int socket;

  if (sockfd == -1) {
    fprintf(stderr, "error: IPC not initialized\n");
    return 1;
  }

  socket = accept(sockfd, NULL, NULL);
  if (socket == -1) {
    perror("accept");
    return 2;
  }

  conn->socket = socket;
  return 0;
}

int ipc_conn_recv_cmd(ipc_conn_t *conn, ipc_command_t *command) {
  char *buf;
  int ret;
  ssize_t b;

  buf = calloc(BUFFER_SIZE, sizeof(char));
  b = recv(conn->socket, buf, BUFFER_SIZE * sizeof(char), 0);
  if (b == -1 || b == 0) {
    if (b == -1) {
      perror("recv");
    }
    free(buf);
    close(conn->socket);
    return 1;
  }

  if ((ret = parse_command(buf, command))) {
    char *errstr = errtostr(ret);
    char msg[256];
    snprintf(msg, 256, "error: %s\n", errstr);

    if ((send(conn->socket, msg, strlen(msg), 0)) == -1) {
      perror("send");
      free(buf);
      close(conn->socket);
      return 3;
    }
    return 2;
  }
  return 0;
}

int ipc_conn_send(ipc_conn_t *conn, char *msg) {
  if ((send(conn->socket, msg, strlen(msg), 0)) == -1) {
    perror("send");
    close(conn->socket);
    return 1;
  }
  return 0;
}

int ipc_conn_close(ipc_conn_t *conn) {
  if (close(conn->socket) == -1) {
    perror("close");
    return 1;
  }
  return 0;
}
