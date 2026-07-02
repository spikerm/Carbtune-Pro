#include "Storage/SdManager.h"

#include <SD.h>

#include "BoardConfig.h"

namespace {
constexpr const char *DefaultFolders[] = {
    "/carbtune",
    "/carbtune/logs",
    "/carbtune/config",
    "/carbtune/backups",
    "/carbtune/firmware",
    "/carbtune/exports",
    "/carbtune/screenshots",
};
constexpr const char *TestFilePath = "/carbtune/test.txt";
constexpr const char *SettingsPath = "/carbtune/config/settings.json";
constexpr const char *FoldersStatusPath = "/carbtune/config/folders.txt";
}  // namespace

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

bool SdManager::isMounted() const {
  return mounted_;
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

uint32_t SdManager::freeSpaceMb() const {
  if (!mounted_) {
    return 0;
  }
  const uint64_t total = SD.totalBytes();
  const uint64_t used = SD.usedBytes();
  return total > used ? static_cast<uint32_t>((total - used) / (1024ULL * 1024ULL)) : 0;
}

const char *SdManager::lastError() const {
  return lastErrorName();
}

const char *SdManager::lastErrorName() const {
  switch (lastError_) {
    case Error::None:
      return "OK";
    case Error::NoCard:
      return "NO CARD";
    case Error::InitFailed:
      return "INIT";
    case Error::NotMounted:
      return "NOT MOUNTED";
    case Error::Directory:
      return "DIR";
    case Error::File:
      return "FILE";
    case Error::Verify:
      return "VERIFY";
  }
  return "INIT";
}

uint8_t SdManager::initAttempts() const {
  return initAttempts_;
}

bool SdManager::folderLayoutOk() const {
  return folderLayoutOk_;
}

String SdManager::statusText() const {
  if (mounted_) {
    return String("SD OK ") + String(cardSizeMb_) + "MB";
  }
  return String("SD FAIL ") + lastErrorName();
}

bool SdManager::ensureDirectory(const char *path) {
  if (!ensureMounted()) {
    return false;
  }
  if (SD.exists(path)) {
    Serial.print("SD folder exists ");
    Serial.println(path);
    return true;
  }
  if (!SD.mkdir(path)) {
    Serial.print("SD folder failed ");
    Serial.println(path);
    lastError_ = Error::Directory;
    return false;
  }
  Serial.print("SD folder created ");
  Serial.println(path);
  return true;
}

bool SdManager::ensureDefaultFolders() {
  bool ok = true;
  for (const char *path : DefaultFolders) {
    ok &= ensureDirectory(path);
  }
  folderLayoutOk_ = ok;
  return ok;
}

bool SdManager::writeTestFile() {
  if (!ensureDefaultFolders()) {
    return false;
  }
  if (SD.exists(TestFilePath)) {
    SD.remove(TestFilePath);
  }
  File file = SD.open(TestFilePath, FILE_WRITE);
  if (!file) {
    lastError_ = Error::File;
    return false;
  }
  file.print("carbtune sd test");
  file.close();
  return true;
}

bool SdManager::readTestFile() {
  if (!ensureMounted()) {
    return false;
  }
  File file = SD.open(TestFilePath, FILE_READ);
  if (!file) {
    lastError_ = Error::File;
    return false;
  }
  const String content = file.readString();
  file.close();
  const bool ok = content == "carbtune sd test";
  lastError_ = ok ? Error::None : Error::Verify;
  return ok;
}

bool SdManager::repairFilesystemLayout() {
  bool ok = ensureDefaultFolders();
  if (ok) {
    if (SD.exists(FoldersStatusPath)) {
      SD.remove(FoldersStatusPath);
    }
    File file = SD.open(FoldersStatusPath, FILE_WRITE);
    if (!file) {
      lastError_ = Error::File;
      ok = false;
    } else {
      file.println("Carbtune SD folders repaired");
      file.println("/carbtune/logs");
      file.println("/carbtune/config");
      file.println("/carbtune/backups");
      file.println("/carbtune/firmware");
      file.println("/carbtune/exports");
      file.println("/carbtune/screenshots");
      file.close();
      lastError_ = Error::None;
    }
  }
  Serial.println(ok ? "SD folders repaired" : "SD folders repair failed");
  return ok;
}

bool SdManager::exportSettings(const SettingsManager &settings) {
  if (!ensureDefaultFolders()) {
    return false;
  }
  if (SD.exists(SettingsPath)) {
    SD.remove(SettingsPath);
  }
  File file = SD.open(SettingsPath, FILE_WRITE);
  if (!file) {
    lastError_ = Error::File;
    return false;
  }
  file.println("{");
  file.printf("  \"cylinders\": %u,\n", settings.cylinders());
  file.printf("  \"units\": %u,\n", static_cast<uint8_t>(settings.unitsMode()));
  file.printf("  \"autoScale\": %s,\n", settings.autoScale() ? "true" : "false");
  file.printf("  \"damping\": \"%s\",\n", settings.dampingName());
  file.printf("  \"autoBrightness\": %s,\n", settings.autoBrightness() ? "true" : "false");
  file.printf("  \"brightness\": %u,\n", settings.brightnessManualPercent());
  file.printf("  \"demoMode\": %s,\n", settings.demoFallback() ? "true" : "false");
  file.printf("  \"alarmDeltaKpa\": %d,\n", static_cast<int>(settings.alarmDeltaKpa() * 10.0f));
  file.printf("  \"rpmSource\": %u,\n", static_cast<uint8_t>(settings.rpmSource()));
  file.printf("  \"engineStroke\": %s\n", settings.engineStrokeName()[0] == '4' ? "4" : "2");
  file.println("}");
  file.close();
  lastError_ = Error::None;
  return true;
}

bool SdManager::importSettings(SettingsManager &settings) {
  if (!ensureMounted()) {
    return false;
  }
  File file = SD.open(SettingsPath, FILE_READ);
  if (!file) {
    lastError_ = Error::File;
    return false;
  }
  const String json = file.readString();
  file.close();

  int intValue = 0;
  bool boolValue = false;
  if (parseJsonInt(json, "cylinders", intValue)) {
    settings.setCylinders(static_cast<uint8_t>(intValue));
  }
  if (parseJsonInt(json, "units", intValue)) {
    settings.setUnits(static_cast<SettingsManager::Units>(intValue));
  }
  if (parseJsonBool(json, "autoScale", boolValue)) {
    settings.setAutoScale(boolValue);
  }
  if (json.indexOf("\"damping\": \"laag\"") >= 0) {
    settings.setDampingMode(SettingsManager::DampingMode::Low);
  } else if (json.indexOf("\"damping\": \"hoog\"") >= 0) {
    settings.setDampingMode(SettingsManager::DampingMode::High);
  } else if (json.indexOf("\"damping\": \"normaal\"") >= 0) {
    settings.setDampingMode(SettingsManager::DampingMode::Normal);
  }
  if (parseJsonBool(json, "autoBrightness", boolValue)) {
    settings.setAutoBrightness(boolValue);
  }
  if (parseJsonInt(json, "brightness", intValue)) {
    settings.setBrightnessManualPercent(static_cast<uint8_t>(intValue));
  }
  if (parseJsonBool(json, "demoMode", boolValue)) {
    settings.setDemoFallback(boolValue);
  }
  if (parseJsonInt(json, "alarmDeltaKpa", intValue)) {
    settings.setAlarmDeltaKpa(intValue / 10.0f);
  }
  if (parseJsonInt(json, "rpmSource", intValue)) {
    settings.setRpmSource(static_cast<SettingsManager::RpmSource>(intValue));
  }
  if (parseJsonInt(json, "engineStroke", intValue)) {
    settings.setEngineStroke(intValue == 2 ? SettingsManager::EngineStroke::TwoStroke
                                           : SettingsManager::EngineStroke::FourStroke);
  }

  const bool saved = settings.save();
  lastError_ = saved ? Error::None : Error::File;
  return saved;
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
  folderLayoutOk_ = false;

  Serial.print("SD result=ok type=");
  Serial.print(cardTypeName());
  Serial.print(" sizeMB=");
  Serial.println(cardSizeMb_);
  return true;
}

bool SdManager::ensureMounted() {
  if (mounted_) {
    return true;
  }
  begin();
  if (!mounted_) {
    lastError_ = lastError_ == Error::None ? Error::NotMounted : lastError_;
    return false;
  }
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

bool SdManager::parseJsonInt(const String &json, const char *key, int &value) const {
  const String token = String("\"") + key + "\"";
  int pos = json.indexOf(token);
  if (pos < 0) {
    return false;
  }
  pos = json.indexOf(':', pos);
  if (pos < 0) {
    return false;
  }
  value = json.substring(pos + 1).toInt();
  return true;
}

bool SdManager::parseJsonBool(const String &json, const char *key, bool &value) const {
  const String token = String("\"") + key + "\"";
  int pos = json.indexOf(token);
  if (pos < 0) {
    return false;
  }
  pos = json.indexOf(':', pos);
  if (pos < 0) {
    return false;
  }
  const String tail = json.substring(pos + 1);
  if (tail.indexOf("true") >= 0 && (tail.indexOf("false") < 0 || tail.indexOf("true") < tail.indexOf("false"))) {
    value = true;
    return true;
  }
  if (tail.indexOf("false") >= 0) {
    value = false;
    return true;
  }
  return false;
}
