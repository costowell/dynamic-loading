#ifndef DRAW_H
#define DRAW_H

#include "modules.h"
#include <stdatomic.h>
#include <stdbool.h>

int draw_thread_start(module_t *module);
int draw_thread_stop();

#endif
