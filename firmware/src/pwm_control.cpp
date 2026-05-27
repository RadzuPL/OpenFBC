// =============================================================================
// OpenNerfESC - pwm_control.cpp
// DC motor control via MOSFET (LEDC PWM) and trigger input.
//
// Control sequence:
//   Trigger pressed:
//     Phase 1 (Spin-Up): PWM = 100% for spinUpTime [ms]  <- full startup torque
//                       only if trigger was released for at least spinUpRearmTime [ms]
//                       otherwise use reTriggerSpinUpTime [ms]
//     Phase 2 (Cruise):  PWM = targetSpeed [%]           <- operating speed
//   Trigger released:
//     Immediate stop: PWM = 0%
//
// Both motors wired in parallel -> single LEDC channel, single pin.
// =============================================================================
#include <Arduino.h>
#include "pwm_control.h"
#include "params.h"
#include "config.h"

// Motor control phases
enum MotorPhase { PHASE_IDLE, PHASE_SPINUP, PHASE_CRUISE };

// --- Internal state ---
static MotorPhase phase       = PHASE_IDLE;
static uint32_t   phaseStartMs = 0;
static uint16_t   activeSpinUpDurationMs = 0;
static uint32_t   releaseStartMs = 0;
static bool       previousTriggerHeld = false;
static bool       spinUpArmed = true;

// Converts percent (0-100) to LEDC duty value (0-255 for 8-bit)
static inline uint32_t dutyToLedc(float pct) {
  if (pct <= 0.0f)   return 0;
  if (pct >= 100.0f) return 255;
  return (uint32_t)((pct / 100.0f) * 255.0f);
}

void setupPwm() {
  // Arduino Core 3.x (pioarduino): use ledcAttach() unified API.
  // Both envs (esp32c3-prod and esp32c6-test) use pioarduino, so this is consistent.
  ledcAttach(PIN_PWM_MOTORS, PWM_FREQ_HZ, PWM_RESOLUTION);
  ledcWrite(PIN_PWM_MOTORS, 0);  // motors stopped at startup

  pinMode(PIN_TRIGGER, INPUT_PULLUP);  // active LOW, button to GND

#if DEBUG_MODE
  Serial.printf("[PWM] Setup: pin=%d freq=%uHz %dbit | trigger=pin%d\n",
                PIN_PWM_MOTORS, (unsigned)PWM_FREQ_HZ, PWM_RESOLUTION, PIN_TRIGGER);
#endif
}

void updateMotors() {
  uint32_t now = millis();
  bool triggerHeld = (digitalRead(PIN_TRIGGER) == LOW);

  if (!triggerHeld) {
    // --- Trigger released: immediate stop ---
    if (previousTriggerHeld) {
      releaseStartMs = now;
      spinUpArmed = false;
    }

    if (phase != PHASE_IDLE) {
      phase = PHASE_IDLE;
      ledcWrite(PIN_PWM_MOTORS, 0);
#if DEBUG_MODE
      Serial.println("[PWM] Trigger OFF -> STOP");
#endif
    }

    if (!spinUpArmed && (now - releaseStartMs) >= spinUpRearmTime) {
      spinUpArmed = true;
#if DEBUG_MODE
      Serial.printf("[PWM] Spin-Up re-armed after %ums release\n", (unsigned)spinUpRearmTime);
#endif
    }

    previousTriggerHeld = false;
    return;
  }

  // --- Trigger pressed ---
  if (!previousTriggerHeld && phase == PHASE_IDLE) {
    // Rising edge: start Spin-Up phase
    phaseStartMs = now;
    activeSpinUpDurationMs = spinUpArmed ? spinUpTime : reTriggerSpinUpTime;
    if (activeSpinUpDurationMs == 0) {
      // zero spin-up means go directly to Cruise
      phase = PHASE_CRUISE;
      ledcWrite(PIN_PWM_MOTORS, dutyToLedc((float)targetSpeed));
#if DEBUG_MODE
      if (spinUpArmed) {
        Serial.printf("[PWM] Trigger ON -> CRUISE immediately @ %u%%\n", (unsigned)targetSpeed);
      } else {
        Serial.printf("[PWM] Trigger ON -> CRUISE @ %u%% (quick spin-up disabled)\n",
                      (unsigned)targetSpeed);
      }
#endif
    } else {
      phase = PHASE_SPINUP;
      ledcWrite(PIN_PWM_MOTORS, 255);  // 100% PWM
#if DEBUG_MODE
      if (spinUpArmed) {
        Serial.printf("[PWM] Trigger ON -> SPIN-UP 100%% for %ums, then CRUISE @ %u%%\n",
                      (unsigned)activeSpinUpDurationMs, (unsigned)targetSpeed);
      } else {
        Serial.printf("[PWM] Trigger ON -> QUICK SPIN-UP 100%% for %ums (rearm in %ums), then CRUISE @ %u%%\n",
                      (unsigned)activeSpinUpDurationMs, (unsigned)spinUpRearmTime, (unsigned)targetSpeed);
      }
#endif
    }
  } else if (phase == PHASE_IDLE) {
    ledcWrite(PIN_PWM_MOTORS, dutyToLedc((float)targetSpeed));
  }

  if (phase == PHASE_SPINUP) {
    // Check if Spin-Up time has elapsed
    if ((now - phaseStartMs) >= (uint32_t)activeSpinUpDurationMs) {
      phase = PHASE_CRUISE;
      ledcWrite(PIN_PWM_MOTORS, dutyToLedc((float)targetSpeed));
#if DEBUG_MODE
      Serial.printf("[PWM] SPIN-UP done -> CRUISE @ %u%%\n", (unsigned)targetSpeed);
#endif
    }
    // else: stay at 100%, nothing to change
  }

  // PHASE_CRUISE: PWM already set, nothing to do
  // (targetSpeed changes via BLE will take effect on next trigger press)
  previousTriggerHeld = true;
}
