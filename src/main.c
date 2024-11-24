#include "modules/module.h"
#include <dlfcn.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define STRLEN 80

module_data_t *init_module_data() {
  module_data_t *data = calloc(1, sizeof(module_data_t));
  data->buf = calloc(STRLEN + 1, sizeof(char));
  data->bufsize = STRLEN;

  pthread_mutex_init(&data->mutex, NULL);

  return data;
}

void *main_loop(void *d) {
  module_data_t *data = (module_data_t *)d;
  while (true) {
    pthread_mutex_lock(&data->mutex);
    printf("%s\r", data->buf);
    pthread_mutex_unlock(&data->mutex);
  }
}

int main() {
  void *handle = dlopen("dyno_simple.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen() %s\n", dlerror());
    exit(1);
  }
  dlerror();

  int (*run_func)(module_data_t *) = (int (*)())dlsym(handle, "run");

  module_data_t *data = init_module_data();
  pthread_t thread;
  pthread_create(&thread, NULL, &main_loop, data);
  run_func(data);
}
