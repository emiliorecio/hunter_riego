/**
 * This file includes all the functions that are called by the webserver to
 * serve API paths. It makes it easier to control the actions to be performed to
 * the Hunter Irrigation System.
 *
 * First version: July 2020 - Eloi Codina Torras
 * Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
 */
#include "Schedule.h"

#include "ota.h"
#include "settings.h"
#include "web_server_api.h"
#include "web_server_scheduled.h"
#include "wifi.h"

/**
 * Extract the latest parameter on the URL given the request and the base path.
 * It has to be done with this "hack" as REGEX doesnt't work properly on the
 * ESP8266 currently.
 *
 * Example:
 * AsyncWebServerRequest request </api/start/zone/3>
 * int id = getIdFromURL(request, "/api/start/zone/");
 * Serial.println(id);
 *  --> 3
 *
 * @param request request made by the client
 * @param root base path of the URL.
 */
int getIdFromURL(AsyncWebServerRequest *request, String root) {
  String string_id = request->url();
  string_id.replace(root, "");
  int id = string_id.toInt();
  return id;
}

/**
 * Function to be called when the client makes a request to
 * /api/start/zone
 * It checks that all the parameters have been passed and schedules a function
 * to be run at the next loop to start the zone with the HunterRoam library.
 *
 * @param request request made by the client
 */
void startZoneRequest(AsyncWebServerRequest *request) {
  if (!(request->hasParam("time"))) {
    request->send_P(400, "text/plain", "Please include parameter 'time'.");
  } else {
    int num = getIdFromURL(request, "/api/start/zone/");
    int time = request->getParam("time")->value().toInt();
    if (num < 1 || num > MAX_ZONES) {
      request->send(400, "text/plain", "Zone must be between 1 and " + String(MAX_ZONES) + ".");
      return;
    }
    if (time < 1 || time > 240) {
      request->send(400, "text/plain", "Parameter 'time' must be between 1 and 240 minutes.");
      return;
    }

    String result;
    String toReturn;
    if (!(request->hasParam("result"))) {
      result = "";
      toReturn = "\"As you have not specified a 'result' argument, the result "
                 "of the operation can't be send to you.\"";
    } else {
      result = (String)request->getParam("result")->value();
      toReturn = "\"After the action, the result will be sent to the webhook "
                 "url you have specified.\"";
    }

    schedule_function([num, time, result]() { startZone(num, time, result); });

    request->send_P(200, "application/json",
                    ((String) "{\"status\": \"Request received and processed "
                              "correctly.\"," +
                     (String) "\"result\": " + toReturn + "}")
                        .c_str());
  }
}

/**
 * Function to be called when the client makes a request to
 * /api/start/program
 * It checks that all the parameters have been passed and schedules a function
 * to be run at the next loop to start the program with the HunterRoam library.
 *
 * @param request request made by the client
 */
void startProgramRequest(AsyncWebServerRequest *request) {

  String result;
  String toReturn;
  if (!(request->hasParam("result"))) {
    result = "";
    toReturn = "\"As you have not specified a 'result' argument, the result of "
               "the operation can't be send to you.\"";
  } else {
    result = request->getParam("result")->value();
    toReturn = "\"After the action, the result will be sent to the webhook url "
               "you have specified.\"";
  }

  int num = getIdFromURL(request, "/api/start/program/");
  if (num < 1 || num > 4) {
    request->send(400, "text/plain", "Program must be between 1 and 4.");
    return;
  }
  schedule_function([num, result]() { startProgram(num, result); });

  request->send_P(
      200, "application/json",
      ((String) "{\"status\": \"Request received and processed correctly.\"," +
       (String) "\"result\": " + toReturn + "}")
          .c_str());
}

/**
 * Function to be called when the client makes a request to
 * /api/stop/zone
 * It checks that all the parameters have been passed and schedules a function
 * to be run at the next loop to stop the zone with the HunterRoam library.
 *
 * @param request request made by the client
 */
void stopZoneRequest(AsyncWebServerRequest *request) {

  String result;
  String toReturn;
  if (!(request->hasParam("result"))) {
    result = "";
    toReturn = "\"As you have not specified a 'result' argument, the result of "
               "the operation can't be send to you.\"";
  } else {
    result = request->getParam("result")->value();
    toReturn = "\"After the action, the result will be sent to the webhook url "
               "you have specified.\"";
  }

  int num = getIdFromURL(request, "/api/stop/zone/");
  if (num < 1 || num > MAX_ZONES) {
    request->send(400, "text/plain", "Zone must be between 1 and " + String(MAX_ZONES) + ".");
    return;
  }
  schedule_function([num, result]() { stopZone(num, result); });

  request->send_P(
      200, "application/json",
      ((String) "{\"status\": \"Request received and processed correctly.\"," +
       (String) "\"result\": " + toReturn + "}")
          .c_str());
}

/**
 * GET /api/status: what was last started from this board (see getStatusJson).
 */
void statusRequest(AsyncWebServerRequest *request) {
  request->send(200, "application/json", getStatusJson());
}

/**
 * GET /api/settings: zone names and schedules.
 */
void getSettingsRequest(AsyncWebServerRequest *request) {
  request->send(200, "application/json", getSettingsJson());
}

/**
 * POST /api/settings with a form field 'data' holding the settings JSON. It is
 * checked here and written to flash at the next loop.
 */
void saveSettingsRequest(AsyncWebServerRequest *request) {
  if (!request->hasParam("data", true)) {
    request->send(400, "text/plain", "Please include field 'data'.");
    return;
  }
  String data = request->getParam("data", true)->value();
  String error;
  if (!validateSettings(data, error)) {
    request->send(400, "text/plain", error);
    return;
  }
  schedule_function([data]() { saveSettings(data); });
  request->send(200, "application/json", "{\"status\": \"Settings saved.\"}");
}

/**
 * POST /api/portal: reboot into the WiFi configuration portal.
 */
void portalRequest(AsyncWebServerRequest *request) {
  requestConfigPortal();
  scheduleReboot();
  request->send(200, "application/json",
                "{\"status\": \"Rebooting into the configuration portal.\"}");
}
