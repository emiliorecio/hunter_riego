// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
#ifndef WEB_SERVER_API_H
#define WEB_SERVER_API_H

#include "ESPAsyncWebServer.h"

void startZoneRequest(AsyncWebServerRequest *request);
void startProgramRequest(AsyncWebServerRequest *request);
void stopZoneRequest(AsyncWebServerRequest *request);
void statusRequest(AsyncWebServerRequest *request);
void getSettingsRequest(AsyncWebServerRequest *request);
void saveSettingsRequest(AsyncWebServerRequest *request);
void portalRequest(AsyncWebServerRequest *request);
int getIdFromURL(AsyncWebServerRequest *request, String root);

#endif