#pragma once

#include <Arduino_GFX_Library.h>

class GraphScreen {
 public:
  explicit GraphScreen(Arduino_GFX &display);

  void begin();
  bool isHomeHit(int16_t x, int16_t y) const;
  bool isSettingsHit(int16_t x, int16_t y) const;

 private:
  void drawGraph();
  void drawLegend();

  Arduino_GFX &display_;
};
