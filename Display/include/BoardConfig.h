#pragma once

// =============================
// ESP32-2432S028R
// Cheap Yellow Display
// =============================

// TFT
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1
#define TFT_BL   21

// Touch
#define TOUCH_IRQ   36
#define TOUCH_MOSI  32
#define TOUCH_MISO  39
#define TOUCH_CLK   25
#define TOUCH_CS    33
#define TOUCH_IDLE_X 8191
#define TOUCH_IDLE_Y 8191
#define TOUCH_MIN_X 320
#define TOUCH_MAX_X 3860
#define TOUCH_MIN_Y 260
#define TOUCH_MAX_Y 3900
#define TOUCH_SWAP_XY 0
#define TOUCH_INVERT_X 0
#define TOUCH_INVERT_Y 0
#define TOUCH_SCREEN_WIDTH 320
#define TOUCH_SCREEN_HEIGHT 240

// SD
#define SD_MISO 19
#define SD_MOSI 23
#define SD_SCK  18
#define SD_CS   5

// RGB LED (active LOW)
#define LED_RED    4
#define LED_GREEN 16
#define LED_BLUE  17

// Extra
#define SPEAKER_PIN 26
#define LDR_PIN     34

// UART SensorNode
#define SENSOR_RX 27
#define SENSOR_TX 22
