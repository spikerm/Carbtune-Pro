#pragma once

#include <Arduino_GFX_Library.h>

#include "Widgets/Geometry.h"

class Panel {
 public:
  explicit Panel(Rect bounds);

  void draw(Arduino_GFX &display) const;

 private:
  Rect bounds_;
};
