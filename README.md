# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v7.0.0-alpha1`.

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

The current v7 start keeps the working dark dashboard, touch MENU flow, serial
navigation, and demo channel data while introducing `AppController`,
`ScreenManager`, theme/widget primitives, runtime settings, demo
`SensorManager`, and LDR based `BacklightManager`.

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
- Dashboard: four kPa channels, vertical meters, max difference, status, MENU.
- Settings: cylinder controls, brightness settings, calibration shortcut, navigation.
- Graph: dark demo graph view.
- Calibration: zero-kPa placeholder workflow.
- Diagnostics: touch raw/mapped values plus LDR/backlight diagnostics.

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
