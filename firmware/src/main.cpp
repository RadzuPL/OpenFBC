// =============================================================================
// OpenNerfESC - main.cpp
// Entry point. Initialises Serial (debug), BLE server and runs the main loop.
// PWM generation is stubbed out - this build tests BLE parameter transfer only.
// =============================================================================
#include <Arduino.h>
#include "config.h"
#include "params.h"
#include "ble_server.h"

void setup() {
#if DEBUG_MODE
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for USB Serial on ESP32-C6
  Serial.println("[MAIN] OpenNerfESC starting (DEBUG build)");
  Serial.printf("[MAIN] Default spinUpTime : %u ms\n", (uint16_t)DEFAULT_SPIN_UP_TIME);
  Serial.printf("[MAIN] Default targetSpeed : %u %%\n", (uint8_t)DEFAULT_TARGET_SPEED);
  Serial.printf("[MAIN] Default minVoltage : %.2f V\n", (float)DEFAULT_MIN_VOLTAGE);
#endif
  initBleServer();
#if DEBUG_MODE
  Serial.println("[MAIN] BLE server ready");
#endif
}

void loop() {
  // BLE callbacks update spinUpTime, targetSpeed and minVoltage asynchronously.
  // In this test build we only print current values periodically to verify transfer.
#if DEBUG_MODE
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 5000) {
    lastPrint = millis();
    Serial.printf("[MAIN] spinUpTime=%u ms | targetSpeed=%u%% | minVoltage=%.2fV | BLE=%s\n",
                  spinUpTime,
                  targetSpeed,
                  minVoltage,
                  isBleConnected() ? "connected" : "advertising"
    );
  }
#endif
  // TODO: call pwm_control update here in future builds
  delay(10);
}
