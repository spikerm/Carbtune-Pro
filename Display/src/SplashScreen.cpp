#include "SplashScreen.h"

#include <math.h>

#include "UiTheme.h"
#include "version.h"

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

  display_.setTextSize(2);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(58, 28);
  display_.print("CARBTUNE");
  display_.setCursor(126, 50);
  display_.print("PRO");
  display_.setCursor(132, 74);
  display_.print("ESP32");

  display_.setTextSize(1);
  display_.setCursor(94, 100);
  display_.print("Firmware ");
  display_.print(FW_VERSION);

  display_.setCursor(112, 120);
  display_.print("INITIALIZING...");

  display_.drawFastHLine(36, 136, 248, UiTheme::PanelBorder);
  display_.setCursor(56, 148);
  display_.setTextColor(UiTheme::GoodGreen);
  display_.print("Display OK");
  display_.setCursor(56, 162);
  display_.print("Touch   OK");
  display_.setCursor(56, 176);
  display_.print("UART    OK");
  display_.setCursor(56, 190);
  display_.setTextColor(UiTheme::WarnYellow);
  display_.print("SD      -");
  display_.setCursor(56, 204);
  display_.setTextColor(UiTheme::AccentBlue);
  display_.print("Sensors DEMO");

  display_.setTextColor(UiTheme::AlarmRed);
  display_.drawFastHLine(72, 68, 176, UiTheme::AlarmRed);
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
    const uint16_t color = angle > 295 ? UiTheme::AlarmRed : UiTheme::Text;
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
    display_.fillCircle(x, 226, 4, dot <= activeDots ? UiTheme::AlarmRed : UiTheme::TextMuted);
  }
}
