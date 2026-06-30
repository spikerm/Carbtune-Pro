#include "SplashScreen.h"

#include <math.h>

#include "UiTheme.h"

static constexpr uint32_t SplashDurationMs = 2000;

SplashScreen::SplashScreen(Arduino_GFX &display) : display_(display) {}

void SplashScreen::begin(uint32_t nowMs) {
  startedMs_ = nowMs;
  lastDots_ = 255;
  drawStatic();
}

bool SplashScreen::update(uint32_t nowMs) {
  const uint32_t elapsed = nowMs - startedMs_;
  const uint8_t dots = min<uint8_t>(7, elapsed / 320);
  drawProgress(dots);
  return elapsed >= SplashDurationMs;
}

void SplashScreen::drawStatic() {
  display_.fillScreen(UiTheme::Background);
  drawGauge();

  display_.setTextSize(3);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(48, 90);
  display_.print("CARB");
  display_.setTextColor(UiTheme::Danger);
  display_.print("TUNE");

  display_.drawFastHLine(70, 122, 58, UiTheme::Danger);
  display_.drawFastHLine(190, 122, 58, UiTheme::Danger);

  display_.setTextSize(2);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(134, 128);
  display_.print("PRO");

  display_.setCursor(128, 162);
  display_.print("ESP32");

  display_.setTextSize(1);
  display_.setCursor(116, 198);
  display_.print("INITIALIZING...");
}

void SplashScreen::drawGauge() {
  const int16_t cx = 160;
  const int16_t cy = 84;
  const int16_t radius = 48;

  for (int16_t angle = 205; angle <= 335; angle += 10) {
    const float rad = angle * DEG_TO_RAD;
    const int16_t x1 = cx + cosf(rad) * (radius - 7);
    const int16_t y1 = cy + sinf(rad) * (radius - 7);
    const int16_t x2 = cx + cosf(rad) * radius;
    const int16_t y2 = cy + sinf(rad) * radius;
    const uint16_t color = angle > 295 ? UiTheme::Danger : UiTheme::Text;
    display_.drawLine(x1, y1, x2, y2, color);
    display_.drawLine(x1 + 1, y1, x2 + 1, y2, color);
  }

  display_.fillCircle(cx, cy, 7, UiTheme::Text);
  display_.drawLine(cx, cy, cx + 26, cy - 26, UiTheme::Text);
  display_.drawLine(cx + 1, cy, cx + 27, cy - 26, UiTheme::Text);
}

void SplashScreen::drawProgress(uint8_t activeDots) {
  if (activeDots == lastDots_) {
    return;
  }

  lastDots_ = activeDots;
  for (uint8_t dot = 0; dot < 7; ++dot) {
    const int16_t x = 122 + (dot * 13);
    display_.fillCircle(x, 222, 4, dot <= activeDots ? UiTheme::Danger : UiTheme::Muted);
  }
}
