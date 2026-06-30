#include "SettingsScreen.h"

#include "UiButton.h"
#include "UiTheme.h"

namespace {
constexpr UiRect HomeRect{8, 205, 96, 30};
constexpr UiRect GraphRect{112, 205, 96, 30};
constexpr UiRect SettingsRect{216, 205, 96, 30};
constexpr UiRect CalibrationRect{238, 176, 58, 18};

void drawHeader(Arduino_GFX &display) {
  display.fillScreen(UiTheme::Background);
  display.fillRect(0, 0, UiTheme::ScreenWidth, 30, 0x0208);
  display.drawLine(0, 30, UiTheme::ScreenWidth, 30, UiTheme::Border);
  UiButton({8, 5, 32, 22}, "<").draw(display);
  display.setTextColor(UiTheme::Text);
  display.setTextSize(2);
  display.setCursor(106, 8);
  display.print("INSTELLINGEN");
}

void drawRowLabel(Arduino_GFX &display, const char *label, int16_t y) {
  display.setTextColor(UiTheme::Text);
  display.setTextSize(1);
  display.setCursor(14, y);
  display.print(label);
}
}  // namespace

SettingsScreen::SettingsScreen(Arduino_GFX &display) : display_(display) {}

void SettingsScreen::begin() {
  drawHeader(display_);

  UiTheme::drawPanel(display_, 8, 36, 304, 157);
  drawRowLabel(display_, "AANTAL CILINDERS", 48);
  UiButton({14, 64, 50, 26}, "1").draw(display_);
  UiButton({78, 64, 50, 26}, "2").draw(display_);
  UiButton({142, 64, 50, 26}, "3").draw(display_);
  UiButton({206, 64, 50, 26}, "4").draw(display_, true);

  display_.drawLine(8, 98, 312, 98, UiTheme::Border);
  drawRowLabel(display_, "EENHEDEN", 107);
  UiButton({176, 102, 58, 22}, "kPa").draw(display_, true);
  UiButton({238, 102, 58, 22}, "inHg").draw(display_);

  display_.drawLine(8, 130, 312, 130, UiTheme::Border);
  drawRowLabel(display_, "AUTO SCHAAL", 139);
  display_.fillRoundRect(266, 134, 32, 17, 8, UiTheme::Good);
  display_.fillCircle(289, 142, 7, UiTheme::Text);

  display_.drawLine(8, 154, 312, 154, UiTheme::Border);
  drawRowLabel(display_, "DEMPING / FILTER", 160);
  display_.setCursor(246, 160);
  display_.print("0.7 s");
  display_.setCursor(290, 160);
  display_.print(">");

  display_.drawLine(8, 172, 312, 172, UiTheme::Border);
  drawRowLabel(display_, "AUTO HELDERHEID", 178);
  display_.fillRoundRect(184, 174, 32, 17, 8, UiTheme::Good);
  display_.fillCircle(207, 182, 7, UiTheme::Text);
  drawRowLabel(display_, "HELDERHEID", 191);
  display_.setCursor(184, 191);
  display_.print("80%");

  drawRowLabel(display_, "KALIBRATIE", 202);
  UiButton(CalibrationRect, "START").draw(display_);

  UiButton(HomeRect, "HOME").draw(display_);
  UiButton(GraphRect, "GRAFIEK").draw(display_);
  UiButton(SettingsRect, "INSTELLINGEN").draw(display_, true);
}

bool SettingsScreen::isHomeHit(int16_t x, int16_t y) const {
  return UiButton(HomeRect, "").contains(x, y);
}

bool SettingsScreen::isGraphHit(int16_t x, int16_t y) const {
  return UiButton(GraphRect, "").contains(x, y);
}

bool SettingsScreen::isSettingsHit(int16_t x, int16_t y) const {
  return UiButton(SettingsRect, "").contains(x, y);
}

bool SettingsScreen::isCalibrationHit(int16_t x, int16_t y) const {
  return UiButton(CalibrationRect, "").contains(x, y);
}
