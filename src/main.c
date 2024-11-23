#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
  void *handle = dlopen("dyno_simple.so", RTLD_LAZY);
  if (!handle) {
    fprintf(stderr, "dlopen() %s\n", dlerror());
    exit(1);
  }
  dlerror();

  int (*run_func)() = (int (*)())dlsym(handle, "run");
  run_func();
}
