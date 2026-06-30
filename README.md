# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

## Layout

- `Display/` - ESP32-2432S028R Cheap Yellow Display firmware
- `SensorNode/` - UART sensor node firmware
- `Shared/` - shared protocol/version PlatformIO library

## Build

```sh
pio run
```

The display target uses Arduino_GFX for the TFT.
