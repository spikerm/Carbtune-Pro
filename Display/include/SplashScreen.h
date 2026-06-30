#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class SplashScreen {
 public:
  explicit SplashScreen(Arduino_GFX &display);

  void begin(uint32_t nowMs);
  bool update(uint32_t nowMs);

 private:
  void drawStatic();
  void drawGauge();
  void drawProgress(uint8_t activeDots);

  Arduino_GFX &display_;
  uint32_t startedMs_ = 0;
  uint8_t lastDots_ = 255;
};
