#pragma once

#include <Arduino.h>

#define SENSOR_UART_RX 22
#define SENSOR_UART_TX 27

static constexpr uint8_t SensorAnalogPins[] = {34, 35, 32, 33};
static constexpr uint8_t SensorAnalogPinCount = sizeof(SensorAnalogPins) / sizeof(SensorAnalogPins[0]);
static constexpr uint8_t SupplySensePin = 36;
