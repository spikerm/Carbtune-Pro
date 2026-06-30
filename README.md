# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v6.5.2-alpha1`.

## Layout

- `Display/` - ESP32-2432S028R Cheap Yellow Display firmware
- `SensorNode/` - UART sensor node firmware
- `Shared/` - shared protocol/version PlatformIO library

## Build

```sh
pio run
pio run -e display -t upload
pio device monitor -b 115200
```

The display target uses Arduino_GFX for the TFT.

On the ESP32-2432S028R display, the firmware shows the hardware selftest for
5 seconds, then opens the first Carbtune dashboard with demo channel data.

## Touch Calibration

Touch input is read through the display firmware `TouchInput` module. It filters
idle values such as `8191,8191`, maps valid XPT2046 readings to the 320x240
screen, and logs each change on the serial monitor:

```text
Touch raw=1234,2345 mapped=82,137 pressed
Touch raw=8191,8191 mapped=not pressed released
```

To tune a different panel, open `Display/include/BoardConfig.h` and adjust:

- `TOUCH_CAL_MIN_X`
- `TOUCH_CAL_MAX_X`
- `TOUCH_CAL_MIN_Y`
- `TOUCH_CAL_MAX_Y`

Use `pio device monitor -b 115200`, press the screen corners, and copy the
observed raw values into those constants.
