// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
#include "LittleFS.h"
#include "Updater.h"
#include "flash_hal.h"

#include "ota.h"

static bool updatingFS = false;
static bool updateOk = false;
static String uploadError;
static bool rebootPending = false;

// Used when /update.html is missing (e.g. after a failed LittleFS update), so
// the dashboard can still be uploaded again without USB.
static const char FALLBACK_FORM[] PROGMEM =
    "<head><meta charset='utf-8'><meta name='viewport' "
    "content='width=device-width, initial-scale=1.0'></head>"
    "<body style='font-family:sans-serif'>"
    "<h3>Dashboard no encontrado: subir la imagen LittleFS de data/</h3>"
    "<form method='POST' action='/doUpdate?type=fs' enctype='multipart/form-data'>"
    "<input type='file' name='update' required> <input type='submit' value='Subir dashboard'></form>"
    "<h3>Firmware</h3>"
    "<form method='POST' action='/doUpdate' enctype='multipart/form-data'>"
    "<input type='file' name='update' required> <input type='submit' value='Subir firmware'></form>"
    "</body>";

void handleUpdate(AsyncWebServerRequest *request) {
  if (LittleFS.exists("/update.html")) {
    request->send(LittleFS, "/update.html", String());
  } else {
    request->send_P(200, "text/html", FALLBACK_FORM);
  }
}

/**
 * Reboot from the loop in a second, so the current response reaches the
 * browser first.
 */
void scheduleReboot() { rebootPending = true; }

/**
 * Receive an uploaded image and write it to flash. It is the LittleFS image
 * (the dashboard) when the form sends ?type=fs or the file name contains
 * "littlefs" or "spiffs"; otherwise it is the firmware.
 */
void handleDoUpdate(AsyncWebServerRequest *request, const String &filename,
                    size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    updateOk = false;
    uploadError = "";
    updatingFS = (request->hasParam("type") && request->getParam("type")->value() == "fs") ||
                 filename.indexOf("littlefs") > -1 || filename.indexOf("spiffs") > -1;
    Serial.printf("Update %s: %s\n", updatingFS ? "LittleFS" : "firmware", filename.c_str());
    // A LittleFS image has "littlefs" at offset 8. Check it before erasing
    // the current file system with something else (e.g. a firmware .bin).
    if (updatingFS && (len < 16 || memcmp(data + 8, "littlefs", 8) != 0)) {
      uploadError = "the file is not a LittleFS image";
      Serial.println(uploadError);
      return;
    }
    // The request length includes the multipart headers, so it can be larger
    // than the image: size the update from the partition instead.
    size_t maxSize;
    if (updatingFS) {
      LittleFS.end();
      maxSize = FS_PHYS_SIZE;
    } else {
      maxSize = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    }
    Update.runAsync(true);
    if (!Update.begin(maxSize, updatingFS ? U_FS : U_FLASH)) {
      Update.printError(Serial);
    }
  }

  if (uploadError != "") {
    return;
  }
  if (!Update.hasError()) {
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
    } else {
      Serial.printf("Progress: %u bytes\n", Update.progress());
    }
  }

  if (final && !Update.hasError()) {
    if (!Update.end(true)) {
      Update.printError(Serial);
    } else {
      Serial.println("Update complete");
      updateOk = true;
    }
  }
}

/**
 * Called once the upload has finished: report the result and, if it worked,
 * reboot from the loop once the response has been sent.
 */
void handleUpdateDone(AsyncWebServerRequest *request) {
  if (!updateOk) {
    String error = uploadError != "" ? uploadError
                   : Update.hasError() ? Update.getErrorString()
                                       : String("no file received");
    if (uploadError == "") {
      Update.end();       // Discard the partial update
      if (updatingFS) {
        LittleFS.begin(); // Serve whatever is left of the dashboard
      }
    }
    request->send(500, "text/html",
                  "<head><meta charset='utf-8'></head><body>Error: " + error +
                      "<br><a href='/update'>Volver</a></body>");
    return;
  }
  request->send(200, "text/html",
                "<head><meta charset='utf-8'><meta http-equiv='refresh' "
                "content='15;URL=/'/></head><body>Listo. La placa se está "
                "reiniciando...</body>");
  scheduleReboot();
}

/**
 * Call from the loop: does the reboot asked for with scheduleReboot().
 */
void otaLoop() {
  static unsigned long requestedAt = 0;
  if (!rebootPending) {
    return;
  }
  if (!requestedAt) {
    requestedAt = millis();
  } else if (millis() - requestedAt > 1000) {
    ESP.restart();
  }
}
