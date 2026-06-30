# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v6.6.0-alpha1`.

## Layout

- `Display/` - ESP32-2432S028R Cheap Yellow Display firmware
- `SensorNode/` - UART sensor node firmware
- `Shared/` - shared protocol/version PlatformIO library

## Build

```sh
pio run
pio run -e display -t upload
pio run -e sensornode -t upload
pio device monitor -e display
pio device monitor -e sensornode
```

The display target uses Arduino_GFX for the TFT.

Display upload/monitor port: `COM14`.
SensorNode upload/monitor port: `COM9`.

On the ESP32-2432S028R display, the firmware shows a dark Carbtune splash
screen for about 2.5 seconds, then opens the Carbtune ESP32 dashboard with demo
channel data.

## Display UI

The v6.6 display UI uses a dark 320x240 layout with white text, blue controls,
green normal meters, and yellow/red warning states.

- Splash screen: `CARBTUNE PRO ESP32` with initializing progress indicator.
- Dashboard: 2- or 4-cylinder meter layout, kPa values, max difference,
  status, and a `MENU` button.
- Settings: cylinder count, units, auto scale, damping/filter, calibration
  start, about row, and bottom navigation.
- Graph: 60 second demo history with four colored traces and range controls.
- Calibration: zero-kPa instruction screen with current pressure and START.
- Diagnostics: live XPT2046 touch raw/mapped values and controller state.

Touch is still being debugged, so every screen can also be selected from the
display serial monitor:

```text
h = home/dashboard
s = settings
g = graph
c = calibration
d = touch diagnostics
```

When touch is available, `MENU` opens settings. The settings bottom navigation
can return HOME or open GRAFIEK, and the calibration row opens the zero-kPa
calibration screen.

## Touch Diagnostics And Calibration

Touch input is read through the display firmware `TouchInput` module. It polls
the XPT2046 controller over SPI, filters idle values such as `8191,8191`, maps
valid readings to the 320x240 screen, and logs touches on the serial monitor:

```text
Touch raw=1234,2345 mapped=82,137 pressed
```

Send `d` in `pio device monitor -e display` to open the touch diagnostics
screen. It shows raw X/Y, pressure Z, mapped screen X/Y, pressed state,
long-press state, IRQ pin state, and the active touch controller.

To tune a different panel, open `Display/include/BoardConfig.h` and adjust:

- `TOUCH_MIN_X`
- `TOUCH_MAX_X`
- `TOUCH_MIN_Y`
- `TOUCH_MAX_Y`
- `TOUCH_SWAP_XY`
- `TOUCH_INVERT_X`
- `TOUCH_INVERT_Y`

Use `pio device monitor -e display`, press the screen corners, and copy the
observed raw values into those constants.
