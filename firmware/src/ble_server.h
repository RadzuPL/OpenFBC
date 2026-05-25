// =============================================================================
// OpenNerfESC - ble_server.h
// BLE GATT server public interface.
// Exposes extern parameters and helper functions.
// =============================================================================
#pragma once

#include <stdint.h>

// --- Shared parameters (defined in ble_server.cpp) ---
// Not volatile: BLE callbacks run in FreeRTOS tasks, not ISR.
extern uint16_t spinUpTime;
extern uint8_t  targetSpeed;
extern float    minVoltage;

// --- Public API ---
void initBleServer();
bool isBleConnected();
