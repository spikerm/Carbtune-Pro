#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

namespace UiTheme {

static constexpr int16_t ScreenWidth = 320;
static constexpr int16_t ScreenHeight = 240;

static constexpr uint16_t Background = 0x0000;
static constexpr uint16_t Panel = 0x0841;
static constexpr uint16_t PanelAlt = 0x1082;
static constexpr uint16_t Border = 0x39E7;
static constexpr uint16_t Text = 0xFFFF;
static constexpr uint16_t Muted = 0x8410;
static constexpr uint16_t Accent = 0x04BF;
static constexpr uint16_t AccentDark = 0x0255;
static constexpr uint16_t Good = 0x47E0;
static constexpr uint16_t Warning = 0xFFE0;
static constexpr uint16_t Danger = 0xF800;

void drawPanel(Arduino_GFX &display, int16_t x, int16_t y, int16_t w, int16_t h);
void drawTopBar(Arduino_GFX &display, const char *title, const char *clockText = "12:45");
void drawBottomDivider(Arduino_GFX &display);
void drawWifiIcon(Arduino_GFX &display, int16_t x, int16_t y);

}  // namespace UiTheme
