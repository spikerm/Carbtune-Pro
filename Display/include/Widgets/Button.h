#pragma once

#include <Arduino_GFX_Library.h>

#include "Widgets/Geometry.h"

class Button {
 public:
  Button() = default;
  Button(Rect bounds, const char *label);

  void draw(Arduino_GFX &display, bool active = false) const;
  bool contains(int16_t x, int16_t y) const;

 private:
  Rect bounds_{0, 0, 0, 0};
  const char *label_ = "";
};
