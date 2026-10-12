/**
 * Dashboard settings (zone names and watering schedules) and the schedule
 * runner.
 *
 * Settings are kept as JSON in the emulated EEPROM, a flash sector of its own:
 * it survives both firmware and LittleFS (dashboard) uploads, which replace
 * the whole file system.
 *
 * Format:
 *   {"zones": ["Front garden", ""],
 *    "schedules": [{"on": true, "days": 62, "h": 6, "m": 30, "zone": 1, "min": 10}]}
 * "zones" has one name per zone (its length is the number of zones). "days" is
 * a bitmask of weekdays: bit 0 is Sunday ... bit 6 is Saturday.
 */
#include "ArduinoJson.h"
#include "EEPROM.h"
#include "time.h"

#include "settings.h"
#include "web_server_scheduled.h"

#define EEPROM_SIZE    4096
#define EEPROM_MAGIC_0 'H'
#define EEPROM_MAGIC_1 'R'
#define EEPROM_VERSION 1
#define EEPROM_HEADER  5     // magic (2) + version (1) + JSON length (2)

// Worst case (48 names of MAX_NAME_LENGTH bytes, 16 schedules) fits in these
#define MAX_JSON_LENGTH (EEPROM_SIZE - EEPROM_HEADER)
#define DOC_CAPACITY    7168 // Parsed input: it copies keys and names
#define OUT_CAPACITY    3072 // Normalized output: it points to the input names

#define DEFAULT_SETTINGS "{\"zones\":[\"\",\"\",\"\",\"\"],\"schedules\":[]}"

struct Schedule {
  bool on;
  byte days;
  byte hour;
  byte minute;
  byte zone;
  byte minutes;
};

static String settingsJson = DEFAULT_SETTINGS;
static Schedule schedules[MAX_SCHEDULES];
static byte scheduleCount = 0;

/**
 * Parse and check a settings JSON. If it is valid, fill the schedules (when
 * given) and the normalized JSON, which drops any unknown field.
 *
 * @return an empty string if valid, otherwise what is wrong
 */
static String parseSettings(const String &json, Schedule *outSchedules,
                            byte *outCount, String *normalized) {
  if (json.length() > MAX_JSON_LENGTH) {
    return "Settings too long";
  }
  // ArduinoJson 6 API (7 still accepts it), like the rest of the project
  DynamicJsonDocument doc(DOC_CAPACITY);
  if (deserializeJson(doc, json)) {
    return "Invalid JSON";
  }
  JsonArray zones = doc["zones"].as<JsonArray>();
  if (zones.isNull() || zones.size() < 1 || zones.size() > MAX_ZONES) {
    return "'zones' must have between 1 and " + String(MAX_ZONES) + " names";
  }
  DynamicJsonDocument out(OUT_CAPACITY);
  JsonArray outZones = out.createNestedArray("zones");
  for (JsonVariant name : zones) {
    if (!name.is<const char *>() || strlen(name.as<const char *>()) > MAX_NAME_LENGTH) {
      return "Zone names must be text of up to " + String(MAX_NAME_LENGTH) + " bytes";
    }
    outZones.add(name.as<const char *>());
  }

  JsonArray list = doc["schedules"].as<JsonArray>();
  if (list.isNull() || list.size() > MAX_SCHEDULES) {
    return "'schedules' must be a list of up to " + String(MAX_SCHEDULES);
  }
  JsonArray outList = out.createNestedArray("schedules");
  byte count = 0;
  for (JsonObject item : list) {
    int days = item["days"] | -1, hour = item["h"] | -1, minute = item["m"] | -1;
    int zone = item["zone"] | -1, minutes = item["min"] | -1;
    if (days < 0 || days > 127 || hour < 0 || hour > 23 || minute < 0 ||
        minute > 59 || zone < 1 || zone > (int)zones.size() || minutes < 1 ||
        minutes > 240) {
      return "Schedule " + String(count + 1) + " has an invalid value";
    }
    Schedule s = {item["on"] | false, (byte)days, (byte)hour, (byte)minute,
                  (byte)zone, (byte)minutes};
    if (outSchedules) {
      outSchedules[count] = s;
    }
    JsonObject o = outList.createNestedObject();
    o["on"] = s.on;
    o["days"] = s.days;
    o["h"] = s.hour;
    o["m"] = s.minute;
    o["zone"] = s.zone;
    o["min"] = s.minutes;
    count++;
  }
  if (outCount) {
    *outCount = count;
  }
  if (normalized) {
    normalized->clear();
    serializeJson(out, *normalized);
    if (normalized->length() > MAX_JSON_LENGTH) {
      return "Settings too long";
    }
  }
  return "";
}

/**
 * Check a settings JSON received from the dashboard.
 *
 * @param json settings to check
 * @param error filled with what is wrong when invalid
 */
bool validateSettings(const String &json, String &error) {
  String normalized;
  error = parseSettings(json, nullptr, nullptr, &normalized);
  return error == "";
}

/**
 * Apply new settings and store them in flash. It writes to flash, so it must
 * run from the loop (schedule_function), not from a web server callback.
 */
void saveSettings(const String &json) {
  String normalized;
  Schedule parsed[MAX_SCHEDULES];
  byte count;
  if (parseSettings(json, parsed, &count, &normalized) != "") {
    return;
  }
  if (normalized == settingsJson) {
    return;
  }
  settingsJson = normalized;
  memcpy(schedules, parsed, sizeof(schedules));
  scheduleCount = count;

  // Keep the 4 KB EEPROM buffer in RAM only while it is used
  size_t len = settingsJson.length();
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, EEPROM_MAGIC_0);
  EEPROM.write(1, EEPROM_MAGIC_1);
  EEPROM.write(2, EEPROM_VERSION);
  EEPROM.write(3, len & 0xff);
  EEPROM.write(4, len >> 8);
  memcpy(EEPROM.getDataPtr() + EEPROM_HEADER, settingsJson.c_str(), len);
  bool ok = EEPROM.commit();
  EEPROM.end();
  Serial.println(ok ? "Settings saved" : "Error saving settings");
}

/**
 * Load the settings stored in flash, or the defaults if there are none.
 */
void loadSettings() {
  String stored;
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(0) == EEPROM_MAGIC_0 && EEPROM.read(1) == EEPROM_MAGIC_1 &&
      EEPROM.read(2) == EEPROM_VERSION) {
    size_t len = EEPROM.read(3) | (EEPROM.read(4) << 8);
    if (len <= EEPROM_SIZE - EEPROM_HEADER) {
      stored.concat((const char *)EEPROM.getConstDataPtr() + EEPROM_HEADER, len);
    }
  }
  EEPROM.end();

  if (stored == "" ||
      parseSettings(stored, schedules, &scheduleCount, &settingsJson) != "") {
    Serial.println("No valid settings stored, using defaults");
    parseSettings(DEFAULT_SETTINGS, schedules, &scheduleCount, &settingsJson);
  }
  Serial.printf("Settings loaded: %d schedules\n", scheduleCount);
}

const String &getSettingsJson() { return settingsJson; }

/**
 * Start the zones whose schedule matches the current minute. Call it from the
 * loop. It does nothing until the clock has been set by NTP, and checks each
 * minute only once.
 */
void runSchedules() {
  static long lastMinute = -1;
  time_t now = time(nullptr);
  if (now < 1700000000) { // Clock not set yet
    return;
  }
  long minute = now / 60;
  if (minute == lastMinute) {
    return;
  }
  lastMinute = minute;

  struct tm local;
  localtime_r(&now, &local);
  for (byte i = 0; i < scheduleCount; i++) {
    const Schedule &s = schedules[i];
    if (s.on && (s.days & (1 << local.tm_wday)) && s.hour == local.tm_hour &&
        s.minute == local.tm_min) {
      Serial.printf("Schedule %d: zone %d for %d minutes\n", i + 1, s.zone,
                    s.minutes);
      startZone(s.zone, s.minutes, "");
    }
  }
}
