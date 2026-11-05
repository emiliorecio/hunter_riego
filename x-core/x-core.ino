/**
 * First version: July 2020 - Eloi Codina Torras
 * Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
 */
#include "LittleFS.h"
#include "ESP8266mDNS.h"

#include "ota.h"
#include "settings.h"
#include "web_server.h"
#include "web_server_scheduled.h"
#include "wifi.h"

AsyncWebServer server = AsyncWebServer(80);

void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.printf("\n%s v%s\n", FW_NAME, FW_VERSION);
  setupStatusLed();
  loadSettings();
  Serial.println("mounting FS...");
  if (LittleFS.begin()) {
    Serial.println("mounted file system");
  } else {
    Serial.println("failed to mount FS");
  }
  setup_wifi();
  configTime(TZ_INFO, NTP_SERVER);
  setup_WebServer();
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Ready! Open http://%s.local in your browser\n", HOSTNAME);
}

void loop() {
  MDNS.update();
  runSchedules();
  otaLoop();
}
