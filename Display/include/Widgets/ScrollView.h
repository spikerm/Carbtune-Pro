#pragma once

#include <Arduino_GFX_Library.h>

#include "TouchInput.h"
#include "Widgets/Geometry.h"

class ScrollView {
 public:
  ScrollView(Rect viewport, int16_t contentHeight);

  void begin();
  void update(uint32_t nowMs, const TouchState &touchState);
  void setContentHeight(int16_t contentHeight);
  int16_t offset() const;
  bool isDragging() const;
  bool wasClickReleased() const;
  int16_t clickX() const;
  int16_t clickY() const;
  int16_t contentY(int16_t screenY) const;
  bool contains(int16_t x, int16_t y) const;
  void drawScrollbar(Arduino_GFX &display) const;

 private:
  int16_t maxOffset() const;
  void clampOffset();

  Rect viewport_;
  int16_t contentHeight_ = 0;
  int16_t offset_ = 0;
  int16_t startY_ = 0;
  int16_t lastY_ = 0;
  int16_t clickX_ = -1;
  int16_t clickY_ = -1;
  float velocity_ = 0.0f;
  uint32_t lastUpdateMs_ = 0;
  bool wasPressed_ = false;
  bool tracking_ = false;
  bool dragging_ = false;
  bool clickReleased_ = false;
};
