#include "LittleFS.h"
#include "web_interface.h"
#include "ESPAsyncWebServer.h"

void handleHomePage(AsyncWebServerRequest *request) {
  request->send(LittleFS, "/index.html", String());
}