#include <Arduino.h>
#include <CarbtuneShared.h>
#include "SensorNodeConfig.h"

static uint32_t lastFrameMs = 0;

static void sendHello() {
  Serial2.write(Carbtune::PacketMagic);
  Serial2.write(Carbtune::PacketVersion);
  Serial2.write(static_cast<uint8_t>(Carbtune::PacketType::Hello));
  Serial2.write(Carbtune::checksum8(
      reinterpret_cast<const uint8_t *>(Carbtune::FirmwareVersion),
      strlen(Carbtune::FirmwareVersion)));
}

static Carbtune::SensorFrame readFrame() {
  Carbtune::SensorFrame frame{};
  frame.uptimeMs = millis();

  for (uint8_t channel = 0; channel < Carbtune::ChannelCount; ++channel) {
    frame.vacuumRaw[channel] = analogRead(SensorAnalogPins[channel]);
  }

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
  sendHello();

  Serial.println();
  Serial.println("Carbtune Pro SensorNode");
  Serial.println(Carbtune::FirmwareVersion);
}

void loop() {
  if (millis() - lastFrameMs < 50) {
    return;
  }

  lastFrameMs = millis();
  const Carbtune::SensorFrame frame = readFrame();
  sendFrame(frame);
  Serial.println(Carbtune::formatSensorFrame(frame));
}
