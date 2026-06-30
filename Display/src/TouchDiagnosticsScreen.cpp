#include "TouchDiagnosticsScreen.h"

#include "DisplayColors.h"

static constexpr int16_t ValueX = 126;
static constexpr int16_t RowHeight = 18;
static constexpr int16_t MenuX = 8;
static constexpr int16_t MenuY = 218;
static constexpr int16_t MenuWidth = 76;
static constexpr int16_t MenuHeight = 20;

TouchDiagnosticsScreen::TouchDiagnosticsScreen(Arduino_GFX &display, TouchInput &touchInput)
    : display_(display), touchInput_(touchInput) {}

void TouchDiagnosticsScreen::begin() {
  firstDraw_ = true;
  lastState_ = {};
  drawStatic();
}

void TouchDiagnosticsScreen::update(const TouchState &touchState) {
  drawRow("raw X", String(touchState.rawX), 54, firstDraw_ || touchState.rawX != lastState_.rawX);
  drawRow("raw Y", String(touchState.rawY), 54 + RowHeight, firstDraw_ || touchState.rawY != lastState_.rawY);
  drawRow("pressure Z", String(touchState.rawZ), 54 + (RowHeight * 2), firstDraw_ || touchState.rawZ != lastState_.rawZ);
  drawRow("screen X", touchState.pressed ? String(touchState.screenX) : "-", 54 + (RowHeight * 3),
          firstDraw_ || touchState.screenX != lastState_.screenX || touchState.pressed != lastState_.pressed);
  drawRow("screen Y", touchState.pressed ? String(touchState.screenY) : "-", 54 + (RowHeight * 4),
          firstDraw_ || touchState.screenY != lastState_.screenY || touchState.pressed != lastState_.pressed);
  drawRow("pressed", touchState.pressed ? "true" : "false", 54 + (RowHeight * 5),
          firstDraw_ || touchState.pressed != lastState_.pressed);
  drawRow("long press", touchState.longPressed ? "true" : "false", 54 + (RowHeight * 6),
          firstDraw_ || touchState.longPressed != lastState_.longPressed);
  drawRow("IRQ", touchState.irqActive ? "LOW active" : "HIGH idle", 54 + (RowHeight * 7),
          firstDraw_ || touchState.irqActive != lastState_.irqActive);

  lastState_ = touchState;
  firstDraw_ = false;
}

void TouchDiagnosticsScreen::drawStatic() {
  display_.fillScreen(ColorBlack);
  display_.setTextSize(2);
  display_.setTextColor(ColorCyan);
  display_.setCursor(34, 10);
  display_.print("TOUCH DIAGNOSTICS");

  display_.setTextSize(1);
  display_.setTextColor(ColorWhite);
  display_.setCursor(10, 34);
  display_.print("Controller: ");
  display_.print(touchInput_.controllerName());

  display_.drawFastHLine(8, 48, 304, ColorDarkGrey);
  display_.drawFastHLine(8, 206, 304, ColorDarkGrey);
  drawMenu();
}

void TouchDiagnosticsScreen::drawRow(const char *label, const String &value, int16_t y, bool force) {
  if (!force) {
    return;
  }

  display_.fillRect(ValueX, y, 180, 12, ColorBlack);
  display_.setTextSize(1);
  display_.setTextColor(ColorDarkGrey);
  display_.setCursor(12, y);
  display_.print(label);
  display_.setTextColor(ColorWhite);
  display_.setCursor(ValueX, y);
  display_.print(value);
}

void TouchDiagnosticsScreen::drawMenu() {
  display_.fillRoundRect(MenuX, MenuY, MenuWidth, MenuHeight, 3, ColorYellow);
  display_.drawRoundRect(MenuX, MenuY, MenuWidth, MenuHeight, 3, ColorWhite);
  display_.setTextSize(2);
  display_.setTextColor(ColorBlack);
  display_.setCursor(MenuX + 12, MenuY + 3);
  display_.print("MENU");
}
