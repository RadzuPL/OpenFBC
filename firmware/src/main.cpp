// =============================================================================
// OpenNerfESC - main.cpp
// Entry point: Serial debug, BLE server, PWM motor control, trigger loop.
// =============================================================================
#include <Arduino.h>
#include "config.h"
#include "params.h"
#include "ble_server.h"
#include "pwm_control.h"

void setup() {
#if DEBUG_MODE
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("[MAIN] OpenNerfESC starting (DEBUG build)");
  Serial.printf("[MAIN] Default spinUpTime : %u ms\n", (uint16_t)DEFAULT_SPIN_UP_TIME);
  Serial.printf("[MAIN] Default targetSpeed : %u %%\n", (uint8_t)DEFAULT_TARGET_SPEED);
  Serial.printf("[MAIN] Default minVoltage : %.2f V\n", (float)DEFAULT_MIN_VOLTAGE);
  Serial.printf("[MAIN] Pins: PWM_M1=%d PWM_M2=%d TRIGGER=%d\n",
                PIN_PWM_M1, PIN_PWM_M2, PIN_TRIGGER);
#endif

  initBleServer();   // ładuje params z NVS, startuje BLE
  setupPwm();        // inicjalizuje LEDC i pin triggera

#if DEBUG_MODE
  Serial.println("[MAIN] Ready. Waiting for trigger.");
#endif
}

void loop() {
  // Główna pętla: obsługa triggera i regulacja PWM
  updateMotors();

#if DEBUG_MODE
  // Co 5 sekund wydrukuj stan parametrów
  static uint32_t lastPrint = 0;
  if (millis() - lastPrint >= 5000) {
    lastPrint = millis();
    Serial.printf("[MAIN] spinUpTime=%u ms | targetSpeed=%u%% | minVoltage=%.2fV | BLE=%s\n",
                  spinUpTime,
                  targetSpeed,
                  minVoltage,
                  isBleConnected() ? "connected" : "advertising");
  }
#endif

  delay(5);  // 200 Hz pętla - wystarczająca rozdzielczość rampy
}
