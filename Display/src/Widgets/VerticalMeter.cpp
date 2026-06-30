#include "Widgets/VerticalMeter.h"

#include "Theme/Theme.h"

VerticalMeter::VerticalMeter(Rect bounds) : bounds_(bounds) {}

void VerticalMeter::drawFrame(Arduino_GFX &display) const {
  display.drawRect(bounds_.x, bounds_.y, bounds_.w, bounds_.h, Theme::PanelBorder);
}

void VerticalMeter::drawValue(Arduino_GFX &display, float valueKpa, uint16_t color) const {
  const int16_t fillHeight =
      constrain(static_cast<int16_t>((-valueKpa) * bounds_.h / 100.0f), 0, bounds_.h);
  const int16_t fillY = bounds_.y + bounds_.h - fillHeight;
  display.fillRect(bounds_.x + 1, bounds_.y + 1, bounds_.w - 2, bounds_.h - 2, Theme::Background);
  display.fillRect(bounds_.x + 1, fillY, bounds_.w - 2, fillHeight, color);
  display.drawFastHLine(bounds_.x - 4, fillY, bounds_.w + 8, Theme::Text);
}
