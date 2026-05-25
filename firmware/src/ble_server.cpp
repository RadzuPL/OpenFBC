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
volatile uint16_t spinUpTime  = DEFAULT_SPIN_UP_TIME;
volatile uint8_t  targetSpeed = DEFAULT_TARGET_SPEED;
volatile float    minVoltage  = DEFAULT_MIN_VOLTAGE;

// --- Connection state ---
static bool bleConnected = false;

// --- NVS helpers ---
static void nvsSave() {
  Preferences prefs;
  prefs.begin(NVS_NS, false);
  prefs.putUShort(NVS_SPIN,  (uint16_t)spinUpTime);
  prefs.putUChar (NVS_SPEED, (uint8_t)targetSpeed);
  prefs.putFloat (NVS_VOLT,  (float)minVoltage);
  prefs.end();
#if DEBUG_MODE
  Serial.printf("[NVS] Saved: spinUp=%u ms  speed=%u%%  volt=%.2f V\n",
                (uint16_t)spinUpTime, (uint8_t)targetSpeed, (float)minVoltage);
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
                (uint16_t)spinUpTime, (uint8_t)targetSpeed, (float)minVoltage);
#endif
}

// --- BLE server callbacks ---
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    bleConnected = true;
#if DEBUG_MODE
    Serial.println("[BLE] Client connected");
#endif
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    bleConnected = false;
#if DEBUG_MODE
    Serial.println("[BLE] Client disconnected - restarting advertising");
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
        Serial.printf("[BLE] spinUpTime updated: %u ms\n", (uint16_t)spinUpTime);
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
        Serial.printf("[BLE] targetSpeed updated: %u %%\n", (uint8_t)targetSpeed);
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
        Serial.printf("[BLE] minVoltage updated: %.2f V\n", (float)minVoltage);
#endif
      }
    }
  }
};

void initBleServer() {
  // Restore last saved parameters from NVS
  nvsLoad();

  NimBLEDevice::init(BLE_DEVICE_NAME);
  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

  NimBLECharacteristic* pSpinUpTime = pService->createCharacteristic(
    BLE_CHAR_SPIN_UP_TIME,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pSpinUpTime->setCallbacks(new SpinUpTimeCallback());
  { uint16_t v = (uint16_t)spinUpTime; pSpinUpTime->setValue(v); }

  NimBLECharacteristic* pTargetSpeed = pService->createCharacteristic(
    BLE_CHAR_TARGET_SPEED,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pTargetSpeed->setCallbacks(new TargetSpeedCallback());
  { uint8_t v = (uint8_t)targetSpeed; pTargetSpeed->setValue(v); }

  NimBLECharacteristic* pMinVoltage = pService->createCharacteristic(
    BLE_CHAR_MIN_VOLTAGE,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pMinVoltage->setCallbacks(new MinVoltageCallback());
  { float v = (float)minVoltage; pMinVoltage->setValue(v); }

  pServer->start();

  // Explicit advertising: name in adv data, 128-bit UUID in scan response.
  // This is required for Android and Web Bluetooth to discover the device.
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
