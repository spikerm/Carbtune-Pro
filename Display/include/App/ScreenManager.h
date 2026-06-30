#pragma once

#include <Arduino.h>

#include "CalibrationScreen.h"
#include "DashboardScreen.h"
#include "GraphScreen.h"
#include "SettingsScreen.h"
#include "SplashScreen.h"
#include "TouchDiagnosticsScreen.h"
#include "TouchInput.h"

enum class ScreenId {
  Splash,
  Dashboard,
  Settings,
  Graph,
  Calibration,
  Diagnostics,
  About,
};

class ScreenManager {
 public:
  ScreenManager(SplashScreen &splash, DashboardScreen &dashboard, SettingsScreen &settings,
                GraphScreen &graph, CalibrationScreen &calibration,
                TouchDiagnosticsScreen &diagnostics, TouchInput &touchInput);

  void show(ScreenId id);
  void update(uint32_t nowMs, const TouchState &touchState);
  void handleTouch(const TouchState &touchState);
  ScreenId current() const;

 private:
  bool isDashboardMenuTarget(int16_t x, int16_t y) const;

  SplashScreen &splash_;
  DashboardScreen &dashboard_;
  SettingsScreen &settings_;
  GraphScreen &graph_;
  CalibrationScreen &calibration_;
  TouchDiagnosticsScreen &diagnostics_;
  TouchInput &touchInput_;
  ScreenId current_ = ScreenId::Splash;
  bool splashVisible_ = false;
  bool touchWasPressed_ = false;
};
