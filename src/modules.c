#include "modules.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

module_t **list_modules() {
  module_t **modules = calloc(80, sizeof(module_t *));
  DIR *modules_dir = opendir(MODULEDIR);
  int i = 0;
  struct dirent *dir;

  while ((dir = readdir(modules_dir))) {
    if (dir->d_type != DT_REG || strncasecmp(dir->d_name, "dyno_", 5) ||
        strncasecmp(dir->d_name + strlen(dir->d_name) - 3, ".so", 3))
      continue;

    char *path = calloc(PATH_MAX + 1, sizeof(char));
    strcpy(path, MODULEDIR);
    strcat(path, "/");
    strcat(path, dir->d_name);

    module_t *module = calloc(1, sizeof(module_t));
    module->path = path;
    modules[i] = module;
    i++;
  }

  closedir(modules_dir);

  return modules;
}
