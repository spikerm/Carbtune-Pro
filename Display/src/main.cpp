#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SD.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <CarbtuneShared.h>
#include "BoardConfig.h"
#include "version.h"

struct SelfTestResult {
  const char *name;
  bool passed;
  String detail;
};

static Arduino_DataBus *displayBus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, TFT_MISO);
static Arduino_GFX *gfx = new Arduino_ILI9341(displayBus, TFT_RST, 1);
static SPIClass sdSpi(VSPI);
static XPT2046_Touchscreen touch(TOUCH_CS, TOUCH_IRQ);
static SelfTestResult results[7];
static uint8_t resultCount = 0;
static uint32_t lastStatusMs = 0;

static constexpr uint16_t ColorBlack = 0x0000;
static constexpr uint16_t ColorBlue = 0x001F;
static constexpr uint16_t ColorGreen = 0x07E0;
static constexpr uint16_t ColorRed = 0xF800;
static constexpr uint16_t ColorWhite = 0xFFFF;
static constexpr uint16_t ColorYellow = 0xFFE0;

static void setRgb(bool red, bool green, bool blue) {
  digitalWrite(LED_RED, red ? LOW : HIGH);
  digitalWrite(LED_GREEN, green ? LOW : HIGH);
  digitalWrite(LED_BLUE, blue ? LOW : HIGH);
}

static void addResult(const char *name, bool passed, const String &detail) {
  if (resultCount >= (sizeof(results) / sizeof(results[0]))) {
    return;
  }

  results[resultCount++] = {name, passed, detail};
}

static void drawResults() {
  gfx->fillScreen(ColorBlack);
  gfx->setTextSize(2);
  gfx->setTextColor(ColorWhite);
  gfx->setCursor(8, 8);
  gfx->print(FW_NAME);
  gfx->setCursor(8, 30);
  gfx->print(FW_VERSION);

  gfx->setTextSize(1);
  for (uint8_t index = 0; index < resultCount; ++index) {
    const int16_t y = 58 + (index * 22);
    gfx->fillCircle(14, y + 4, 5, results[index].passed ? ColorGreen : ColorRed);
    gfx->setCursor(28, y);
    gfx->setTextColor(results[index].passed ? ColorGreen : ColorRed);
    gfx->print(results[index].name);
    gfx->setTextColor(ColorWhite);
    gfx->setCursor(116, y);
    gfx->print(results[index].detail);
  }
}

static bool testTft() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  const bool ok = gfx->begin();
  if (ok) {
    gfx->fillScreen(ColorBlue);
    delay(150);
    gfx->fillScreen(ColorBlack);
  }

  addResult("TFT", ok, ok ? "Arduino_GFX ready" : "begin failed");
  return ok;
}

static bool testTouch() {
  SPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  const bool ok = touch.begin();
  touch.setRotation(1);
  addResult("Touch", ok, ok ? "XPT2046 ready" : "begin failed");
  return ok;
}

static bool testSd() {
  sdSpi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  const bool ok = SD.begin(SD_CS, sdSpi);
  addResult("SD", ok, ok ? String(SD.cardSize() / (1024 * 1024)) + " MB" : "not mounted");
  if (ok) {
    SD.end();
  }

  return ok;
}

static bool testRgbLed() {
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);

  setRgb(true, false, false);
  delay(120);
  setRgb(false, true, false);
  delay(120);
  setRgb(false, false, true);
  delay(120);
  setRgb(false, false, false);

  addResult("RGB LED", true, "cycled");
  return true;
}

static bool testSpeaker() {
  const bool ok = ledcAttach(SPEAKER_PIN, 2000, 8);
  if (ok) {
    ledcWriteTone(SPEAKER_PIN, 880);
    delay(140);
    ledcWriteTone(SPEAKER_PIN, 0);
    ledcDetach(SPEAKER_PIN);
  }

  addResult("Speaker", ok, ok ? "tone played" : "PWM failed");
  return ok;
}

static bool testLdr() {
  pinMode(LDR_PIN, INPUT);
  const int raw = analogRead(LDR_PIN);
  const bool ok = raw >= 0 && raw <= 4095;
  addResult("LDR", ok, String(raw));
  return ok;
}

static bool testUart() {
  Serial2.begin(Carbtune::UartBaud, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
  Serial2.write(Carbtune::PacketMagic);
  Serial2.write(Carbtune::PacketVersion);
  Serial2.write(static_cast<uint8_t>(Carbtune::PacketType::SelfTest));
  addResult("UART", true, "Serial2 started");
  return true;
}

static void runSelfTest() {
  resultCount = 0;
  testTft();
  testTouch();
  testSd();
  testRgbLed();
  testSpeaker();
  testLdr();
  testUart();
  drawResults();

  for (uint8_t index = 0; index < resultCount; ++index) {
    Serial.print(results[index].name);
    Serial.print(": ");
    Serial.print(results[index].passed ? "PASS" : "FAIL");
    Serial.print(" ");
    Serial.println(results[index].detail);
  }
}

void setup() {
  Serial.begin(Carbtune::UartBaud);

  Serial.println();
  Serial.println(FW_NAME);
  Serial.println(FW_VERSION);
  Serial.println(FW_BUILD);

  runSelfTest();
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
