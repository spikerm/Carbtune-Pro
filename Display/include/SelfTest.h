#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <SPI.h>

#include "TouchInput.h"

struct SelfTestResult {
  const char *name;
  bool passed;
  String detail;
};

class SelfTest {
 public:
  SelfTest(Arduino_GFX &display, TouchInput &touchInput, SPIClass &sdSpi);

  void run();

 private:
  void addResult(const char *name, bool passed, const String &detail);
  void drawResults();
  void drawBadge(int16_t x, int16_t y, bool passed);
  void setRgb(bool red, bool green, bool blue);
  String touchDetail() const;

  bool testTft();
  bool testTouch();
  bool testSd();
  bool testRgbLed();
  bool testSpeaker();
  bool testLdr();
  bool testUart();

  Arduino_GFX &display_;
  TouchInput &touchInput_;
  SPIClass &sdSpi_;
  SelfTestResult results_[7];
  uint8_t resultCount_ = 0;
};
