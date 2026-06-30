#include "App/SettingsManager.h"

void SettingsManager::begin() {}

uint8_t SettingsManager::cylinders() const {
  return cylinders_;
}

bool SettingsManager::autoBrightness() const {
  return autoBrightness_;
}

uint8_t SettingsManager::brightnessMinPercent() const {
  return brightnessMinPercent_;
}

uint8_t SettingsManager::brightnessMaxPercent() const {
  return brightnessMaxPercent_;
}

uint8_t SettingsManager::brightnessManualPercent() const {
  return brightnessManualPercent_;
}

const char *SettingsManager::units() const {
  return "kPa";
}

bool SettingsManager::autoScale() const {
  return autoScale_;
}

float SettingsManager::filterSeconds() const {
  return filterSeconds_;
}

float SettingsManager::alarmDeltaKpa() const {
  return alarmDeltaKpa_;
}

bool SettingsManager::demoFallback() const {
  return demoFallback_;
}

bool SettingsManager::liveUartEnabled() const {
  return liveUartEnabled_;
}

void SettingsManager::setAutoBrightness(bool enabled) {
  autoBrightness_ = enabled;
}

void SettingsManager::setBrightnessManualPercent(uint8_t percent) {
  brightnessManualPercent_ = constrain(percent, 0, 100);
}
