// =============================================================================
// OpenNerfESC - params.cpp
// NVS (Non-Volatile Storage) persistence for ESC parameters.
// Provides loadParams() / saveParams() used by ble_server and main.
// =============================================================================
#include "params.h"
#include "config.h"
#include <Preferences.h>
#include <Arduino.h>

// --- NVS key strings ---
static const char* NVS_NS    = "nerf";
static const char* NVS_SPIN  = "spinUp";
static const char* NVS_SPEED = "speed";
static const char* NVS_VOLT  = "volt";

// --- Parameter definitions (extern declared in params.h) ---
uint16_t spinUpTime  = DEFAULT_SPIN_UP_TIME;
uint8_t  targetSpeed = DEFAULT_TARGET_SPEED;
float    minVoltage  = DEFAULT_MIN_VOLTAGE;

// ---------------------------------------------------------------------------
void loadParams() {
  Preferences prefs;
  prefs.begin(NVS_NS, true);  // read-only
  spinUpTime  = prefs.getUShort(NVS_SPIN,  DEFAULT_SPIN_UP_TIME);
  targetSpeed = prefs.getUChar (NVS_SPEED, DEFAULT_TARGET_SPEED);
  minVoltage  = prefs.getFloat (NVS_VOLT,  DEFAULT_MIN_VOLTAGE);
  prefs.end();
#if DEBUG_MODE
  Serial.printf("[NVS] Loaded: spinUp=%u ms  speed=%u%%  volt=%.2f V\n",
                spinUpTime, targetSpeed, minVoltage);
#endif
}

void saveParams() {
  Preferences prefs;
  prefs.begin(NVS_NS, false);
  prefs.putUShort(NVS_SPIN,  spinUpTime);
  prefs.putUChar (NVS_SPEED, targetSpeed);
  prefs.putFloat (NVS_VOLT,  minVoltage);
  prefs.end();
#if DEBUG_MODE
  Serial.printf("[NVS] Saved: spinUp=%u ms  speed=%u%%  volt=%.2f V\n",
                spinUpTime, targetSpeed, minVoltage);
#endif
}
