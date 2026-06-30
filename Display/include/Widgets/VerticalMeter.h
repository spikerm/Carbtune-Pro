#pragma once

#include <Arduino_GFX_Library.h>

#include "Widgets/Geometry.h"

class VerticalMeter {
 public:
  explicit VerticalMeter(Rect bounds);

  void drawFrame(Arduino_GFX &display) const;
  void drawValue(Arduino_GFX &display, float valueKpa, uint16_t color) const;

 private:
  Rect bounds_;
};
