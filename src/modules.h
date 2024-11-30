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

module_t *list_modules(size_t *);
int load_module(module_t *);
int unload_module(module_t *);

#endif
