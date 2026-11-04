/**
 * Connect to WiFi or setup a captive portal to configure WiFi parameters.
 *
 * First version: July 2020 - Eloi Codina Torras
 */

#include "DNSServer.h"

#include "global_config.h"
#include "web_server.h"
#include "wifi.h"

#define PORTAL_FLAG    0x504f5254 // "PORT", kept in RTC memory across a reboot
#define PORTAL_TIMEOUT 300        // seconds

/**
 * Make the next boot open the configuration portal. The flag lives in RTC
 * memory, which survives a software restart without writing to flash.
 */
void requestConfigPortal() {
  uint32_t flag = PORTAL_FLAG;
  ESP.rtcUserMemoryWrite(0, &flag, sizeof(flag));
}

/**
 * Whether requestConfigPortal() asked for the portal before this boot. It
 * clears the flag, so the portal opens only once.
 */
static bool portalRequested() {
  uint32_t flag = 0;
  ESP.rtcUserMemoryRead(0, &flag, sizeof(flag));
  if (flag != PORTAL_FLAG) {
    return false;
  }
  flag = 0;
  ESP.rtcUserMemoryWrite(0, &flag, sizeof(flag));
  return true;
}

void setup_wifi() {
  DNSServer dns;
  WiFi.persistent(true);
  // Keep the radio awake: with the default modem sleep the ESP8266 misses
  // multicast packets and stops answering mDNS (X-CORE.local).
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  AsyncWiFiManager wifiManager(&server, &dns);
  // wifiManager.resetSettings();
  if (portalRequested()) {
    // Opened from the dashboard. If nobody saves it in time, reboot and
    // connect with the saved credentials as usual.
    Serial.println("Opening the configuration portal on request");
    wifiManager.setConfigPortalTimeout(PORTAL_TIMEOUT);
    if (!wifiManager.startConfigPortal(SSID_AP, PWD_AP)) {
      ESP.restart();
    }
    return;
  }
  // Connect with the saved credentials; only open the config portal if that
  // fails, so the device reconnects on its own after a power cut.
  wifiManager.autoConnect(SSID_AP, PWD_AP);
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