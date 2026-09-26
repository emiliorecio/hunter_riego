/**
 * First version: July 2020 - Eloi Codina Torras
 */
#include "LittleFS.h"
#include "ESP8266mDNS.h"

#include "web_server.h"
#include "wifi.h"

AsyncWebServer server = AsyncWebServer(80);

void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.println("mounting FS...");
  if (LittleFS.begin()) {
    Serial.println("mounted file system");
  } else {
    Serial.println("failed to mount FS");
  }
  setup_wifi();
  setup_WebServer();
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("Ready! Open http://%s.local in your browser\n", HOSTNAME);
}

void loop() {
  MDNS.update();
}
