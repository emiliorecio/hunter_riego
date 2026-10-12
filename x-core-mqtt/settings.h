#ifndef SETTINGS_H
#define SETTINGS_H

#include "Arduino.h"

#define MAX_ZONES       48   // What the SmartPort protocol supports
#define MAX_SCHEDULES   16
#define MAX_NAME_LENGTH 64   // Bytes: the dashboard allows 32 characters, accents take 2 bytes

void loadSettings();
const String &getSettingsJson();
bool validateSettings(const String &json, String &error);
void saveSettings(const String &json);
void runSchedules();

#endif
