#include <CarbtuneShared.h>

namespace Carbtune {

uint8_t checksum8(const uint8_t *data, size_t length) {
  uint8_t checksum = 0;

  for (size_t index = 0; index < length; ++index) {
    checksum ^= data[index];
  }

  return checksum;
}

String packetTypeName(PacketType type) {
  switch (type) {
    case PacketType::Hello:
      return "hello";
    case PacketType::SensorFrame:
      return "sensor-frame";
    case PacketType::Command:
      return "command";
    case PacketType::SelfTest:
      return "self-test";
  }

  return "unknown";
}

String formatSensorFrame(const SensorFrame &frame) {
  String output = "uptime=" + String(frame.uptimeMs);

  for (uint8_t channel = 0; channel < ChannelCount; ++channel) {
    output += " ch";
    output += String(channel + 1);
    output += " raw=";
    output += String(frame.raw[channel]);
    output += " filt=";
    output += String(frame.filtered[channel]);
    output += " hz=";
    output += String(frame.pulseHzCenti[channel] / 100.0f, 2);
  }

  output += " rpm=";
  output += String(frame.rpm);
  output += " flags=0x";
  output += String(frame.flags, HEX);
  output += " supplyMv=";
  output += String(frame.supplyMv);
  output += " tempC=";
  output += String(frame.temperatureCentiC / 100.0f, 2);

  return output;
}

}  // namespace Carbtune
