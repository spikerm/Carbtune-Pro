# Carbtune-Pro

ESP32 carbtune firmware workspace for the display and sensor node.

Current firmware version: `v7.0.0-alpha5`.

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

The v7.0.0-alpha5 build keeps the dark dashboard and touch MENU flow, adds a
full-screen settings workflow with save/cancel buttons, stores normal
configuration in ESP32 NVS, and adds SD card maintenance actions for folders,
test files, settings export, and settings import.

## Build

```sh
pio run
```

## GitHub Firmware Artifacts

Every push to `main` and every `v*` tag builds both PlatformIO environments in
GitHub Actions:

```sh
pio run -e display
pio run -e sensornode
```

The workflow uploads one downloadable artifact named:

```text
carbtune-firmware-<git-sha>
```

It contains:

```text
display-firmware.bin      Display firmware, used later for display OTA updates
sensornode-firmware.bin   SensorNode firmware
version.txt               firmware version, commit SHA, build time, environments
```

Download it from GitHub:

1. Open the repository on GitHub.
2. Go to `Actions`.
3. Open the latest successful PlatformIO run.
4. Download `carbtune-firmware-<git-sha>` from the artifacts section.

For future OTA updates, the display updater will use `display-firmware.bin`.
SensorNode update support will be handled separately later.

When a tag matching `v*` is pushed, the same files are also uploaded as release
assets.

## Display OTA Update

The display web interface can install a display firmware update directly from
GitHub. The updater downloads:

```text
https://github.com/spikerm/Carbtune-Pro/releases/latest/download/display-firmware.bin
```

Requirements:

- The display must be connected to WiFi as a station.
- AP mode can stay active for service access, but OTA download does not work
  through AP-only mode.
- Keep the device powered during the update.
- Publish a `v*` GitHub release first so `display-firmware.bin` exists as a
  release asset.

Steps:

1. Connect to the Carbtune web interface.
2. Save WiFi credentials and wait until station mode is connected.
3. Open the `GitHub OTA update` section.
4. Press `Display firmware updaten`.
5. The display downloads the release asset, writes the OTA partition, and
   restarts automatically after a complete update.

HTTPS verification is enabled with the pinned GitHub/Sectigo root certificate.
If GitHub changes its certificate chain in the future, `OtaManager` may need an
updated root certificate.

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
- Settings: full-screen scrollable list with fixed header/footer, 2/4/6 cylinder
  segment buttons, brightness settings, demo mode toggle, damping, engine stroke,
  RPM source, calibration shortcut, SD card maintenance, HOME, ANNULEREN, and
  OPSLAAN.
- Graph: dark demo graph view.
- Calibration: zero-kPa placeholder workflow.
- Diagnostics: touch raw/mapped values, LDR/backlight diagnostics, sensor mode,
  cylinder count, CH1 reference value, per-channel delta, max delta, SD status,
  raw/filtered ADC values, pulse Hz, RPM, and RPM stability.

## Web Interface And WiFi

The display starts a local access point and a small built-in web interface.
Connect to:

```text
SSID: Carbtune-Pro
Password: carbtunepro
URL: http://192.168.4.1/
```

The web interface can scan nearby WiFi networks, save SSID/password to ESP32
NVS, reconnect as a station, and keep AP mode available for service access.
It also exposes basic device settings plus customer and vehicle profile fields.
Saved WiFi credentials, customer name, vehicle name, and profile notes are
stored in ESP32 NVS, not on SD.

## SD Card

The display initializes SD through `SdManager` after TFT setup using a separate
SPI bus instance. Before TFT, touch, or SD SPI init, all chip-select pins are
set high:

- `TFT_CS`
- `TOUCH_CS`
- `SD_CS`

SD starts at 1 MHz and retries once at 400 kHz. Before each SD init attempt,
TFT, touch, and SD chip-select pins are driven high and held briefly so the SD
card gets a clean SPI bus. Splash and diagnostics show `SD OK <MB>` or
`SD NIET BESCHIKBAAR`. SD failures are logged but do not block display or touch.
After a failed mount, firmware does not keep retrying in the background. Re-seat
the card and use `TEST SD` in the settings menu for one explicit retry.

Use a FAT32-formatted card. An 8-32GB SD card is recommended for the CYD board.

Normal settings are stored in ESP32 NVS, not on SD. The SD card is reserved for
logging, export/import, backups, screenshots, and future firmware update files.
The settings menu can test the SD card, repair the default folders, export the
current settings to `/carbtune/config/settings.json`, and import that file back
into NVS.

Default SD folder layout:

```text
/carbtune/logs        CSV logs
/carbtune/config      settings export/import
/carbtune/backups     backups
/carbtune/firmware    future firmware updates
/carbtune/exports     exports
/carbtune/screenshots future screenshots
```

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
