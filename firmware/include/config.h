// =============================================================================
// OpenNerfESC - config.h
// Central configuration: pins, BLE UUIDs, parameter defaults and ranges.
// =============================================================================
#pragma once

// --- Hardware (ESP32-C3 Super Mini) ---
// Motor 1 PWM output  → MOSFET module IN  (silnik 1)
#define PIN_PWM_M1        2
// Motor 2 PWM output  → MOSFET module IN  (silnik 2)
#define PIN_PWM_M2        3
// Trigger input       → spust wyrzutni (active HIGH, internal pull-down)
#define PIN_TRIGGER       4
// ADC input           → dzielnik napięcia LiPo
#define PIN_VOLTAGE_ADC   1

// --- LEDC PWM ---
// Częstotliwość 4 kHz - bezpieczna dla modułu z BJT gate-driver.
// Docelowo 20 kHz po montażu TC4420 na PCB.
#define PWM_FREQ_HZ       4000
#define PWM_RESOLUTION    8       // bity → zakres 0-255
#define LEDC_CHANNEL_M1   0
#define LEDC_CHANNEL_M2   1

// --- Debug ---
// Set by build_flags in platformio.ini (-D DEBUG_MODE=1 / 0)
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

// --- BLE ---
#define BLE_DEVICE_NAME "OpenNerfESC"

// Service and characteristic UUIDs.
// NOTE: Replace with properly generated UUIDs before production release.
#define BLE_SERVICE_UUID       "12345678-1234-1234-1234-123456789abc"
#define BLE_CHAR_SPIN_UP_TIME  "12345678-1234-1234-1234-123456789001"
#define BLE_CHAR_TARGET_SPEED  "12345678-1234-1234-1234-123456789002"
#define BLE_CHAR_MIN_VOLTAGE   "12345678-1234-1234-1234-123456789003"

// --- Parameter defaults ---
#define DEFAULT_SPIN_UP_TIME  200u   // ms - ramp time from 0 to target speed
#define DEFAULT_TARGET_SPEED  75u    // %  - steady-state duty cycle
#define DEFAULT_MIN_VOLTAGE   11.1f  // V  - LiPo 3S cutoff

// --- Parameter validation ranges ---
// spinUpTime: 0 - 500 ms
#define SPIN_UP_TIME_MIN  0u
#define SPIN_UP_TIME_MAX  500u

// targetSpeed: 0 - 100 %
#define TARGET_SPEED_MIN  0u
#define TARGET_SPEED_MAX  100u

// minVoltage: 3.0 - 15.0 V  (supports 1S-4S LiPo)
#define MIN_VOLTAGE_MIN   3.0f
#define MIN_VOLTAGE_MAX   15.0f
