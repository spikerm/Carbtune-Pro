#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include "App/SettingsManager.h"
#include "Network/OtaManager.h"

class WebInterface {
 public:
  WebInterface(SettingsManager &settings, OtaManager &otaManager);

  void begin();
  void update();

  const char *apSsid() const;
  IPAddress apIp() const;
  IPAddress stationIp() const;
  bool stationConnected() const;

 private:
  void load();
  void saveWifi(const String &ssid, const String &password);
  void saveProfiles();
  void startAccessPoint();
  void startStation();
  void configureRoutes();
  void handleClient();
  void handleRequest(WiFiClient &client, const String &requestLine);
  void sendPage(WiFiClient &client, const String &body);
  void redirectHome(WiFiClient &client);
  String queryArg(const String &path, const char *name) const;
  String urlDecode(const String &value) const;
  String page(const String &body) const;
  String wifiSection();
  String otaSection() const;
  String settingsSection() const;
  String profilesSection() const;
  String statusSection() const;
  String htmlEscape(const String &value) const;

  SettingsManager &settings_;
  OtaManager &otaManager_;
  WiFiServer server_{80};
  String wifiSsid_;
  String wifiPassword_;
  String customerName_;
  String vehicleName_;
  String vehicleProfiles_;
  bool stationStarted_ = false;
  uint32_t stationStartMs_ = 0;
};
