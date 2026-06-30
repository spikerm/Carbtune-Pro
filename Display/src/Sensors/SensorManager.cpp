#include "Sensors/SensorManager.h"

#include <math.h>

void SensorManager::begin() {}

void SensorManager::update(uint32_t nowMs) {
  if (nowMs - lastUpdateMs_ < 100) {
    return;
  }

  lastUpdateMs_ = nowMs;
  const float t = nowMs / 1000.0f;
  valuesKpa_[0] = -42.0f + sinf(t * 1.1f) * 4.0f;
  valuesKpa_[1] = -40.0f + sinf(t * 0.9f + 1.0f) * 3.5f;
  valuesKpa_[2] = -41.0f + sinf(t * 1.3f + 2.0f) * 4.2f;
  valuesKpa_[3] = -44.0f + sinf(t * 0.8f + 3.0f) * 5.0f;
}

uint8_t SensorManager::channelCount() const {
  return 4;
}

float SensorManager::valueKpa(uint8_t channel) const {
  return channel < channelCount() ? valuesKpa_[channel] : 0.0f;
}

float SensorManager::maxDeltaKpa() const {
  float lowest = valuesKpa_[0];
  float highest = valuesKpa_[0];
  for (uint8_t index = 1; index < channelCount(); ++index) {
    lowest = min(lowest, valuesKpa_[index]);
    highest = max(highest, valuesKpa_[index]);
  }
  return highest - lowest;
}

SensorManager::Status SensorManager::status() const {
  const float delta = maxDeltaKpa();
  if (delta > 10.0f) {
    return Status::Adjust;
  }
  if (delta > 6.0f) {
    return Status::Warning;
  }
  return Status::Good;
}
