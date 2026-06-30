#pragma once

#include <Arduino_GFX_Library.h>

#include "Widgets/Geometry.h"

class StatusBadge {
 public:
  StatusBadge(Rect bounds, const char *label);

  void draw(Arduino_GFX &display, uint16_t color) const;

 private:
  Rect bounds_;
  const char *label_;
};
