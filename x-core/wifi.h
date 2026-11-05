// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
#ifndef WIFI_H
#define WIFI_H

#include "ESP8266WiFi.h"
#include "ESPAsyncWiFiManager.h"
#include "ESPAsyncWebServer.h"
#include "ESPAsyncTCP.h"

#include "global_config.h"

void setup_wifi();
void requestConfigPortal();
bool checkWifiConnection(bool nextTimeShouldReboot);

#endif
