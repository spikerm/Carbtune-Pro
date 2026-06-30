#pragma once

#include <Arduino.h>
#include <CarbtuneShared.h>

#include "App/SettingsManager.h"

class SensorManager {
 public:
  static constexpr uint8_t MaxChannels = 6;

  explicit SensorManager(SettingsManager &settings);

  enum class Mode {
    Demo,
    LiveUart,
    NoData,
  };

  enum class Status {
    Good,
    Warning,
    Adjust,
  };

  void begin();
  void update(uint32_t nowMs);

  uint8_t channelCount() const;
  float valueKpa(uint8_t channel) const;
  int16_t rawValue(uint8_t channel) const;
  float maxDeltaKpa() const;
  Status status() const;
  Mode mode() const;
  const char *modeName() const;
  uint32_t lastFrameAgeMs(uint32_t nowMs) const;
  uint32_t packetCount() const;
  uint32_t checksumErrors() const;
  uint16_t supplyMv() const;
  int16_t temperatureCentiC() const;

 private:
  enum class ParseState {
    Magic,
    Version,
    Type,
    Length,
    Payload,
    Checksum,
  };

  void updateDemo(uint32_t nowMs);
  void readUart(uint32_t nowMs);
  void acceptFrame(const Carbtune::SensorFrame &frame, uint32_t nowMs);
  float rawToKpa(int16_t raw) const;

  SettingsManager &settings_;
  ParseState parseState_ = ParseState::Magic;
  uint8_t packetType_ = 0;
  uint8_t payloadLength_ = 0;
  uint8_t payloadIndex_ = 0;
  uint8_t payload_[sizeof(Carbtune::SensorFrame)]{};
  float valuesKpa_[MaxChannels] = {-42.0f, -40.0f, -41.0f, -44.0f, -43.0f, -45.0f};
  int16_t rawValues_[MaxChannels] = {0, 0, 0, 0, 0, 0};
  uint16_t supplyMv_ = 0;
  int16_t temperatureCentiC_ = 0;
  Mode mode_ = Mode::Demo;
  uint32_t lastFrameMs_ = 0;
  uint32_t lastUpdateMs_ = 0;
  uint32_t packetCount_ = 0;
  uint32_t checksumErrors_ = 0;
};
