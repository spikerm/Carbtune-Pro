#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

struct UiRect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

class UiButton {
 public:
  UiButton() = default;
  UiButton(UiRect rect, const char *label);

  void draw(Arduino_GFX &display, bool active = false) const;
  bool contains(int16_t x, int16_t y) const;
  UiRect rect() const;

 private:
  UiRect rect_{0, 0, 0, 0};
  const char *label_ = "";
};
