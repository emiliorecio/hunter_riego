# HTTP API Reference

This project provides an HTTP API to start and stop programs / zones.

Because of limitations in the chosen libraries, the API can only return whether it has correctly received the request. If you add a `result` parameter in your request, the board will send the result of parsing it and starting / stopping a zone or program to the URL you specify.

Note that Hunter's 1-wire protocol is write-only. Therefore, if the board returns that it started a program / zone correctly, it only means that there were no errors in the request and the program, but has no way of knowing if the Hunter Sprinkler System has actually done it. For the same reason, `/api/status` only reports what was started from this board.

Invalid zone numbers (1-48), program numbers (1-4) or times (1-240 minutes) are rejected with `400` and a plain-text message.

## The "result" parameter

If you add a `result` parameter in your request, the board will send a JSON to the webhook you specify containing:

 - `action`: a friendly name of the requested action.
 - `action_num`: a number to indicate the requested action
	 - `1`: start a zone
	 - `2`: stop a zone
	 - `3`: start a program
 - `program` / `zone`: the program or zone started or stopped.
 - `result`: a friendly name of the result of the request.
 - `result_num`: a number to indicate the result of the request.
	 - `0`: no error
	 - `1`: invalid zone number
	 - `2`: invalid watering time
	 - `3`: invalid program number
	 - `Any other`: unknown error

You can change the JSON in the following files:

 - `web_server_scheduled.cpp`, modifying the `toSend` variable.
 - `HunterRoam.cpp`, inside `HunterRoam::errorHint(byte error)`.

## Endpoint to start a zone

GET /api/start/zone/[ZONE-NUMBER]?time=[TIME-MINUTES]&result=[WEBHOOK]

 - Zone number: station to start. It supports up to 48 stations.
 - Time: watering minutes.
 - Result (optional): your webhook.

It returns a JSON containing:

 - `status`: whether it has correctly received your request.
 - `result`: whether the board will send the result to your webhook.

**Example**

GET /api/start/zone/3?time=10&result=https%3A%2F%2Fexample.com%2Fwebhook

It will start zone 3 during 10 minutes and send the result to https://example.com/webhook

## Endpoint to stop a zone

GET /api/stop/zone/[ZONE-NUMBER]?result=[WEBHOOK]

 - Zone number: station to start. It supports up to 48 stations.
 - Result (optional): your webhook.

It returns a JSON containing:

 - `status`: whether it has correctly received your request.
 - `result`: whether the board will send the result to your webhook.

**Example**

GET /api/stop/zone/3?result=https%3A%2F%2Fexample.com%2Fwebhook

It will stop zone 3 and send the result to https://example.com/webhook


## Endpoint to start a program

GET /api/start/program/[PROGRAM-NUM]?result=[WEBHOOK]

 - Program number: program to start. It supports up to 4 programs. Note that Hunter specifies programs with a letter, so `1` is `A`.
 - Result (optional): your webhook.

It returns a JSON containing:

 - `status`: whether it has correctly received your request.
 - `result`: whether the board will send the result to your webhook.

**Example**

GET /api/start/program/2?result=https%3A%2F%2Fexample.com%2Fwebhook

It will start program 2 (Hunter's *B*) and send the result to https://example.com/webhook

## Endpoint to get the status

GET /api/status

It returns a JSON containing:

 - `zone`: zone started from this board that should still be watering, or `0`.
 - `remaining`: seconds left for that zone.
 - `program`: last program started from this board, or `0`.
 - `program_ago`: seconds since that program was started.
 - `time`: board clock (`"YYYY-MM-DD HH:MM"`), or `null` until it syncs with NTP.

It does not see the X-Core's own programs, and it is lost when the board reboots.

## Endpoint to get the settings

GET /api/settings

It returns the zone names and the schedules:

```json
{"zones": ["Front garden", "Back", "", ""],
 "schedules": [{"on": true, "days": 62, "h": 6, "m": 30, "zone": 1, "min": 10}]}
```

 - `zones`: one name per zone (1-48 names, up to 64 bytes each). Its length is the number of zones shown in the dashboard.
 - `schedules`: up to 16. `days` is a bitmask of weekdays (bit 0 = Sunday ... bit 6 = Saturday; `62` is Monday to Friday), `h`:`m` is the start time, `zone` the zone and `min` the watering minutes (1-240).

## Endpoint to save the settings

POST /api/settings

Form field `data` (`application/x-www-form-urlencoded`) with the settings JSON, in the same format as above. It returns `400` and a message if the JSON is not valid. Settings are stored in the emulated EEPROM, so they survive firmware and LittleFS updates.

## Endpoint to open the configuration portal

POST /api/portal

The board reboots and opens the WiFi configuration portal for 5 minutes. If it is not saved in time, the board reboots and connects with the saved credentials.

## Endpoint to get the firmware version

GET /api/version

It returns `{"name": "x-core", "version": "0.2.0"}`.
