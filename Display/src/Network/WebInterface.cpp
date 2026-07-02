#include "Network/WebInterface.h"

#include <cstdlib>
#include <nvs.h>

#include "version.h"

namespace {
constexpr const char *Namespace = "webcfg";
constexpr const char *DefaultApSsid = "Carbtune-Pro";
constexpr const char *DefaultApPassword = "carbtunepro";
constexpr uint32_t StationTimeoutMs = 15000;

String readString(nvs_handle_t handle, const char *key, size_t maxLen) {
  size_t length = 0;
  if (nvs_get_str(handle, key, nullptr, &length) != ESP_OK || length == 0 || length > maxLen) {
    return "";
  }
  String value;
  char *buffer = new char[length];
  if (nvs_get_str(handle, key, buffer, &length) == ESP_OK) {
    value = buffer;
  }
  delete[] buffer;
  return value;
}

void writeString(nvs_handle_t handle, const char *key, const String &value) {
  nvs_set_str(handle, key, value.c_str());
}
}  // namespace

WebInterface::WebInterface(SettingsManager &settings) : settings_(settings) {}

void WebInterface::begin() {
  load();
  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  startAccessPoint();
  startStation();
  server_.begin();
  Serial.print("WEB AP ");
  Serial.print(apSsid());
  Serial.print(" ip=");
  Serial.println(apIp());
}

void WebInterface::update() {
  handleClient();
  if (stationStarted_ && !stationConnected() && millis() - stationStartMs_ > StationTimeoutMs) {
    stationStarted_ = false;
    Serial.println("WiFi STA timeout; AP remains active");
  }
}

const char *WebInterface::apSsid() const {
  return DefaultApSsid;
}

IPAddress WebInterface::apIp() const {
  return WiFi.softAPIP();
}

IPAddress WebInterface::stationIp() const {
  return WiFi.localIP();
}

bool WebInterface::stationConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

void WebInterface::load() {
  nvs_handle_t handle;
  if (nvs_open(Namespace, NVS_READONLY, &handle) != ESP_OK) {
    return;
  }
  wifiSsid_ = readString(handle, "ssid", 64);
  wifiPassword_ = readString(handle, "pass", 96);
  customerName_ = readString(handle, "customer", 64);
  vehicleName_ = readString(handle, "vehicle", 64);
  vehicleProfiles_ = readString(handle, "profiles", 900);
  nvs_close(handle);
}

void WebInterface::saveWifi(const String &ssid, const String &password) {
  nvs_handle_t handle;
  if (nvs_open(Namespace, NVS_READWRITE, &handle) != ESP_OK) {
    return;
  }
  writeString(handle, "ssid", ssid);
  writeString(handle, "pass", password);
  nvs_commit(handle);
  nvs_close(handle);
  wifiSsid_ = ssid;
  wifiPassword_ = password;
}

void WebInterface::saveProfiles() {
  nvs_handle_t handle;
  if (nvs_open(Namespace, NVS_READWRITE, &handle) != ESP_OK) {
    return;
  }
  writeString(handle, "customer", customerName_);
  writeString(handle, "vehicle", vehicleName_);
  writeString(handle, "profiles", vehicleProfiles_);
  nvs_commit(handle);
  nvs_close(handle);
}

void WebInterface::startAccessPoint() {
  WiFi.softAP(DefaultApSsid, DefaultApPassword);
}

void WebInterface::startStation() {
  if (wifiSsid_.isEmpty()) {
    Serial.println("WiFi STA skipped: no saved SSID");
    return;
  }
  Serial.print("WiFi STA connecting ssid=");
  Serial.println(wifiSsid_);
  WiFi.begin(wifiSsid_.c_str(), wifiPassword_.c_str());
  stationStarted_ = true;
  stationStartMs_ = millis();
}

void WebInterface::configureRoutes() {
}

void WebInterface::handleClient() {
  WiFiClient client = server_.accept();
  if (!client) {
    return;
  }

  const uint32_t startMs = millis();
  while (client.connected() && !client.available() && millis() - startMs < 80) {
    delay(1);
  }
  if (!client.available()) {
    client.stop();
    return;
  }

  const String requestLine = client.readStringUntil('\r');
  while (client.available()) {
    const String line = client.readStringUntil('\n');
    if (line == "\r" || line.length() <= 1) {
      break;
    }
  }
  handleRequest(client, requestLine);
  client.stop();
}

void WebInterface::handleRequest(WiFiClient &client, const String &requestLine) {
  int firstSpace = requestLine.indexOf(' ');
  int secondSpace = requestLine.indexOf(' ', firstSpace + 1);
  if (firstSpace < 0 || secondSpace < 0) {
    sendPage(client, statusSection());
    return;
  }
  const String path = requestLine.substring(firstSpace + 1, secondSpace);
  const String route = path.substring(0, path.indexOf('?') >= 0 ? path.indexOf('?') : path.length());

  if (route == "/wifi") {
    String ssid = queryArg(path, "ssid_manual");
    if (ssid.isEmpty()) {
      ssid = queryArg(path, "ssid");
    }
    const String password = queryArg(path, "password");
    if (!ssid.isEmpty()) {
      saveWifi(ssid, password);
      WiFi.disconnect(false, false);
      startStation();
    }
    redirectHome(client);
    return;
  }

  if (route == "/settings") {
    settings_.setCylinders(static_cast<uint8_t>(queryArg(path, "cylinders").toInt()));
    settings_.setDemoFallback(queryArg(path, "demo") == "1");
    settings_.setAutoScale(queryArg(path, "autoscale") == "1");
    settings_.setAutoBrightness(queryArg(path, "autobright") == "1");
    settings_.setBrightnessManualPercent(static_cast<uint8_t>(queryArg(path, "brightness").toInt()));
    settings_.setAlarmDeltaKpa(queryArg(path, "alarm").toFloat());
    settings_.setUnits(queryArg(path, "units") == "inhg" ? SettingsManager::Units::InHg
                                                         : SettingsManager::Units::Kpa);
    settings_.save();
    redirectHome(client);
    return;
  }

  if (route == "/profiles") {
    customerName_ = queryArg(path, "customer");
    vehicleName_ = queryArg(path, "vehicle");
    vehicleProfiles_ = queryArg(path, "profiles");
    saveProfiles();
    redirectHome(client);
    return;
  }

  String body = statusSection();
  body += wifiSection();
  body += settingsSection();
  body += profilesSection();
  sendPage(client, body);
}

void WebInterface::sendPage(WiFiClient &client, const String &body) {
  const String html = page(body);
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Content-Type: text/html; charset=utf-8"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(html.length());
  client.println();
  client.print(html);
}

void WebInterface::redirectHome(WiFiClient &client) {
  client.println(F("HTTP/1.1 303 See Other"));
  client.println(F("Location: /"));
  client.println(F("Connection: close"));
  client.println();
}

String WebInterface::queryArg(const String &path, const char *name) const {
  const int queryStart = path.indexOf('?');
  if (queryStart < 0) {
    return "";
  }
  const String token = String(name) + "=";
  int pos = path.indexOf(token, queryStart + 1);
  while (pos >= 0) {
    const bool startsArg = pos == queryStart + 1 || path[pos - 1] == '&';
    if (startsArg) {
      int end = path.indexOf('&', pos);
      if (end < 0) {
        end = path.length();
      }
      return urlDecode(path.substring(pos + token.length(), end));
    }
    pos = path.indexOf(token, pos + 1);
  }
  return "";
}

String WebInterface::urlDecode(const String &value) const {
  String decoded;
  decoded.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value[i];
    if (c == '+') {
      decoded += ' ';
    } else if (c == '%' && i + 2 < value.length()) {
      char hex[3] = {value[i + 1], value[i + 2], '\0'};
      decoded += static_cast<char>(strtol(hex, nullptr, 16));
      i += 2;
    } else {
      decoded += c;
    }
  }
  return decoded;
}

String WebInterface::page(const String &body) const {
  String html;
  html.reserve(body.length() + 1600);
  html += F("<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>");
  html += F("<title>Carbtune Pro</title><style>");
  html += F("body{font-family:Arial,sans-serif;background:#05080d;color:#f6f8fb;margin:0;padding:16px}");
  html += F("h1{font-size:24px;margin:0 0 12px}h2{font-size:18px;margin:0 0 10px}");
  html += F(".card{background:#101821;border:1px solid #2d3b48;border-radius:8px;padding:14px;margin:0 0 14px}");
  html += F("label{display:block;margin:10px 0 4px;color:#aeb8c2}input,select,textarea{width:100%;box-sizing:border-box;background:#060a10;color:#fff;border:1px solid #405568;border-radius:5px;padding:10px}");
  html += F("textarea{min-height:120px}button{background:#0077cc;color:white;border:0;border-radius:5px;padding:11px 14px;margin-top:12px;font-weight:bold}");
  html += F(".row{display:grid;grid-template-columns:1fr 1fr;gap:10px}.ok{color:#49e042}.warn{color:#ffc247}.muted{color:#9ba8b3}.net{padding:6px 0;border-bottom:1px solid #24303b}");
  html += F("</style></head><body><h1>CARBTUNE PRO ESP32</h1>");
  html += body;
  html += F("</body></html>");
  return html;
}

String WebInterface::wifiSection() {
  String html;
  html += F("<section class='card'><h2>WiFi</h2>");
  html += F("<form method='get' action='/wifi'><label>Netwerk</label><select name='ssid'>");
  const int count = WiFi.scanNetworks();
  if (count <= 0) {
    html += F("<option value=''>Geen netwerken gevonden</option>");
  } else {
    for (int i = 0; i < count; ++i) {
      const String ssid = WiFi.SSID(i);
      html += F("<option value='");
      html += htmlEscape(ssid);
      html += "'";
      if (ssid == wifiSsid_) {
        html += F(" selected");
      }
      html += ">";
      html += htmlEscape(ssid);
      html += " (";
      html += String(WiFi.RSSI(i));
      html += F(" dBm)</option>");
    }
  }
  html += F("</select><label>Of SSID handmatig</label><input name='ssid_manual'>");
  html += F("<label>Wachtwoord</label><input name='password' type='password' value='");
  html += htmlEscape(wifiPassword_);
  html += F("'><button>WiFi opslaan en verbinden</button></form></section>");
  WiFi.scanDelete();
  return html;
}

String WebInterface::settingsSection() const {
  String html;
  html += F("<section class='card'><h2>Instellingen</h2><form method='get' action='/settings'>");
  html += F("<div class='row'><div><label>Cilinders</label><select name='cylinders'>");
  const uint8_t cylinderOptions[] = {2, 4, 6};
  for (uint8_t index = 0; index < sizeof(cylinderOptions) / sizeof(cylinderOptions[0]); ++index) {
    const uint8_t count = cylinderOptions[index];
    html += F("<option value='");
    html += String(count);
    html += "'";
    if (settings_.cylinders() == count) {
      html += F(" selected");
    }
    html += ">";
    html += String(count);
    html += F("</option>");
  }
  html += F("</select></div><div><label>Eenheden</label><select name='units'>");
  html += String("<option value='kpa'") + (settings_.unitsMode() == SettingsManager::Units::Kpa ? " selected" : "") + ">kPa</option>";
  html += String("<option value='inhg'") + (settings_.unitsMode() == SettingsManager::Units::InHg ? " selected" : "") + ">inHg</option>";
  html += F("</select></div></div>");
  html += F("<label><input type='checkbox' name='demo' value='1' ");
  html += settings_.demoFallback() ? F("checked") : F("");
  html += F("> Demo modus</label><label><input type='checkbox' name='autoscale' value='1' ");
  html += settings_.autoScale() ? F("checked") : F("");
  html += F("> Auto schaal</label><label><input type='checkbox' name='autobright' value='1' ");
  html += settings_.autoBrightness() ? F("checked") : F("");
  html += F("> Auto helderheid</label>");
  html += F("<div class='row'><div><label>Helderheid %</label><input name='brightness' type='number' min='0' max='100' value='");
  html += String(settings_.brightnessManualPercent());
  html += F("'></div><div><label>Alarm delta kPa</label><input name='alarm' type='number' min='1' max='50' step='0.5' value='");
  html += String(settings_.alarmDeltaKpa(), 1);
  html += F("'></div></div><button>Instellingen opslaan</button></form></section>");
  return html;
}

String WebInterface::profilesSection() const {
  String html;
  html += F("<section class='card'><h2>Klanten en voertuigprofielen</h2><form method='get' action='/profiles'>");
  html += F("<label>Klant</label><input name='customer' value='");
  html += htmlEscape(customerName_);
  html += F("'><label>Voertuig</label><input name='vehicle' value='");
  html += htmlEscape(vehicleName_);
  html += F("'><label>Profielen / notities</label><textarea name='profiles'>");
  html += htmlEscape(vehicleProfiles_);
  html += F("</textarea><button>Profielen opslaan</button></form></section>");
  return html;
}

String WebInterface::statusSection() const {
  String html;
  html += F("<section class='card'><h2>Status</h2>");
  html += F("<div>Firmware: <span class='muted'>");
  html += FW_VERSION;
  html += F("</span></div><div>AP: <span class='ok'>");
  html += DefaultApSsid;
  html += F("</span> ");
  html += apIp().toString();
  html += F("</div><div>WiFi: ");
  if (stationConnected()) {
    html += F("<span class='ok'>verbonden</span> ");
    html += stationIp().toString();
  } else if (wifiSsid_.isEmpty()) {
    html += F("<span class='warn'>geen opgeslagen netwerk</span>");
  } else {
    html += F("<span class='warn'>niet verbonden</span> ");
    html += htmlEscape(wifiSsid_);
  }
  html += F("</div></section>");
  return html;
}

String WebInterface::htmlEscape(const String &value) const {
  String escaped;
  escaped.reserve(value.length());
  for (size_t i = 0; i < value.length(); ++i) {
    const char c = value[i];
    if (c == '&') {
      escaped += F("&amp;");
    } else if (c == '<') {
      escaped += F("&lt;");
    } else if (c == '>') {
      escaped += F("&gt;");
    } else if (c == '"') {
      escaped += F("&quot;");
    } else if (c == '\'') {
      escaped += F("&#39;");
    } else {
      escaped += c;
    }
  }
  return escaped;
}
