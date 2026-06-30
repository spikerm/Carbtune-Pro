#pragma once

#include <Arduino.h>
#include <XPT2046_Touchscreen.h>

struct TouchState {
  bool pressed = false;
  bool changed = false;
  uint16_t rawX = 0;
  uint16_t rawY = 0;
  int16_t screenX = -1;
  int16_t screenY = -1;
};

class TouchInput {
 public:
  explicit TouchInput(XPT2046_Touchscreen &touch);

  bool begin();
  TouchState update(uint32_t nowMs);
  TouchState current() const;

 private:
  bool isValidRaw(uint16_t rawX, uint16_t rawY) const;
  int16_t mapAxis(uint16_t raw, uint16_t rawMin, uint16_t rawMax, int16_t screenMax) const;
  void logState(const TouchState &state) const;

  XPT2046_Touchscreen &touch_;
  TouchState state_;
  uint32_t lastSampleMs_ = 0;
};
