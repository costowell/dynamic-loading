#ifndef MODULE_H
#define MODULE_H

#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MODULE_SUCCESS 0

typedef struct _color {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} color_t;

typedef struct _module_data {
  color_t *colors;
  size_t quantity;
} module_data_t;

#endif
