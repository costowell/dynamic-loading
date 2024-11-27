#include "util.h"
#include <stdio.h>

void cursor_visible(bool visible) {
  if (visible) {
    printf("\033[?25h");
  } else {
    printf("\033[?25l");
  }
}
