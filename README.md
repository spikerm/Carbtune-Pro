# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v7.0.0-alpha2`.

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

The v7.0.0-alpha2 build keeps the dark dashboard and touch MENU flow, adds a
scrollable settings list for the 320x240 display, and can use SensorNode UART
frames with demo fallback.

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
  source, CH1 reference line, max delta in the header, large status badge, MENU.
- Settings: scrollable list, 2/3/4/5/6 cylinder selection, brightness settings,
  calibration shortcut, navigation.
- Graph: dark demo graph view.
- Calibration: zero-kPa placeholder workflow.
- Diagnostics: touch raw/mapped values, LDR/backlight diagnostics, sensor mode,
  cylinder count, CH1 reference value, per-channel delta, and max delta.

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
