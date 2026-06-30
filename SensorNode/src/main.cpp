#include <Arduino.h>
#include <CarbtuneShared.h>
#include "SensorNodeConfig.h"

enum class EngineStroke : uint8_t {
  TwoStroke = 2,
  FourStroke = 4,
};

static constexpr uint32_t SampleIntervalMs = 5;
static constexpr uint32_t FrameIntervalMs = 50;
static constexpr uint32_t PulseTimeoutMs = 1500;
static constexpr float FilterAlpha = 0.10f;
static constexpr int16_t PulseHighThreshold = 180;
static constexpr int16_t PulseLowThreshold = 80;

static uint32_t lastFrameMs = 0;
static uint32_t lastSampleMs = 0;
static uint32_t lastLogMs = 0;
static EngineStroke engineStroke = EngineStroke::FourStroke;
static int16_t rawValues[Carbtune::ChannelCount] = {};
static float filteredValues[Carbtune::ChannelCount] = {};
static uint16_t pulseHzCenti[Carbtune::ChannelCount] = {};
static uint32_t lastPulseMs[Carbtune::ChannelCount] = {};
static bool pulseArmed[Carbtune::ChannelCount] = {};
static uint16_t rpmEstimate = 0;
static bool rpmStable = false;

static void sendHello() {
  Serial2.write(Carbtune::PacketMagic);
  Serial2.write(Carbtune::PacketVersion);
  Serial2.write(static_cast<uint8_t>(Carbtune::PacketType::Hello));
  Serial2.write(Carbtune::checksum8(
      reinterpret_cast<const uint8_t *>(Carbtune::FirmwareVersion),
      strlen(Carbtune::FirmwareVersion)));
}

static void sampleSensors() {
  const uint32_t nowMs = millis();
  for (uint8_t channel = 0; channel < Carbtune::ChannelCount; ++channel) {
    if (channel < SensorAnalogPinCount) {
      rawValues[channel] = analogRead(SensorAnalogPins[channel]);
    } else {
      rawValues[channel] = 0;
    }

    if (filteredValues[channel] == 0.0f) {
      filteredValues[channel] = rawValues[channel];
    } else {
      filteredValues[channel] += FilterAlpha * (rawValues[channel] - filteredValues[channel]);
    }

    const int16_t pulseSignal = rawValues[channel] - static_cast<int16_t>(filteredValues[channel]);
    if (pulseArmed[channel] && pulseSignal > PulseHighThreshold) {
      if (lastPulseMs[channel] > 0) {
        const uint32_t periodMs = nowMs - lastPulseMs[channel];
        if (periodMs >= 30 && periodMs <= PulseTimeoutMs) {
          pulseHzCenti[channel] = static_cast<uint16_t>(constrain(100000UL / periodMs, 0UL, 65535UL));
        }
      }
      lastPulseMs[channel] = nowMs;
      pulseArmed[channel] = false;
    } else if (!pulseArmed[channel] && pulseSignal < PulseLowThreshold) {
      pulseArmed[channel] = true;
    }

    if (lastPulseMs[channel] == 0 || nowMs - lastPulseMs[channel] > PulseTimeoutMs) {
      pulseHzCenti[channel] = 0;
    }
  }

  rpmStable = pulseHzCenti[0] > 0;
  const uint32_t rpmMultiplier = engineStroke == EngineStroke::FourStroke ? 120UL : 60UL;
  rpmEstimate = rpmStable ? static_cast<uint16_t>(min<uint32_t>((pulseHzCenti[0] * rpmMultiplier) / 100UL, 65535UL)) : 0;
}

static Carbtune::SensorFrame readFrame() {
  Carbtune::SensorFrame frame{};
  frame.uptimeMs = millis();

  for (uint8_t channel = 0; channel < Carbtune::ChannelCount; ++channel) {
    frame.raw[channel] = rawValues[channel];
    frame.filtered[channel] = static_cast<int16_t>(filteredValues[channel]);
    frame.pulseHzCenti[channel] = pulseHzCenti[channel];
  }

  frame.rpm = rpmEstimate;
  frame.flags = rpmStable ? Carbtune::SensorFlagRpmStable : 0;
  frame.supplyMv = static_cast<uint16_t>((analogReadMilliVolts(SupplySensePin) * 2U));
  frame.temperatureCentiC = 0;
  return frame;
}

static void sendFrame(const Carbtune::SensorFrame &frame) {
  const uint8_t *payload = reinterpret_cast<const uint8_t *>(&frame);

  Serial2.write(Carbtune::PacketMagic);
  Serial2.write(Carbtune::PacketVersion);
  Serial2.write(static_cast<uint8_t>(Carbtune::PacketType::SensorFrame));
  Serial2.write(sizeof(frame));
  Serial2.write(payload, sizeof(frame));
  Serial2.write(Carbtune::checksum8(payload, sizeof(frame)));
}

void setup() {
  Serial.begin(Carbtune::UartBaud);
  Serial2.begin(Carbtune::UartBaud, SERIAL_8N1, SENSOR_UART_RX, SENSOR_UART_TX);

  analogReadResolution(12);
  for (uint8_t channel = 0; channel < Carbtune::ChannelCount; ++channel) {
    pulseArmed[channel] = true;
  }
  sampleSensors();
  sendHello();

  Serial.println();
  Serial.println("Carbtune Pro SensorNode");
  Serial.println(Carbtune::FirmwareVersion);
}

void loop() {
  if (millis() - lastSampleMs >= SampleIntervalMs) {
    lastSampleMs = millis();
    sampleSensors();
  }

  if (millis() - lastFrameMs < FrameIntervalMs) {
    return;
  }

  lastFrameMs = millis();
  const Carbtune::SensorFrame frame = readFrame();
  sendFrame(frame);
  if (millis() - lastLogMs >= 1000) {
    lastLogMs = millis();
    Serial.println(Carbtune::formatSensorFrame(frame));
  }
}
