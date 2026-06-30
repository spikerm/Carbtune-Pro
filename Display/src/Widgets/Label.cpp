#include "Widgets/Label.h"

Label::Label(int16_t x, int16_t y, const char *text, uint8_t size)
    : x_(x), y_(y), text_(text), size_(size) {}

void Label::draw(Arduino_GFX &display, uint16_t color) const {
  display.setTextSize(size_);
  display.setTextColor(color);
  display.setCursor(x_, y_);
  display.print(text_);
}
