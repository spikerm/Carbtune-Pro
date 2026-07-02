#pragma once

#include <Arduino.h>

class OtaManager {
 public:
  static constexpr const char *DefaultFirmwareUrl =
      "https://github.com/spikerm/Carbtune-Pro/releases/latest/download/display-firmware.bin";

  void begin();
  bool updateFromGithub(const char *firmwareUrl = DefaultFirmwareUrl);

  const String &lastStatus() const;
  uint32_t lastHttpCode() const;
  size_t lastBytesWritten() const;

 private:
  void setStatus(const __FlashStringHelper *status);
  void setStatus(const String &status);

  String lastStatus_ = "Nog niet uitgevoerd";
  uint32_t lastHttpCode_ = 0;
  size_t lastBytesWritten_ = 0;
};
