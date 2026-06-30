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

const char *SettingsManager::dampingName() const {
  switch (dampingMode_) {
    case DampingMode::Low:
      return "laag";
    case DampingMode::Normal:
      return "normaal";
    case DampingMode::High:
      return "hoog";
  }
  return "normaal";
}

float SettingsManager::dampingAlpha() const {
  switch (dampingMode_) {
    case DampingMode::Low:
      return 0.20f;
    case DampingMode::Normal:
      return 0.10f;
    case DampingMode::High:
      return 0.05f;
  }
  return 0.10f;
}

const char *SettingsManager::engineStrokeName() const {
  return engineStroke_ == EngineStroke::FourStroke ? "4-takt" : "2-takt";
}

const char *SettingsManager::rpmSourceName() const {
  switch (rpmSource_) {
    case RpmSource::Ch1:
      return "CH1";
    case RpmSource::Ch2:
      return "CH2";
    case RpmSource::Ch3:
      return "CH3";
    case RpmSource::Ch4:
      return "CH4";
    case RpmSource::Auto:
      return "Auto";
  }
  return "CH1";
}

void SettingsManager::setAutoBrightness(bool enabled) {
  autoBrightness_ = enabled;
}

void SettingsManager::setBrightnessManualPercent(uint8_t percent) {
  brightnessManualPercent_ = constrain(percent, 0, 100);
}

void SettingsManager::setCylinders(uint8_t cylinders) {
  cylinders_ = constrain(cylinders, 2, 6);
}

void SettingsManager::setDemoFallback(bool enabled) {
  demoFallback_ = enabled;
}

void SettingsManager::toggleDemoFallback() {
  demoFallback_ = !demoFallback_;
}

void SettingsManager::cycleDampingMode() {
  switch (dampingMode_) {
    case DampingMode::Low:
      dampingMode_ = DampingMode::Normal;
      break;
    case DampingMode::Normal:
      dampingMode_ = DampingMode::High;
      break;
    case DampingMode::High:
      dampingMode_ = DampingMode::Low;
      break;
  }
}

void SettingsManager::toggleEngineStroke() {
  engineStroke_ = engineStroke_ == EngineStroke::FourStroke ? EngineStroke::TwoStroke
                                                            : EngineStroke::FourStroke;
}

void SettingsManager::cycleRpmSource() {
  const uint8_t next = (static_cast<uint8_t>(rpmSource_) + 1) % 5;
  rpmSource_ = static_cast<RpmSource>(next);
}
