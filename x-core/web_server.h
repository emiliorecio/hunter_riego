// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include "ESPAsyncWebServer.h"

extern AsyncWebServer server;

void setup_WebServer();
void setup_APIRoutes();
void handleNotFound(AsyncWebServerRequest *request);

#endif