# TODO

## Seguridad
- [ ] (Pospuesto) Contraseña para la API (`/api/start/...`, `/api/stop/...`, y desde 0.2.0 `POST /api/settings` y `POST /api/portal`): hoy cualquiera en la red WiFi puede regar, cambiar los horarios o reiniciar la placa en modo portal.
- [ ] (Pospuesto) Contraseña para `/update`: hoy cualquiera en la red puede grabar otro firmware.
- [x] Contraseña para la red del portal de configuración: `PWD_AP` en `secrets.h`, con chequeo de 8-63 caracteres al compilar.
- [ ] (Pospuesto) Parámetro `result` (webhook): la placa hace un POST a cualquier URL que le pasen. Se deja para usarlo más adelante con Home Assistant.

## Instalación en el X-Core
- [ ] Cablear: D1 → REM, VIN → AC#2 + 5 V del cargador, GND → 0 V del cargador (cargador aislado).
- [ ] Probar una zona 1 minuto + Detener, y un programa.
- [ ] Probar el portal *WateringSystemAP* desde cero (Erase Flash: "All Flash Contents", luego volver a subir el LittleFS): la placa tenía el WiFi guardado de un proyecto anterior y el portal nunca apareció.
- [ ] Actualizar el diagrama `docs/images/connection.png` (todavía muestra D0).
- [ ] Alimentar la placa desde los 24 VAC del X-Core, sin cargador aparte.

## OTA / actualización
- [ ] Permitir subir la imagen del LittleFS (dashboard) desde `/update`. Implementado en 0.2.0, falta probar en la placa.

## Dashboard / personalización
- [ ] Nombres para las zonas ("Jardín del frente", etc.). Implementado en 0.2.0, falta probar en la placa.
- [ ] Mostrar qué zona está regando y cuánto le falta. Implementado en 0.2.0 (solo lo que se inicia desde la placa), falta probar en la placa.
- [ ] Horarios propios desde la web. Implementado en 0.2.0, falta probar en la placa (que la hora NTP sea la correcta).
- [ ] (Opcional) Que el LED parpadee solo mientras se envía un comando. Implementado en 0.2.0 (LED de la ESP-12, D4), falta probar en la placa.

## x-core-mqtt
- [ ] Forma de reabrir el portal para cambiar los datos del broker MQTT (con `autoConnect` ya no se abre solo). Implementado en 0.2.0 (link en el dashboard, en los dos sketches), falta probar en la placa.

## Limpieza de código
- [x] `HUNTER_PIN` en `HunterRoam.h` no se usa y dice D0: borrarlo.
