#pragma once

#include <Arduino_GFX_Library.h>

class CalibrationScreen {
 public:
  explicit CalibrationScreen(Arduino_GFX &display);

  void begin();
  bool isBackHit(int16_t x, int16_t y) const;
  bool isStartHit(int16_t x, int16_t y) const;

 private:
  Arduino_GFX &display_;
};
