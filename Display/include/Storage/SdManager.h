#pragma once

#include <Arduino.h>
#include <SPI.h>

#include "App/SettingsManager.h"

class SdManager {
 public:
  enum class Error {
    None,
    NoCard,
    InitFailed,
    NotMounted,
    Directory,
    File,
    Verify,
  };

  SdManager();

  void begin();
  bool isMounted() const;
  bool mounted() const;
  const char *cardTypeName() const;
  uint32_t cardSizeMb() const;
  uint32_t freeSpaceMb() const;
  const char *lastError() const;
  const char *lastErrorName() const;
  uint8_t initAttempts() const;
  bool folderLayoutOk() const;
  String statusText() const;
  bool ensureDirectory(const char *path);
  bool ensureDefaultFolders();
  bool writeTestFile();
  bool readTestFile();
  bool repairFilesystemLayout();
  bool exportSettings(const SettingsManager &settings);
  bool importSettings(SettingsManager &settings);

 private:
  bool tryBegin(uint32_t speedHz, uint8_t attempt);
  bool ensureMounted();
  void prepareChipSelects() const;
  void logPins() const;
  bool parseJsonInt(const String &json, const char *key, int &value) const;
  bool parseJsonBool(const String &json, const char *key, bool &value) const;

  SPIClass sdSpi_;
  bool mounted_ = false;
  Error lastError_ = Error::InitFailed;
  uint8_t initAttempts_ = 0;
  uint8_t cardType_ = 0;
  uint32_t cardSizeMb_ = 0;
  bool folderLayoutOk_ = false;
};
