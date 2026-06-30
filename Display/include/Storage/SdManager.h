#pragma once

#include <Arduino.h>
#include <SPI.h>

class SdManager {
 public:
  enum class Error {
    None,
    NoCard,
    InitFailed,
  };

  SdManager();

  void begin();
  bool mounted() const;
  const char *cardTypeName() const;
  uint32_t cardSizeMb() const;
  const char *lastErrorName() const;
  uint8_t initAttempts() const;
  String statusText() const;

 private:
  bool tryBegin(uint32_t speedHz, uint8_t attempt);
  void prepareChipSelects() const;
  void logPins() const;

  SPIClass sdSpi_;
  bool mounted_ = false;
  Error lastError_ = Error::InitFailed;
  uint8_t initAttempts_ = 0;
  uint8_t cardType_ = 0;
  uint32_t cardSizeMb_ = 0;
};
