#include "UiTheme.h"

namespace UiTheme {

void drawPanel(Arduino_GFX &display, int16_t x, int16_t y, int16_t w, int16_t h) {
  display.fillRoundRect(x, y, w, h, 4, Panel);
  display.drawRoundRect(x, y, w, h, 4, Border);
}

void drawTopBar(Arduino_GFX &display, const char *title, const char *clockText) {
  display.fillRect(0, 0, ScreenWidth, 30, Background);
  drawWifiIcon(display, 12, 11);
  display.setTextSize(2);
  display.setTextColor(Text);
  display.setCursor(82, 8);
  display.print(title);
  display.setTextSize(1);
  display.setCursor(282, 10);
  display.print(clockText);
  display.drawFastHLine(8, 30, ScreenWidth - 16, Border);
}

void drawBottomDivider(Arduino_GFX &display) {
  display.drawFastHLine(8, 202, ScreenWidth - 16, Border);
}

void drawWifiIcon(Arduino_GFX &display, int16_t x, int16_t y) {
  display.drawCircle(x, y, 12, Text);
  display.drawCircle(x, y, 8, Text);
  display.drawCircle(x, y, 4, Text);
  display.fillRect(x - 14, y, 28, 14, Background);
  display.fillCircle(x, y + 4, 2, Text);
}

}  // namespace UiTheme
