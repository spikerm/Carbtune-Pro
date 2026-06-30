#pragma once

#include <Arduino_GFX_Library.h>

namespace Theme {

static constexpr int16_t ScreenWidth = 320;
static constexpr int16_t ScreenHeight = 240;

static constexpr uint16_t Background = 0x0004;
static constexpr uint16_t Panel = 0x0820;
static constexpr uint16_t PanelAlt = 0x1041;
static constexpr uint16_t PanelBorder = 0x2945;
static constexpr uint16_t Text = 0xFFFF;
static constexpr uint16_t TextMuted = 0x8410;
static constexpr uint16_t AccentBlue = 0x04BF;
static constexpr uint16_t AccentBlueDark = 0x0255;
static constexpr uint16_t GoodGreen = 0x47E0;
static constexpr uint16_t WarnYellow = 0xFEA0;
static constexpr uint16_t AlarmRed = 0xF800;

void clear(Arduino_GFX &display);
void drawPanel(Arduino_GFX &display, int16_t x, int16_t y, int16_t w, int16_t h);
void drawTopBar(Arduino_GFX &display, const char *title, const char *clockText = "");

}  // namespace Theme
