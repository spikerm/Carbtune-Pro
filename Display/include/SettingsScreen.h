#pragma once

#include <Arduino_GFX_Library.h>

#include "App/SettingsManager.h"
#include "Storage/SdManager.h"
#include "TouchInput.h"
#include "Widgets/ScrollView.h"

enum class SettingsAction {
  None,
  Home,
  Graph,
  Calibration,
  Diagnostics,
};

class SettingsScreen {
 public:
  SettingsScreen(Arduino_GFX &display, SettingsManager &settings, SdManager &sdManager);

  void begin();
  void update(uint32_t nowMs, const TouchState &touchState);
  SettingsAction takeAction();
  bool isHomeHit(int16_t x, int16_t y) const;
  bool isGraphHit(int16_t x, int16_t y) const;
  bool isSettingsHit(int16_t x, int16_t y) const;
  bool isCalibrationHit(int16_t x, int16_t y) const;

 private:
  struct DraftSettings {
    uint8_t cylinders = 4;
    SettingsManager::Units units = SettingsManager::Units::Kpa;
    bool autoScale = true;
    SettingsManager::DampingMode damping = SettingsManager::DampingMode::Normal;
    bool autoBrightness = true;
    uint8_t brightness = 80;
    bool demoFallback = true;
    float alarmDeltaKpa = 10.0f;
    SettingsManager::RpmSource rpmSource = SettingsManager::RpmSource::Ch1;
    SettingsManager::EngineStroke engineStroke = SettingsManager::EngineStroke::FourStroke;
  };

  void loadDraft();
  void applyDraft();
  void drawHeader();
  void drawFooter();
  void drawList();
  void drawMessage(uint32_t nowMs);
  void drawSection(const char *title, int16_t y);
  void drawRow(const char *label, const String &value, int16_t y, bool button = false,
               bool active = false);
  void drawSegmentButton(const char *label, int16_t x, int16_t y, int16_t w, bool active);
  void handleClick(int16_t screenX, int16_t screenY);
  void setMessage(const char *message, uint32_t nowMs);
  const char *unitsName() const;
  const char *dampingName() const;
  const char *engineStrokeName() const;
  const char *rpmSourceName() const;

  Arduino_GFX &display_;
  SettingsManager &settings_;
  SdManager &sdManager_;
  ScrollView scrollView_{{4, 44, 312, 152}, 760};
  DraftSettings draft_;
  SettingsAction pendingAction_ = SettingsAction::None;
  int16_t lastOffset_ = -1;
  uint32_t messageUntilMs_ = 0;
  char message_[32] = "";
  bool touchWasPressed_ = false;
  int16_t lastTouchX_ = -1;
  int16_t lastTouchY_ = -1;
};
