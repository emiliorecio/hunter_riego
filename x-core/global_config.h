#ifndef GLOBAL_CONFIG_H
#define GLOBAL_CONFIG_H

#define SERIAL_SPEED 9600
#define REM_PIN 5    // D1 (D0/GPIO16 has the onboard LED, which stays lit while the line idles LOW)

#define SSID_AP    "WateringSystemAP"   // SSID of the AP created to configure WiFi
#define HOSTNAME "X-CORE"

#define STATUS_LED 2         // GPIO2/D4, the ESP-12 LED (active LOW): on while a command is sent. -1 disables it
#define TZ_INFO    "<-03>3"  // POSIX timezone for the web schedules (Argentina, UTC-3, no DST)
#define NTP_SERVER "pool.ntp.org"

#if !__has_include("secrets.h")
#error "Missing secrets.h: copy secrets.h.example to secrets.h and set your passwords"
#endif
#include "secrets.h"      // PWD_AP: password of the AP created to configure WiFi
static_assert(sizeof(PWD_AP) - 1 >= 8 && sizeof(PWD_AP) - 1 <= 63,
              "PWD_AP in secrets.h must be 8-63 characters, or the portal AP stays open");

#define FW_NAME    "x-core"            // Which sketch is flashed
#define FW_VERSION "0.2.1"

#endif
