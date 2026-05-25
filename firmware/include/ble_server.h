#pragma once

// =============================================================================
// OpenNerfESC - ble_server.h
// BLE GATT server interface.
// Handles advertising, connection callbacks and characteristic write callbacks.
// =============================================================================

#include <stdint.h>

// Initialise BLE server, register service and characteristics, start advertising.
void initBleServer();

// Returns true if a BLE central is currently connected.
bool isBleConnected();

// --- Shared parameter variables (written by BLE callbacks, read by main/pwm) ---
// These are declared here and defined in ble_server.cpp.
// Access is safe in single-core Arduino loop context.
extern volatile uint16_t spinUpTime;   // ms, range 0-5000
extern volatile uint8_t  targetSpeed;  // %, range 0-100
extern volatile float    minVoltage;   // V, range 3.0-25.0
