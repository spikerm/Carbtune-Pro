#include "Widgets/Button.h"

#include "Theme/Theme.h"

Button::Button(Rect bounds, const char *label) : bounds_(bounds), label_(label) {}

void Button::draw(Arduino_GFX &display, bool active) const {
  display.fillRoundRect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, 4,
                        active ? Theme::AccentBlue : Theme::PanelAlt);
  display.drawRoundRect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, 4,
                        active ? Theme::Text : Theme::AccentBlue);
  display.setTextSize(1);
  display.setTextColor(Theme::Text);
  display.setCursor(bounds_.x + 8, bounds_.y + ((bounds_.h - 8) / 2));
  display.print(label_);
}

bool Button::contains(int16_t x, int16_t y) const {
  return bounds_.contains(x, y);
}
