#include "modules.h"
#include <dirent.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

module_array_t *init_module_array() {
  module_array_t *module_array = calloc(1, sizeof(module_array_t));
  module_array->count = 0;
  module_array->modules = calloc(80, sizeof(module_t));
  return module_array;
}

module_array_t *list_modules() {
  module_array_t *module_array = init_module_array();
  DIR *modules_dir = opendir(MODULEDIR);
  size_t i = 0;
  struct dirent *dir;

  while ((dir = readdir(modules_dir))) {
    if (dir->d_type != DT_REG || strncasecmp(dir->d_name, "dyno_", 5) ||
        strncasecmp(dir->d_name + strlen(dir->d_name) - 3, ".so", 3))
      continue;

    char *path = calloc(PATH_MAX + 1, sizeof(char));
    strcpy(path, MODULEDIR);
    strcat(path, "/");
    strcat(path, dir->d_name);

    module_array->modules[i++].path = path;
  }

  closedir(modules_dir);
  module_array->count = i;

  return module_array;
}

int load_module(module_t *module) {
  void *handle = dlopen(module->path, RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen() %s\n", dlerror());
    return 1;
  }
  dlerror();

  module->handle = handle;
  module->init = (int (*)(module_data_t *))dlsym(handle, "module_init");
  module->update = (int (*)())dlsym(handle, "module_update");
  if (module->init == NULL || module->update == NULL)
    return 2;
  return 0;
}

int unload_module(module_t *module) {
  if (!module->handle) {
    fprintf(stderr, "error: module not loaded\n");
    return 1;
  }

  dlclose(module->handle);
  module->handle = NULL;
  module->init = NULL;
  module->update = NULL;

  return 0;
}

void free_module_array(module_array_t *arr) {
  for (size_t i = 0; i < arr->count; ++i) {
    free_module(&arr->modules[i]);
  }
  free(arr->modules);
}
void free_module(module_t *module) { free(module->path); }
