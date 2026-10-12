#include "LittleFS.h"
#include "web_interface.h"
#include "ESPAsyncWebServer.h"

void handleHomePage(AsyncWebServerRequest *request) {
  if (!LittleFS.exists("/index.html")) {
    request->redirect("/update"); // Dashboard missing: offer to upload it
    return;
  }
  request->send(LittleFS, "/index.html", String());
}