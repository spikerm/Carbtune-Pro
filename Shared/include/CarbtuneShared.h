#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace Carbtune {

static constexpr const char *FirmwareVersion = "v7.0.0-alpha2";
static constexpr uint32_t UartBaud = 115200;
static constexpr uint8_t PacketMagic = 0xC6;
static constexpr uint8_t PacketVersion = 1;
static constexpr uint8_t ChannelCount = 6;

enum class PacketType : uint8_t {
  Hello = 0x01,
  SensorFrame = 0x10,
  Command = 0x20,
  SelfTest = 0x30,
};

struct SensorFrame {
  uint32_t uptimeMs;
  int16_t vacuumRaw[ChannelCount];
  uint16_t supplyMv;
  uint16_t temperatureCentiC;
};

uint8_t checksum8(const uint8_t *data, size_t length);
String packetTypeName(PacketType type);
String formatSensorFrame(const SensorFrame &frame);

}  // namespace Carbtune
