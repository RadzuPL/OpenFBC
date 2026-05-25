// =============================================================================
// OpenNerfESC - ble_server.cpp
// BLE GATT server implementation using NimBLE-Arduino 2.x library.
// Exposes three writable characteristics: spinUpTime, targetSpeed, minVoltage.
// All incoming values are validated before being applied.
// Parameters are persisted to NVS (Preferences) and restored on boot.
// =============================================================================
#include <Arduino.h>
#include "ble_server.h"
#include "config.h"
#include <NimBLEDevice.h>
#include <Preferences.h>

// --- NVS namespace ---
static const char* NVS_NS    = "nerf";
static const char* NVS_SPIN  = "spinUp";
static const char* NVS_SPEED = "speed";
static const char* NVS_VOLT  = "volt";

// --- Parameter storage (definitions for extern declarations in ble_server.h) ---
// Note: not volatile - BLE callbacks on ESP32 run in FreeRTOS tasks, not ISR.
uint16_t spinUpTime  = DEFAULT_SPIN_UP_TIME;
uint8_t  targetSpeed = DEFAULT_TARGET_SPEED;
float    minVoltage  = DEFAULT_MIN_VOLTAGE;

// --- Connection state ---
static bool bleConnected = false;

// --- NVS helpers ---
static void nvsSave() {
  Preferences prefs;
  prefs.begin(NVS_NS, false);
  prefs.putUShort(NVS_SPIN,  spinUpTime);
  prefs.putUChar (NVS_SPEED, targetSpeed);
  prefs.putFloat (NVS_VOLT,  minVoltage);
  prefs.end();
#if DEBUG_MODE
  Serial.printf("[NVS] Saved: spinUp=%u ms  speed=%u%%  volt=%.2f V\n",
                spinUpTime, targetSpeed, minVoltage);
#endif
}

static void nvsLoad() {
  Preferences prefs;
  prefs.begin(NVS_NS, true);  // read-only
  spinUpTime  = prefs.getUShort(NVS_SPIN,  DEFAULT_SPIN_UP_TIME);
  targetSpeed = prefs.getUChar (NVS_SPEED, DEFAULT_TARGET_SPEED);
  minVoltage  = prefs.getFloat (NVS_VOLT,  DEFAULT_MIN_VOLTAGE);
  prefs.end();
#if DEBUG_MODE
  Serial.printf("[NVS] Loaded: spinUp=%u ms  speed=%u%%  volt=%.2f V\n",
                spinUpTime, targetSpeed, minVoltage);
#endif
}

// --- BLE server callbacks ---
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    bleConnected = true;
#if DEBUG_MODE
    Serial.printf("[BLE] Client connected, handle=%d\n", connInfo.getConnHandle());
#endif
    // Stop advertising while connected (optional, saves power)
    NimBLEDevice::stopAdvertising();
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    bleConnected = false;
#if DEBUG_MODE
    Serial.printf("[BLE] Client disconnected, reason=%d - restarting advertising\n", reason);
#endif
    NimBLEDevice::startAdvertising();
  }
};

class SpinUpTimeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getLength() == sizeof(uint16_t)) {
      uint16_t value = 0;
      memcpy(&value, pChar->getValue().data(), sizeof(uint16_t));
      if (value >= SPIN_UP_TIME_MIN && value <= SPIN_UP_TIME_MAX) {
        spinUpTime = value;
        nvsSave();
#if DEBUG_MODE
        Serial.printf("[BLE] spinUpTime updated: %u ms\n", spinUpTime);
#endif
      }
    }
  }
};

class TargetSpeedCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getLength() == sizeof(uint8_t)) {
      uint8_t value = 0;
      memcpy(&value, pChar->getValue().data(), sizeof(uint8_t));
      if (value >= TARGET_SPEED_MIN && value <= TARGET_SPEED_MAX) {
        targetSpeed = value;
        nvsSave();
#if DEBUG_MODE
        Serial.printf("[BLE] targetSpeed updated: %u %%\n", targetSpeed);
#endif
      }
    }
  }
};

class MinVoltageCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getLength() == sizeof(float)) {
      float value = 0.0f;
      memcpy(&value, pChar->getValue().data(), sizeof(float));
      if (value >= MIN_VOLTAGE_MIN && value <= MIN_VOLTAGE_MAX) {
        minVoltage = value;
        nvsSave();
#if DEBUG_MODE
        Serial.printf("[BLE] minVoltage updated: %.2f V\n", minVoltage);
#endif
      }
    }
  }
};

void initBleServer() {
  // Restore last saved parameters from NVS
  nvsLoad();

  // Disable security / bonding - required for Web Bluetooth compatibility
  NimBLEDevice::setSecurityAuth(false, false, false);

  NimBLEDevice::init(BLE_DEVICE_NAME);
  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

  NimBLECharacteristic* pSpinUpTime = pService->createCharacteristic(
    BLE_CHAR_SPIN_UP_TIME,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pSpinUpTime->setCallbacks(new SpinUpTimeCallback());
  pSpinUpTime->setValue(spinUpTime);

  NimBLECharacteristic* pTargetSpeed = pService->createCharacteristic(
    BLE_CHAR_TARGET_SPEED,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pTargetSpeed->setCallbacks(new TargetSpeedCallback());
  pTargetSpeed->setValue(targetSpeed);

  NimBLECharacteristic* pMinVoltage = pService->createCharacteristic(
    BLE_CHAR_MIN_VOLTAGE,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pMinVoltage->setCallbacks(new MinVoltageCallback());
  pMinVoltage->setValue(minVoltage);

  pService->start();
  pServer->start();

  // Advertising: device name + service UUID in scan response for Web BT / Android
  NimBLEAdvertising* pAdv = NimBLEDevice::getAdvertising();
  pAdv->setName(BLE_DEVICE_NAME);
  pAdv->addServiceUUID(BLE_SERVICE_UUID);
  pAdv->enableScanResponse(true);
  pAdv->start();

#if DEBUG_MODE
  Serial.println("[BLE] Server initialised, advertising as: " BLE_DEVICE_NAME);
#endif
}

bool isBleConnected() {
  return bleConnected;
}
