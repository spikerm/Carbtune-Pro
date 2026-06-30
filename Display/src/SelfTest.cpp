#include "SelfTest.h"

#include <CarbtuneShared.h>
#include <SD.h>

#include "BoardConfig.h"
#include "DisplayColors.h"
#include "version.h"

SelfTest::SelfTest(Arduino_GFX &display, XPT2046_Touchscreen &touch, SPIClass &sdSpi)
    : display_(display), touch_(touch), sdSpi_(sdSpi) {}

void SelfTest::run() {
  resultCount_ = 0;
  testTft();
  testTouch();
  testSd();
  testRgbLed();
  testSpeaker();
  testLdr();
  testUart();
  drawResults();

  for (uint8_t index = 0; index < resultCount_; ++index) {
    Serial.print(results_[index].name);
    Serial.print(": ");
    Serial.print(results_[index].passed ? "PASS" : "FAIL");
    Serial.print(" ");
    Serial.println(results_[index].detail);
  }
}

void SelfTest::addResult(const char *name, bool passed, const String &detail) {
  if (resultCount_ >= (sizeof(results_) / sizeof(results_[0]))) {
    return;
  }

  results_[resultCount_++] = {name, passed, detail};
}

void SelfTest::drawResults() {
  display_.fillScreen(ColorBlack);
  display_.setTextSize(2);
  display_.setTextColor(ColorWhite);
  display_.setCursor(8, 8);
  display_.print(FW_NAME);
  display_.setCursor(8, 30);
  display_.print(FW_VERSION);

  display_.setTextSize(1);
  for (uint8_t index = 0; index < resultCount_; ++index) {
    const int16_t y = 58 + (index * 22);
    display_.fillCircle(14, y + 4, 5, results_[index].passed ? ColorGreen : ColorRed);
    display_.setCursor(28, y);
    display_.setTextColor(results_[index].passed ? ColorGreen : ColorRed);
    display_.print(results_[index].name);
    display_.setTextColor(ColorWhite);
    display_.setCursor(116, y);
    display_.print(results_[index].detail);
  }
}

void SelfTest::setRgb(bool red, bool green, bool blue) {
  digitalWrite(LED_RED, red ? LOW : HIGH);
  digitalWrite(LED_GREEN, green ? LOW : HIGH);
  digitalWrite(LED_BLUE, blue ? LOW : HIGH);
}

bool SelfTest::testTft() {
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
  const bool ok = display_.begin();
  if (ok) {
    display_.fillScreen(ColorBlue);
    delay(150);
    display_.fillScreen(ColorBlack);
  }

  addResult("TFT", ok, ok ? "Arduino_GFX ready" : "begin failed");
  return ok;
}

bool SelfTest::testTouch() {
  SPI.begin(TOUCH_CLK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
  const bool ok = touch_.begin();
  touch_.setRotation(1);
  addResult("Touch", ok, ok ? "XPT2046 ready" : "begin failed");
  return ok;
}

bool SelfTest::testSd() {
  sdSpi_.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  const bool ok = SD.begin(SD_CS, sdSpi_);
  addResult("SD", ok, ok ? String(SD.cardSize() / (1024 * 1024)) + " MB" : "not mounted");
  if (ok) {
    SD.end();
  }

  return ok;
}

bool SelfTest::testRgbLed() {
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

bool SelfTest::testSpeaker() {
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

bool SelfTest::testLdr() {
  pinMode(LDR_PIN, INPUT);
  const int raw = analogRead(LDR_PIN);
  const bool ok = raw >= 0 && raw <= 4095;
  addResult("LDR", ok, String(raw));
  return ok;
}

bool SelfTest::testUart() {
  Serial2.begin(Carbtune::UartBaud, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
  Serial2.write(Carbtune::PacketMagic);
  Serial2.write(Carbtune::PacketVersion);
  Serial2.write(static_cast<uint8_t>(Carbtune::PacketType::SelfTest));
  addResult("UART", true, "Serial2 started");
  return true;
}
