#include "Widgets/StatusBadge.h"

#include "Theme/Theme.h"

StatusBadge::StatusBadge(Rect bounds, const char *label) : bounds_(bounds), label_(label) {}

void StatusBadge::draw(Arduino_GFX &display, uint16_t color) const {
  display.fillRoundRect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, 4, Theme::Panel);
  display.drawRoundRect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, 4, color);
  display.setTextSize(1);
  display.setTextColor(color);
  display.setCursor(bounds_.x + 8, bounds_.y + ((bounds_.h - 8) / 2));
  display.print(label_);
}
