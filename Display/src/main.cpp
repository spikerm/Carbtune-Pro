#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <CarbtuneShared.h>
#include "BoardConfig.h"
#include "DisplayColors.h"
#include "SelfTest.h"
#include "version.h"

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static SPIClass sdSpi(VSPI);
static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
static SelfTest selfTest(*gfx, touch, sdSpi);
static uint32_t lastStatusMs = 0;

void setup() {
  Serial.begin(Carbtune::UartBaud);

  Serial.println();
  Serial.println(FW_NAME);
  Serial.println(FW_VERSION);
  Serial.println(FW_BUILD);

  selfTest.run();
}

void loop() {
  if (millis() - lastStatusMs < 500) {
    return;
  }

  lastStatusMs = millis();
  if (touch.touched()) {
    const TS_Point point = touch.getPoint();
    gfx->fillRect(0, 216, 320, 24, ColorBlack);
    gfx->setTextColor(ColorYellow);
    gfx->setTextSize(1);
    gfx->setCursor(8, 224);
    gfx->print("Touch ");
    gfx->print(point.x);
    gfx->print(",");
    gfx->print(point.y);
  }

}
