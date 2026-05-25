// ==========================================================================
// OpenNerfESC - ble_server.h
// BLE GATT server public interface.
// Exposes extern volatile parameters and helper functions.
// ==========================================================================
#pragma once

#include <stdint.h>

// --- Shared parameters (defined in ble_server.cpp) ---
extern volatile uint16_t spinUpTime;
extern volatile uint8_t  targetSpeed;
extern volatile float    minVoltage;

// --- Public API ---
void initBleServer();
bool isBleConnected();
