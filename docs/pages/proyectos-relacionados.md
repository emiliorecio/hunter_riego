# Proyectos relacionados

Análisis (noviembre de 2026) de otros proyectos para controlar riegos Hunter por
el puerto REM/SmartPort, comparados con `hunter_riego`.

## Qué buscamos

- Hunter **X-Core de 4 zonas**, controlado desde una **NodeMCU (ESP8266)**.
- Un **dashboard web propio** en la placa: zonas con nombre, horarios propios,
  estado y actualización OTA.
- **Funcionamiento independiente**, sin Home Assistant (por ahora). MQTT queda
  como opción en `x-core-mqtt`.

## Comparación

| Proyecto | Hardware / plataforma | Controlador | Qué tiene | Qué nos aporta |
|---|---|---|---|---|
| [ecodina/hunter-wifi](https://github.com/ecodina/hunter-wifi) | ESP8266, Arduino/PlatformIO, GPL v3 | X-Core / Roam | API REST, portal WiFi, webhook | **Es nuestra base**: protocolo REM, API y portal vienen de acá. |
| [The-Coobaz/hunter-hack](https://github.com/The-Coobaz/hunter-hack) | ESP8266 (D1 mini), Arduino, GPL v3 | No especificado | API HTTP básica (zona, programa, stop) | Nada nuevo. |
| [czsmith/HunterMonitor](https://github.com/czsmith/HunterMonitor) | ESP8266, ESPHome, MIT | **X-Core 400** (4 zonas) | Monitoreo real de las 4 zonas, start/stop de zonas y programas, suspender riego simulando el sensor de lluvia, **alimentación desde los 24 VAC**, web local, Home Assistant | **Detección del estado real de las válvulas** y **alimentación desde 24 VAC**. La referencia más útil para nuestro hardware. |
| [luisfosoares/Hunter_LS](https://github.com/luisfosoares/Hunter_LS) | Placa con XIAO ESP32-C6, ESPHome | X-Core | Hasta 8 zonas, feedback del estado de las válvulas. v2.0.0 (jun 2026), el más activo | Otra referencia para leer el estado de las válvulas. Requiere otro hardware. |
| [aprather21/hunter-x2-esphome](https://github.com/aprather21/hunter-x2-esphome) | ESP32 + buffer 74AHCT125, ESPHome | **X2** (probado en X2-800) | Zonas, stop, programas A/B/C en Home Assistant. Sin estado | **Advertencias de cableado/GND** y el **buffer a 5 V** para la señal REM. Buena documentación del protocolo. |
| [skontzias1/hunter-wifi-esp32](https://github.com/skontzias1/hunter-wifi-esp32) | ESP32, PlatformIO, GPL v3 | Pro-C (y X-Core según otros) | Fork de ecodina para ESP32: web, REST, MQTT, portal, mDNS | Solo si algún día pasamos a ESP32 (manejo de interrupciones con FreeRTOS). |
| [Hilo de Home Assistant: X-Core por REM](https://community.home-assistant.io/t/irrigation-hunter-x-core-remote-control-using-rem-pin/320786/3) | — | X-Core (8 zonas) | Historia: ecodina + `rest_command` en Home Assistant, fork de anubisg1 con MQTT, mDNS y OTA, paquetes de Home Assistant | Ya está incorporado (x-core-mqtt viene de esa línea). Útil si algún día se integra con Home Assistant por MQTT. |

## Conclusión

**`hunter_riego` sigue siendo la opción más cercana a lo que queremos.** Es la
evolución de ecodina + el fork con MQTT/OTA, y es el único con dashboard propio,
horarios y nombres de zonas.

De los demás tomamos ideas, no código:

1. **Cableado seguro** (de hunter-x2-esphome): GND nunca al X-Core, cargador
   aislado, nunca USB de la compu con la placa cableada. Ver
   [hunterconnection.md](hunterconnection.md).
2. **Buffer 74AHCT125** si el X-Core ignora comandos (la señal de 3,3 V tiene
   poco margen).
3. **Estado real de las válvulas** (de HunterMonitor / Hunter_LS):
   optoacopladores en las salidas de zona. Hoy REM es de un solo sentido y el
   dashboard solo muestra lo que la placa mandó.
4. **Alimentación desde los 24 VAC** (de HunterMonitor), cuidando el
   aislamiento.

Si en algún momento Home Assistant pasa a ser lo principal, **HunterMonitor**
es lo más armado para el X-Core 400. A cambio se pierde el dashboard propio.

Los pendientes concretos están en [TODO.md](../../TODO.md).
