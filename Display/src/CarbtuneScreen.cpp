#include "CarbtuneScreen.h"

#include <math.h>

#include "DisplayColors.h"

static constexpr int16_t ScreenWidth = 320;
static constexpr int16_t ScreenHeight = 240;
static constexpr int16_t BarX = 64;
static constexpr int16_t BarWidth = 184;
static constexpr int16_t BarHeight = 16;
static constexpr int16_t ValueX = 258;
static constexpr int16_t MenuX = 8;
static constexpr int16_t MenuY = 218;
static constexpr int16_t MenuWidth = 76;
static constexpr int16_t MenuHeight = 20;
static constexpr int16_t TouchStatusX = 96;
static constexpr int16_t TouchStatusY = 224;
static constexpr int16_t DeltaY = 210;
static constexpr int16_t ChannelY[4] = {70, 105, 140, 175};
static constexpr float MinKpa = 0.0f;
static constexpr float MaxKpa = 80.0f;

CarbtuneScreen::CarbtuneScreen(Arduino_GFX &display) : display_(display) {}

void CarbtuneScreen::begin() {
  for (uint8_t channel = 0; channel < 4; ++channel) {
  lastBarWidth_[channel] = -1;
    lastValueCentiKpa_[channel] = -32768;
  }
  lastDeltaCentiKpa_ = -32768;
  lastTouchX_ = -2;
  lastTouchY_ = -2;
  lastUpdateMs_ = 0;

  drawStatic();
  update(millis());
}

void CarbtuneScreen::update(uint32_t nowMs) {
  if (nowMs - lastUpdateMs_ < 50) {
    return;
  }

  lastUpdateMs_ = nowMs;
  updateDemoValues(nowMs);

  for (uint8_t channel = 0; channel < 4; ++channel) {
    drawChannel(channel);
  }

  drawDelta();
}

void CarbtuneScreen::drawStatic() {
  display_.fillScreen(ColorBlack);
  display_.setTextColor(ColorCyan);
  display_.setTextSize(2);
  display_.setCursor(78, 10);
  display_.print("CARBTUNE PRO");

  display_.drawFastHLine(8, 36, ScreenWidth - 16, ColorDarkGrey);

  display_.setTextSize(1);
  display_.setTextColor(ColorWhite);
  display_.setCursor(260, 42);
  display_.print("kPa");

  for (uint8_t channel = 0; channel < 4; ++channel) {
    const int16_t y = ChannelY[channel];
    display_.setTextColor(ColorWhite);
    display_.setTextSize(2);
    display_.setCursor(12, y);
    display_.print("CH");
    display_.print(channel + 1);

    display_.drawRoundRect(BarX, y, BarWidth, BarHeight, 3, ColorWhite);
  }

  display_.drawFastHLine(8, 202, ScreenWidth - 16, ColorDarkGrey);
  drawMenu();
  showNotPressed();
}

void CarbtuneScreen::updateDemoValues(uint32_t nowMs) {
  const float t = nowMs / 1000.0f;
  valuesKpa_[0] = 42.0f + (sinf(t * 1.1f) * 8.0f);
  valuesKpa_[1] = 45.0f + (sinf(t * 0.9f + 1.0f) * 7.0f);
  valuesKpa_[2] = 40.0f + (sinf(t * 1.3f + 2.0f) * 9.0f);
  valuesKpa_[3] = 44.0f + (sinf(t * 0.8f + 3.0f) * 6.0f);
}

void CarbtuneScreen::drawChannel(uint8_t channel) {
  const int16_t y = ChannelY[channel];
  const float value = constrain(valuesKpa_[channel], MinKpa, MaxKpa);
  const int16_t barFill = static_cast<int16_t>((value - MinKpa) * (BarWidth - 4) / (MaxKpa - MinKpa));
  const int16_t centiKpa = static_cast<int16_t>(value * 100.0f);

  if (barFill != lastBarWidth_[channel]) {
    display_.fillRect(BarX + 2, y + 2, BarWidth - 4, BarHeight - 4, ColorBlack);
    display_.fillRect(BarX + 2, y + 2, barFill, BarHeight - 4, ColorGreen);
    lastBarWidth_[channel] = barFill;
  }

  if (centiKpa != lastValueCentiKpa_[channel]) {
    display_.fillRect(ValueX, y - 1, 58, 20, ColorBlack);
    display_.setTextColor(ColorWhite);
    display_.setTextSize(2);
    display_.setCursor(ValueX, y);
    display_.print(value, 1);
    lastValueCentiKpa_[channel] = centiKpa;
  }
}

void CarbtuneScreen::drawDelta() {
  float lowest = valuesKpa_[0];
  float highest = valuesKpa_[0];

  for (uint8_t channel = 1; channel < 4; ++channel) {
    lowest = min(lowest, valuesKpa_[channel]);
    highest = max(highest, valuesKpa_[channel]);
  }

  const float delta = highest - lowest;
  const int16_t centiKpa = static_cast<int16_t>(delta * 100.0f);
  if (centiKpa == lastDeltaCentiKpa_) {
    return;
  }

  display_.fillRect(96, DeltaY, 156, 20, ColorBlack);
  display_.setTextSize(2);
  display_.setTextColor(ColorWhite);
  display_.setCursor(96, DeltaY);
  display_.print("DELTA");
  display_.setCursor(172, DeltaY);
  display_.print(delta, 1);
  display_.print(" kPa");
  lastDeltaCentiKpa_ = centiKpa;
}

void CarbtuneScreen::drawMenu() {
  display_.fillRoundRect(MenuX, MenuY, MenuWidth, MenuHeight, 3, ColorYellow);
  display_.drawRoundRect(MenuX, MenuY, MenuWidth, MenuHeight, 3, ColorWhite);
  display_.setTextSize(2);
  display_.setTextColor(ColorBlack);
  display_.setCursor(MenuX + 12, MenuY + 3);
  display_.print("MENU");
}

void CarbtuneScreen::showTouchStatus(int16_t x, int16_t y) {
  if (x == lastTouchX_ && y == lastTouchY_) {
    return;
  }

  display_.fillRect(TouchStatusX, TouchStatusY, 210, 12, ColorBlack);
  display_.setTextSize(1);
  display_.setTextColor(ColorWhite);
  display_.setCursor(TouchStatusX, TouchStatusY);
  display_.print("Touch ");
  display_.print(x);
  display_.print(",");
  display_.print(y);
  lastTouchX_ = x;
  lastTouchY_ = y;
}

void CarbtuneScreen::showNotPressed() {
  if (lastTouchX_ == -1 && lastTouchY_ == -1) {
    return;
  }

  display_.fillRect(TouchStatusX, TouchStatusY, 210, 12, ColorBlack);
  display_.setTextSize(1);
  display_.setTextColor(ColorDarkGrey);
  display_.setCursor(TouchStatusX, TouchStatusY);
  display_.print("Not pressed");
  lastTouchX_ = -1;
  lastTouchY_ = -1;
}
