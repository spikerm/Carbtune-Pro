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
#include "Sensors/SensorManager.h"
#include "SettingsScreen.h"
#include "SplashScreen.h"
#include "Storage/SdManager.h"
#include "Network/OtaManager.h"
#include "Network/WebInterface.h"
#include "TouchDiagnosticsScreen.h"
#include "TouchInput.h"

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static XPT2046_Touchscreen touch(TOUCH_CS);

static SettingsManager settingsManager;
static BacklightManager backlightManager(settingsManager);
static SensorManager sensorManager(settingsManager);
static SdManager sdManager;
static OtaManager otaManager;
static WebInterface webInterface(settingsManager, otaManager, sensorManager, sdManager);
static TouchInput touchInput(touch);
static CalibrationScreen calibrationScreen(*gfx);
static DashboardScreen dashboardScreen(*gfx, sensorManager);
static GraphScreen graphScreen(*gfx);
static SettingsScreen settingsScreen(*gfx, settingsManager, sdManager);
static SplashScreen splashScreen(*gfx, sdManager);
static TouchDiagnosticsScreen touchDiagnosticsScreen(*gfx, touchInput, backlightManager, sensorManager,
                                                     sdManager);
static ScreenManager screenManager(splashScreen, dashboardScreen, settingsScreen, graphScreen,
                                   calibrationScreen, touchDiagnosticsScreen, touchInput);
static AppController appController(*gfx, touchInput, backlightManager, sensorManager, sdManager,
                                   webInterface, screenManager);

void setup() {
  settingsManager.begin();
  appController.begin();
}

void loop() {
  appController.update();
}
