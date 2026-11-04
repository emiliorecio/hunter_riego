/**
 * Connect to WiFi or setup a captive portal to configure WiFi parameters.
 *
 * First version: July 2020 - Eloi Codina Torras
 */

#include "DNSServer.h"

#include "global_config.h"
#include "mqtt.h"
#include "web_server.h"
#include "wifi.h"

#define PORTAL_FLAG    0x504f5254 // "PORT", kept in RTC memory across a reboot
#define PORTAL_TIMEOUT 300        // seconds
// RTC user memory block for the flag. Blocks 0-31 (the first 128 bytes) are
// overwritten by the core/eboot (OTA command), which erased the flag on boot.
#define PORTAL_RTC_BLOCK 32

/**
 * Make the next boot open the configuration portal. The flag lives in RTC
 * memory, which survives a software restart without writing to flash.
 */
void requestConfigPortal() {
  uint32_t flag = PORTAL_FLAG;
  ESP.rtcUserMemoryWrite(PORTAL_RTC_BLOCK, &flag, sizeof(flag));
}

/**
 * Whether requestConfigPortal() asked for the portal before this boot. It
 * clears the flag, so the portal opens only once.
 */
static bool portalRequested() {
  uint32_t flag = 0;
  ESP.rtcUserMemoryRead(PORTAL_RTC_BLOCK, &flag, sizeof(flag));
  if (flag != PORTAL_FLAG) {
    return false;
  }
  flag = 0;
  ESP.rtcUserMemoryWrite(PORTAL_RTC_BLOCK, &flag, sizeof(flag));
  return true;
}

/**
 * Connect to WiFi, opening the portal if needed.
 *
 * @param current saved MQTT settings: the portal fields start with them, so
 * saving the portal without retyping them keeps them
 */
MQTT setup_wifi(const MQTT &current) {
  DNSServer dns;
  MQTT MqttSettings;
  char MQTT_ENABLE[2] = "0";
  char MQTT_SERVER[40] = {0};
  char MQTT_PORT[6] = "1883";
  char MQTT_USER[100] = {0};
  char MQTT_PASSWORD[100] = {0};
  MQTT_ENABLE[0] = current.enable ? '1' : '0';
  strlcpy(MQTT_SERVER, current.server.c_str(), sizeof(MQTT_SERVER));
  if (current.port > 0) {
    snprintf(MQTT_PORT, sizeof(MQTT_PORT), "%d", current.port);
  }
  strlcpy(MQTT_USER, current.username.c_str(), sizeof(MQTT_USER));
  strlcpy(MQTT_PASSWORD, current.password.c_str(), sizeof(MQTT_PASSWORD));
  WiFi.persistent(true);
  // Keep the radio awake: with the default modem sleep the ESP8266 misses
  // multicast packets and stops answering mDNS (X-CORE.local).
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  AsyncWiFiManagerParameter custom_mqtt_enable("enable", "Enable MQTT (1/0)",
                                               MQTT_ENABLE, 1);
  AsyncWiFiManagerParameter custom_mqtt_server("server", "MQTT server",
                                               MQTT_SERVER, 40);
  AsyncWiFiManagerParameter custom_mqtt_port("port", "MQTT port", MQTT_PORT, 5);
  AsyncWiFiManagerParameter custom_mqtt_username("username", "MQTT username",
                                                 MQTT_USER, 100);
  AsyncWiFiManagerParameter custom_mqtt_password("password", "MQTT password",
                                                 MQTT_PASSWORD, 100);
  AsyncWiFiManager wifiManager(&server, &dns);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.addParameter(&custom_mqtt_enable);
  wifiManager.addParameter(&custom_mqtt_server);
  wifiManager.addParameter(&custom_mqtt_port);
  wifiManager.addParameter(&custom_mqtt_username);
  wifiManager.addParameter(&custom_mqtt_password);
  // wifiManager.resetSettings();
  if (portalRequested()) {
    // Opened from the dashboard. If nobody saves it in time, reboot and
    // connect with the saved credentials as usual.
    Serial.println("Opening the configuration portal on request");
    wifiManager.setConfigPortalTimeout(PORTAL_TIMEOUT);
    if (!wifiManager.startConfigPortal(SSID_AP, PWD_AP)) {
      ESP.restart();
    }
  } else {
    // Connect with the saved credentials; only open the config portal if that
    // fails, so the device reconnects on its own after a power cut.
    wifiManager.autoConnect(SSID_AP, PWD_AP);
  }
  MqttSettings.server = custom_mqtt_server.getValue();
  String port_number = custom_mqtt_port.getValue();
  MqttSettings.port = port_number.toInt();
  MqttSettings.username = custom_mqtt_username.getValue();
  MqttSettings.password = custom_mqtt_password.getValue();
  String en = custom_mqtt_enable.getValue();
  MqttSettings.enable = (bool)en.toInt();
  return MqttSettings;
}

/*
 * Checks, every 5 seconds, if WiFi is still active.
 * The first time it detects WiFi is not active, it returns "true".
 * If the next time this function is called (so nextTimeShouldReboot == true),
 * WiFi is still not active, it restarts ESP8266.
 *
 * @param nextTimeShouldReboot should be true if the last time WiFi connection
 * was checked, it was disconnected.
 */

bool checkWifiConnection(bool nextTimeShouldReboot) {
  delay(5000);
  if (WiFi.status() != WL_CONNECTED) {
    if (!nextTimeShouldReboot) {
      return true;
    }
    ESP.restart();
  }
  return false;
}