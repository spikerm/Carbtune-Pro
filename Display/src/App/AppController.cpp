#include "App/AppController.h"

#include <CarbtuneShared.h>

#include "BoardConfig.h"
#include "UiTheme.h"
#include "version.h"

AppController::AppController(Arduino_GFX &display, TouchInput &touchInput,
                             BacklightManager &backlightManager, SensorManager &sensorManager,
                             SdManager &sdManager, ScreenManager &screenManager)
    : display_(display),
      touchInput_(touchInput),
      backlightManager_(backlightManager),
      sensorManager_(sensorManager),
      sdManager_(sdManager),
      screenManager_(screenManager) {}

void AppController::begin() {
  Serial.begin(Carbtune::UartBaud);
  backlightManager_.begin();
  pinMode(TFT_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TOUCH_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
  display_.begin();
  display_.invertDisplay(TFT_INVERT_COLORS);
  display_.fillScreen(UiTheme::Background);
  sdManager_.begin();
  touchInput_.begin();
  sensorManager_.begin();

  Serial.println();
  Serial.println(FW_NAME);
  Serial.println(FW_VERSION);
  Serial.println(FW_BUILD);

  screenManager_.show(ScreenId::Splash);
}

void AppController::update() {
  const uint32_t nowMs = millis();
  backlightManager_.update(nowMs);
  sensorManager_.update(nowMs);
  const TouchState touchState = touchInput_.update(nowMs);
  handleSerialInput();
  screenManager_.update(nowMs, touchState);
}

void AppController::handleSerialInput() {
  while (Serial.available() > 0) {
    const char command = static_cast<char>(Serial.read());
    switch (command) {
      case 'h':
      case 'H':
        screenManager_.show(ScreenId::Dashboard);
        break;
      case 's':
      case 'S':
        screenManager_.show(ScreenId::Settings);
        break;
      case 'g':
      case 'G':
        screenManager_.show(ScreenId::Graph);
        break;
      case 'c':
      case 'C':
        screenManager_.show(ScreenId::Calibration);
        break;
      case 'd':
      case 'D':
        screenManager_.show(ScreenId::Diagnostics);
        break;
      case 'r':
      case 'R':
        screenManager_.show(ScreenId::Splash);
        break;
      default:
        break;
    }
  }
}
