// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
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