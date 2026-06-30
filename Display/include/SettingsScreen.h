#pragma once

#include <Arduino_GFX_Library.h>

#include "App/SettingsManager.h"
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
  SettingsScreen(Arduino_GFX &display, SettingsManager &settings);

  void begin();
  void update(uint32_t nowMs, const TouchState &touchState);
  SettingsAction takeAction();
  bool isHomeHit(int16_t x, int16_t y) const;
  bool isGraphHit(int16_t x, int16_t y) const;
  bool isSettingsHit(int16_t x, int16_t y) const;
  bool isCalibrationHit(int16_t x, int16_t y) const;

 private:
  void drawHeader();
  void drawFooter();
  void drawList();
  void drawSection(const char *title, int16_t y);
  void drawRow(const char *label, const char *value, int16_t y, bool button = false);
  void handleClick(int16_t screenX, int16_t screenY);

  Arduino_GFX &display_;
  SettingsManager &settings_;
  ScrollView scrollView_{{8, 34, 304, 168}, 616};
  SettingsAction pendingAction_ = SettingsAction::None;
  int16_t lastOffset_ = -1;
};
