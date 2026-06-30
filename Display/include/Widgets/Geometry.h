#pragma once

#include <Arduino.h>

struct Rect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;

  bool contains(int16_t px, int16_t py) const {
    return px >= x && px < x + w && py >= y && py < y + h;
  }
};
