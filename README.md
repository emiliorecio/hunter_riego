# Hunter Riego

Control WiFi para un **Hunter X-Core de 4 zonas** con una **NodeMCU (ESP8266)**
conectada al puerto REM (SmartPort / Roam). Trae un dashboard web para arrancar
y parar zonas y lanzar los programas A/B/C, y una API HTTP.

## Estructura

```
hunter_riego/
├── x-core/          Sketch sin MQTT: dashboard web + API HTTP
├── x-core-mqtt/     Mismo sketch + MQTT (p. ej. para Home Assistant)
└── docs/            API HTTP, API MQTT y conexión al SmartPort
```

Cada carpeta `x-core*` es un sketch independiente de Arduino IDE. La carpeta
tiene que llamarse igual que su `.ino`, así que no hay que renombrar una sin la
otra. La subcarpeta `data/` es el dashboard y se sube aparte al LittleFS.

## Requisitos

**Placa:** paquete *esp8266* en el Boards Manager → **NodeMCU 1.0 (ESP-12E Module)**.

**Librerías:** se instalan desde el gestor de librerías, salvo la que dice ZIP:

| Librería | Versión | `x-core` | `x-core-mqtt` |
|---|---|:-:|:-:|
| ESP Async WebServer (ESP32Async) | 3.12.1 | ✔ | ✔ |
| ESP Async TCP (ESP32Async) | 2.0.0 | ✔ | ✔ |
| [ESP Async WiFi Manager](https://github.com/alanswx/ESPAsyncWiFiManager/archive/refs/heads/master.zip) (alanswx, ZIP) | 0.31 | ✔ | ✔ |
| PubSubClient (Nick O'Leary) | 2.8 | | ✔ |
| ArduinoJson (Benoit Blanchon) | 6.21.6 (**no** la 7.x) | | ✔ |

No instalar las versiones de *me-no-dev* ni de *dvarrel* de ESPAsyncTCP o
ESPAsyncWebServer, porque chocan con las de ESP32Async.

**Subida del LittleFS (IDE 2.x):** plugin
[arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload/releases)
1.6.3. Hay que copiar el `.vsix` a `~/.arduinoIDE/plugins/` y reiniciar el IDE.

## Compilar y subir

1. Abrir `x-core/x-core.ino` (o `x-core-mqtt/x-core-mqtt.ino`) con **File → Open…**.
2. Elegir la placa y el puerto, y darle a **Upload**.
3. Cerrar el Serial Monitor, apretar `Ctrl+Shift+P` y elegir **Upload LittleFS to Pico/ESP8266/ESP32**
   para subir `data/`.

Después de la primera instalación se puede actualizar por WiFi desde
`http://X-CORE.local/update`.

## Configuración

En `global_config.h` de cada sketch:

| Define | Por defecto | Qué es |
|---|---|---|
| `REM_PIN` | `16` (D0) | GPIO conectado al REM del X-Core |
| `HOSTNAME` | `X-CORE` | Nombre mDNS (`http://X-CORE.local`) |
| `SSID_AP` / `PWD_AP` | `WateringSystemAP` / vacío | Red del portal de configuración WiFi |
| `SERIAL_SPEED` | `9600` | Baudios del Serial Monitor |

## Primer uso

1. Al encender por primera vez aparece la red abierta **WateringSystemAP**.
2. Conectarse y completar el portal con el WiFi de casa. Si no se abre solo,
   entrar a `192.168.4.1`. En `x-core-mqtt` ahí también se cargan los datos del broker.
3. Abrir `http://X-CORE.local`.

En los siguientes arranques se conecta solo con el WiFi guardado. El portal
vuelve a aparecer solamente si no logra conectarse.

## Documentación

- [API HTTP](docs/pages/api.md)
- [API MQTT](docs/pages/mqtt.md) (solo `x-core-mqtt`)
- [Conexión al SmartPort](docs/pages/hunterconnection.md)

## Créditos y licencia

Basado en [hunter-wifi](https://github.com/ecodina/hunter-wifi) de Eloi Codina
Torras (2020-2022), que a su vez parte del trabajo de
[Sebastien](https://github.com/seb821/OpenSprinkler-Firmware-Hunter).
Se distribuye bajo **GNU GPL v3**, igual que el original. Ver [LICENSE](LICENSE).
