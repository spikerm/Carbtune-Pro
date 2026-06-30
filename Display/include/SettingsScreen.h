#pragma once

#include <Arduino_GFX_Library.h>

class SettingsScreen {
 public:
  explicit SettingsScreen(Arduino_GFX &display);

  void begin();
  bool isHomeHit(int16_t x, int16_t y) const;
  bool isGraphHit(int16_t x, int16_t y) const;
  bool isSettingsHit(int16_t x, int16_t y) const;
  bool isCalibrationHit(int16_t x, int16_t y) const;

 private:
  Arduino_GFX &display_;
};
