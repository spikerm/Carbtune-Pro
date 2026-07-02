#include "App/ScreenManager.h"

ScreenManager::ScreenManager(SplashScreen &splash, DashboardScreen &dashboard,
                             SettingsScreen &settings, GraphScreen &graph,
                             CalibrationScreen &calibration, TouchDiagnosticsScreen &diagnostics,
                             TouchInput &touchInput)
    : splash_(splash),
      dashboard_(dashboard),
      settings_(settings),
      graph_(graph),
      calibration_(calibration),
      diagnostics_(diagnostics),
      touchInput_(touchInput) {}

void ScreenManager::show(ScreenId id) {
  current_ = id;
  touchWasPressed_ = false;

  switch (id) {
    case ScreenId::Splash:
      splash_.begin(millis());
      splashVisible_ = true;
      break;
    case ScreenId::Dashboard:
      splashVisible_ = false;
      dashboard_.begin();
      break;
    case ScreenId::Settings:
      splashVisible_ = false;
      settings_.begin();
      break;
    case ScreenId::Graph:
      splashVisible_ = false;
      graph_.begin();
      break;
    case ScreenId::Calibration:
      splashVisible_ = false;
      calibration_.begin();
      break;
    case ScreenId::Diagnostics:
      splashVisible_ = false;
      diagnostics_.begin();
      diagnostics_.update(touchInput_.current());
      break;
    case ScreenId::About:
      splashVisible_ = false;
      settings_.begin();
      break;
  }
}

void ScreenManager::update(uint32_t nowMs, const TouchState &touchState) {
  if (splashVisible_) {
    if (splash_.update(nowMs)) {
      show(ScreenId::Dashboard);
    }
    return;
  }

  if (current_ == ScreenId::Settings) {
    settings_.update(nowMs, touchState);
    switch (settings_.takeAction()) {
      case SettingsAction::Home:
        show(ScreenId::Dashboard);
        break;
      case SettingsAction::Graph:
        show(ScreenId::Graph);
        break;
      case SettingsAction::Calibration:
        show(ScreenId::Calibration);
        break;
      case SettingsAction::Diagnostics:
        show(ScreenId::Diagnostics);
        break;
      case SettingsAction::None:
        break;
    }
    return;
  }

  handleTouch(touchState);
  if (current_ == ScreenId::Dashboard) {
    dashboard_.update(nowMs);
  } else if (current_ == ScreenId::Diagnostics) {
    diagnostics_.update(touchState);
  }
}

void ScreenManager::handleTouch(const TouchState &touchState) {
  const bool touchStarted = touchState.pressed && !touchWasPressed_;
  if (touchState.changed) {
    const bool menuHit = touchState.pressed &&
                         isDashboardMenuTarget(touchState.screenX, touchState.screenY);
    Serial.print("Touch mapped=");
    Serial.print(touchState.screenX);
    Serial.print(",");
    Serial.print(touchState.screenY);
    Serial.print(touchState.pressed ? " pressed" : " released");
    Serial.print(" MENU=");
    Serial.println(menuHit ? "yes" : "no");
  }

  touchWasPressed_ = touchState.pressed;
  if (!touchStarted) {
    if (current_ == ScreenId::Dashboard) {
      if (touchState.pressed) {
        dashboard_.showTouchStatus(touchState.screenX, touchState.screenY);
      } else {
        dashboard_.showNotPressed();
      }
    }
    return;
  }

  const int16_t x = touchState.screenX;
  const int16_t y = touchState.screenY;
  if (handleGlobalTouch(x, y)) {
    return;
  }

  switch (current_) {
    case ScreenId::Dashboard:
      dashboard_.showTouchStatus(x, y);
      if (isDashboardMenuTarget(x, y)) {
        show(ScreenId::Settings);
      }
      break;
    case ScreenId::Settings:
      break;
    case ScreenId::Graph:
      if (graph_.isHomeHit(x, y)) {
        show(ScreenId::Dashboard);
      } else if (graph_.isSettingsHit(x, y)) {
        show(ScreenId::Settings);
      }
      break;
    case ScreenId::Calibration:
      if (calibration_.isBackHit(x, y)) {
        show(ScreenId::Settings);
      }
      break;
    case ScreenId::Diagnostics:
    case ScreenId::Splash:
    case ScreenId::About:
      break;
  }
}

ScreenId ScreenManager::current() const {
  return current_;
}

bool ScreenManager::isDashboardMenuTarget(int16_t x, int16_t y) const {
  (void)x;
  (void)y;
  return false;
}

bool ScreenManager::handleGlobalTouch(int16_t x, int16_t y) {
  if (isGlobalHomeTarget(x, y) && current_ != ScreenId::Dashboard) {
    Serial.print("button=HOME screen=");
    Serial.print(screenName(current_));
    Serial.println(" action=Dashboard");
    show(ScreenId::Dashboard);
    return true;
  }

  return false;
}

bool ScreenManager::isGlobalHomeTarget(int16_t x, int16_t y) const {
  return x >= 0 && x < 112 && y >= 195 && y < 240;
}

bool ScreenManager::isGlobalMenuTarget(int16_t x, int16_t y) const {
  (void)x;
  (void)y;
  return false;
}

const char *ScreenManager::screenName(ScreenId id) const {
  switch (id) {
    case ScreenId::Splash:
      return "Splash";
    case ScreenId::Dashboard:
      return "Dashboard";
    case ScreenId::Settings:
      return "Settings";
    case ScreenId::Graph:
      return "Graph";
    case ScreenId::Calibration:
      return "Calibration";
    case ScreenId::Diagnostics:
      return "Diagnostics";
    case ScreenId::About:
      return "About";
  }
  return "Unknown";
}
