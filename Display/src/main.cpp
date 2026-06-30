#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <CarbtuneShared.h>
#include "BoardConfig.h"
#include "CarbtuneScreen.h"
#include "DisplayColors.h"
#include "SelfTest.h"
#include "SplashScreen.h"
#include "TouchDiagnosticsScreen.h"
#include "TouchInput.h"
#include "version.h"

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static SPIClass sdSpi(VSPI);
static XPT2046_Touchscreen touch(TOUCH_CS);
static TouchInput touchInput(touch);
static SelfTest selfTest(*gfx, touchInput, sdSpi);
static CarbtuneScreen carbtuneScreen(*gfx);
static SplashScreen splashScreen(*gfx);
static TouchDiagnosticsScreen touchDiagnosticsScreen(*gfx, touchInput);
static bool splashVisible = false;
static bool dashboardStarted = false;
static bool dashboardVisible = false;
static bool diagnosticsVisible = false;
static bool menuWasPressed = false;

static void showSelfTestScreen() {
  selfTest.run();
  dashboardVisible = false;
}

static void showDashboard() {
  carbtuneScreen.begin();
  dashboardStarted = true;
  dashboardVisible = true;
  diagnosticsVisible = false;
}

static void showTouchDiagnosticsScreen(const TouchState &touchState) {
  touchDiagnosticsScreen.begin();
  touchDiagnosticsScreen.update(touchState);
  dashboardVisible = false;
  diagnosticsVisible = true;
}

static bool isMenuTouchTarget(const TouchState &touchState) {
  return touchState.pressed &&
         touchState.screenX >= 0 && touchState.screenX < 112 &&
         touchState.screenY >= 198 && touchState.screenY < 240;
}

static void handleTouch(const TouchState &touchState) {
  const bool menuPressed = isMenuTouchTarget(touchState);
  if (menuPressed && !menuWasPressed) {
    if (dashboardVisible) {
      showTouchDiagnosticsScreen(touchState);
    } else {
      showDashboard();
    }

    menuWasPressed = true;
    return;
  }

  menuWasPressed = menuPressed;

  if (dashboardVisible) {
    if (touchState.pressed) {
      carbtuneScreen.showTouchStatus(touchState.screenX, touchState.screenY);
    } else {
      carbtuneScreen.showNotPressed();
    }
  }

  if (diagnosticsVisible) {
    touchDiagnosticsScreen.update(touchState);
  }
}

void setup() {
  Serial.begin(Carbtune::UartBaud);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  gfx->begin();
  touchInput.begin();

  Serial.println();
  Serial.println(FW_NAME);
  Serial.println(FW_VERSION);
  Serial.println(FW_BUILD);

  splashScreen.begin(millis());
  splashVisible = true;
}

void loop() {
  const uint32_t nowMs = millis();
  const TouchState touchState = touchInput.update(nowMs);

  if (splashVisible) {
    if (splashScreen.update(nowMs)) {
      splashVisible = false;
      showDashboard();
    }
    return;
  }

  if (!dashboardStarted && !dashboardVisible && !diagnosticsVisible) {
    return;
  }

  handleTouch(touchState);

  if (dashboardVisible) {
    carbtuneScreen.update(nowMs);
  }
}
