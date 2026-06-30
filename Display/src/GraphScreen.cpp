#include "GraphScreen.h"

#include "UiButton.h"
#include "UiTheme.h"

namespace {
constexpr int16_t PlotX = 44;
constexpr int16_t PlotY = 42;
constexpr int16_t PlotW = 248;
constexpr int16_t PlotH = 100;
constexpr UiRect PauseRect{8, 204, 38, 31};
constexpr UiRect SixtyRect{52, 204, 58, 31};
constexpr UiRect OneTwentyRect{116, 204, 58, 31};
constexpr UiRect ThreeHundredRect{180, 204, 58, 31};
constexpr UiRect ClearRect{244, 204, 66, 31};

int16_t mapKpa(float value) {
  if (value > 0.0f) {
    value = 0.0f;
  }
  if (value < -100.0f) {
    value = -100.0f;
  }
  return PlotY + static_cast<int16_t>((-value * PlotH) / 100.0f);
}

void drawHeader(Arduino_GFX &display) {
  display.fillScreen(UiTheme::Background);
  display.fillRect(0, 0, UiTheme::ScreenWidth, 30, 0x0208);
  display.drawLine(0, 30, UiTheme::ScreenWidth, 30, UiTheme::Border);
  UiButton({8, 5, 32, 22}, "<").draw(display);
  display.setTextColor(UiTheme::Text);
  display.setTextSize(2);
  display.setCursor(128, 8);
  display.print("GRAFIEK");
  display.setTextSize(1);
  display.setCursor(278, 10);
  display.print("12:47");
}
}  // namespace

GraphScreen::GraphScreen(Arduino_GFX &display) : display_(display) {}

void GraphScreen::begin() {
  drawHeader(display_);
  UiTheme::drawPanel(display_, 8, 36, 304, 162);
  drawGraph();
  drawLegend();

  UiButton(PauseRect, "||").draw(display_, true);
  UiButton(SixtyRect, "60 SEC").draw(display_, true);
  UiButton(OneTwentyRect, "120 SEC").draw(display_);
  UiButton(ThreeHundredRect, "300 SEC").draw(display_);
  UiButton(ClearRect, "WISSEN").draw(display_);
}

bool GraphScreen::isHomeHit(int16_t x, int16_t y) const {
  return x < 60 && y < 38;
}

bool GraphScreen::isSettingsHit(int16_t x, int16_t y) const {
  return UiButton(ClearRect, "").contains(x, y);
}

void GraphScreen::drawGraph() {
  display_.drawRect(PlotX, PlotY, PlotW, PlotH, UiTheme::Border);
  display_.setTextColor(UiTheme::Text);
  display_.setTextSize(1);
  const int16_t labels[] = {0, -25, -50, -75, -100};
  for (uint8_t i = 0; i < 5; ++i) {
    const int16_t y = PlotY + (i * PlotH / 4);
    display_.drawFastHLine(PlotX, y, PlotW, UiTheme::Border);
    display_.setCursor(16, y - 3);
    display_.print(labels[i]);
  }
  for (uint8_t i = 1; i < 4; ++i) {
    const int16_t x = PlotX + (i * PlotW / 4);
    display_.drawFastVLine(x, PlotY, PlotH, UiTheme::Border);
  }

  const uint16_t colors[] = {UiTheme::Warning, 0x07FF, UiTheme::Good, UiTheme::Danger};
  const float base[] = {-42.0f, -48.0f, -40.0f, -62.0f};
  for (uint8_t channel = 0; channel < 4; ++channel) {
    int16_t prevX = PlotX;
    int16_t prevY = mapKpa(base[channel]);
    for (uint8_t sample = 1; sample < 60; ++sample) {
      const int16_t x = PlotX + static_cast<int16_t>((sample * PlotW) / 59);
      const float wobble = ((sample + channel * 7) % 9) - 4;
      const float value = base[channel] + wobble + ((sample % 13) == 0 ? 3.0f : 0.0f);
      const int16_t y = mapKpa(value);
      display_.drawLine(prevX, prevY, x, y, colors[channel]);
      prevX = x;
      prevY = y;
    }
  }

  display_.setTextColor(UiTheme::Text);
  display_.setCursor(40, 150);
  display_.print("-60s");
  display_.setCursor(126, 150);
  display_.print("-30s");
  display_.setCursor(274, 150);
  display_.print("0s");
}

void GraphScreen::drawLegend() {
  const uint16_t colors[] = {UiTheme::Warning, 0x07FF, UiTheme::Good, UiTheme::Danger};
  const char *labels[] = {"1  -41 kPa", "2  -48 kPa", "3  -40 kPa", "4  -62 kPa"};
  for (uint8_t i = 0; i < 4; ++i) {
    const int16_t x = (i % 2 == 0) ? 52 : 176;
    const int16_t y = (i < 2) ? 166 : 184;
    display_.drawFastHLine(x, y + 4, 16, colors[i]);
    display_.setTextColor(UiTheme::Text);
    display_.setTextSize(1);
    display_.setCursor(x + 24, y);
    display_.print(labels[i]);
  }
}
