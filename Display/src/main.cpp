#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <CarbtuneShared.h>
#include "BoardConfig.h"
#include "CarbtuneScreen.h"
#include "DisplayColors.h"
#include "SelfTest.h"
#include "TouchInput.h"
#include "version.h"

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static SPIClass sdSpi(VSPI);
static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
static TouchInput touchInput(touch);
static SelfTest selfTest(*gfx, touchInput, sdSpi);
static CarbtuneScreen carbtuneScreen(*gfx);
static uint32_t selfTestShownMs = 0;
static bool dashboardStarted = false;
static bool dashboardVisible = false;
static bool menuWasPressed = false;

static void showSelfTestScreen() {
  selfTest.run();
  dashboardVisible = false;
}

static void showDashboard() {
  carbtuneScreen.begin();
  dashboardStarted = true;
  dashboardVisible = true;
}

static void handleTouch(const TouchState &touchState) {
  if (dashboardVisible) {
    if (touchState.pressed) {
      carbtuneScreen.showTouchStatus(touchState.screenX, touchState.screenY);
    } else {
      carbtuneScreen.showNotPressed();
    }
  }

  const bool menuPressed = touchState.pressed &&
                           carbtuneScreen.isMenuHit(touchState.screenX, touchState.screenY);
  if (menuPressed && !menuWasPressed) {
    if (dashboardVisible) {
      showSelfTestScreen();
    } else {
      showDashboard();
    }
  }

  menuWasPressed = menuPressed;
}

void setup() {
  Serial.begin(Carbtune::UartBaud);

  Serial.println();
  Serial.println(FW_NAME);
  Serial.println(FW_VERSION);
  Serial.println(FW_BUILD);

  showSelfTestScreen();
  selfTestShownMs = millis();
}

void loop() {
  const uint32_t nowMs = millis();
  const TouchState touchState = touchInput.update(nowMs);

  if (!dashboardStarted && !dashboardVisible) {
    if (nowMs - selfTestShownMs >= 5000) {
      showDashboard();
    }
    return;
  }

  handleTouch(touchState);

  if (dashboardVisible) {
    carbtuneScreen.update(nowMs);
  }
}
