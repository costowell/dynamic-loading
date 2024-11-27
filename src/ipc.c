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

command_type_t get_command_type(char *cmd) {
  if (strcasecmp(cmd, "select") == 0)
    return COMMAND_SELECT;
  else
    return -1;
}

int parse_command(char *buf, command_t **command) {
  char *token, *cmd, **argv, *delims = " \n";
  int argc = 0;
  command_type_t type;

  token = strtok(buf, delims);
  cmd = token;
  if ((type = get_command_type(cmd)) == -1)
    return 1;

  argv = calloc(MAX_ARGUMENTS, sizeof(char *));

  while (token != NULL) {
    if (argc == MAX_ARGUMENTS) {
      free(argv);
      return 2;
    }
    argv[argc++] = token;
    token = strtok(NULL, delims);
  }

  command_t *c = calloc(1, sizeof(command_t));
  c->argc = argc - 1;
  c->argv = argv;
  c->type = type;
  *command = c;
  return 0;
}

char *errtostr(int num) {
  switch (num) {
  case 0:
    return "no error";
  case 1:
    return "invalid command";
  case 2:
    return "maximum number of arguments reached";
  default:
    return "unknown error";
  }
}

command_t *ipc_listen() {
  int socket, ret;
  char *buf = " \n";

  if (sockfd == -1) {
    fprintf(stderr, "error: IPC not initialized\n");
    return NULL;
  }
  socket = accept(sockfd, NULL, NULL);
  if (socket == -1) {
    perror("accept");
    return NULL;
  }

  buf = calloc(BUFFER_SIZE, sizeof(char));
  if ((recv(socket, buf, BUFFER_SIZE * sizeof(char), 0)) == -1) {
    perror("recv");
    free(buf);
    close(socket);
    return NULL;
  }

  command_t *command = NULL;
  if ((ret = parse_command(buf, &command))) {
    char *errstr = errtostr(ret);
    char msg[256];
    snprintf(msg, 256, "error: %s\n", errstr);

    if ((send(socket, msg, strlen(msg), 0)) == -1) {
      perror("send");
      free(buf);
      close(socket);
      return NULL;
    }
  }
  close(socket);

  return command;
}
