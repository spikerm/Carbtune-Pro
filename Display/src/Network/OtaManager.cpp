#include "Network/OtaManager.h"

#include <HTTPClient.h>
#include <Update.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

namespace {
constexpr char GithubRootCa[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICOjCCAcGgAwIBAgIQQvLM2htpN0RfFf51KBC49DAKBggqhkjOPQQDAzBfMQswCQYDVQQGEwJH
QjEYMBYGA1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQDEy1TZWN0aWdvIFB1YmxpYyBTZXJ2
ZXIgQXV0aGVudGljYXRpb24gUm9vdCBFNDYwHhcNMjEwMzIyMDAwMDAwWhcNNDYwMzIxMjM1OTU5
WjBfMQswCQYDVQQGEwJHQjEYMBYGA1UEChMPU2VjdGlnbyBMaW1pdGVkMTYwNAYDVQQDEy1TZWN0
aWdvIFB1YmxpYyBTZXJ2ZXIgQXV0aGVudGljYXRpb24gUm9vdCBFNDYwdjAQBgcqhkjOPQIBBgUr
gQQAIgNiAAR2+pmpbiDt+dd34wc7qNs9Xzjoq1WmVk/WSOrsfy2qw7LFeeyZYX8QeccCWvkEN/U0
NSt3zn8gj1KjAIns1aeibVvjS5KToID1AZTc8GgHHs3u/iVStSBDHBv+6xnOQ6OjQjBAMB0GA1Ud
DgQWBBTRItpMWfFLXyY4qp3W7usNw/upYTAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB
/zAKBggqhkjOPQQDAwNnADBkAjAn7qRaqCG76UeXlImldCBteU/IvZNeWBj7LRoAasm4PdCkT0RH
lAFWovgzJQxC36oCMB3q4S6ILuH5px0CMk7yn2xVdOOurvulGu7t0vzCAxHrRVxgED1cf5kDW21U
SAGKcw==
-----END CERTIFICATE-----
)EOF";
}  // namespace

void OtaManager::begin() {
  setStatus(F("Klaar voor GitHub OTA"));
}

bool OtaManager::updateFromGithub(const char *firmwareUrl) {
  lastHttpCode_ = 0;
  lastBytesWritten_ = 0;

  if (WiFi.status() != WL_CONNECTED) {
    setStatus(F("WiFi niet verbonden"));
    Serial.println("OTA FAIL wifi not connected");
    return false;
  }

  Serial.print("OTA START url=");
  Serial.println(firmwareUrl);
  setStatus(F("GitHub firmware downloaden"));

  WiFiClientSecure client;
  client.setCACert(GithubRootCa);

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(20000);
  if (!http.begin(client, firmwareUrl)) {
    setStatus(F("HTTP start mislukt"));
    Serial.println("OTA FAIL http begin");
    return false;
  }

  const int httpCode = http.GET();
  lastHttpCode_ = static_cast<uint32_t>(httpCode > 0 ? httpCode : 0);
  if (httpCode != HTTP_CODE_OK) {
    setStatus(String("GitHub HTTP fout ") + String(httpCode));
    Serial.print("OTA FAIL http=");
    Serial.println(httpCode);
    http.end();
    return false;
  }

  const int contentLength = http.getSize();
  if (contentLength == 0) {
    setStatus(F("Lege firmware download"));
    Serial.println("OTA FAIL empty firmware");
    http.end();
    return false;
  }

  const size_t updateSize = contentLength > 0 ? static_cast<size_t>(contentLength) : UPDATE_SIZE_UNKNOWN;
  if (!Update.begin(updateSize)) {
    setStatus(String("OTA start fout ") + String(Update.errorString()));
    Serial.print("OTA FAIL update begin ");
    Serial.println(Update.errorString());
    http.end();
    return false;
  }

  setStatus(F("Firmware schrijven"));
  WiFiClient *stream = http.getStreamPtr();
  lastBytesWritten_ = Update.writeStream(*stream);

  if (contentLength > 0 && lastBytesWritten_ != static_cast<size_t>(contentLength)) {
    setStatus(F("OTA download onvolledig"));
    Serial.print("OTA FAIL short write bytes=");
    Serial.println(lastBytesWritten_);
    Update.abort();
    http.end();
    return false;
  }

  if (!Update.end()) {
    setStatus(String("OTA eind fout ") + String(Update.errorString()));
    Serial.print("OTA FAIL update end ");
    Serial.println(Update.errorString());
    http.end();
    return false;
  }

  if (!Update.isFinished()) {
    setStatus(F("OTA niet compleet"));
    Serial.println("OTA FAIL not finished");
    http.end();
    return false;
  }

  setStatus(F("OTA OK, herstarten"));
  Serial.print("OTA OK bytes=");
  Serial.println(lastBytesWritten_);
  http.end();
  delay(500);
  ESP.restart();
  return true;
}

const String &OtaManager::lastStatus() const {
  return lastStatus_;
}

uint32_t OtaManager::lastHttpCode() const {
  return lastHttpCode_;
}

size_t OtaManager::lastBytesWritten() const {
  return lastBytesWritten_;
}

void OtaManager::setStatus(const __FlashStringHelper *status) {
  lastStatus_ = status;
  Serial.print("OTA STATUS ");
  Serial.println(lastStatus_);
}

void OtaManager::setStatus(const String &status) {
  lastStatus_ = status;
  Serial.print("OTA STATUS ");
  Serial.println(lastStatus_);
}
