/**
 * First version: July 2020 - Eloi Codina Torras
 * Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
 */
#include "LittleFS.h"
#include "ArduinoJson.h"
#include "ESP8266mDNS.h"

#include "mqtt.h"
#include "ota.h"
#include "settings.h"
#include "web_server.h"
#include "web_server_scheduled.h"
#include "wifi.h"

AsyncWebServer server = AsyncWebServer(80);
bool wifiDisconnected = false;
bool enableMQTT = true;
MQTT MqttSettings;
MQTT from_wifi_manager;

void setup() {
  Serial.begin(SERIAL_SPEED);
  Serial.printf("\n%s v%s\n", FW_NAME, FW_VERSION);
  setupStatusLed();
  loadSettings();
  // WiFi.mode(WIFI_STA);
  Serial.println("mounting FS...");
  if (LittleFS.begin()) {
    Serial.println("mounted file system");
    if (LittleFS.exists("/config.json")) {
      // file exists, reading and loading
      Serial.println("reading config file");
      File configFile = LittleFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, configFile);
        serializeJson(doc, Serial);
        if (error) {
          Serial.print(F("\ndeserializeJson() failed: "));
          Serial.println(error.c_str());
          Serial.println("failed to load json config");
        } else {
          Serial.println("\njson successfully parsed");
          MqttSettings.server = doc["mqtt_server"].as<String>();
          MqttSettings.port = doc["mqtt_port"];
          MqttSettings.username = doc["mqtt_username"].as<String>();
          MqttSettings.password = doc["mqtt_password"].as<String>();
          MqttSettings.enable = doc["mqtt_enable"].as<bool>();
        }
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  // this is here just to make sure that if wifimanager is
  // skipped when config is saved, MQTT still works, otherwise empty data would
  // override what we learned from config file saved on disk
  from_wifi_manager = setup_wifi(MqttSettings);
  if (from_wifi_manager.server != "") {
    MqttSettings = from_wifi_manager;
  }
  configTime(TZ_INFO, NTP_SERVER);
  setup_WebServer();
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonDocument doc(1024);
    doc["mqtt_server"] = MqttSettings.server;
    doc["mqtt_port"] = MqttSettings.port;
    doc["mqtt_username"] = MqttSettings.username;
    doc["mqtt_password"] = MqttSettings.password;
    doc["mqtt_enable"] = MqttSettings.enable;
    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }
    serializeJson(doc, Serial);
    serializeJsonPretty(doc, configFile);
    configFile.close();
    // end save
  }
  if (MqttSettings.enable) {
    client.setServer(MqttSettings.server.c_str(), MqttSettings.port);
    client.setCallback(mqtt_callback);
  }
  Serial.print("\nIP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("%s:%d\n", MqttSettings.server.c_str(), MqttSettings.port);
  Serial.printf("Ready! Open http://%s.local in your browser\n", HOSTNAME);
}

void loop() {
  // Non-blocking reconnect: retry every 5 seconds so the web server and
  // scheduled zone commands keep working while the broker is unreachable.
  static unsigned long lastAttempt = 0;
  static int retries = 0;
  if (MqttSettings.enable && !client.connected() &&
      millis() - lastAttempt >= 5000) {
    lastAttempt = millis();
    mqtt_connect(MqttSettings.username.c_str(), MqttSettings.password.c_str());
    if (client.connected()) {
      retries = 0;
    } else if (++retries >= 50) {
      ESP.restart();
    }
  }
  // wifiDisconnected = checkWifiConnection(wifiDisconnected);
  client.loop();
  MDNS.update();
  runSchedules();
  otaLoop();
  // timer.run();
}