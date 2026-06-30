#include "SelfTest.h"

#include <CarbtuneShared.h>
#include <SD.h>

#include "BoardConfig.h"
#include "DisplayColors.h"
#include "version.h"

SelfTest::SelfTest(Arduino_GFX &display, TouchInput &touchInput, SPIClass &sdSpi)
    : display_(display), touchInput_(touchInput), sdSpi_(sdSpi) {}

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
  display_.setTextColor(ColorCyan);
  display_.setCursor(8, 8);
  display_.print("HARDWARE SELFTEST");

  display_.setTextSize(1);
  display_.setTextColor(ColorWhite);
  display_.setCursor(8, 32);
  display_.print(FW_VERSION);
  display_.print("  ");
  display_.print(FW_BUILD);

  display_.setCursor(8, 44);
  display_.print("Heap ");
  display_.print(ESP.getFreeHeap());
  display_.print(" B  Flash ");
  display_.print(ESP.getFlashChipSize() / (1024 * 1024));
  display_.print(" MB");

  display_.setTextSize(1);
  for (uint8_t index = 0; index < resultCount_; ++index) {
    const int16_t y = 66 + (index * 22);
    drawBadge(8, y - 4, results_[index].passed);
    display_.setCursor(62, y);
    display_.setTextColor(ColorWhite);
    display_.print(results_[index].name);
    display_.setCursor(132, y);
    display_.print(results_[index].detail);
  }
}

void SelfTest::drawBadge(int16_t x, int16_t y, bool passed) {
  const uint16_t color = passed ? ColorGreen : ColorRed;
  display_.fillRoundRect(x, y, 45, 16, 2, color);
  display_.drawRoundRect(x, y, 45, 16, 2, ColorWhite);
  display_.setTextColor(ColorBlack);
  display_.setCursor(x + 8, y + 4);
  display_.print(passed ? "PASS" : "FAIL");
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
  const bool ok = touchInput_.begin();
  touchInput_.update(millis());
  addResult("Touch", ok, ok ? touchDetail() : "begin failed");
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
  addResult("LDR", ok, "raw=" + String(raw));
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

String SelfTest::touchDetail() const {
  const TouchState touch = touchInput_.current();
  if (!touch.pressed) {
    return "not pressed";
  }

  return "raw=" + String(touch.rawX) + "," + String(touch.rawY);
}
