#include "Sensors/SensorManager.h"

#include <math.h>

#include "BoardConfig.h"

SensorManager::SensorManager(SettingsManager &settings) : settings_(settings) {}

void SensorManager::begin() {
  Serial2.begin(Carbtune::UartBaud, SERIAL_8N1, SENSOR_RX, SENSOR_TX);
}

void SensorManager::update(uint32_t nowMs) {
  if (settings_.liveUartEnabled()) {
    readUart(nowMs);
  }

  if (mode_ == Mode::LiveUart && nowMs - lastFrameMs_ > 2000) {
    mode_ = settings_.demoFallback() ? Mode::Demo : Mode::NoData;
  }

  if (mode_ == Mode::Demo || (mode_ == Mode::NoData && settings_.demoFallback())) {
    updateDemo(nowMs);
  }
}

uint8_t SensorManager::channelCount() const {
  return Carbtune::ChannelCount;
}

float SensorManager::valueKpa(uint8_t channel) const {
  return channel < channelCount() ? valuesKpa_[channel] : 0.0f;
}

int16_t SensorManager::rawValue(uint8_t channel) const {
  return channel < channelCount() ? rawValues_[channel] : 0;
}

float SensorManager::maxDeltaKpa() const {
  float lowest = valuesKpa_[0];
  float highest = valuesKpa_[0];
  for (uint8_t index = 1; index < channelCount(); ++index) {
    lowest = min(lowest, valuesKpa_[index]);
    highest = max(highest, valuesKpa_[index]);
  }
  return highest - lowest;
}

SensorManager::Status SensorManager::status() const {
  const float delta = maxDeltaKpa();
  if (delta > 10.0f) {
    return Status::Adjust;
  }
  if (delta > 6.0f) {
    return Status::Warning;
  }
  return Status::Good;
}

SensorManager::Mode SensorManager::mode() const {
  return mode_;
}

const char *SensorManager::modeName() const {
  switch (mode_) {
    case Mode::Demo:
      return "DEMO";
    case Mode::LiveUart:
      return "LIVE";
    case Mode::NoData:
      return "NO DATA";
  }
  return "UNKNOWN";
}

uint32_t SensorManager::lastFrameAgeMs(uint32_t nowMs) const {
  return lastFrameMs_ == 0 ? 0 : nowMs - lastFrameMs_;
}

uint32_t SensorManager::packetCount() const {
  return packetCount_;
}

uint32_t SensorManager::checksumErrors() const {
  return checksumErrors_;
}

uint16_t SensorManager::supplyMv() const {
  return supplyMv_;
}

int16_t SensorManager::temperatureCentiC() const {
  return temperatureCentiC_;
}

void SensorManager::updateDemo(uint32_t nowMs) {
  if (nowMs - lastUpdateMs_ < 100) {
    return;
  }

  mode_ = Mode::Demo;
  lastUpdateMs_ = nowMs;
  const float t = nowMs / 1000.0f;
  valuesKpa_[0] = -42.0f + sinf(t * 1.1f) * 4.0f;
  valuesKpa_[1] = -40.0f + sinf(t * 0.9f + 1.0f) * 3.5f;
  valuesKpa_[2] = -41.0f + sinf(t * 1.3f + 2.0f) * 4.2f;
  valuesKpa_[3] = -44.0f + sinf(t * 0.8f + 3.0f) * 5.0f;
  for (uint8_t channel = 0; channel < channelCount(); ++channel) {
    rawValues_[channel] = static_cast<int16_t>(map(static_cast<long>(-valuesKpa_[channel]), 0, 100, 0, 4095));
  }
}

void SensorManager::readUart(uint32_t nowMs) {
  while (Serial2.available() > 0) {
    const uint8_t byte = Serial2.read();
    switch (parseState_) {
      case ParseState::Magic:
        if (byte == Carbtune::PacketMagic) {
          parseState_ = ParseState::Version;
        }
        break;
      case ParseState::Version:
        parseState_ = byte == Carbtune::PacketVersion ? ParseState::Type : ParseState::Magic;
        break;
      case ParseState::Type:
        packetType_ = byte;
        parseState_ = ParseState::Length;
        break;
      case ParseState::Length:
        payloadLength_ = byte;
        payloadIndex_ = 0;
        if (packetType_ != static_cast<uint8_t>(Carbtune::PacketType::SensorFrame) ||
            payloadLength_ != sizeof(Carbtune::SensorFrame)) {
          parseState_ = ParseState::Magic;
        } else {
          parseState_ = ParseState::Payload;
        }
        break;
      case ParseState::Payload:
        payload_[payloadIndex_++] = byte;
        if (payloadIndex_ >= payloadLength_) {
          parseState_ = ParseState::Checksum;
        }
        break;
      case ParseState::Checksum:
        if (byte == Carbtune::checksum8(payload_, payloadLength_)) {
          Carbtune::SensorFrame frame{};
          memcpy(&frame, payload_, sizeof(frame));
          acceptFrame(frame, nowMs);
        } else {
          ++checksumErrors_;
        }
        parseState_ = ParseState::Magic;
        break;
    }
  }
}

void SensorManager::acceptFrame(const Carbtune::SensorFrame &frame, uint32_t nowMs) {
  for (uint8_t channel = 0; channel < channelCount(); ++channel) {
    rawValues_[channel] = frame.vacuumRaw[channel];
    valuesKpa_[channel] = rawToKpa(frame.vacuumRaw[channel]);
  }
  supplyMv_ = frame.supplyMv;
  temperatureCentiC_ = static_cast<int16_t>(frame.temperatureCentiC);
  lastFrameMs_ = nowMs;
  mode_ = Mode::LiveUart;
  ++packetCount_;
}

float SensorManager::rawToKpa(int16_t raw) const {
  // TODO: replace temporary ADC mapping with calibrated pressure conversion.
  const int16_t clamped = constrain(raw, 0, 4095);
  return -100.0f + (static_cast<float>(clamped) * 100.0f / 4095.0f);
}
