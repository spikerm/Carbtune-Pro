#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "Sensors/SensorManager.h"

class DashboardScreen {
 public:
  DashboardScreen(Arduino_GFX &display, SensorManager &sensorManager);

  void begin();
  void update(uint32_t nowMs);
  void setCylinderCount(uint8_t count);
  uint8_t cylinderCount() const;
  bool isMenuHit(int16_t x, int16_t y) const;
  void showTouchStatus(int16_t x, int16_t y);
  void showNotPressed();

 private:
  void drawStatic();
  void drawHeader(bool force);
  void drawCylinder(uint8_t index, int16_t x, int16_t y, int16_t w, bool force);
  void drawBottomBar(bool force);
  float maxDifference() const;
  uint8_t outlierIndex() const;
  const char *statusText() const;
  uint16_t statusColor() const;

  Arduino_GFX &display_;
  SensorManager &sensorManager_;
  float valuesKpa_[SensorManager::MaxChannels] = {-42.0f, -40.0f, -41.0f, -44.0f, -43.0f, -45.0f};
  int16_t lastValueTenths_[SensorManager::MaxChannels] = {-32768, -32768, -32768,
                                                          -32768, -32768, -32768};
  int16_t lastDiffTenths_ = -32768;
  SensorManager::Mode lastMode_ = SensorManager::Mode::NoData;
  SensorManager::Status lastStatus_ = SensorManager::Status::Warning;
  int16_t lastTouchX_ = -2;
  int16_t lastTouchY_ = -2;
  uint8_t cylinderCount_ = 4;
  uint32_t lastUpdateMs_ = 0;
};
