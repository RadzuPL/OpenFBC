// =============================================================================
// OpenFBC - battery_monitor.cpp
// Battery voltage measurement, cached idle-only cutoff sampling and helpers.
// =============================================================================
#include <Arduino.h>
#include "battery_monitor.h"
#include "config.h"
#include "params.h"

static float    lastIdleBatteryVoltage = 0.0f;
static bool     idleBatteryVoltageValid = false;
static uint32_t motorsIdleSinceMs = 0;
static uint32_t lastIdleMeasurementMs = 0;

float readBatteryVoltageNow() {
  uint32_t pinMilliVolts = analogReadMilliVolts(PIN_VOLTAGE_ADC);
  if (pinMilliVolts == 0) {
    uint16_t raw = analogRead(PIN_VOLTAGE_ADC);
    pinMilliVolts = (uint32_t)(((float)raw / 4095.0f) * 3300.0f);
  }
  return ((float)pinMilliVolts / 1000.0f) * BATTERY_DIVIDER_RATIO;
}

void setupBatteryMonitor() {
  pinMode(PIN_VOLTAGE_ADC, INPUT);
  analogReadResolution(12);
#if defined(ARDUINO_ARCH_ESP32)
  analogSetPinAttenuation(PIN_VOLTAGE_ADC, ADC_11db);
#endif

  motorsIdleSinceMs = millis();
  lastIdleMeasurementMs = 0;
  idleBatteryVoltageValid = false;
}

void updateBatteryMonitor(bool motorsActive, uint32_t now) {
  if (motorsActive) {
    motorsIdleSinceMs = now;
    return;
  }

  if ((now - motorsIdleSinceMs) < BATTERY_IDLE_SAMPLE_DELAY_MS) {
    return;
  }

  if (idleBatteryVoltageValid && (now - lastIdleMeasurementMs) < BATTERY_IDLE_SAMPLE_INTERVAL_MS) {
    return;
  }

  lastIdleBatteryVoltage = readBatteryVoltageNow();
  idleBatteryVoltageValid = true;
  lastIdleMeasurementMs = now;

#if DEBUG_MODE
  Serial.printf("[BAT] Idle sample: %.2f V (min=%.2f V, low=%s)\n",
                lastIdleBatteryVoltage,
                minVoltage,
                isBatteryLow() ? "yes" : "no");
#endif
}

float getLastIdleBatteryVoltage() {
  return lastIdleBatteryVoltage;
}

bool hasIdleBatteryVoltageSample() {
  return idleBatteryVoltageValid;
}

bool isBatteryLow() {
  return idleBatteryVoltageValid && (lastIdleBatteryVoltage < minVoltage);
}
