#include "App/SettingsManager.h"

#include <nvs.h>
#include <nvs_flash.h>

static constexpr const char *PreferencesNamespace = "carbtune";

void SettingsManager::begin() {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
  }
  load();
}

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

SettingsManager::Units SettingsManager::unitsMode() const {
  return units_;
}

const char *SettingsManager::units() const {
  return units_ == Units::Kpa ? "kPa" : "inHg";
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

SettingsManager::RpmSource SettingsManager::rpmSource() const {
  return rpmSource_;
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

bool SettingsManager::save() {
  nvs_handle_t handle;
  if (nvs_open(PreferencesNamespace, NVS_READWRITE, &handle) != ESP_OK) {
    return false;
  }

  bool ok = nvs_set_u8(handle, "cyl", cylinders_) == ESP_OK;
  ok &= nvs_set_u8(handle, "units", static_cast<uint8_t>(units_)) == ESP_OK;
  ok &= nvs_set_u8(handle, "autoscale", autoScale_ ? 1 : 0) == ESP_OK;
  ok &= nvs_set_u8(handle, "damping", static_cast<uint8_t>(dampingMode_)) == ESP_OK;
  ok &= nvs_set_u8(handle, "autobright", autoBrightness_ ? 1 : 0) == ESP_OK;
  ok &= nvs_set_u8(handle, "bright", brightnessManualPercent_) == ESP_OK;
  ok &= nvs_set_u8(handle, "demo", demoFallback_ ? 1 : 0) == ESP_OK;
  ok &= nvs_set_u16(handle, "alarm10", static_cast<uint16_t>(alarmDeltaKpa_ * 10.0f)) == ESP_OK;
  ok &= nvs_set_u8(handle, "rpmsrc", static_cast<uint8_t>(rpmSource_)) == ESP_OK;
  ok &= nvs_set_u8(handle, "stroke", static_cast<uint8_t>(engineStroke_)) == ESP_OK;
  ok &= nvs_commit(handle) == ESP_OK;
  nvs_close(handle);
  return ok;
}

void SettingsManager::reload() {
  load();
}

void SettingsManager::resetDefaults() {
  cylinders_ = 4;
  units_ = Units::Kpa;
  autoBrightness_ = true;
  brightnessMinPercent_ = 40;
  brightnessMaxPercent_ = 100;
  brightnessManualPercent_ = 80;
  autoScale_ = true;
  filterSeconds_ = 0.7f;
  alarmDeltaKpa_ = 10.0f;
  demoFallback_ = true;
  liveUartEnabled_ = true;
  dampingMode_ = DampingMode::Normal;
  engineStroke_ = EngineStroke::FourStroke;
  rpmSource_ = RpmSource::Ch1;
}

void SettingsManager::setAutoBrightness(bool enabled) {
  autoBrightness_ = enabled;
}

void SettingsManager::setAutoScale(bool enabled) {
  autoScale_ = enabled;
}

void SettingsManager::setUnits(Units units) {
  units_ = units;
}

void SettingsManager::setBrightnessManualPercent(uint8_t percent) {
  brightnessManualPercent_ = constrain(percent, 0, 100);
}

void SettingsManager::setAlarmDeltaKpa(float threshold) {
  alarmDeltaKpa_ = constrain(threshold, 1.0f, 50.0f);
}

void SettingsManager::setCylinders(uint8_t cylinders) {
  cylinders_ = cylinders <= 2 ? 2 : (cylinders <= 4 ? 4 : 6);
}

void SettingsManager::setDemoFallback(bool enabled) {
  demoFallback_ = enabled;
}

void SettingsManager::toggleDemoFallback() {
  demoFallback_ = !demoFallback_;
}

void SettingsManager::toggleAutoBrightness() {
  autoBrightness_ = !autoBrightness_;
}

void SettingsManager::toggleAutoScale() {
  autoScale_ = !autoScale_;
}

void SettingsManager::toggleUnits() {
  units_ = units_ == Units::Kpa ? Units::InHg : Units::Kpa;
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

void SettingsManager::load() {
  nvs_handle_t handle;
  if (nvs_open(PreferencesNamespace, NVS_READONLY, &handle) != ESP_OK) {
    return;
  }

  uint8_t value8 = 0;
  uint16_t value16 = 0;
  if (nvs_get_u8(handle, "cyl", &value8) == ESP_OK) {
    cylinders_ = value8;
  }
  setCylinders(cylinders_);
  if (nvs_get_u8(handle, "units", &value8) == ESP_OK) {
    units_ = static_cast<Units>(value8);
  }
  if (units_ != Units::Kpa && units_ != Units::InHg) {
    units_ = Units::Kpa;
  }
  if (nvs_get_u8(handle, "autoscale", &value8) == ESP_OK) {
    autoScale_ = value8 != 0;
  }
  if (nvs_get_u8(handle, "damping", &value8) == ESP_OK) {
    dampingMode_ = static_cast<DampingMode>(value8);
  }
  if (static_cast<uint8_t>(dampingMode_) > static_cast<uint8_t>(DampingMode::High)) {
    dampingMode_ = DampingMode::Normal;
  }
  if (nvs_get_u8(handle, "autobright", &value8) == ESP_OK) {
    autoBrightness_ = value8 != 0;
  }
  if (nvs_get_u8(handle, "bright", &value8) == ESP_OK) {
    brightnessManualPercent_ = constrain(value8, static_cast<uint8_t>(0), static_cast<uint8_t>(100));
  }
  if (nvs_get_u8(handle, "demo", &value8) == ESP_OK) {
    demoFallback_ = value8 != 0;
  }
  if (nvs_get_u16(handle, "alarm10", &value16) == ESP_OK) {
    alarmDeltaKpa_ = constrain(value16 / 10.0f, 1.0f, 50.0f);
  }
  if (nvs_get_u8(handle, "rpmsrc", &value8) == ESP_OK) {
    rpmSource_ = static_cast<RpmSource>(value8);
  }
  if (static_cast<uint8_t>(rpmSource_) > static_cast<uint8_t>(RpmSource::Auto)) {
    rpmSource_ = RpmSource::Ch1;
  }
  if (nvs_get_u8(handle, "stroke", &value8) == ESP_OK) {
    engineStroke_ = static_cast<EngineStroke>(value8);
  }
  if (engineStroke_ != EngineStroke::TwoStroke && engineStroke_ != EngineStroke::FourStroke) {
    engineStroke_ = EngineStroke::FourStroke;
  }
  nvs_close(handle);
}
