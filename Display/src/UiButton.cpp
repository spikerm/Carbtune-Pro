#include "UiButton.h"

#include "UiTheme.h"

UiButton::UiButton(UiRect rect, const char *label) : rect_(rect), label_(label) {}

void UiButton::draw(Arduino_GFX &display, bool active) const {
  display.fillRoundRect(rect_.x, rect_.y, rect_.w, rect_.h, 4,
                        active ? UiTheme::Accent : UiTheme::PanelAlt);
  display.drawRoundRect(rect_.x, rect_.y, rect_.w, rect_.h, 4,
                        active ? UiTheme::Text : UiTheme::Border);
  display.setTextSize(1);
  display.setTextColor(UiTheme::Text);
  display.setCursor(rect_.x + 8, rect_.y + ((rect_.h - 8) / 2));
  display.print(label_);
}

bool UiButton::contains(int16_t x, int16_t y) const {
  return x >= rect_.x && x < rect_.x + rect_.w &&
         y >= rect_.y && y < rect_.y + rect_.h;
}

UiRect UiButton::rect() const {
  return rect_;
}
