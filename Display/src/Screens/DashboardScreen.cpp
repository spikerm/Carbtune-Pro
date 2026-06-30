#include "DashboardScreen.h"

#include <math.h>

#include "UiTheme.h"

static constexpr float GoodThresholdKpa = 2.0f;
static constexpr float WarnThresholdKpa = 5.0f;
static constexpr uint16_t DashboardUpdateIntervalMs = 100;
static constexpr int16_t HeaderH = 34;
static constexpr int16_t PanelX = 8;
static constexpr int16_t PanelY = 36;
static constexpr int16_t PanelW = 304;
static constexpr int16_t PanelH = 158;
static constexpr int16_t MeterTop = 96;
static constexpr int16_t MeterHeight = 58;
static constexpr int16_t FooterY = 198;
static constexpr int16_t MenuX = 238;
static constexpr int16_t MenuY = 207;
static constexpr int16_t MenuW = 74;
static constexpr int16_t MenuH = 24;

DashboardScreen::DashboardScreen(Arduino_GFX &display, SensorManager &sensorManager)
    : display_(display), sensorManager_(sensorManager) {}

void DashboardScreen::begin() {
  for (uint8_t index = 0; index < SensorManager::MaxChannels; ++index) {
    lastValueTenths_[index] = -32768;
  }
  lastDiffTenths_ = -32768;
  lastRefY_ = -32768;
  lastRpm_ = 65535;
  lastRpmStable_ = false;
  lastMode_ = SensorManager::Mode::NoData;
  lastStatus_ = SensorManager::Status::Warning;
  lastTouchX_ = -2;
  lastTouchY_ = -2;
  lastUpdateMs_ = 0;
  cylinderCount_ = sensorManager_.channelCount();
  needsFullRedraw_ = true;
  drawStatic();
  needsFullRedraw_ = false;
  update(millis());
}

void DashboardScreen::update(uint32_t nowMs) {
  if (nowMs - lastUpdateMs_ < DashboardUpdateIntervalMs) {
    return;
  }

  lastUpdateMs_ = nowMs;
  const uint8_t newCylinderCount = sensorManager_.channelCount();
  for (uint8_t index = 0; index < SensorManager::MaxChannels; ++index) {
    valuesKpa_[index] = sensorManager_.valueKpa(index);
  }

  if (newCylinderCount != cylinderCount_) {
    cylinderCount_ = newCylinderCount;
    needsFullRedraw_ = true;
  }
  if (needsFullRedraw_) {
    drawStatic();
    needsFullRedraw_ = false;
  }

  drawHeader(false);

  const int16_t refY =
      MeterTop + MeterHeight -
      constrain(static_cast<int16_t>((-valuesKpa_[0]) * MeterHeight / 100.0f), 0, MeterHeight);
  if (refY != lastRefY_) {
    for (uint8_t index = 0; index < SensorManager::MaxChannels; ++index) {
      lastValueTenths_[index] = -32768;
    }
    lastRefY_ = refY;
  }

  const int16_t panelWidth = PanelW / cylinderCount_;
  const int16_t startX = PanelX + ((PanelW - (panelWidth * cylinderCount_)) / 2);
  for (uint8_t index = 0; index < cylinderCount_; ++index) {
    drawCylinder(index, startX + (index * panelWidth), PanelY, panelWidth, false);
  }

  drawBottomBar(false);
}

void DashboardScreen::setCylinderCount(uint8_t count) {
  cylinderCount_ = constrain(count, 2, SensorManager::MaxChannels);
}

uint8_t DashboardScreen::cylinderCount() const {
  return cylinderCount_;
}

bool DashboardScreen::isMenuHit(int16_t x, int16_t y) const {
  return x >= 230 && x < 320 && y >= 195 && y < 240;
}

void DashboardScreen::showTouchStatus(int16_t x, int16_t y) {
  lastTouchX_ = x;
  lastTouchY_ = y;
}

void DashboardScreen::showNotPressed() {
  lastTouchX_ = -1;
  lastTouchY_ = -1;
}

void DashboardScreen::drawStatic() {
  display_.fillScreen(UiTheme::Background);
  drawHeader(true);
  UiTheme::drawPanel(display_, PanelX, PanelY, PanelW, PanelH);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::AccentBlue);
  display_.setCursor(PanelX + 12, PanelY + 6);
  display_.print("REF CH1");
  const int16_t panelWidth = PanelW / cylinderCount_;
  const int16_t startX = PanelX + ((PanelW - (panelWidth * cylinderCount_)) / 2);
  for (uint8_t index = 0; index < cylinderCount_; ++index) {
    drawCylinder(index, startX + (index * panelWidth), PanelY, panelWidth, true);
  }
  drawBottomBar(true);
}

void DashboardScreen::drawHeader(bool force) {
  const float diff = maxDifference();
  const int16_t diffTenths = static_cast<int16_t>(diff * 10.0f);
  const SensorManager::Mode mode = sensorManager_.mode();
  const uint16_t rpm = sensorManager_.rpm();
  const bool rpmStable = sensorManager_.rpmStable();
  if (!force && diffTenths == lastDiffTenths_ && mode == lastMode_ && rpm == lastRpm_ &&
      rpmStable == lastRpmStable_) {
    return;
  }

  display_.fillRect(0, 0, UiTheme::ScreenWidth, HeaderH, UiTheme::Background);
  display_.drawLine(0, HeaderH - 1, UiTheme::ScreenWidth, HeaderH - 1, UiTheme::Border);
  display_.setTextSize(1);
  display_.setTextColor(rpmStable ? UiTheme::GoodGreen : UiTheme::WarnYellow);
  display_.setCursor(10, 12);
  display_.print("RPM ");
  if (rpmStable) {
    display_.print(rpm);
  } else {
    display_.print("--");
  }

  display_.setTextSize(2);
  display_.setTextColor(statusColor());
  display_.setCursor(118, 7);
  display_.print("D ");
  display_.print(diff, 1);
  display_.print(" kPa");

  display_.setTextSize(1);
  display_.setTextColor(mode == SensorManager::Mode::LiveUart ? UiTheme::GoodGreen
                                                              : UiTheme::WarnYellow);
  display_.setCursor(mode == SensorManager::Mode::NoData ? 266 : 282, 12);
  display_.print(sensorManager_.modeName());

  lastDiffTenths_ = diffTenths;
  lastMode_ = mode;
  lastRpm_ = rpm;
  lastRpmStable_ = rpmStable;
}

void DashboardScreen::drawCylinder(uint8_t index, int16_t x, int16_t y, int16_t w, bool force) {
  const int16_t valueTenths = static_cast<int16_t>(valuesKpa_[index] * 10.0f);
  if (!force && valueTenths == lastValueTenths_[index]) {
    return;
  }

  const float delta = valuesKpa_[index] - valuesKpa_[0];
  const float absDelta = fabsf(delta);
  const bool alarm = absDelta > WarnThresholdKpa;
  const bool warning = absDelta > GoodThresholdKpa;
  const uint16_t meterColor =
      alarm ? UiTheme::AlarmRed : (warning ? UiTheme::WarnYellow : UiTheme::GoodGreen);
  const int16_t center = x + (w / 2);
  const bool compact = cylinderCount_ > 4;
  const int16_t meterW = constrain(static_cast<int16_t>(w / 3), 10, 24);
  const int16_t meterX = center - (meterW / 2);
  const int16_t fillHeight =
      constrain(static_cast<int16_t>((-valuesKpa_[index]) * MeterHeight / 100.0f), 0, MeterHeight);
  const int16_t fillY = MeterTop + MeterHeight - fillHeight;

  if (force) {
    display_.fillRect(x + 2, y + 4, w - 4, PanelH - 8, UiTheme::Panel);
    display_.setTextColor(UiTheme::Text);
    display_.setTextSize(compact ? 1 : 2);
    display_.setCursor(center - (compact ? 3 : 6), y + 8);
    display_.print(index + 1);
    display_.setTextSize(1);
    display_.setCursor(center - 10, y + 47);
    display_.print("kPa");

    display_.setTextColor(UiTheme::Text);
    display_.setCursor(meterX - 18, MeterTop);
    display_.print("0");
    if (!compact) {
      display_.setCursor(meterX - 24, MeterTop + 28);
      display_.print("-50");
    }
    display_.setCursor(meterX - (compact ? 18 : 30), MeterTop + 54);
    display_.print("-100");
    display_.drawRoundRect(center - (compact ? 22 : 28), y + 124, compact ? 44 : 56, 18, 3,
                           UiTheme::PanelBorder);
  }

  display_.fillRect(center - (compact ? 17 : 23), y + (compact ? 30 : 28),
                    compact ? 34 : 46, compact ? 8 : 16, UiTheme::Panel);
  display_.fillRect(meterX + 1, MeterTop + 1, meterW, MeterHeight - 2, UiTheme::Background);
  display_.fillRect(center - (compact ? 21 : 27), y + 128, compact ? 42 : 54, 22, UiTheme::Panel);

  display_.drawRect(meterX, MeterTop, meterW + 2, MeterHeight, UiTheme::PanelBorder);

  display_.setTextColor(UiTheme::Text);
  display_.setTextSize(compact ? 1 : 2);
  display_.setCursor(center - (compact ? 14 : 22), y + (compact ? 30 : 28));
  display_.print(valuesKpa_[index], 0);

  display_.fillRect(meterX + 1, fillY, meterW, fillHeight, meterColor);
  display_.drawFastHLine(meterX - 3, fillY, meterW + 8, UiTheme::Text);
  if (lastRefY_ >= MeterTop && lastRefY_ <= MeterTop + MeterHeight) {
    display_.drawFastHLine(meterX - 3, lastRefY_, meterW + 8, UiTheme::AccentBlue);
  }

  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(center - (compact ? 17 : 22), y + 128);
  display_.print(static_cast<int>(valuesKpa_[index]));
  if (!compact) {
    display_.print(" kPa");
  }
  display_.setTextColor(index == 0 ? UiTheme::AccentBlue : meterColor);
  display_.setCursor(center - (compact ? 21 : 25), y + 140);
  if (index == 0) {
    display_.print("REF");
  } else {
    display_.print("d");
    display_.print(delta >= 0.0f ? "+" : "");
    display_.print(delta, compact ? 0 : 1);
  }
  lastValueTenths_[index] = valueTenths;
}

void DashboardScreen::drawBottomBar(bool force) {
  const float delta = maxDifference();
  const SensorManager::Status currentStatus =
      delta > WarnThresholdKpa
          ? SensorManager::Status::Adjust
          : (delta > GoodThresholdKpa ? SensorManager::Status::Warning : SensorManager::Status::Good);
  if (!force && currentStatus == lastStatus_) {
    return;
  }

  if (force) {
    display_.fillRect(0, FooterY, UiTheme::ScreenWidth, UiTheme::ScreenHeight - FooterY,
                      UiTheme::Background);
    display_.drawFastHLine(8, FooterY, 304, UiTheme::Border);
    display_.fillRoundRect(MenuX, MenuY, MenuW, MenuH, 4, UiTheme::AccentDark);
    display_.drawRoundRect(MenuX, MenuY, MenuW, MenuH, 4, UiTheme::Accent);
    display_.setTextSize(1);
    display_.setTextColor(UiTheme::Text);
    display_.setCursor(MenuX + 10, MenuY + 8);
    display_.print("MENU *");
  }

  display_.fillRoundRect(12, 204, 204, 32, 5, statusColor());
  display_.drawRoundRect(12, 204, 204, 32, 5, UiTheme::Text);
  display_.setTextColor(UiTheme::Background);
  display_.setTextSize(2);
  display_.setCursor(maxDifference() > WarnThresholdKpa ? 42 : 76, 212);
  display_.print(statusText());
  lastStatus_ = currentStatus;
}

float DashboardScreen::maxDifference() const {
  float maxDelta = 0.0f;
  for (uint8_t index = 1; index < cylinderCount_; ++index) {
    maxDelta = max(maxDelta, fabsf(valuesKpa_[index] - valuesKpa_[0]));
  }
  return maxDelta;
}

uint8_t DashboardScreen::outlierIndex() const {
  uint8_t outlier = 0;
  float farthest = 0.0f;
  for (uint8_t index = 1; index < cylinderCount_; ++index) {
    const float distance = fabsf(valuesKpa_[index] - valuesKpa_[0]);
    if (distance > farthest) {
      farthest = distance;
      outlier = index;
    }
  }
  return outlier;
}

const char *DashboardScreen::statusText() const {
  const float delta = maxDifference();
  if (delta > WarnThresholdKpa) {
    return "BIJSTELLEN";
  }
  if (delta > GoodThresholdKpa) {
    return "LET OP";
  }
  return "GOED";
}

uint16_t DashboardScreen::statusColor() const {
  const float delta = maxDifference();
  if (delta > WarnThresholdKpa) {
    return UiTheme::AlarmRed;
  }
  if (delta > GoodThresholdKpa) {
    return UiTheme::WarnYellow;
  }
  return UiTheme::GoodGreen;
}
