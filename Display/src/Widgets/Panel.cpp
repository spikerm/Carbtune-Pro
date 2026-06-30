#include "Widgets/Panel.h"

#include "Theme/Theme.h"

Panel::Panel(Rect bounds) : bounds_(bounds) {}

void Panel::draw(Arduino_GFX &display) const {
  Theme::drawPanel(display, bounds_.x, bounds_.y, bounds_.w, bounds_.h);
}
