#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class CarbtuneScreen {
 public:
  explicit CarbtuneScreen(Arduino_GFX &display);

  void begin();
  void update(uint32_t nowMs);
  void showTouchStatus(int16_t x, int16_t y);
  void showNotPressed();
  bool isMenuHit(int16_t x, int16_t y) const;

 private:
  void drawStatic();
  void updateDemoValues(uint32_t nowMs);
  void drawChannel(uint8_t channel);
  void drawDelta();
  void drawMenu();

  Arduino_GFX &display_;
  float valuesKpa_[4] = {};
  int16_t lastBarWidth_[4] = {-1, -1, -1, -1};
  int16_t lastValueCentiKpa_[4] = {-32768, -32768, -32768, -32768};
  int16_t lastDeltaCentiKpa_ = -32768;
  int16_t lastTouchX_ = -2;
  int16_t lastTouchY_ = -2;
  uint32_t lastUpdateMs_ = 0;
};
