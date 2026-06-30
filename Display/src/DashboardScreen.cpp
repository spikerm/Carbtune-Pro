#include "DashboardScreen.h"

#include <math.h>

#include "UiTheme.h"

static constexpr float AlarmThresholdKpa = 8.0f;
static constexpr int16_t MeterTop = 88;
static constexpr int16_t MeterHeight = 76;
static constexpr int16_t MenuX = 238;
static constexpr int16_t MenuY = 210;
static constexpr int16_t MenuW = 74;
static constexpr int16_t MenuH = 24;
static constexpr int16_t TouchStatusX = 96;
static constexpr int16_t TouchStatusY = 224;

DashboardScreen::DashboardScreen(Arduino_GFX &display) : display_(display) {}

void DashboardScreen::begin() {
  for (uint8_t index = 0; index < 4; ++index) {
    lastValueTenths_[index] = -32768;
  }
  lastDiffTenths_ = -32768;
  lastTouchX_ = -2;
  lastTouchY_ = -2;
  lastUpdateMs_ = 0;
  drawStatic();
  update(millis());
}

void DashboardScreen::update(uint32_t nowMs) {
  if (nowMs - lastUpdateMs_ < 60) {
    return;
  }

  lastUpdateMs_ = nowMs;
  updateDemoValues(nowMs);

  const int16_t panelY = 36;
  const int16_t panelH = 162;
  if (cylinderCount_ == 2) {
    drawCylinder(0, 72, panelY, 76, false);
    drawCylinder(1, 172, panelY, 76, false);
  } else {
    for (uint8_t index = 0; index < 4; ++index) {
      drawCylinder(index, 10 + (index * 77), panelY, 70, false);
    }
  }

  (void)panelH;
  drawBottomBar(false);
}

void DashboardScreen::setCylinderCount(uint8_t count) {
  cylinderCount_ = constrain(count, 1, 4);
}

uint8_t DashboardScreen::cylinderCount() const {
  return cylinderCount_;
}

bool DashboardScreen::isMenuHit(int16_t x, int16_t y) const {
  return x >= 220 && x < 320 && y >= 196 && y < 240;
}

void DashboardScreen::showTouchStatus(int16_t x, int16_t y) {
  if (x == lastTouchX_ && y == lastTouchY_) {
    return;
  }

  display_.fillRect(TouchStatusX, TouchStatusY, 118, 10, UiTheme::Background);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(TouchStatusX, TouchStatusY);
  display_.print("Touch ");
  display_.print(x);
  display_.print(",");
  display_.print(y);
  lastTouchX_ = x;
  lastTouchY_ = y;
}

void DashboardScreen::showNotPressed() {
  if (lastTouchX_ == -1 && lastTouchY_ == -1) {
    return;
  }

  display_.fillRect(TouchStatusX, TouchStatusY, 118, 10, UiTheme::Background);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Muted);
  display_.setCursor(TouchStatusX, TouchStatusY);
  display_.print("Not pressed");
  lastTouchX_ = -1;
  lastTouchY_ = -1;
}

void DashboardScreen::drawStatic() {
  display_.fillScreen(UiTheme::Background);
  UiTheme::drawTopBar(display_, "CARBTUNE ESP32", "12:45");
  UiTheme::drawPanel(display_, 8, 36, 304, 162);

  if (cylinderCount_ == 2) {
    drawCylinder(0, 72, 36, 76, true);
    drawCylinder(1, 172, 36, 76, true);
  } else {
    for (uint8_t index = 0; index < 4; ++index) {
      drawCylinder(index, 10 + (index * 77), 36, 70, true);
    }
  }

  UiTheme::drawBottomDivider(display_);
  drawBottomBar(true);
  showNotPressed();
}

void DashboardScreen::drawCylinder(uint8_t index, int16_t x, int16_t y, int16_t w, bool force) {
  const int16_t valueTenths = static_cast<int16_t>(valuesKpa_[index] * 10.0f);
  if (!force && valueTenths == lastValueTenths_[index]) {
    return;
  }

  const bool alarm = maxDifference() > AlarmThresholdKpa && index == outlierIndex();
  const uint16_t meterColor = alarm ? UiTheme::Warning : UiTheme::Good;
  const int16_t center = x + (w / 2);
  const int16_t meterX = center - 12;
  const int16_t fillHeight = constrain(static_cast<int16_t>((-valuesKpa_[index]) * MeterHeight / 100.0f), 0, MeterHeight);
  const int16_t fillY = MeterTop + MeterHeight - fillHeight;

  display_.fillRect(x + 2, y + 4, w - 4, 156, UiTheme::Panel);
  display_.setTextColor(UiTheme::Text);
  display_.setTextSize(2);
  display_.setCursor(center - 6, y + 12);
  display_.print(index + 1);
  display_.setTextSize(2);
  display_.setCursor(center - 22, y + 36);
  display_.print(static_cast<int>(valuesKpa_[index]));
  display_.setTextSize(1);
  display_.setCursor(center - 10, y + 58);
  display_.print("kPa");

  display_.setTextColor(UiTheme::Text);
  display_.setCursor(meterX - 18, MeterTop);
  display_.print("0");
  display_.setCursor(meterX - 24, MeterTop + 34);
  display_.print("-50");
  display_.setCursor(meterX - 30, MeterTop + 72);
  display_.print("-100");
  display_.drawRect(meterX, MeterTop, 25, MeterHeight, UiTheme::Border);
  display_.fillRect(meterX + 1, MeterTop + 1, 23, MeterHeight - 2, UiTheme::Background);
  display_.fillRect(meterX + 1, fillY, 23, fillHeight, meterColor);
  display_.drawFastHLine(meterX - 4, fillY, 33, UiTheme::Text);

  display_.drawRoundRect(center - 28, y + 132, 56, 20, 3, UiTheme::Border);
  display_.setTextSize(1);
  display_.setCursor(center - 22, y + 138);
  display_.print(static_cast<int>(valuesKpa_[index]));
  display_.print(" kPa");
  lastValueTenths_[index] = valueTenths;
}

void DashboardScreen::drawBottomBar(bool force) {
  const float diff = maxDifference();
  const int16_t diffTenths = static_cast<int16_t>(diff * 10.0f);
  if (!force && diffTenths == lastDiffTenths_) {
    return;
  }

  const bool alarm = diff > AlarmThresholdKpa;
  display_.fillRect(8, 205, 304, 33, UiTheme::Background);

  UiTheme::drawPanel(display_, 10, 207, 94, 30);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(20, 212);
  display_.print("MAX VERSCHIL");
  display_.setTextColor(alarm ? UiTheme::Danger : UiTheme::Good);
  display_.setTextSize(2);
  display_.setCursor(38, 224);
  display_.print(static_cast<int>(diff));
  display_.print(" kPa");

  UiTheme::drawPanel(display_, 112, 207, 94, 30);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(142, 212);
  display_.print("STATUS");
  display_.setTextSize(1);
  display_.setTextColor(alarm ? UiTheme::Danger : UiTheme::Good);
  display_.setCursor(alarm ? 128 : 146, 226);
  display_.print(alarm ? "BIJSTELLEN" : "GOED");

  display_.fillRoundRect(MenuX, MenuY, MenuW, MenuH, 4, UiTheme::AccentDark);
  display_.drawRoundRect(MenuX, MenuY, MenuW, MenuH, 4, UiTheme::Accent);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(MenuX + 10, MenuY + 8);
  display_.print("MENU *");
  lastDiffTenths_ = diffTenths;
}

void DashboardScreen::updateDemoValues(uint32_t nowMs) {
  const float t = nowMs / 1000.0f;
  valuesKpa_[0] = -42.0f + sinf(t * 1.1f) * 4.0f;
  valuesKpa_[1] = -40.0f + sinf(t * 0.9f + 1.0f) * 3.5f;
  valuesKpa_[2] = -41.0f + sinf(t * 1.3f + 2.0f) * 4.2f;
  valuesKpa_[3] = -44.0f + sinf(t * 0.8f + 3.0f) * 5.0f;
}

float DashboardScreen::maxDifference() const {
  float lowest = valuesKpa_[0];
  float highest = valuesKpa_[0];
  for (uint8_t index = 1; index < cylinderCount_; ++index) {
    lowest = min(lowest, valuesKpa_[index]);
    highest = max(highest, valuesKpa_[index]);
  }
  return highest - lowest;
}

uint8_t DashboardScreen::outlierIndex() const {
  uint8_t outlier = 0;
  float farthest = 0.0f;
  float average = 0.0f;
  for (uint8_t index = 0; index < cylinderCount_; ++index) {
    average += valuesKpa_[index];
  }
  average /= cylinderCount_;

  for (uint8_t index = 0; index < cylinderCount_; ++index) {
    const float distance = fabsf(valuesKpa_[index] - average);
    if (distance > farthest) {
      farthest = distance;
      outlier = index;
    }
  }
  return outlier;
}
