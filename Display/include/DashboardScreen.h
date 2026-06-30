#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

class DashboardScreen {
 public:
  explicit DashboardScreen(Arduino_GFX &display);

  void begin();
  void update(uint32_t nowMs);
  void setCylinderCount(uint8_t count);
  uint8_t cylinderCount() const;
  bool isMenuHit(int16_t x, int16_t y) const;
  void showTouchStatus(int16_t x, int16_t y);
  void showNotPressed();

 private:
  void drawStatic();
  void drawCylinder(uint8_t index, int16_t x, int16_t y, int16_t w, bool force);
  void drawBottomBar(bool force);
  void updateDemoValues(uint32_t nowMs);
  float maxDifference() const;
  uint8_t outlierIndex() const;

  Arduino_GFX &display_;
  float valuesKpa_[4] = {-42.0f, -40.0f, -41.0f, -44.0f};
  int16_t lastValueTenths_[4] = {-32768, -32768, -32768, -32768};
  int16_t lastDiffTenths_ = -32768;
  int16_t lastTouchX_ = -2;
  int16_t lastTouchY_ = -2;
  uint8_t cylinderCount_ = 4;
  uint32_t lastUpdateMs_ = 0;
};
