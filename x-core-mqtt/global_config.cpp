// Based on hunter-wifi by Eloi Codina Torras (GPL v3).
// Modified by Emilio Recio, 2026 (hunter_riego): see the git history.
#include "Arduino.h"

#include "global_config.h"

bool shouldSaveConfig = false;

// callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}