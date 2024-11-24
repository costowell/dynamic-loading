#include "../../module.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

color_t hsv_to_rgb(float h, float s, float v) {
  float r, g, b;
  float c, x, m;

  // Normalize hue to [0, 360) degrees
  h = fmod(h, 360.0f);
  if (h < 0)
    h += 360.0f;

  // Chroma
  c = v * s;

  // Intermediate value x
  x = c * (1 - fabs(fmod(h / 60.0f, 2) - 1));

  // Match color sector to RGB
  if (h >= 0 && h < 60) {
    r = c;
    g = x;
    b = 0;
  } else if (h >= 60 && h < 120) {
    r = x;
    g = c;
    b = 0;
  } else if (h >= 120 && h < 180) {
    r = 0;
    g = c;
    b = x;
  } else if (h >= 180 && h < 240) {
    r = 0;
    g = x;
    b = c;
  } else if (h >= 240 && h < 300) {
    r = x;
    g = 0;
    b = c;
  } else {
    r = c;
    g = 0;
    b = x;
  }

  // Add the match value (m) to each component
  m = v - c;
  r += m;
  g += m;
  b += m;

  // Convert from float [0,1] to uint8_t [0, 255]
  color_t result;
  result.r = (uint8_t)(r * 255);
  result.g = (uint8_t)(g * 255);
  result.b = (uint8_t)(b * 255);

  return result;
}

static module_data_t *data;
static int shift = 0;

int module_init(module_data_t *d) {
  data = d;
  return MODULE_SUCCESS;
}

int module_update() {
  if (!data)
    return 1;
  for (int i = 0; i < data->quantity; ++i) {
    data->colors[i] = hsv_to_rgb(
        ((float)(i + shift) / (float)data->quantity) * 360.0f, 1.0f, 1.0f);
  }
  shift++;
  return MODULE_SUCCESS;
}
