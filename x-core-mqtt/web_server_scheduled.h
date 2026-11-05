// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
#ifndef WEB_SERVER_SCHEDULED_H
#define WEB_SERVER_SCHEDULED_H

#include "ESPAsyncWebServer.h"

void startZone(byte num, byte time, String webhook);
void startProgram(byte num, String webhook);
void stopZone(byte num, String webhook);
void setupStatusLed();
String getStatusJson();

#endif