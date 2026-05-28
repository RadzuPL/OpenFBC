// =============================================================================
// OpenFBC - config.h
// Central configuration: pins, BLE UUIDs, parameter defaults and ranges.
// =============================================================================
#pragma once

// --- Hardware (ESP32-C3 Super Mini) ---
// Both motors wired in parallel -> single PWM pin -> single MOSFET
#define PIN_PWM_MOTORS    2   // LEDC PWM output -> MOSFET IN
// Trigger (flywheel rev trigger), active LOW with internal pull-up (button to GND)
#define PIN_TRIGGER       4
// ADC: LiPo voltage divider
#define PIN_VOLTAGE_ADC   1

// --- LEDC PWM ---
// 4 kHz: safe for test module with BJT gate-driver.
// Target 20 kHz after TC4420 is mounted on PCB.
#define PWM_FREQ_HZ       4000
#define PWM_RESOLUTION    8        // bits -> range 0-255
#define LEDC_CHANNEL_MOT  0

// --- Debug ---
// Set by build_flags in platformio.ini (-D DEBUG_MODE=1 / 0)
#ifndef DEBUG_MODE
#define DEBUG_MODE 0
#endif

// --- BLE ---
#define BLE_DEVICE_NAME "OpenFBC"

// Service and characteristic UUIDs.
// NOTE: Replace with properly generated UUIDs before production release.
#define BLE_SERVICE_UUID       "12345678-1234-1234-1234-123456789abc"
#define BLE_CHAR_SPIN_UP_TIME  "12345678-1234-1234-1234-123456789001"
#define BLE_CHAR_TARGET_SPEED  "12345678-1234-1234-1234-123456789002"
#define BLE_CHAR_MIN_VOLTAGE   "12345678-1234-1234-1234-123456789003"
#define BLE_CHAR_SPIN_UP_REARM_TIME   "12345678-1234-1234-1234-123456789004"
#define BLE_CHAR_RETRIGGER_SPIN_UP_TIME "12345678-1234-1234-1234-123456789005"
#define BLE_CHAR_BATTERY_VOLTAGE      "12345678-1234-1234-1234-123456789006"

// --- Parameter defaults ---
#define DEFAULT_SPIN_UP_TIME  200u   // ms  - Spin-Up phase duration (100% PWM)
#define DEFAULT_TARGET_SPEED   75u   // %   - PWM duty cycle in Cruise phase
#define DEFAULT_MIN_VOLTAGE  11.1f   // V   - LiPo 3S cutoff (3.7V/cell * 3)
#define DEFAULT_SPIN_UP_REARM_TIME 3000u // ms - minimum trigger release time before main spin-up can arm again
#define DEFAULT_RETRIGGER_SPIN_UP_TIME 50u // ms - short spin-up duration for quick re-trigger

// --- Parameter validation ranges ---
// spinUpTime: 0 - 500 ms
#define SPIN_UP_TIME_MIN  0u
#define SPIN_UP_TIME_MAX  500u

// spinUpRearmTime: 0 - 5000 ms
#define SPIN_UP_REARM_TIME_MIN 0u
#define SPIN_UP_REARM_TIME_MAX 5000u

// reTriggerSpinUpTime: 0 - 100 ms
#define RETRIGGER_SPIN_UP_TIME_MIN 0u
#define RETRIGGER_SPIN_UP_TIME_MAX 100u

// targetSpeed: 0 - 100 %
#define TARGET_SPEED_MIN  0u
#define TARGET_SPEED_MAX  100u

// minVoltage: 3.0 - 15.0 V  (supports 1S-4S LiPo; 4S max = 16.8V but cutoff ~12V)
#define MIN_VOLTAGE_MIN  3.0f
#define MIN_VOLTAGE_MAX  15.0f

// ADC voltage conversion: battery voltage = ADC pin voltage * divider ratio.
// Divider: R1=300kΩ, R2=51kΩ -> ratio = (300+51)/51 = 6.882
// At 16.8V (4S full): ADC pin = 2.44V (safe for ESP32-C3/C6 <2.5V limit and S3 <3.1V)
// At 12.6V (3S full): ADC pin = 1.83V
// Requires 100nF cap across R2 as charge-bucket for ADC sample-hold.
#define BATTERY_DIVIDER_RATIO 6.882f
