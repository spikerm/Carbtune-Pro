#include "SettingsScreen.h"

#include "UiButton.h"
#include "UiTheme.h"
#include "version.h"

namespace {
constexpr UiRect HomeRect{8, 205, 96, 30};
constexpr UiRect GraphRect{112, 205, 96, 30};
constexpr UiRect SettingsRect{216, 205, 96, 30};
constexpr int16_t ViewX = 8;
constexpr int16_t ViewY = 34;
constexpr int16_t ViewW = 304;
constexpr int16_t ViewH = 168;
constexpr int16_t ContentHeight = 410;
constexpr int16_t HeaderHeight = 32;
constexpr int16_t FooterY = 203;

bool contentHit(int16_t contentY, int16_t top, int16_t height = 28) {
  return contentY >= top && contentY < top + height;
}
}  // namespace

SettingsScreen::SettingsScreen(Arduino_GFX &display) : display_(display) {}

void SettingsScreen::begin() {
  pendingAction_ = SettingsAction::None;
  scrollView_.begin();
  scrollView_.setContentHeight(ContentHeight);
  lastOffset_ = -1;
  display_.fillScreen(UiTheme::Background);
  drawHeader();
  drawFooter();
  drawList();
}

void SettingsScreen::update(uint32_t nowMs, const TouchState &touchState) {
  scrollView_.update(nowMs, touchState);
  if (scrollView_.offset() != lastOffset_) {
    drawList();
  }
  if (scrollView_.wasClickReleased()) {
    handleClick(scrollView_.clickX(), scrollView_.clickY());
  }
}

SettingsAction SettingsScreen::takeAction() {
  const SettingsAction action = pendingAction_;
  pendingAction_ = SettingsAction::None;
  return action;
}

bool SettingsScreen::isHomeHit(int16_t x, int16_t y) const {
  return UiButton(HomeRect, "").contains(x, y);
}

bool SettingsScreen::isGraphHit(int16_t x, int16_t y) const {
  return UiButton(GraphRect, "").contains(x, y);
}

bool SettingsScreen::isSettingsHit(int16_t x, int16_t y) const {
  return UiButton(SettingsRect, "").contains(x, y);
}

bool SettingsScreen::isCalibrationHit(int16_t x, int16_t y) const {
  const int16_t contentY = scrollView_.contentY(y);
  return scrollView_.contains(x, y) && contentHit(contentY, 236);
}

void SettingsScreen::drawHeader() {
  display_.fillRect(0, 0, UiTheme::ScreenWidth, HeaderHeight, UiTheme::Background);
  display_.drawLine(0, HeaderHeight - 1, UiTheme::ScreenWidth, HeaderHeight - 1, UiTheme::Border);
  UiButton({8, 5, 32, 22}, "<").draw(display_);
  display_.setTextColor(UiTheme::Text);
  display_.setTextSize(2);
  display_.setCursor(94, 8);
  display_.print("INSTELLINGEN");
}

void SettingsScreen::drawFooter() {
  display_.fillRect(0, FooterY, UiTheme::ScreenWidth, UiTheme::ScreenHeight - FooterY,
                    UiTheme::Background);
  display_.drawLine(8, FooterY, 312, FooterY, UiTheme::Border);
  UiButton(HomeRect, "HOME").draw(display_);
  UiButton(GraphRect, "GRAFIEK").draw(display_);
  UiButton(SettingsRect, "INSTELLINGEN").draw(display_, true);
}

void SettingsScreen::drawList() {
  display_.fillRect(ViewX, ViewY, ViewW, ViewH, UiTheme::Background);
  UiTheme::drawPanel(display_, ViewX, ViewY, ViewW, ViewH);
  const int16_t offset = scrollView_.offset();

  drawSection("ALGEMEEN", 12 - offset);
  drawRow("Aantal cilinders", "1  2  3  [4]", 34 - offset);
  drawRow("Eenheden", "[kPa]  inHg", 62 - offset);

  drawSection("WEERGAVE", 102 - offset);
  drawRow("Auto schaal", "aan", 124 - offset);
  drawRow("Demping/filter", "0.7 s", 152 - offset);
  drawRow("Auto helderheid", "aan", 180 - offset);
  drawRow("Helderheid", "80%", 208 - offset);

  drawSection("SENSOREN", 250 - offset);
  drawRow("Kalibratie", "START", 272 - offset, true);
  drawRow("Sensor informatie", "DEMO", 300 - offset);
  drawRow("Demo mode", "aan", 328 - offset);
  drawRow("Live UART", "aan", 356 - offset);

  drawSection("SYSTEEM", 396 - offset);
  drawRow("Firmware", FW_VERSION, 418 - offset);
  drawRow("Opslag (SD)", "-", 446 - offset);
  drawRow("Diagnostics", "OPEN", 474 - offset, true);
  drawRow("Over apparaat", "OPEN", 502 - offset, true);

  scrollView_.drawScrollbar(display_);
  lastOffset_ = offset;
}

void SettingsScreen::drawSection(const char *title, int16_t y) {
  const int16_t screenY = ViewY + y;
  if (screenY < ViewY - 12 || screenY > ViewY + ViewH) {
    return;
  }
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Accent);
  display_.setCursor(ViewX + 10, screenY);
  display_.print(title);
  display_.drawFastHLine(ViewX + 10, screenY + 12, ViewW - 28, UiTheme::Border);
}

void SettingsScreen::drawRow(const char *label, const char *value, int16_t y, bool button) {
  const int16_t screenY = ViewY + y;
  if (screenY < ViewY - 22 || screenY > ViewY + ViewH) {
    return;
  }

  display_.fillRect(ViewX + 8, screenY, ViewW - 24, 24, UiTheme::Panel);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(ViewX + 14, screenY + 8);
  display_.print(label);
  if (button) {
    UiButton({static_cast<int16_t>(ViewX + 220), static_cast<int16_t>(screenY + 2), 58, 20},
             value)
        .draw(display_);
  } else {
    display_.setTextColor(UiTheme::TextMuted);
    display_.setCursor(ViewX + 190, screenY + 8);
    display_.print(value);
  }
}

void SettingsScreen::handleClick(int16_t screenX, int16_t screenY) {
  if (isHomeHit(screenX, screenY)) {
    pendingAction_ = SettingsAction::Home;
    return;
  }
  if (isGraphHit(screenX, screenY)) {
    pendingAction_ = SettingsAction::Graph;
    return;
  }

  if (!scrollView_.contains(screenX, screenY)) {
    return;
  }

  const int16_t y = scrollView_.contentY(screenY);
  if (contentHit(y, 272)) {
    pendingAction_ = SettingsAction::Calibration;
  } else if (contentHit(y, 474)) {
    pendingAction_ = SettingsAction::Diagnostics;
  }
}
