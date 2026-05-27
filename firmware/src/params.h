// =============================================================================
// OpenNerfESC - params.h
// Shared ESC parameters: runtime variables + NVS persist/load API.
// Include this in any module that reads or writes ESC parameters.
// =============================================================================
#pragma once

#include <stdint.h>

// --- Runtime parameter values ---
// Defined in params.cpp; extern here so all modules share one instance.
extern uint16_t spinUpTime;   // ms   [SPIN_UP_TIME_MIN .. SPIN_UP_TIME_MAX]
extern uint16_t spinUpRearmTime; // ms [SPIN_UP_REARM_TIME_MIN .. SPIN_UP_REARM_TIME_MAX]
extern uint16_t reTriggerSpinUpTime; // ms [RETRIGGER_SPIN_UP_TIME_MIN .. RETRIGGER_SPIN_UP_TIME_MAX], capped to spinUpTime
extern uint8_t  targetSpeed;  // %    [TARGET_SPEED_MIN .. TARGET_SPEED_MAX]
extern float    minVoltage;   // V    [MIN_VOLTAGE_MIN  .. MIN_VOLTAGE_MAX]

// --- NVS persistence API ---
// loadParams() - call once at boot; fills above vars from NVS flash.
//                Falls back to defaults from config.h if NVS is empty.
// saveParams() - call after any change; atomically writes all persisted values.
void loadParams();
void saveParams();
