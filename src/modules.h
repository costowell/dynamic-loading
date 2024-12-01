#ifndef _MODULES_H
#define _MODULES_H

#include "modules/module.h"

#ifndef MODULEDIR
#define MODULEDIR "/var/lib/dyno/modules"
#endif

typedef struct _module {
  char *path;
  void *handle;
  int (*init)(module_data_t *);
  int (*update)();
} module_t;

typedef struct _module_array {
  module_t *modules;
  size_t count;
} module_array_t;

module_array_t *list_modules();
int load_module(module_t *);
int unload_module(module_t *);
void free_module_array(module_array_t *);
void free_module(module_t *);

#endif
