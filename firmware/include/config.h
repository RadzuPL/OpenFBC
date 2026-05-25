#pragma once

// =============================================================================
// OpenNerfESC - config.h
// Central configuration file. All constants and default values are defined here.
// See /docs/CODING_GUIDELINES.md for naming conventions.
// =============================================================================

// --- Debug Mode ---
// Set to 1 to enable Serial debug output, 0 to disable.
// Controlled via platformio.ini build_flags: -D DEBUG_MODE=1
#ifndef DEBUG_MODE
  #define DEBUG_MODE 0
#endif

// --- BLE Configuration ---
#define BLE_DEVICE_NAME         "OpenNerfESC"
#define BLE_SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define BLE_CHAR_SPIN_UP_TIME   "12345678-1234-1234-1234-123456789001"
#define BLE_CHAR_TARGET_SPEED   "12345678-1234-1234-1234-123456789002"
#define BLE_CHAR_MIN_VOLTAGE    "12345678-1234-1234-1234-123456789003"

// --- Pin Definitions (ESP32-S3 Super Mini) ---
// Note: update for ESP32-C3 in production build
#define PIN_PWM_OUTPUT          2
#define PIN_TRIGGER_INPUT       4
#define PIN_VOLTAGE_ADC         1

// --- Default Parameter Values ---
// spinUpTime: duration in milliseconds of full-power spin-up phase (0-5000 ms)
#define DEFAULT_SPIN_UP_TIME    ((uint16_t)500)

// targetSpeed: PWM duty cycle after spin-up phase, in percent (0-100 %)
#define DEFAULT_TARGET_SPEED    ((uint8_t)75)

// minVoltage: battery voltage threshold below which PWM output is disabled (V)
#define DEFAULT_MIN_VOLTAGE     ((float)6.0f)

// --- Parameter Validation Ranges ---
#define SPIN_UP_TIME_MIN        ((uint16_t)0)
#define SPIN_UP_TIME_MAX        ((uint16_t)5000)
#define TARGET_SPEED_MIN        ((uint8_t)0)
#define TARGET_SPEED_MAX        ((uint8_t)100)
#define MIN_VOLTAGE_MIN         ((float)3.0f)
#define MIN_VOLTAGE_MAX         ((float)25.0f)

// --- PWM Configuration (placeholder, not active in BLE-test build) ---
#define PWM_FREQUENCY_HZ        20000
#define PWM_RESOLUTION_BITS     8
#define PWM_CHANNEL             0
