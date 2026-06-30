#pragma once

#include <Arduino.h>

#include "App/SettingsManager.h"

class BacklightManager {
 public:
  explicit BacklightManager(SettingsManager &settings);

  void begin();
  void update(uint32_t nowMs);

  uint16_t ldrRaw() const;
  float filteredLight() const;
  uint8_t brightnessPercent() const;
  bool pwmActive() const;
  bool ldrWarning() const;
  bool autoBrightness() const;

 private:
  uint8_t targetPercent() const;
  void applyBrightness(uint8_t percent);

  SettingsManager &settings_;
  uint16_t ldrRaw_ = 0;
  float filteredLight_ = 0.0f;
  uint8_t brightnessPercent_ = 100;
  bool pwmActive_ = false;
  bool ldrWarning_ = false;
  uint32_t lastSampleMs_ = 0;
};
