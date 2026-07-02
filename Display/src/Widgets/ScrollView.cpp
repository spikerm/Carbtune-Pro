#include "Widgets/ScrollView.h"

#include "Theme/Theme.h"

static constexpr int16_t DragThresholdPx = 5;
static constexpr int16_t ScrollGain = 2;

ScrollView::ScrollView(Rect viewport, int16_t contentHeight)
    : viewport_(viewport), contentHeight_(contentHeight) {}

void ScrollView::begin() {
  offset_ = 0;
  velocity_ = 0.0f;
  wasPressed_ = false;
  dragging_ = false;
  clickReleased_ = false;
  lastUpdateMs_ = millis();
}

void ScrollView::update(uint32_t nowMs, const TouchState &touchState) {
  clickReleased_ = false;
  const bool inside = touchState.pressed && contains(touchState.screenX, touchState.screenY);

  if (inside && !wasPressed_) {
    startY_ = touchState.screenY;
    lastY_ = touchState.screenY;
    clickX_ = touchState.screenX;
    clickY_ = touchState.screenY;
    velocity_ = 0.0f;
    dragging_ = false;
  } else if (inside && wasPressed_) {
    const int16_t dy = touchState.screenY - lastY_;
    if (!dragging_ && abs(touchState.screenY - startY_) > DragThresholdPx) {
      dragging_ = true;
    }
    if (dragging_) {
      offset_ -= dy * ScrollGain;
      velocity_ = (velocity_ * 0.45f) + (static_cast<float>(-dy * ScrollGain) * 0.55f);
      clampOffset();
    }
    lastY_ = touchState.screenY;
  } else if (!touchState.pressed && wasPressed_) {
    clickReleased_ = !dragging_;
  }

  if (!touchState.pressed && fabsf(velocity_) > 0.25f) {
    offset_ += static_cast<int16_t>(velocity_);
    velocity_ *= 0.82f;
    clampOffset();
  }

  if (!touchState.pressed && (offset_ == 0 || offset_ == maxOffset())) {
    velocity_ = 0.0f;
  }

  wasPressed_ = touchState.pressed;
  lastUpdateMs_ = nowMs;
}

void ScrollView::setContentHeight(int16_t contentHeight) {
  contentHeight_ = contentHeight;
  clampOffset();
}

int16_t ScrollView::offset() const {
  return offset_;
}

bool ScrollView::isDragging() const {
  return dragging_;
}

bool ScrollView::wasClickReleased() const {
  return clickReleased_;
}

int16_t ScrollView::clickX() const {
  return clickX_;
}

int16_t ScrollView::clickY() const {
  return clickY_;
}

int16_t ScrollView::contentY(int16_t screenY) const {
  return screenY - viewport_.y + offset_;
}

bool ScrollView::contains(int16_t x, int16_t y) const {
  return viewport_.contains(x, y);
}

void ScrollView::drawScrollbar(Arduino_GFX &display) const {
  display.fillRect(viewport_.x + viewport_.w - 4, viewport_.y, 3, viewport_.h, Theme::PanelAlt);
  if (contentHeight_ <= viewport_.h) {
    return;
  }

  const int16_t thumbH = max<int16_t>(18, (viewport_.h * viewport_.h) / contentHeight_);
  const int16_t trackH = viewport_.h - thumbH;
  const int16_t thumbY = viewport_.y + ((trackH * offset_) / maxOffset());
  display.fillRoundRect(viewport_.x + viewport_.w - 5, thumbY, 5, thumbH, 2, Theme::AccentBlue);
}

int16_t ScrollView::maxOffset() const {
  return max<int16_t>(0, contentHeight_ - viewport_.h);
}

void ScrollView::clampOffset() {
  offset_ = constrain(offset_, 0, maxOffset());
}
