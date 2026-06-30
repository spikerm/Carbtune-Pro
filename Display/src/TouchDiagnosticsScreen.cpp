#include "TouchDiagnosticsScreen.h"

#include "DisplayColors.h"

static constexpr int16_t ValueX = 118;
static constexpr int16_t RowHeight = 8;
static constexpr int16_t HomeX = 8;
static constexpr int16_t MenuX = 236;
static constexpr int16_t MenuY = 218;
static constexpr int16_t MenuWidth = 76;
static constexpr int16_t MenuHeight = 20;

TouchDiagnosticsScreen::TouchDiagnosticsScreen(Arduino_GFX &display, TouchInput &touchInput,
                                               BacklightManager &backlightManager,
                                               SensorManager &sensorManager, SdManager &sdManager)
    : display_(display),
      touchInput_(touchInput),
      backlightManager_(backlightManager),
      sensorManager_(sensorManager),
      sdManager_(sdManager) {}

void TouchDiagnosticsScreen::begin() {
  firstDraw_ = true;
  lastState_ = {};
  lastLdrRaw_ = 65535;
  lastFilteredLight_ = -1;
  lastBrightnessPercent_ = 255;
  drawStatic();
}

void TouchDiagnosticsScreen::update(const TouchState &touchState) {
  drawRow("raw X", String(touchState.rawX), 52, firstDraw_ || touchState.rawX != lastState_.rawX);
  drawRow("raw Y", String(touchState.rawY), 52 + RowHeight, firstDraw_ || touchState.rawY != lastState_.rawY);
  drawRow("pressure Z", String(touchState.rawZ), 52 + (RowHeight * 2), firstDraw_ || touchState.rawZ != lastState_.rawZ);
  drawRow("screen X", touchState.pressed ? String(touchState.screenX) : "-", 52 + (RowHeight * 3),
          firstDraw_ || touchState.screenX != lastState_.screenX || touchState.pressed != lastState_.pressed);
  drawRow("screen Y", touchState.pressed ? String(touchState.screenY) : "-", 52 + (RowHeight * 4),
          firstDraw_ || touchState.screenY != lastState_.screenY || touchState.pressed != lastState_.pressed);
  drawRow("pressed", touchState.pressed ? "true" : "false", 52 + (RowHeight * 5),
          firstDraw_ || touchState.pressed != lastState_.pressed);
  drawRow("long press", touchState.longPressed ? "true" : "false", 52 + (RowHeight * 6),
          firstDraw_ || touchState.longPressed != lastState_.longPressed);
  drawRow("IRQ", touchState.irqActive ? "LOW active" : "HIGH idle", 52 + (RowHeight * 7),
          firstDraw_ || touchState.irqActive != lastState_.irqActive);

  const uint16_t ldrRaw = backlightManager_.ldrRaw();
  const int16_t filteredLight = static_cast<int16_t>(backlightManager_.filteredLight());
  const uint8_t brightnessPercent = backlightManager_.brightnessPercent();
  const bool autoBrightness = backlightManager_.autoBrightness();
  const bool ldrWarning = backlightManager_.ldrWarning();
  drawRow("LDR raw", ldrWarning ? "0 / check pin" : String(ldrRaw), 52 + (RowHeight * 8),
          firstDraw_ || ldrRaw != lastLdrRaw_ || ldrWarning != lastLdrWarning_);
  drawRow("light filt", String(filteredLight), 52 + (RowHeight * 9),
          firstDraw_ || filteredLight != lastFilteredLight_);
  drawRow("backlight", String(brightnessPercent) + "%", 52 + (RowHeight * 10),
          firstDraw_ || brightnessPercent != lastBrightnessPercent_);
  drawRow("auto bright", autoBrightness ? "on" : "off", 52 + (RowHeight * 11),
          firstDraw_ || autoBrightness != lastAutoBrightness_);

  String deltas;
  const float reference = sensorManager_.valueKpa(0);
  for (uint8_t channel = 0; channel < sensorManager_.channelCount(); ++channel) {
    if (channel > 0) {
      deltas += ",";
    }
    deltas += String(sensorManager_.valueKpa(channel) - reference, 1);
  }
  drawRow("cylinders", String(sensorManager_.channelCount()), 52 + (RowHeight * 12), true);
  drawRow("CH1 ref", String(reference, 1) + " kPa", 52 + (RowHeight * 13), true);
  drawRow("CH deltas", deltas, 52 + (RowHeight * 14), true);
  drawRow("max delta", String(sensorManager_.maxDeltaKpa(), 1) + " kPa", 52 + (RowHeight * 15), true);
  drawRow("sensor mode", sensorManager_.modeName(), 52 + (RowHeight * 16), true);
  drawRow("raw", String(sensorManager_.rawValue(0)) + "," + String(sensorManager_.rawValue(1)) + "," +
                     String(sensorManager_.rawValue(2)) + "," + String(sensorManager_.rawValue(3)),
          52 + (RowHeight * 17), true);
  drawRow("filtered", String(sensorManager_.filteredValue(0)) + "," +
                          String(sensorManager_.filteredValue(1)) + "," +
                          String(sensorManager_.filteredValue(2)) + "," +
                          String(sensorManager_.filteredValue(3)),
          52 + (RowHeight * 18), true);
  drawRow("pulse Hz", String(sensorManager_.pulseHz(0), 1), 52 + (RowHeight * 19), true);
  drawRow("RPM", sensorManager_.rpmStable() ? String(sensorManager_.rpm()) : "--",
          52 + (RowHeight * 20), true);
  drawRow("RPM status", sensorManager_.rpmStable() ? "stable" : "unstable",
          52 + (RowHeight * 21), true);
  drawRow("SD mounted", sdManager_.mounted() ? "true" : "false", 52 + (RowHeight * 22), true);
  drawRow("SD type", sdManager_.cardTypeName(), 52 + (RowHeight * 23), true);
  drawRow("SD size", String(sdManager_.cardSizeMb()) + " MB", 52 + (RowHeight * 24), true);
  drawRow("SD error", sdManager_.lastErrorName(), 52 + (RowHeight * 25), true);
  drawRow("SD attempts", String(sdManager_.initAttempts()), 52 + (RowHeight * 26), true);

  lastState_ = touchState;
  lastLdrRaw_ = ldrRaw;
  lastFilteredLight_ = filteredLight;
  lastBrightnessPercent_ = brightnessPercent;
  lastAutoBrightness_ = autoBrightness;
  lastLdrWarning_ = ldrWarning;
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
  display_.drawFastHLine(8, 222, 304, ColorDarkGrey);
  drawMenu();
}

void TouchDiagnosticsScreen::drawRow(const char *label, const String &value, int16_t y, bool force) {
  if (!force) {
    return;
  }

  display_.fillRect(ValueX, y, 190, 8, ColorBlack);
  display_.setTextSize(1);
  display_.setTextColor(ColorDarkGrey);
  display_.setCursor(12, y);
  display_.print(label);
  display_.setTextColor(ColorWhite);
  display_.setCursor(ValueX, y);
  display_.print(value);
}

void TouchDiagnosticsScreen::drawMenu() {
  display_.fillRoundRect(HomeX, MenuY, MenuWidth, MenuHeight, 3, ColorDarkGrey);
  display_.drawRoundRect(HomeX, MenuY, MenuWidth, MenuHeight, 3, ColorWhite);
  display_.setTextSize(2);
  display_.setTextColor(ColorWhite);
  display_.setCursor(HomeX + 12, MenuY + 3);
  display_.print("HOME");

  display_.fillRoundRect(MenuX, MenuY, MenuWidth, MenuHeight, 3, ColorYellow);
  display_.drawRoundRect(MenuX, MenuY, MenuWidth, MenuHeight, 3, ColorWhite);
  display_.setTextSize(2);
  display_.setTextColor(ColorBlack);
  display_.setCursor(MenuX + 12, MenuY + 3);
  display_.print("MENU");
}
