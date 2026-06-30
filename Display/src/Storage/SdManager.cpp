#include "Storage/SdManager.h"

#include <SD.h>

#include "BoardConfig.h"

SdManager::SdManager() : sdSpi_(VSPI) {}

void SdManager::begin() {
  mounted_ = false;
  lastError_ = Error::InitFailed;
  initAttempts_ = 0;
  cardType_ = 0;
  cardSizeMb_ = 0;

  Serial.println("SD INIT START");
  logPins();

  if (tryBegin(4000000, 1)) {
    return;
  }
  SD.end();
  delay(5);
  tryBegin(1000000, 2);
}

bool SdManager::mounted() const {
  return mounted_;
}

const char *SdManager::cardTypeName() const {
  if (!mounted_) {
    return "-";
  }
  switch (cardType_) {
    case CARD_MMC:
      return "MMC";
    case CARD_SD:
      return "SDSC";
    case CARD_SDHC:
      return "SDHC";
    default:
      return "UNKNOWN";
  }
}

uint32_t SdManager::cardSizeMb() const {
  return cardSizeMb_;
}

const char *SdManager::lastErrorName() const {
  switch (lastError_) {
    case Error::None:
      return "OK";
    case Error::NoCard:
      return "NO CARD";
    case Error::InitFailed:
      return "INIT";
  }
  return "INIT";
}

uint8_t SdManager::initAttempts() const {
  return initAttempts_;
}

String SdManager::statusText() const {
  if (mounted_) {
    return String("SD OK ") + String(cardSizeMb_) + "MB";
  }
  return String("SD FAIL ") + lastErrorName();
}

bool SdManager::tryBegin(uint32_t speedHz, uint8_t attempt) {
  initAttempts_ = attempt;
  prepareChipSelects();
  sdSpi_.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  Serial.print("SD attempt=");
  Serial.print(attempt);
  Serial.print(" speed=");
  Serial.println(speedHz);

  if (!SD.begin(SD_CS, sdSpi_, speedHz)) {
    Serial.println("SD result=begin failed");
    lastError_ = Error::InitFailed;
    mounted_ = false;
    return false;
  }

  cardType_ = SD.cardType();
  if (cardType_ == CARD_NONE) {
    Serial.println("SD result=no card");
    lastError_ = Error::NoCard;
    mounted_ = false;
    SD.end();
    return false;
  }

  cardSizeMb_ = static_cast<uint32_t>(SD.cardSize() / (1024ULL * 1024ULL));
  mounted_ = true;
  lastError_ = Error::None;

  Serial.print("SD result=ok type=");
  Serial.print(cardTypeName());
  Serial.print(" sizeMB=");
  Serial.println(cardSizeMb_);
  return true;
}

void SdManager::prepareChipSelects() const {
  pinMode(TFT_CS, OUTPUT);
  pinMode(TOUCH_CS, OUTPUT);
  pinMode(SD_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  digitalWrite(TOUCH_CS, HIGH);
  digitalWrite(SD_CS, HIGH);
}

void SdManager::logPins() const {
  Serial.print("SD pins cs=");
  Serial.print(SD_CS);
  Serial.print(" sck=");
  Serial.print(SD_SCK);
  Serial.print(" miso=");
  Serial.print(SD_MISO);
  Serial.print(" mosi=");
  Serial.println(SD_MOSI);
}
