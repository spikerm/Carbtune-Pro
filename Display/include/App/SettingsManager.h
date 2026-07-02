#pragma once

#include <Arduino.h>

class SettingsManager {
 public:
  enum class Units : uint8_t {
    Kpa,
    InHg,
  };

  enum class DampingMode : uint8_t {
    Low,
    Normal,
    High,
  };

  enum class EngineStroke : uint8_t {
    TwoStroke = 2,
    FourStroke = 4,
  };

  enum class RpmSource : uint8_t {
    Ch1,
    Ch2,
    Ch3,
    Ch4,
    Auto,
  };

  void begin();

  uint8_t cylinders() const;
  bool autoBrightness() const;
  uint8_t brightnessMinPercent() const;
  uint8_t brightnessMaxPercent() const;
  uint8_t brightnessManualPercent() const;
  Units unitsMode() const;
  const char *units() const;
  bool autoScale() const;
  float filterSeconds() const;
  float alarmDeltaKpa() const;
  bool demoFallback() const;
  bool liveUartEnabled() const;
  const char *dampingName() const;
  DampingMode dampingMode() const;
  float dampingAlpha() const;
  const char *engineStrokeName() const;
  EngineStroke engineStroke() const;
  RpmSource rpmSource() const;
  const char *rpmSourceName() const;

  bool save();
  void reload();
  void resetDefaults();
  void setAutoBrightness(bool enabled);
  void setAutoScale(bool enabled);
  void setUnits(Units units);
  void setDampingMode(DampingMode mode);
  void setEngineStroke(EngineStroke stroke);
  void setRpmSource(RpmSource source);
  void setBrightnessManualPercent(uint8_t percent);
  void setAlarmDeltaKpa(float threshold);
  void setCylinders(uint8_t cylinders);
  void setDemoFallback(bool enabled);
  void toggleDemoFallback();
  void toggleAutoBrightness();
  void toggleAutoScale();
  void toggleUnits();
  void cycleDampingMode();
  void toggleEngineStroke();
  void cycleRpmSource();

 private:
  void load();

  uint8_t cylinders_ = 4;
  Units units_ = Units::Kpa;
  bool autoBrightness_ = true;
  uint8_t brightnessMinPercent_ = 40;
  uint8_t brightnessMaxPercent_ = 100;
  uint8_t brightnessManualPercent_ = 80;
  bool autoScale_ = true;
  float filterSeconds_ = 0.7f;
  float alarmDeltaKpa_ = 10.0f;
  bool demoFallback_ = true;
  bool liveUartEnabled_ = true;
  DampingMode dampingMode_ = DampingMode::Normal;
  EngineStroke engineStroke_ = EngineStroke::FourStroke;
  RpmSource rpmSource_ = RpmSource::Ch1;
};
