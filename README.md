# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v6.5.3-alpha1`.

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

On the ESP32-2432S028R display, the firmware shows the hardware selftest for
5 seconds, then opens the first Carbtune dashboard with demo channel data.

## Touch Diagnostics And Calibration

Touch input is read through the display firmware `TouchInput` module. It polls
the XPT2046 controller over SPI, filters idle values such as `8191,8191`, maps
valid readings to the 320x240 screen, and logs touches on the serial monitor:

```text
Touch raw=1234,2345 mapped=82,137 pressed
```

Tap `MENU` on the dashboard to open the touch diagnostics screen. Tap `MENU`
again to return to the dashboard. The diagnostics screen shows raw X/Y,
pressure Z, mapped screen X/Y, pressed state, long-press state, IRQ pin state,
and the active touch controller.

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
