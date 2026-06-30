#include "Sensors/BacklightManager.h"

#include "BoardConfig.h"

static constexpr uint16_t SampleIntervalMs = 250;
static constexpr uint8_t PwmResolutionBits = 8;
static constexpr uint32_t PwmFrequencyHz = 5000;

BacklightManager::BacklightManager(SettingsManager &settings) : settings_(settings) {}

void BacklightManager::begin() {
  pinMode(LDR_PIN, INPUT);
  pinMode(TFT_BL, OUTPUT);
  pwmActive_ = ledcAttach(TFT_BL, PwmFrequencyHz, PwmResolutionBits);
  ldrRaw_ = analogRead(LDR_PIN);
  filteredLight_ = static_cast<float>(ldrRaw_);
  applyBrightness(targetPercent());
}

void BacklightManager::update(uint32_t nowMs) {
  if (nowMs - lastSampleMs_ < SampleIntervalMs) {
    return;
  }

  lastSampleMs_ = nowMs;
  ldrRaw_ = analogRead(LDR_PIN);
  filteredLight_ = (filteredLight_ * 0.90f) + (static_cast<float>(ldrRaw_) * 0.10f);
  ldrWarning_ = ldrRaw_ == 0;
  applyBrightness(targetPercent());
}

uint16_t BacklightManager::ldrRaw() const {
  return ldrRaw_;
}

float BacklightManager::filteredLight() const {
  return filteredLight_;
}

uint8_t BacklightManager::brightnessPercent() const {
  return brightnessPercent_;
}

bool BacklightManager::pwmActive() const {
  return pwmActive_;
}

bool BacklightManager::ldrWarning() const {
  return ldrWarning_;
}

bool BacklightManager::autoBrightness() const {
  return settings_.autoBrightness();
}

uint8_t BacklightManager::targetPercent() const {
  if (!settings_.autoBrightness()) {
    return settings_.brightnessManualPercent();
  }

  const float normalized = constrain(filteredLight_ / 4095.0f, 0.0f, 1.0f);
  const uint8_t minPercent = settings_.brightnessMinPercent();
  const uint8_t maxPercent = settings_.brightnessMaxPercent();
  return minPercent + static_cast<uint8_t>((maxPercent - minPercent) * normalized);
}

void BacklightManager::applyBrightness(uint8_t percent) {
  percent = constrain(percent, 0, 100);
  if (percent == brightnessPercent_) {
    return;
  }

  brightnessPercent_ = percent;
  if (pwmActive_) {
    const uint32_t duty = map(percent, 0, 100, 0, 255);
    ledcWrite(TFT_BL, duty);
  } else {
    digitalWrite(TFT_BL, percent > 0 ? HIGH : LOW);
  }
}
