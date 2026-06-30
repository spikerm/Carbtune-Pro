#include "Theme/Theme.h"

namespace Theme {

void clear(Arduino_GFX &display) {
  display.fillScreen(Background);
}

void drawPanel(Arduino_GFX &display, int16_t x, int16_t y, int16_t w, int16_t h) {
  display.fillRoundRect(x, y, w, h, 4, Panel);
  display.drawRoundRect(x, y, w, h, 4, PanelBorder);
}

void drawTopBar(Arduino_GFX &display, const char *title, const char *clockText) {
  display.fillRect(0, 0, ScreenWidth, 30, Background);
  display.setTextSize(2);
  display.setTextColor(Text);
  display.setCursor(72, 8);
  display.print(title);
  if (clockText && clockText[0] != '\0') {
    display.setTextSize(1);
    display.setCursor(282, 10);
    display.print(clockText);
  }
  display.drawFastHLine(8, 30, ScreenWidth - 16, PanelBorder);
}

}  // namespace Theme
