// =============================================================================
// OpenNerfESC - pwm_control.cpp
// Sterowanie silnikami DC przez MOSFET (LEDC PWM) oraz odczyt triggera.
//
// Sekwencja działania:
//   Trigger wciśnięty:
//     Faza 1 (Spin-Up): PWM = 100% przez spinUpTime [ms]  <- pełny moment rozruchowy
//     Faza 2 (Cruise):  PWM = targetSpeed [%]             <- prędkość robocza
//   Trigger zwolniony:
//     Natychmiastowy stop: PWM = 0%
//
// Oba silniki połączone równolegle -> jeden kanał LEDC, jeden pin.
// =============================================================================
#include <Arduino.h>
#include "pwm_control.h"
#include "params.h"
#include "config.h"

// Fazy sterowania
enum MotorPhase { PHASE_IDLE, PHASE_SPINUP, PHASE_CRUISE };

// --- Stan wewnętrzny ---
static MotorPhase phase         = PHASE_IDLE;
static uint32_t   phaseStartMs  = 0;

// Przelicza procent (0-100) na wartość LEDC (0-255 dla 8-bit)
static inline uint32_t dutyToLedc(float pct) {
  if (pct <= 0.0f)   return 0;
  if (pct >= 100.0f) return 255;
  return (uint32_t)((pct / 100.0f) * 255.0f);
}

void setupPwm() {
  ledcSetup(LEDC_CHANNEL_MOT, PWM_FREQ_HZ, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM_MOTORS, LEDC_CHANNEL_MOT);
  ledcWrite(LEDC_CHANNEL_MOT, 0);  // silniki zatrzymane przy starcie

  pinMode(PIN_TRIGGER, INPUT);  // active HIGH, zewnętrzny pull-down lub spust

#if DEBUG_MODE
  Serial.printf("[PWM] Setup: pin=%d ch=%d freq=%uHz 8bit | trigger=pin%d\n",
    PIN_PWM_MOTORS, LEDC_CHANNEL_MOT, (unsigned)PWM_FREQ_HZ, PIN_TRIGGER);
#endif
}

void updateMotors() {
  bool triggerHeld = (digitalRead(PIN_TRIGGER) == HIGH);

  if (!triggerHeld) {
    // --- Trigger zwolniony: natychmiastowy stop ---
    if (phase != PHASE_IDLE) {
      phase = PHASE_IDLE;
      ledcWrite(LEDC_CHANNEL_MOT, 0);
#if DEBUG_MODE
      Serial.println("[PWM] Trigger OFF -> STOP");
#endif
    }
    return;
  }

  // --- Trigger wciśnięty ---
  if (phase == PHASE_IDLE) {
    // Zbocze narastające: start fazy Spin-Up
    phaseStartMs = millis();
    if (spinUpTime == 0) {
      // spinUpTime=0 oznacza pominięcie Spin-Up -> od razu Cruise
      phase = PHASE_CRUISE;
      ledcWrite(LEDC_CHANNEL_MOT, dutyToLedc((float)targetSpeed));
#if DEBUG_MODE
      Serial.printf("[PWM] Trigger ON -> CRUISE immediately @ %u%%\n", (unsigned)targetSpeed);
#endif
    } else {
      phase = PHASE_SPINUP;
      ledcWrite(LEDC_CHANNEL_MOT, 255);  // 100% PWM
#if DEBUG_MODE
      Serial.printf("[PWM] Trigger ON -> SPIN-UP 100%% for %ums, then CRUISE @ %u%%\n",
                    (unsigned)spinUpTime, (unsigned)targetSpeed);
#endif
    }
    return;
  }

  if (phase == PHASE_SPINUP) {
    // Sprawdź czy czas Spin-Up minął
    if ((millis() - phaseStartMs) >= (uint32_t)spinUpTime) {
      phase = PHASE_CRUISE;
      ledcWrite(LEDC_CHANNEL_MOT, dutyToLedc((float)targetSpeed));
#if DEBUG_MODE
      Serial.printf("[PWM] SPIN-UP done -> CRUISE @ %u%%\n", (unsigned)targetSpeed);
#endif
    }
    // else: pozostajemy na 100%, nic nie zmieniamy
  }

  // PHASE_CRUISE: PWM już ustawione, nic nie robimy
  // (zmiany targetSpeed przez BLE będą zastosowane przy następnym naciśnięciu)
}
