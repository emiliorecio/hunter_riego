// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include "Arduino.h"
#include "ESPAsyncWebServer.h"

struct WebInterface
{
    public:
        String header;
        String body;
        String footer;
};

void handleHomePage(AsyncWebServerRequest *request);

#endif