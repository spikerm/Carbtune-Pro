#pragma once

#include <Arduino.h>

class SensorManager {
 public:
  enum class Status {
    Good,
    Warning,
    Adjust,
  };

  void begin();
  void update(uint32_t nowMs);

  uint8_t channelCount() const;
  float valueKpa(uint8_t channel) const;
  float maxDeltaKpa() const;
  Status status() const;

 private:
  float valuesKpa_[4] = {-42.0f, -40.0f, -41.0f, -44.0f};
  uint32_t lastUpdateMs_ = 0;
};
