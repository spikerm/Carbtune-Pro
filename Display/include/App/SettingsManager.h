#pragma once

#include <Arduino.h>

class SettingsManager {
 public:
  void begin();

  uint8_t cylinders() const;
  bool autoBrightness() const;
  uint8_t brightnessMinPercent() const;
  uint8_t brightnessMaxPercent() const;
  uint8_t brightnessManualPercent() const;
  const char *units() const;
  bool autoScale() const;
  float filterSeconds() const;
  float alarmDeltaKpa() const;
  bool demoFallback() const;
  bool liveUartEnabled() const;

  void setAutoBrightness(bool enabled);
  void setBrightnessManualPercent(uint8_t percent);
  void setCylinders(uint8_t cylinders);

 private:
  uint8_t cylinders_ = 4;
  bool autoBrightness_ = true;
  uint8_t brightnessMinPercent_ = 40;
  uint8_t brightnessMaxPercent_ = 100;
  uint8_t brightnessManualPercent_ = 80;
  bool autoScale_ = true;
  float filterSeconds_ = 0.7f;
  float alarmDeltaKpa_ = 10.0f;
  bool demoFallback_ = true;
  bool liveUartEnabled_ = true;
};
