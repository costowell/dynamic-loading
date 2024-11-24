#include <pthread.h>
#include <stddef.h>

typedef struct _module_data {
  char *buf;
  size_t bufsize;
  pthread_mutex_t mutex;
} module_data_t;
