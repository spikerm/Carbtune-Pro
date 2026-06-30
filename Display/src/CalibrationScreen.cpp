#include "CalibrationScreen.h"

#include "UiButton.h"
#include "UiTheme.h"

namespace {
constexpr UiRect BackRect{8, 5, 32, 22};
constexpr UiRect StartRect{84, 188, 152, 38};

void drawHeader(Arduino_GFX &display) {
  display.fillScreen(UiTheme::Background);
  display.fillRect(0, 0, UiTheme::ScreenWidth, 30, 0x0208);
  display.drawLine(0, 30, UiTheme::ScreenWidth, 30, UiTheme::Border);
  UiButton(BackRect, "<").draw(display);
  display.setTextColor(UiTheme::Text);
  display.setTextSize(2);
  display.setCursor(66, 8);
  display.print("KALIBRATIE");
  display.setTextSize(1);
  display.setCursor(192, 12);
  display.print("(0 kPa)");
}
}  // namespace

CalibrationScreen::CalibrationScreen(Arduino_GFX &display) : display_(display) {}

void CalibrationScreen::begin() {
  drawHeader(display_);
  display_.drawLine(16, 44, 304, 44, UiTheme::Border);

  display_.setTextColor(UiTheme::Text);
  display_.setTextSize(1);
  display_.setCursor(60, 66);
  display_.print("ZORG DAT ALLE SLANGEN LOS ZIJN");
  display_.setCursor(48, 84);
  display_.print("VAN DE MOTOR EN IN DE BUITENLUCHT.");
  display_.setCursor(82, 102);
  display_.print("DRUK DAN OP START.");

  display_.drawCircle(160, 134, 18, 0x07FF);
  display_.drawCircle(140, 146, 16, 0x07FF);
  display_.drawCircle(180, 146, 16, 0x07FF);
  display_.drawFastHLine(128, 160, 64, 0x07FF);

  display_.setCursor(108, 172);
  display_.print("HUIDIGE DRUK");
  display_.setTextSize(3);
  display_.setCursor(104, 146);
  display_.print("0.0");
  display_.setTextSize(2);
  display_.setCursor(180, 154);
  display_.print("kPa");

  display_.fillRoundRect(StartRect.x, StartRect.y, StartRect.w, StartRect.h, 4, UiTheme::Good);
  display_.drawRoundRect(StartRect.x, StartRect.y, StartRect.w, StartRect.h, 4, UiTheme::Text);
  display_.setTextColor(UiTheme::Text);
  display_.setTextSize(2);
  display_.setCursor(128, 201);
  display_.print("START");
}

bool CalibrationScreen::isBackHit(int16_t x, int16_t y) const {
  return UiButton(BackRect, "").contains(x, y);
}

bool CalibrationScreen::isStartHit(int16_t x, int16_t y) const {
  return UiButton(StartRect, "").contains(x, y);
}
