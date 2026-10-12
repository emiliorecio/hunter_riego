/**
 * This file includes all the functions that are scheduled to run after
 * returning a response to the client. The client asks to do some action with
 * the Hunter Irrigation System and these functions contain the necessary logic
 * to perform it and send the result to a webhook.
 *
 * First version: July 2020 - Eloi Codina Torras
 */
#include "ESP8266HTTPClient.h"
#include "ESP8266WiFi.h"
#include "HunterRoam.h"
#include "time.h"

#include "global_config.h"
#include "mqtt.h"
#include "web_server_scheduled.h"

HunterRoam smartPort(REM_PIN);
HTTPClient http;
WiFiClient wifi;

// Last command sent from this board. The bus is write-only, so this is not
// what the X-Core is doing: it misses its own programs and is lost on reboot.
static byte activeZone = 0;
static unsigned long zoneStartedAt = 0;
static unsigned long zoneDuration = 0; // ms
static byte lastProgram = 0;
static unsigned long programStartedAt = 0;

void setupStatusLed() {
  if (STATUS_LED >= 0) {
    pinMode(STATUS_LED, OUTPUT);
    digitalWrite(STATUS_LED, HIGH);
  }
}

static void statusLed(bool on) {
  if (STATUS_LED >= 0) {
    digitalWrite(STATUS_LED, on ? LOW : HIGH);
  }
}

/**
 * JSON with the zone started from here and how many seconds it has left, the
 * last program started from here and the board clock (null until NTP syncs).
 */
String getStatusJson() {
  unsigned long now = millis();
  if (activeZone && now - zoneStartedAt >= zoneDuration) {
    activeZone = 0;
  }
  String json = "{\"zone\": " + String(activeZone);
  json += ", \"remaining\": ";
  json += activeZone ? String((zoneDuration - (now - zoneStartedAt)) / 1000) : "0";
  json += ", \"program\": " + String(lastProgram);
  json += ", \"program_ago\": ";
  json += lastProgram ? String((now - programStartedAt) / 1000) : "0";
  json += ", \"time\": ";
  time_t t = time(nullptr);
  if (t > 1700000000) {
    char buf[24];
    strftime(buf, sizeof(buf), "\"%Y-%m-%d %H:%M\"", localtime(&t));
    json += buf;
  } else {
    json += "null";
  }
  return json + "}";
}

/**
 * Send the result of the action performed on the Hunter Irrigation System to a
 * webhook given by the client when it made the petition.
 *
 * @param toSend information that will be sent. Probably JSON formatted
 * @param webhook url where the information will be sent
 */
void sendResult(String toSend, String webhook) {
  http.begin(wifi, webhook);
  http.addHeader("Content-Type", "application/json");
  http.POST(toSend);
  http.end();
  mqttPublishResult(toSend.c_str());
}

/**
 * Start a zone and send the result of the action to the webhook given by the
 * client.
 *
 * @param num zone number
 * @param time irrigation minutes
 * @param webhook url where the result will be sent
 */
void startZone(byte num, byte time, String webhook) {

  statusLed(true);
  byte result = smartPort.startZone(num, time);
  statusLed(false);
  if (result == 0) {
    activeZone = time ? num : 0;
    zoneStartedAt = millis();
    zoneDuration = time * 60000UL;
  }

  String toSend = "{";
  toSend = toSend + "\"action\": \"Start zone " + (String)num + " for " +
           (String)time + " minutes.\",";
  toSend = toSend + "\"action_num\": 1,";
  toSend = toSend + "\"zone\": " + (String)num + ",";
  toSend = toSend + "\"result\": \"" + smartPort.errorHint(result) + "\",";
  toSend = toSend + "\"result_num\": " + (String)result;
  toSend = toSend + "}";

  sendResult(toSend, webhook);
}

/**
 * Stop a zone and send the result of the action to the webhook given by the
 * client.
 *
 * @param num zone number
 * @param webhook url where the result will be sent
 */
void stopZone(byte num, String webhook) {
  statusLed(true);
  byte result = smartPort.stopZone(num);
  statusLed(false);
  if (result == 0 && num == activeZone) {
    activeZone = 0;
  }

  String toSend = "{";
  toSend = toSend + "\"action\": \"Stop zone " + (String)num + ".\",";
  toSend = toSend + "\"action_num\": 2,";
  toSend = toSend + "\"zone\": \"" + (String)num + "\",";
  toSend = toSend + "\"result\": \"" + smartPort.errorHint(result) + "\",";
  toSend = toSend + "\"result_num\": " + (String)result;
  toSend = toSend + "}";

  sendResult(toSend, webhook);
}

/**
 * Start a program and send the result of the action to the webhook given by the
 * client.
 *
 * @param num program number
 * @param webhook url where the result will be sent
 */
void startProgram(byte num, String webhook) {
  statusLed(true);
  byte result = smartPort.startProgram(num);
  statusLed(false);
  if (result == 0) {
    activeZone = 0;
    lastProgram = num;
    programStartedAt = millis();
  }

  String toSend = "{";
  toSend = toSend + "\"action\": \"Start program " + (String)num + ".\",";
  toSend = toSend + "\"action_num\": 3,";
  toSend = toSend + "\"program\": " + (String)num + ",";
  toSend = toSend + "\"result\": \"" + smartPort.errorHint(result) + "\",";
  toSend = toSend + "\"result_num\": " + (String)result;
  toSend = toSend + "}";

  sendResult(toSend, webhook);
}