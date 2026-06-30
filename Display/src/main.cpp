#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

#include "App/AppController.h"
#include "App/SettingsManager.h"
#include "BoardConfig.h"
#include "CalibrationScreen.h"
#include "DashboardScreen.h"
#include "GraphScreen.h"
#include "Sensors/BacklightManager.h"
#include "SettingsScreen.h"
#include "SplashScreen.h"
#include "TouchDiagnosticsScreen.h"
#include "TouchInput.h"

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static XPT2046_Touchscreen touch(TOUCH_CS);

static SettingsManager settingsManager;
static BacklightManager backlightManager(settingsManager);
static TouchInput touchInput(touch);
static CalibrationScreen calibrationScreen(*gfx);
static DashboardScreen dashboardScreen(*gfx);
static GraphScreen graphScreen(*gfx);
static SettingsScreen settingsScreen(*gfx);
static SplashScreen splashScreen(*gfx);
static TouchDiagnosticsScreen touchDiagnosticsScreen(*gfx, touchInput, backlightManager);
static ScreenManager screenManager(splashScreen, dashboardScreen, settingsScreen, graphScreen,
                                   calibrationScreen, touchDiagnosticsScreen, touchInput);
static AppController appController(*gfx, touchInput, backlightManager, screenManager);

void setup() {
  settingsManager.begin();
  appController.begin();
}

void loop() {
  appController.update();
}
