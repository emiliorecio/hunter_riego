#ifndef OTA_H
#define OTA_H

#include "ESPAsyncWebServer.h"

void handleUpdate(AsyncWebServerRequest *request);
void handleDoUpdate(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final);
void handleUpdateDone(AsyncWebServerRequest *request);
void scheduleReboot();
void otaLoop();

#endif