#pragma once

#include <Arduino_GFX_Library.h>

#include "App/ScreenManager.h"
#include "Sensors/BacklightManager.h"
#include "Sensors/SensorManager.h"
#include "Storage/SdManager.h"
#include "TouchInput.h"
#include "Network/WebInterface.h"

class AppController {
 public:
  AppController(Arduino_GFX &display, TouchInput &touchInput, BacklightManager &backlightManager,
                SensorManager &sensorManager, SdManager &sdManager, WebInterface &webInterface,
                ScreenManager &screenManager);

  void begin();
  void update();

 private:
  void handleSerialInput();

  Arduino_GFX &display_;
  TouchInput &touchInput_;
  BacklightManager &backlightManager_;
  SensorManager &sensorManager_;
  SdManager &sdManager_;
  WebInterface &webInterface_;
  ScreenManager &screenManager_;
};
