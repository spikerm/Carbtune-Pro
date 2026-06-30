#pragma once

#include <Arduino_GFX_Library.h>

#include "App/ScreenManager.h"
#include "Sensors/BacklightManager.h"
#include "Sensors/SensorManager.h"
#include "TouchInput.h"

class AppController {
 public:
  AppController(Arduino_GFX &display, TouchInput &touchInput, BacklightManager &backlightManager,
                SensorManager &sensorManager, ScreenManager &screenManager);

  void begin();
  void update();

 private:
  void handleSerialInput();

  Arduino_GFX &display_;
  TouchInput &touchInput_;
  BacklightManager &backlightManager_;
  SensorManager &sensorManager_;
  ScreenManager &screenManager_;
};
