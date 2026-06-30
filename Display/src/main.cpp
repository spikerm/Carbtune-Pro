#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <CarbtuneShared.h>
#include "BoardConfig.h"
#include "CarbtuneScreen.h"
#include "DisplayColors.h"
#include "SelfTest.h"
#include "version.h"

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static SPIClass sdSpi(VSPI);
static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
static SelfTest selfTest(*gfx, touch, sdSpi);
static CarbtuneScreen carbtuneScreen(*gfx);
static uint32_t selfTestShownMs = 0;
static bool dashboardStarted = false;

void setup() {
  Serial.begin(Carbtune::UartBaud);

  Serial.println();
  Serial.println(FW_NAME);
  Serial.println(FW_VERSION);
  Serial.println(FW_BUILD);

  selfTest.run();
  selfTestShownMs = millis();
}

void loop() {
  const uint32_t nowMs = millis();
  if (!dashboardStarted) {
    if (nowMs - selfTestShownMs >= 5000) {
      carbtuneScreen.begin();
      dashboardStarted = true;
    }
    return;
  }

  carbtuneScreen.update(nowMs);
}
