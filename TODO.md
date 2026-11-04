# TODO

## Seguridad
- [ ] (Pospuesto) Contraseña para la API (`/api/start/...`, `/api/stop/...`, y desde 0.2.0 `POST /api/settings` y `POST /api/portal`): hoy cualquiera en la red WiFi puede regar, cambiar los horarios o reiniciar la placa en modo portal.
- [ ] (Pospuesto) Contraseña para `/update`: hoy cualquiera en la red puede grabar otro firmware.
- [x] Contraseña para la red del portal de configuración: `PWD_AP` en `secrets.h`, con chequeo de 8-63 caracteres al compilar.
- [ ] (Pospuesto) Parámetro `result` (webhook): la placa hace un POST a cualquier URL que le pasen. Se deja para usarlo más adelante con Home Assistant.

## Instalación en el X-Core
- [ ] Cablear: D1 → REM, VIN → AC#2 + 5 V del cargador, GND → 0 V del cargador (cargador aislado).
- [ ] Probar una zona 1 minuto + Detener, y un programa.
- [x] Probar el portal *WateringSystemAP* desde cero (Erase Flash: "All Flash Contents", luego volver a subir el LittleFS). Probado en la mesa el 2026-11-03 con x-core 0.2.0.
- [ ] Actualizar el diagrama `docs/images/connection.png` (todavía muestra D0).
- [ ] Alimentar la placa desde los 24 VAC del X-Core, sin cargador aparte. Ojo: se pierde el aislamiento del cargador; tomar como base el circuito de [HunterMonitor](https://github.com/czsmith/HunterMonitor) en vez de improvisar.
- [ ] Recordar al cablear: GND **solo** al 0 V del cargador (nunca a un borne del X-Core), cargador aislado de 2 patas, y **nunca** enchufar el USB de la compu con la placa cableada (actualizar por `/update`). Detalle en `docs/pages/hunterconnection.md`.
- [ ] Si el X-Core ignora comandos: agregar un buffer SN74AHCT125 entre D1 y REM para que la señal llegue a 5 V (como [hunter-x2-esphome](https://github.com/aprather21/hunter-x2-esphome)).

## Estado real de las válvulas (a futuro)
- [ ] Detectar si cada zona está regando de verdad. Hoy el dashboard solo muestra lo que la placa mandó (REM es de un solo sentido): no ve los programas propios del X-Core ni el dial manual, y no confirma que la válvula abrió. Idea: un optoacoplador por salida de zona (H11AA1, o PC817 + resistencia + diodo) que detecte los 24 VAC hacia la válvula, leído en D2, D5, D6 y D7, y mostrarlo en el dashboard / `/api/status`. Referencias: [HunterMonitor](https://github.com/czsmith/HunterMonitor) (X-Core 400) y [Hunter_LS](https://github.com/luisfosoares/Hunter_LS).

## OTA / actualización
- [ ] Probar subir el firmware por WiFi desde `/update` (el 0.2.1 se subió por USB).
- [ ] Permitir subir la imagen del LittleFS (dashboard) desde `/update`. Implementado en 0.2.0, falta probar en la placa.

## Dashboard / personalización
- [x] Nombres para las zonas ("Jardín del frente", etc.). Probado en la placa (2026-11-03).
- [x] Mostrar qué zona está regando y cuánto le falta (solo lo que se inicia desde la placa). Probado en la placa con los horarios (2026-11-03).
- [x] Horarios propios desde la web. Probado en la placa (2026-11-03): hora NTP correcta y dos horarios disparados a tiempo.
- [ ] (Opcional) Que el LED parpadee solo mientras se envía un comando. Implementado en 0.2.0 (LED de la ESP-12, D4), falta probar en la placa.

## x-core-mqtt
- [ ] Forma de reabrir el portal para cambiar los datos del broker MQTT (con `autoConnect` ya no se abre solo). Implementado en 0.2.0 (link en el dashboard, en los dos sketches), falta probar en la placa.

## Red
- [x] `X-CORE.local` dejaba de resolver (el ESP8266 con modem sleep pierde el multicast de mDNS). Arreglado en 0.2.1 con `WiFi.setSleepMode(WIFI_NONE_SLEEP)`; probado en la placa.
- [ ] Reservar una IP fija para la placa en el router, como respaldo si `.local` falla.

## Limpieza de código
- [x] `HUNTER_PIN` en `HunterRoam.h` no se usa y dice D0: borrarlo.
