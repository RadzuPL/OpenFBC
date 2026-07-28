// =============================================================================
// OpenFBC - pwm_control.cpp
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
#include "battery_monitor.h"
#include "pwm_control.h"
#include "params.h"
#include "config.h"

// Motor control phases
enum MotorPhase { PHASE_IDLE, PHASE_SPINUP, PHASE_CRUISE };
enum MotorAlert { ALERT_NONE, ALERT_INACTIVITY, ALERT_LOW_BATTERY };

// --- Internal state ---
static MotorPhase phase       = PHASE_IDLE;
static uint32_t   phaseStartMs = 0;
static uint16_t   activeSpinUpDurationMs = 0;
static uint32_t   releaseStartMs = 0;
static bool       previousTriggerHeld = false;
static bool       spinUpArmed = true;
static uint32_t   lastTriggerPressMs = 0;
static uint32_t   lastReminderMs = 0;
static MotorAlert alertMode = ALERT_NONE;
static bool       alertOutputHigh = false;
static uint8_t    alertPulseIndex = 0;
static uint32_t   alertSegmentStartMs = 0;

// Converts percent (0-100) to LEDC duty value (0-255 for 8-bit)
static inline uint32_t dutyToLedc(float pct) {
  if (pct <= 0.0f)   return 0;
  if (pct >= 100.0f) return 255;
  return (uint32_t)((pct / 100.0f) * 255.0f);
}

static void stopAlert() {
  alertMode = ALERT_NONE;
  alertOutputHigh = false;
  alertPulseIndex = 0;
}

static void startAlert(MotorAlert mode, uint32_t now) {
  alertMode = mode;
  alertOutputHigh = true;
  alertPulseIndex = 0;
  alertSegmentStartMs = now;
  ledcWrite(PIN_PWM_MOTORS, dutyToLedc((float)MOTOR_ALERT_DUTY_PERCENT));

#if DEBUG_MODE
  Serial.printf("[PWM] Alert start: %s\n",
                mode == ALERT_LOW_BATTERY ? "low-battery" : "inactivity");
#endif
}

static bool updateAlert(uint32_t now) {
  if (alertMode == ALERT_NONE) {
    return false;
  }

  if (alertOutputHigh) {
    if ((now - alertSegmentStartMs) >= MOTOR_ALERT_PULSE_ON_TIME_MS) {
      alertOutputHigh = false;
      alertSegmentStartMs = now;
      ledcWrite(PIN_PWM_MOTORS, 0);
    }
    return true;
  }

  if ((now - alertSegmentStartMs) < MOTOR_ALERT_PULSE_OFF_TIME_MS) {
    return true;
  }

  alertPulseIndex++;
  if (alertPulseIndex >= MOTOR_ALERT_PULSE_COUNT) {
    stopAlert();
    ledcWrite(PIN_PWM_MOTORS, 0);
#if DEBUG_MODE
    Serial.println("[PWM] Alert complete");
#endif
    return false;
  }

  alertOutputHigh = true;
  alertSegmentStartMs = now;
  ledcWrite(PIN_PWM_MOTORS, dutyToLedc((float)MOTOR_ALERT_DUTY_PERCENT));
  return true;
}

static void recordTriggerPress(uint32_t now) {
  lastTriggerPressMs = now;
  lastReminderMs = now;
}

static void finalizeUpdate(uint32_t now, bool triggerHeld) {
  updateBatteryMonitor((phase != PHASE_IDLE) || (alertMode != ALERT_NONE), now);
  previousTriggerHeld = triggerHeld;
}

void setupPwm() {
  // Arduino Core 3.x (pioarduino): use ledcAttach() unified API.
  // Both envs (esp32c3-prod and esp32c6-test) use pioarduino, so this is consistent.
  ledcAttach(PIN_PWM_MOTORS, PWM_FREQ_HZ, PWM_RESOLUTION);
  ledcWrite(PIN_PWM_MOTORS, 0);  // motors stopped at startup

  pinMode(PIN_TRIGGER, INPUT_PULLUP);  // active LOW, button to GND
  lastTriggerPressMs = millis();
  lastReminderMs = lastTriggerPressMs;

#if DEBUG_MODE
  Serial.printf("[PWM] Setup: pin=%d freq=%uHz %dbit | trigger=pin%d\n",
                PIN_PWM_MOTORS, (unsigned)PWM_FREQ_HZ, PWM_RESOLUTION, PIN_TRIGGER);
#endif
}

void updateMotors() {
  uint32_t now = millis();
  bool triggerHeld = (digitalRead(PIN_TRIGGER) == LOW);

  if (triggerHeld && !previousTriggerHeld) {
    recordTriggerPress(now);
  }

  if (triggerHeld && isBatteryLow()) {
    phase = PHASE_IDLE;
    spinUpArmed = false;
    releaseStartMs = now;

    if (alertMode == ALERT_INACTIVITY) {
      stopAlert();
    }
    if (!previousTriggerHeld && alertMode == ALERT_NONE) {
      startAlert(ALERT_LOW_BATTERY, now);
    }

    if (!updateAlert(now)) {
      ledcWrite(PIN_PWM_MOTORS, 0);
    }

#if DEBUG_MODE
    if (!previousTriggerHeld) {
      Serial.printf("[PWM] Trigger blocked by low battery: %.2f V < %.2f V\n",
                    getLastIdleBatteryVoltage(), minVoltage);
    }
#endif

    finalizeUpdate(now, triggerHeld);
    return;
  }

  if (triggerHeld && alertMode != ALERT_NONE) {
    stopAlert();
    ledcWrite(PIN_PWM_MOTORS, 0);
  }

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

    if ((now - lastTriggerPressMs) >= INACTIVITY_REMINDER_DELAY_MS &&
        (now - lastReminderMs) >= INACTIVITY_REMINDER_INTERVAL_MS) {
      startAlert(ALERT_INACTIVITY, now);
      lastReminderMs = now;
    }

    updateAlert(now);

    finalizeUpdate(now, triggerHeld);
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
  finalizeUpdate(now, triggerHeld);
}
