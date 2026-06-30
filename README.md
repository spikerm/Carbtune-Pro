# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v6.5.1-alpha1`.

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
