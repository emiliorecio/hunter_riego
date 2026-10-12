/**
 * This file includes sets up the web server to serve teh API and webpage
 * to esily control the Hunter Irrigation System.
 *
 * First version: July 2020 - Eloi Codina Torras
 */
#include "ESP8266mDNS.h"
#include "LittleFS.h"
#include "ESPAsyncTCP.h"

#include "global_config.h"
#include "ota.h"
#include "web_interface.h"
#include "web_server.h"
#include "web_server_api.h"

/**
 * Configure the paths and start the server.
 */
void setup_WebServer() {
  if (!MDNS.begin(HOSTNAME)) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  setup_APIRoutes();
  server.begin();
  MDNS.setHostname(HOSTNAME);
  MDNS.addService("http", "tcp", 80);
}

/**
 * Set up all the API routes
 */
void setup_APIRoutes() {
  server.on("/api/start/zone", HTTP_GET, startZoneRequest);
  server.on("/api/start/program", HTTP_GET, startProgramRequest);
  server.on("/api/stop/zone", HTTP_GET, stopZoneRequest);
  server.on("/api/start", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(400, "text/plain", "What should I start?");
  });
  server.on("/api/version", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json",
                  "{\"name\": \"" FW_NAME "\", \"version\": \"" FW_VERSION "\"}");
  });
  server.on("/api/status", HTTP_GET, statusRequest);
  server.on("/api/settings", HTTP_GET, getSettingsRequest);
  server.on("/api/settings", HTTP_POST, saveSettingsRequest);
  server.on("/api/portal", HTTP_POST, portalRequest);
  server.on("/api", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(403, "text/plain", "This is the API root");
  });
  server.on("/", HTTP_GET,
            [](AsyncWebServerRequest *request) { request->redirect("/home"); });
  server.on("/home", HTTP_GET,
            [](AsyncWebServerRequest *request) { handleHomePage(request); });
  server.on("/update", HTTP_GET,
            [](AsyncWebServerRequest *request) { handleUpdate(request); });
  server.on(
      "/doUpdate", HTTP_POST, handleUpdateDone,
      [](AsyncWebServerRequest *request, const String &filename, size_t index,
         uint8_t *data, size_t len, bool final) {
        handleDoUpdate(request, filename, index, data, len, final);
      });

  // Dashboard assets bundled in data/ (cached: they only change on a LittleFS upload)
  server.serveStatic("/css/", LittleFS, "/css/").setCacheControl("max-age=86400");
  server.serveStatic("/js/", LittleFS, "/js/").setCacheControl("max-age=86400");

  server.onNotFound(handleNotFound);
}

void handleNotFound(AsyncWebServerRequest *request) {
  request->send(404);
}