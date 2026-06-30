#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "Sensors/BacklightManager.h"
#include "TouchInput.h"

class TouchDiagnosticsScreen {
 public:
  TouchDiagnosticsScreen(Arduino_GFX &display, TouchInput &touchInput,
                         BacklightManager &backlightManager);

  void begin();
  void update(const TouchState &touchState);

 private:
  void drawStatic();
  void drawRow(const char *label, const String &value, int16_t y, bool force);
  void drawMenu();

  Arduino_GFX &display_;
  TouchInput &touchInput_;
  BacklightManager &backlightManager_;
  TouchState lastState_;
  uint16_t lastLdrRaw_ = 65535;
  int16_t lastFilteredLight_ = -1;
  uint8_t lastBrightnessPercent_ = 255;
  bool lastAutoBrightness_ = false;
  bool lastLdrWarning_ = false;
  bool firstDraw_ = true;
};
