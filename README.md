# Hunter Riego

Control WiFi para un **Hunter X-Core de 4 zonas** con una **NodeMCU (ESP8266)**
conectada al puerto REM (SmartPort / Roam). Trae un dashboard web para arrancar
y parar zonas y lanzar los programas A/B/C, ponerles nombre a las zonas, ver qué
zona está regando y cargar horarios propios, y una API HTTP.

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
| ArduinoJson (Benoit Blanchon) | 6.21.6 (**no** la 7.x) | ✔ | ✔ |

No instalar las versiones de *me-no-dev* ni de *dvarrel* de ESPAsyncTCP o
ESPAsyncWebServer, porque chocan con las de ESP32Async.

**Subida del LittleFS (IDE 2.x):** plugin
[arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload/releases)
1.6.3. Hay que copiar el `.vsix` a `~/.arduinoIDE/plugins/` y reiniciar el IDE.

## Compilar y subir

1. En la carpeta del sketch, copiar `secrets.h.example` a `secrets.h` y poner la
   clave del portal (`PWD_AP`, entre 8 y 63 caracteres). Si falta o es muy corta,
   no compila. `secrets.h` está en el `.gitignore`.
2. Abrir `x-core/x-core.ino` (o `x-core-mqtt/x-core-mqtt.ino`) con **File → Open…**.
3. Elegir la placa y el puerto, y darle a **Upload**.
4. Cerrar el Serial Monitor, apretar `Ctrl+Shift+P` y elegir **Upload LittleFS to Pico/ESP8266/ESP32**
   para subir `data/`.

Después de la primera instalación se puede actualizar por WiFi desde
`http://X-CORE.local/update` (ver [Actualizar por WiFi](#actualizar-por-wifi)).

## Configuración

En `global_config.h` de cada sketch:

| Define | Por defecto | Qué es |
|---|---|---|
| `REM_PIN` | `5` (D1) | GPIO conectado al REM del X-Core |
| `HOSTNAME` | `X-CORE` | Nombre mDNS (`http://X-CORE.local`) |
| `SSID_AP` | `WateringSystemAP` | Red del portal de configuración WiFi (su clave, `PWD_AP`, va en `secrets.h`) |
| `SERIAL_SPEED` | `9600` | Baudios del Serial Monitor |
| `STATUS_LED` | `2` (D4) | LED que se prende mientras se envía un comando (`-1` lo desactiva) |
| `TZ_INFO` | `<-03>3` | Zona horaria POSIX de los horarios (Argentina, UTC-3) |
| `NTP_SERVER` | `pool.ntp.org` | Servidor de hora |

## Primer uso

1. Al encender por primera vez aparece la red **WateringSystemAP**, con la clave de `secrets.h`.
2. Conectarse y completar el portal con el WiFi de casa. Si no se abre solo,
   entrar a `192.168.4.1`. En `x-core-mqtt` ahí también se cargan los datos del broker.
3. Abrir `http://X-CORE.local`.

En los siguientes arranques se conecta solo con el WiFi guardado. El portal
vuelve a aparecer solamente si no logra conectarse, o si se pide desde el link
**Reabrir portal de configuración** del dashboard: la placa se reinicia y deja
el portal abierto 5 minutos (en `x-core-mqtt`, con los datos del broker ya
cargados). Si nadie lo guarda, vuelve a conectarse como antes.

## Dashboard

- **Zonas:** cada tarjeta arranca la zona por los minutos indicados (1 a 240) o la detiene.
- **Estado:** muestra la zona que se inició desde la placa y cuánto le falta. El bus
  del X-Core es de una sola vía, así que no ve los programas propios del X-Core y
  se pierde al reiniciar la placa.
- **Configuración → Zonas:** cantidad de zonas (hasta 48) y sus nombres.
- **Configuración → Horarios:** hasta 16 horarios (días, hora, zona, minutos). La hora
  se toma por NTP; hasta que se sincroniza no se dispara ninguno. El X-Core riega
  una zona por vez: si dos horarios coinciden, queda el último.

Los nombres y los horarios se guardan en la EEPROM emulada de la placa, aparte del
LittleFS: no se pierden al subir un firmware ni un dashboard nuevo.

## Actualizar por WiFi

En `http://X-CORE.local/update` se elige qué se sube:

- **Firmware:** el `.bin` del sketch (**Sketch → Export Compiled Binary**).
- **Dashboard:** una imagen LittleFS de `data/`. Se arma con el `mklittlefs` que
  instala el paquete esp8266, con los valores del layout por defecto
  (*4MB, FS:2MB*):

  ```
  ~/.arduino15/packages/esp8266/tools/mklittlefs/*/mklittlefs \
      -c x-core/data -b 8192 -p 256 -s 0x1FA000 x-core.littlefs.bin
  ```

  Subir el dashboard reemplaza todo el LittleFS. En `x-core-mqtt` eso borra
  también los datos del broker (`config.json`): después hay que volver a
  cargarlos desde el portal.

## Documentación

- [API HTTP](docs/pages/api.md)
- [API MQTT](docs/pages/mqtt.md) (solo `x-core-mqtt`)
- [Conexión al SmartPort](docs/pages/hunterconnection.md) (incluye advertencias de GND y aislamiento)
- [Proyectos relacionados](docs/pages/proyectos-relacionados.md): comparación con otros proyectos Hunter y qué tomamos de cada uno

## Créditos y licencia

Basado en [hunter-wifi](https://github.com/ecodina/hunter-wifi) de Eloi Codina
Torras (2020-2022), que a su vez parte del trabajo de
[Sebastien](https://github.com/seb821/OpenSprinkler-Firmware-Hunter).
Modificaciones © 2026 Emilio Recio: los archivos tomados del original y
modificados lo indican en su encabezado, y el detalle de los cambios está en el
historial de git.

Se distribuye bajo **GNU GPL v3**, igual que el original. Ver [LICENSE](LICENSE).
