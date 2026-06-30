#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

#include "TouchInput.h"

class TouchDiagnosticsScreen {
 public:
  TouchDiagnosticsScreen(Arduino_GFX &display, TouchInput &touchInput);

  void begin();
  void update(const TouchState &touchState);

 private:
  void drawStatic();
  void drawRow(const char *label, const String &value, int16_t y, bool force);
  void drawMenu();

  Arduino_GFX &display_;
  TouchInput &touchInput_;
  TouchState lastState_;
  bool firstDraw_ = true;
};
