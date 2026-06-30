# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v7.0.0-alpha4`.

## Layout

- `Display/` - ESP32-2432S028R Cheap Yellow Display firmware
- `SensorNode/` - UART sensor node firmware
- `Shared/` - shared protocol/version PlatformIO library

Display firmware is being refactored into the v7 architecture:

```text
Display/src/App
Display/src/Screens
Display/src/Widgets
Display/src/Theme
Display/src/Touch
Display/src/Sensors
Display/src/Storage
Display/src/Assets
```

The v7.0.0-alpha4 build keeps the dark dashboard and touch MENU flow, fixes the
demo-mode toggle, improves SD detection on the CYD board, reduces dashboard
redraw flicker, and prepares restrictorless hose operation with filtered
pressure and RPM diagnostics.

## Build

```sh
pio run
```

## Upload

```sh
pio run -e display -t upload
pio run -e sensornode -t upload
```

Display upload/monitor port: `COM14`.
SensorNode upload/monitor port: `COM9`.

## Monitor

```sh
pio device monitor -e display
pio device monitor -e sensornode
```

## Display UI

The display target uses Arduino_GFX for the TFT.

- Startup splash: dark screen, firmware version, initialization checklist, then dashboard.
- Dashboard: adaptive 2 to 6 cylinder layout, vertical meters, live/demo/no-data
  source, RPM, CH1 reference line, max delta in the header, large status badge,
  MENU.
- Settings: scrollable list, 2/3/4/5/6 cylinder selection, brightness settings,
  demo mode toggle, damping, engine stroke, RPM source, calibration shortcut,
  navigation.
- Graph: dark demo graph view.
- Calibration: zero-kPa placeholder workflow.
- Diagnostics: touch raw/mapped values, LDR/backlight diagnostics, sensor mode,
  cylinder count, CH1 reference value, per-channel delta, max delta, SD status,
  raw/filtered ADC values, pulse Hz, RPM, and RPM stability.

## SD Card

The display initializes SD through `SdManager` after TFT setup using a separate
SPI bus instance. Before TFT, touch, or SD SPI init, all chip-select pins are
set high:

- `TFT_CS`
- `TOUCH_CS`
- `SD_CS`

SD starts at 4 MHz and retries once at 1 MHz. Splash and diagnostics show
`SD OK <MB>`, `SD FAIL NO CARD`, or `SD FAIL INIT`. SD failures are logged but
do not block display or touch.

## Cylinder Reference Mode

Cylinder 1 is the dashboard reference. The blue `REF CH1` line is drawn across
all meters at the current CH1 vacuum level. Other channels show their delta
relative to CH1. Status thresholds are temporary runtime constants:

- `GOED`: max CH1 delta <= 2 kPa
- `LET OP`: max CH1 delta <= 5 kPa
- `BIJSTELLEN`: max CH1 delta > 5 kPa

These thresholds are intended to become configurable later.

## SensorNode UART

The display listens for shared `SensorFrame` packets on `Serial2` and validates
magic, version, type, payload length, and checksum. Valid frames switch the
sensor source to `LIVE`. If frames stop for more than 2 seconds, the display
falls back to demo data when demo fallback is enabled.

Current pins:

- Display RX: `GPIO27`
- Display TX: `GPIO22`
- SensorNode RX: `GPIO22`
- SensorNode TX: `GPIO27`

Wiring:

```text
Display TX -> SensorNode RX
Display RX -> SensorNode TX
GND        -> GND
```

The temporary raw conversion maps ADC `0..4095` to `-100..0 kPa` until pressure
calibration is added.

## Restrictorless Hose Preparation

SensorNode samples ADC channels every 5 ms and sends SensorFrame packets every
50 ms. Each frame now contains raw ADC values, IIR-filtered ADC values, pulse
frequency per channel, RPM estimate, and status flags. Pulse detection uses a
simple hysteresis window around the filtered value. CH1 is currently the primary
RPM source.

For four-stroke engines:

```text
RPM = pulseFrequencyHz * 120
```

If pulses are not stable, the display shows `RPM --` and diagnostics reports
`RPM unstable`. Damping settings are prepared as runtime options:

- laag: alpha `0.20`
- normaal: alpha `0.10`
- hoog: alpha `0.05`

## Backlight

The display reads the LDR on `LDR_PIN` and smooths the value before changing
backlight brightness. Defaults are:

- `autoBrightness = true`
- `brightnessMin = 40%`
- `brightnessMax = 100%`
- `brightnessManual = 80%`

If PWM/LEDC attach fails, the firmware keeps the display on with a fixed
backlight. If LDR raw remains `0`, diagnostics shows `0 / check pin` while the
firmware continues normally.

## Serial Navigation

Use the display monitor for debug navigation:

```text
h = home/dashboard
s = settings
g = graph
c = calibration
d = diagnostics
r = splash
```

## Touch Diagnostics And Calibration

Touch input polls the XPT2046 controller over SPI, filters idle values such as
`8191,8191`, maps valid readings to the 320x240 screen, and logs touches on the
serial monitor.

To tune a different panel, open `Display/include/BoardConfig.h` and adjust:

- `TOUCH_MIN_X`
- `TOUCH_MAX_X`
- `TOUCH_MIN_Y`
- `TOUCH_MAX_Y`
- `TOUCH_SWAP_XY`
- `TOUCH_INVERT_X`
- `TOUCH_INVERT_Y`
