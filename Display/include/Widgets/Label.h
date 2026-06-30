#pragma once

#include <Arduino_GFX_Library.h>

class Label {
 public:
  Label(int16_t x, int16_t y, const char *text, uint8_t size = 1);

  void draw(Arduino_GFX &display, uint16_t color) const;

 private:
  int16_t x_;
  int16_t y_;
  const char *text_;
  uint8_t size_;
};
