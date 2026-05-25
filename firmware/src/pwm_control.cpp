// =============================================================================
// OpenNerfESC - pwm_control.cpp
// Sterowanie 2x silnikami DC przez MOSFET (LEDC PWM) oraz odczyt triggera.
//
// Logika działania:
//  1. Trigger wciśnięty  -> ramp-UP od 0 do targetSpeed w czasie spinUpTime ms
//  2. Trigger puszczony  -> natychmiastowe wygąszenie PWM (silniki stop)
//
// Oba MOSFETy sterowane identycznym sygnałem (jeden ramp, dwa kanały LEDC).
// =============================================================================
#include <Arduino.h>
#include "pwm_control.h"
#include "params.h"
#include "config.h"

// --- Stan wewnętrzny ---
static bool  triggerWasHeld = false;  // poprzedni stan triggera
static float currentDuty    = 0.0f;  // aktualny duty cycle 0.0 - 100.0
static uint32_t rampStartMs = 0;      // kiedy zaczął się ramp

// Przelicza procent (0-100) na wartość LEDC (0-255 dla 8-bit)
static inline uint32_t dutyToLedc(float pct) {
  if (pct <= 0.0f) return 0;
  if (pct >= 100.0f) return 255;
  return (uint32_t)((pct / 100.0f) * 255.0f);
}

void setupPwm() {
  // Skonfiguruj LEDC dla obu kanałów
  ledcSetup(LEDC_CHANNEL_M1, PWM_FREQ_HZ, PWM_RESOLUTION);
  ledcSetup(LEDC_CHANNEL_M2, PWM_FREQ_HZ, PWM_RESOLUTION);
  ledcAttachPin(PIN_PWM_M1, LEDC_CHANNEL_M1);
  ledcAttachPin(PIN_PWM_M2, LEDC_CHANNEL_M2);

  // Silniki na 0 przy starcie
  ledcWrite(LEDC_CHANNEL_M1, 0);
  ledcWrite(LEDC_CHANNEL_M2, 0);

  // Trigger: pull-down wewnętrzny, aktywny HIGH
  pinMode(PIN_TRIGGER, INPUT);

#if DEBUG_MODE
  Serial.printf("[PWM] LEDC M1=pin%d ch%d, M2=pin%d ch%d @ %uHz 8bit\n",
    PIN_PWM_M1, LEDC_CHANNEL_M1,
    PIN_PWM_M2, LEDC_CHANNEL_M2,
    (unsigned)PWM_FREQ_HZ);
  Serial.printf("[PWM] Trigger=pin%d\n", PIN_TRIGGER);
#endif
}

void updateMotors() {
  bool triggerHeld = (digitalRead(PIN_TRIGGER) == HIGH);

  if (triggerHeld && !triggerWasHeld) {
    // --- Zbocze narastające: zacznij ramp-up ---
    rampStartMs  = millis();
    currentDuty  = 0.0f;
    triggerWasHeld = true;
#if DEBUG_MODE
    Serial.printf("[PWM] Trigger ON  -> ramp-up start, target=%u%% in %ums\n",
                  (unsigned)targetSpeed, (unsigned)spinUpTime);
#endif
  }

  if (triggerHeld) {
    // --- Trigger trzymany: obsługuj ramp lub utrzymaj prędkość docelową ---
    if (spinUpTime == 0) {
      // brak rampy - od razu na pełną prędkość
      currentDuty = (float)targetSpeed;
    } else {
      uint32_t elapsed = millis() - rampStartMs;
      if (elapsed >= (uint32_t)spinUpTime) {
        currentDuty = (float)targetSpeed;
      } else {
        currentDuty = ((float)elapsed / (float)spinUpTime) * (float)targetSpeed;
      }
    }
  } else {
    // --- Trigger puszczony: stop ---
    if (triggerWasHeld) {
      currentDuty    = 0.0f;
      triggerWasHeld = false;
#if DEBUG_MODE
      Serial.println("[PWM] Trigger OFF -> motors stop");
#endif
    }
  }

  uint32_t ledc = dutyToLedc(currentDuty);
  ledcWrite(LEDC_CHANNEL_M1, ledc);
  ledcWrite(LEDC_CHANNEL_M2, ledc);
}
