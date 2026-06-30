#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <CarbtuneShared.h>
#include "BoardConfig.h"
#include "App/SettingsManager.h"
#include "CalibrationScreen.h"
#include "DashboardScreen.h"
#include "DisplayColors.h"
#include "GraphScreen.h"
#include "Sensors/BacklightManager.h"
#include "SelfTest.h"
#include "SettingsScreen.h"
#include "SplashScreen.h"
#include "TouchDiagnosticsScreen.h"
#include "TouchInput.h"
#include "UiTheme.h"
#include "version.h"

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static SPIClass sdSpi(VSPI);
static XPT2046_Touchscreen touch(TOUCH_CS);
static SettingsManager settingsManager;
static BacklightManager backlightManager(settingsManager);
static TouchInput touchInput(touch);
static SelfTest selfTest(*gfx, touchInput, sdSpi);
static CalibrationScreen calibrationScreen(*gfx);
static DashboardScreen dashboardScreen(*gfx);
static GraphScreen graphScreen(*gfx);
static SettingsScreen settingsScreen(*gfx);
static SplashScreen splashScreen(*gfx);
static TouchDiagnosticsScreen touchDiagnosticsScreen(*gfx, touchInput, backlightManager);

enum class ScreenId {
  Splash,
  Dashboard,
  Settings,
  Graph,
  Calibration,
  Diagnostics,
};

static ScreenId currentScreen = ScreenId::Splash;
static bool splashVisible = false;
static bool touchWasPressed = false;

static bool isDashboardMenuTarget(int16_t x, int16_t y) {
  return x >= 230 && x < 320 && y >= 195 && y < 240;
}

static void showSelfTestScreen() {
  selfTest.run();
  currentScreen = ScreenId::Diagnostics;
}

static void showDashboard() {
  dashboardScreen.begin();
  currentScreen = ScreenId::Dashboard;
}

static void showTouchDiagnosticsScreen(const TouchState &touchState) {
  touchDiagnosticsScreen.begin();
  touchDiagnosticsScreen.update(touchState);
  currentScreen = ScreenId::Diagnostics;
}

static void showSettings() {
  settingsScreen.begin();
  currentScreen = ScreenId::Settings;
}

static void showGraph() {
  graphScreen.begin();
  currentScreen = ScreenId::Graph;
}

static void showCalibration() {
  calibrationScreen.begin();
  currentScreen = ScreenId::Calibration;
}

static void handleSerialInput() {
  while (Serial.available() > 0) {
    const char command = static_cast<char>(Serial.read());
    switch (command) {
      case 'h':
      case 'H':
        showDashboard();
        break;
      case 's':
      case 'S':
        showSettings();
        break;
      case 'g':
      case 'G':
        showGraph();
        break;
      case 'c':
      case 'C':
        showCalibration();
        break;
      case 'd':
      case 'D':
        showTouchDiagnosticsScreen(touchInput.current());
        break;
      default:
        break;
    }
  }
}

static void handleTouch(const TouchState &touchState) {
  const bool touchStarted = touchState.pressed && !touchWasPressed;
  if (touchState.changed) {
    const bool menuHit = touchState.pressed &&
                         isDashboardMenuTarget(touchState.screenX, touchState.screenY);
    Serial.print("Touch mapped=");
    Serial.print(touchState.screenX);
    Serial.print(",");
    Serial.print(touchState.screenY);
    Serial.print(touchState.pressed ? " pressed" : " released");
    Serial.print(" MENU=");
    Serial.println(menuHit ? "yes" : "no");
  }

  touchWasPressed = touchState.pressed;

  if (!touchStarted) {
    if (currentScreen == ScreenId::Dashboard) {
      if (touchState.pressed) {
        dashboardScreen.showTouchStatus(touchState.screenX, touchState.screenY);
      } else {
        dashboardScreen.showNotPressed();
      }
    } else if (currentScreen == ScreenId::Diagnostics) {
      touchDiagnosticsScreen.update(touchState);
    }
    return;
  }

  const int16_t x = touchState.screenX;
  const int16_t y = touchState.screenY;

  switch (currentScreen) {
    case ScreenId::Dashboard:
      dashboardScreen.showTouchStatus(x, y);
      if (isDashboardMenuTarget(x, y)) {
        showSettings();
      }
      break;
    case ScreenId::Settings:
      if (settingsScreen.isHomeHit(x, y)) {
        showDashboard();
      } else if (settingsScreen.isGraphHit(x, y)) {
        showGraph();
      } else if (settingsScreen.isCalibrationHit(x, y)) {
        showCalibration();
      }
      break;
    case ScreenId::Graph:
      if (graphScreen.isHomeHit(x, y)) {
        showDashboard();
      } else if (graphScreen.isSettingsHit(x, y)) {
        showSettings();
      }
      break;
    case ScreenId::Calibration:
      if (calibrationScreen.isBackHit(x, y)) {
        showSettings();
      }
      break;
    case ScreenId::Diagnostics:
      touchDiagnosticsScreen.update(touchState);
      break;
    case ScreenId::Splash:
      break;
  }
}

void setup() {
  Serial.begin(Carbtune::UartBaud);
  settingsManager.begin();
  backlightManager.begin();
  gfx->begin();
  gfx->invertDisplay(TFT_INVERT_COLORS);
  gfx->fillScreen(UiTheme::Background);
  touchInput.begin();

  Serial.println();
  Serial.println(FW_NAME);
  Serial.println(FW_VERSION);
  Serial.println(FW_BUILD);

  splashScreen.begin(millis());
  splashVisible = true;
  currentScreen = ScreenId::Splash;
}

void loop() {
  const uint32_t nowMs = millis();
  backlightManager.update(nowMs);
  const TouchState touchState = touchInput.update(nowMs);

  if (splashVisible) {
    handleSerialInput();
    if (splashScreen.update(nowMs)) {
      splashVisible = false;
      showDashboard();
    }
    return;
  }

  handleSerialInput();
  handleTouch(touchState);

  if (currentScreen == ScreenId::Dashboard) {
    dashboardScreen.update(nowMs);
  }
}
