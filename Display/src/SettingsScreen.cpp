#include "SettingsScreen.h"

#include "UiButton.h"
#include "UiTheme.h"
#include "version.h"

namespace {
constexpr UiRect BackRect{0, 0, 44, 42};
constexpr UiRect HomeRect{4, 202, 82, 34};
constexpr UiRect CancelRect{90, 202, 110, 34};
constexpr UiRect SaveRect{204, 202, 112, 34};
constexpr int16_t HeaderHeight = 42;
constexpr int16_t ViewX = 4;
constexpr int16_t ViewY = 44;
constexpr int16_t ViewW = 312;
constexpr int16_t ViewH = 152;
constexpr int16_t FooterY = 198;
constexpr int16_t ContentHeight = 820;
constexpr int16_t RowHeight = 36;

bool contentHit(int16_t contentY, int16_t top, int16_t height = RowHeight) {
  return contentY >= top && contentY < top + height;
}

bool rectHit(const UiRect &rect, int16_t x, int16_t y) {
  return x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h;
}
}  // namespace

SettingsScreen::SettingsScreen(Arduino_GFX &display, SettingsManager &settings,
                               SdManager &sdManager)
    : display_(display), settings_(settings), sdManager_(sdManager) {}

void SettingsScreen::begin() {
  pendingAction_ = SettingsAction::None;
  loadDraft();
  scrollView_.begin();
  scrollView_.setContentHeight(ContentHeight);
  lastOffset_ = -1;
  messageUntilMs_ = 0;
  message_[0] = '\0';
  touchWasPressed_ = false;
  display_.fillScreen(UiTheme::Background);
  drawHeader();
  drawFooter();
  drawList();
}

void SettingsScreen::update(uint32_t nowMs, const TouchState &touchState) {
  const bool pressed = touchState.pressed && !touchWasPressed_;
  const bool released = !touchState.pressed && touchWasPressed_;
  if (touchState.pressed) {
    lastTouchX_ = touchState.screenX;
    lastTouchY_ = touchState.screenY;
  }

  if (pressed && !scrollView_.contains(touchState.screenX, touchState.screenY)) {
    handleClick(touchState.screenX, touchState.screenY);
    touchWasPressed_ = touchState.pressed;
    drawMessage(nowMs);
    return;
  }

  scrollView_.update(nowMs, touchState);
  if (scrollView_.offset() != lastOffset_) {
    drawList();
  }
  if (scrollView_.wasClickReleased()) {
    handleClick(scrollView_.clickX(), scrollView_.clickY());
  }

  drawMessage(nowMs);
  touchWasPressed_ = touchState.pressed;
}

SettingsAction SettingsScreen::takeAction() {
  const SettingsAction action = pendingAction_;
  pendingAction_ = SettingsAction::None;
  return action;
}

bool SettingsScreen::isHomeHit(int16_t x, int16_t y) const {
  return rectHit(HomeRect, x, y) || rectHit(BackRect, x, y);
}

bool SettingsScreen::isGraphHit(int16_t, int16_t) const {
  return false;
}

bool SettingsScreen::isSettingsHit(int16_t, int16_t) const {
  return false;
}

bool SettingsScreen::isCalibrationHit(int16_t x, int16_t y) const {
  const int16_t contentY = scrollView_.contentY(y);
  return scrollView_.contains(x, y) && contentHit(contentY, 440);
}

void SettingsScreen::loadDraft() {
  draft_.cylinders = settings_.cylinders();
  draft_.units = settings_.unitsMode();
  draft_.autoScale = settings_.autoScale();
  draft_.damping = settings_.dampingMode();
  draft_.autoBrightness = settings_.autoBrightness();
  draft_.brightness = settings_.brightnessManualPercent();
  draft_.demoFallback = settings_.demoFallback();
  draft_.alarmDeltaKpa = settings_.alarmDeltaKpa();
  draft_.rpmSource = settings_.rpmSource();
  draft_.engineStroke = settings_.engineStroke();
}

void SettingsScreen::applyDraft() {
  settings_.setCylinders(draft_.cylinders);
  settings_.setUnits(draft_.units);
  settings_.setAutoScale(draft_.autoScale);
  settings_.setDampingMode(draft_.damping);
  settings_.setAutoBrightness(draft_.autoBrightness);
  settings_.setBrightnessManualPercent(draft_.brightness);
  settings_.setDemoFallback(draft_.demoFallback);
  settings_.setAlarmDeltaKpa(draft_.alarmDeltaKpa);
  settings_.setRpmSource(draft_.rpmSource);
  settings_.setEngineStroke(draft_.engineStroke);
}

void SettingsScreen::drawHeader() {
  display_.fillRect(0, 0, UiTheme::ScreenWidth, HeaderHeight, UiTheme::Background);
  UiButton(BackRect, "<").draw(display_);
  display_.setTextColor(UiTheme::Text);
  display_.setTextSize(2);
  display_.setCursor(58, 12);
  display_.print("INSTELLINGEN");
  display_.drawFastHLine(0, HeaderHeight - 1, UiTheme::ScreenWidth, UiTheme::PanelBorder);
}

void SettingsScreen::drawFooter() {
  display_.fillRect(0, FooterY, UiTheme::ScreenWidth, UiTheme::ScreenHeight - FooterY,
                    UiTheme::Background);
  display_.drawFastHLine(0, FooterY, UiTheme::ScreenWidth, UiTheme::PanelBorder);
  UiButton(HomeRect, "HOME").draw(display_);
  UiButton(CancelRect, "ANNULEREN").draw(display_);
  UiButton(SaveRect, "OPSLAAN").draw(display_, true);
}

void SettingsScreen::drawList() {
  display_.fillRect(ViewX, ViewY, ViewW, ViewH, UiTheme::Background);
  const int16_t offset = scrollView_.offset();

  drawSection("ALGEMEEN", 8 - offset);
  drawRow("Aantal cilinders", "", 28 - offset);
  const int16_t segmentY = ViewY + 28 - offset + 6;
  if (segmentY >= ViewY && segmentY + 24 <= ViewY + ViewH) {
    drawSegmentButton("2", ViewX + 172, segmentY, 40, draft_.cylinders == 2);
    drawSegmentButton("4", ViewX + 218, segmentY, 40, draft_.cylinders == 4);
    drawSegmentButton("6", ViewX + 264, segmentY, 40, draft_.cylinders == 6);
  }
  drawRow("Eenheden", unitsName(), 64 - offset, true);
  drawRow("Auto schaal", draft_.autoScale ? "AAN" : "UIT", 100 - offset, true,
          draft_.autoScale);

  drawSection("WEERGAVE", 142 - offset);
  drawRow("Demping", dampingName(), 162 - offset, true);
  drawRow("Auto helderheid", draft_.autoBrightness ? "AAN" : "UIT", 198 - offset, true,
          draft_.autoBrightness);
  drawRow("Helderheid", String(draft_.brightness).c_str(), 234 - offset, true);
  drawRow("Alarmdrempel", String(draft_.alarmDeltaKpa, 0).c_str(), 270 - offset, true);

  drawSection("SENSOREN", 312 - offset);
  drawRow("DEMO MODUS", draft_.demoFallback ? "AAN" : "UIT", 332 - offset, true,
          draft_.demoFallback);
  drawRow("Motor", engineStrokeName(), 368 - offset, true);
  drawRow("RPM bron", rpmSourceName(), 404 - offset, true);
  drawRow("Kalibratie", "OPEN", 440 - offset, true);

  drawSection("SD KAART", 482 - offset);
  drawRow("Status", sdManager_.mounted() ? "OK" : "Niet gevonden", 502 - offset);
  drawRow("Grootte", String(sdManager_.cardSizeMb()) + " MB", 538 - offset);
  drawRow("Vrij", String(sdManager_.freeSpaceMb()) + " MB", 574 - offset);
  drawRow("Laatste fout", sdManager_.lastErrorName(), 610 - offset);
  drawRow("TEST SD", "START", 646 - offset, true);
  drawRow("MAPPEN HERSTELLEN", "START", 682 - offset, true);
  drawRow("EXPORT INSTELLINGEN", "START", 718 - offset, true);
  drawRow("IMPORT INSTELLINGEN", "START", 754 - offset, true);

  scrollView_.drawScrollbar(display_);
  lastOffset_ = offset;
}

void SettingsScreen::drawMessage(uint32_t nowMs) {
  static bool visible = false;
  const bool shouldShow = messageUntilMs_ > nowMs && message_[0] != '\0';
  if (shouldShow == visible) {
    return;
  }
  visible = shouldShow;
  display_.fillRect(48, 180, 224, 16, shouldShow ? UiTheme::PanelAlt : UiTheme::Background);
  if (!shouldShow) {
    return;
  }
  display_.drawRect(48, 180, 224, 16, UiTheme::AccentBlue);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(56, 184);
  display_.print(message_);
}

void SettingsScreen::drawSection(const char *title, int16_t y) {
  const int16_t screenY = ViewY + y;
  if (screenY < ViewY || screenY + 14 > ViewY + ViewH) {
    return;
  }
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::AccentBlue);
  display_.setCursor(ViewX + 8, screenY);
  display_.print(title);
  display_.drawFastHLine(ViewX + 8, screenY + 13, ViewW - 20, UiTheme::PanelBorder);
}

void SettingsScreen::drawRow(const char *label, const String &value, int16_t y, bool button,
                             bool active) {
  const int16_t screenY = ViewY + y;
  if (screenY < ViewY || screenY + RowHeight > ViewY + ViewH) {
    return;
  }

  display_.fillRect(ViewX + 4, screenY, ViewW - 14, RowHeight - 2, UiTheme::Panel);
  display_.drawFastHLine(ViewX + 6, screenY + RowHeight - 2, ViewW - 18, UiTheme::PanelBorder);
  display_.setTextSize(1);
  display_.setTextColor(UiTheme::Text);
  display_.setCursor(ViewX + 12, screenY + 13);
  display_.print(label);
  if (button) {
    UiButton({static_cast<int16_t>(ViewX + 210), static_cast<int16_t>(screenY + 5), 92, 24},
             value.c_str())
        .draw(display_, active);
  } else {
    display_.setTextColor(UiTheme::TextMuted);
    display_.setCursor(ViewX + 204, screenY + 13);
    display_.print(value);
  }
}

void SettingsScreen::drawSegmentButton(const char *label, int16_t x, int16_t y, int16_t w,
                                       bool active) {
  UiButton({x, y, w, 24}, label).draw(display_, active);
}

void SettingsScreen::handleClick(int16_t screenX, int16_t screenY) {
  if (rectHit(BackRect, screenX, screenY) || rectHit(HomeRect, screenX, screenY) ||
      rectHit(CancelRect, screenX, screenY)) {
    loadDraft();
    pendingAction_ = SettingsAction::Home;
    return;
  }
  if (rectHit(SaveRect, screenX, screenY)) {
    applyDraft();
    const bool ok = settings_.save();
    setMessage(ok ? "Instellingen opgeslagen" : "Opslaan mislukt", millis());
    drawList();
    return;
  }

  if (!scrollView_.contains(screenX, screenY)) {
    return;
  }

  const int16_t y = scrollView_.contentY(screenY);
  if (contentHit(y, 28)) {
    if (screenX >= ViewX + 172 && screenX < ViewX + 212) {
      draft_.cylinders = 2;
    } else if (screenX >= ViewX + 218 && screenX < ViewX + 258) {
      draft_.cylinders = 4;
    } else if (screenX >= ViewX + 264 && screenX < ViewX + 304) {
      draft_.cylinders = 6;
    }
    drawList();
  } else if (contentHit(y, 64)) {
    draft_.units = draft_.units == SettingsManager::Units::Kpa ? SettingsManager::Units::InHg
                                                               : SettingsManager::Units::Kpa;
    drawList();
  } else if (contentHit(y, 100)) {
    draft_.autoScale = !draft_.autoScale;
    drawList();
  } else if (contentHit(y, 162)) {
    draft_.damping = draft_.damping == SettingsManager::DampingMode::Low
                         ? SettingsManager::DampingMode::Normal
                         : (draft_.damping == SettingsManager::DampingMode::Normal
                                ? SettingsManager::DampingMode::High
                                : SettingsManager::DampingMode::Low);
    drawList();
  } else if (contentHit(y, 198)) {
    draft_.autoBrightness = !draft_.autoBrightness;
    drawList();
  } else if (contentHit(y, 234)) {
    draft_.brightness = draft_.brightness >= 100 ? 40 : draft_.brightness + 10;
    drawList();
  } else if (contentHit(y, 270)) {
    draft_.alarmDeltaKpa = draft_.alarmDeltaKpa >= 20.0f ? 2.0f : draft_.alarmDeltaKpa + 1.0f;
    drawList();
  } else if (contentHit(y, 332)) {
    draft_.demoFallback = !draft_.demoFallback;
    drawList();
  } else if (contentHit(y, 368)) {
    draft_.engineStroke = draft_.engineStroke == SettingsManager::EngineStroke::FourStroke
                              ? SettingsManager::EngineStroke::TwoStroke
                              : SettingsManager::EngineStroke::FourStroke;
    drawList();
  } else if (contentHit(y, 404)) {
    const uint8_t next = (static_cast<uint8_t>(draft_.rpmSource) + 1) % 5;
    draft_.rpmSource = static_cast<SettingsManager::RpmSource>(next);
    drawList();
  } else if (contentHit(y, 440)) {
    pendingAction_ = SettingsAction::Calibration;
  } else if (contentHit(y, 646)) {
    Serial.println("settings action=TEST_SD");
    const bool ok = sdManager_.writeTestFile() && sdManager_.readTestFile();
    setMessage(ok ? "SD test OK" : sdManager_.lastErrorName(), millis());
    drawList();
  } else if (contentHit(y, 682)) {
    Serial.println("settings action=REPAIR_SD_FOLDERS");
    const bool ok = sdManager_.repairFilesystemLayout();
    setMessage(ok ? "SD mappen hersteld" : sdManager_.lastErrorName(), millis());
    drawList();
  } else if (contentHit(y, 718)) {
    Serial.println("settings action=EXPORT_SETTINGS");
    applyDraft();
    settings_.save();
    const bool ok = sdManager_.exportSettings(settings_);
    setMessage(ok ? "Export OK" : sdManager_.lastErrorName(), millis());
    drawList();
  } else if (contentHit(y, 754)) {
    Serial.println("settings action=IMPORT_SETTINGS");
    const bool ok = sdManager_.importSettings(settings_);
    loadDraft();
    setMessage(ok ? "Import OK" : sdManager_.lastErrorName(), millis());
    drawList();
  }
}

void SettingsScreen::setMessage(const char *message, uint32_t nowMs) {
  strncpy(message_, message, sizeof(message_) - 1);
  message_[sizeof(message_) - 1] = '\0';
  messageUntilMs_ = nowMs + 1800;
}

const char *SettingsScreen::unitsName() const {
  return draft_.units == SettingsManager::Units::Kpa ? "kPa" : "inHg";
}

const char *SettingsScreen::dampingName() const {
  switch (draft_.damping) {
    case SettingsManager::DampingMode::Low:
      return "laag";
    case SettingsManager::DampingMode::Normal:
      return "normaal";
    case SettingsManager::DampingMode::High:
      return "hoog";
  }
  return "normaal";
}

const char *SettingsScreen::engineStrokeName() const {
  return draft_.engineStroke == SettingsManager::EngineStroke::FourStroke ? "4-takt" : "2-takt";
}

const char *SettingsScreen::rpmSourceName() const {
  switch (draft_.rpmSource) {
    case SettingsManager::RpmSource::Ch1:
      return "CH1";
    case SettingsManager::RpmSource::Ch2:
      return "CH2";
    case SettingsManager::RpmSource::Ch3:
      return "CH3";
    case SettingsManager::RpmSource::Ch4:
      return "CH4";
    case SettingsManager::RpmSource::Auto:
      return "Auto";
  }
  return "CH1";
}
