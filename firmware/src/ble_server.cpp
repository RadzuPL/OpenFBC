// =============================================================================
// OpenNerfESC - ble_server.cpp
// BLE GATT server implementation using NimBLE-Arduino 2.x library.
// Exposes three writable characteristics: spinUpTime, targetSpeed, minVoltage.
// All incoming values are validated before being applied.
// =============================================================================

#include "ble_server.h"
#include "config.h"
#include <NimBLEDevice.h>

// --- Parameter storage (definitions for extern declarations in ble_server.h) ---
volatile uint16_t spinUpTime  = DEFAULT_SPIN_UP_TIME;
volatile uint8_t  targetSpeed = DEFAULT_TARGET_SPEED;
volatile float    minVoltage  = DEFAULT_MIN_VOLTAGE;

// --- Connection state ---
static bool bleConnected = false;

// --- BLE server callbacks (connection / disconnection) ---
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

// --- Characteristic write callbacks ---

class SpinUpTimeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getDataLength() == sizeof(uint16_t)) {
      uint16_t value = 0;
      memcpy(&value, pChar->getValue().data(), sizeof(uint16_t));
      if (value >= SPIN_UP_TIME_MIN && value <= SPIN_UP_TIME_MAX) {
        spinUpTime = value;
#if DEBUG_MODE
        Serial.printf("[BLE] spinUpTime updated: %u ms\n", spinUpTime);
#endif
      } else {
#if DEBUG_MODE
        Serial.printf("[BLE] spinUpTime rejected (out of range): %u\n", value);
#endif
      }
    }
  }
};

class TargetSpeedCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getDataLength() == sizeof(uint8_t)) {
      uint8_t value = 0;
      memcpy(&value, pChar->getValue().data(), sizeof(uint8_t));
      if (value >= TARGET_SPEED_MIN && value <= TARGET_SPEED_MAX) {
        targetSpeed = value;
#if DEBUG_MODE
        Serial.printf("[BLE] targetSpeed updated: %u %%\n", targetSpeed);
#endif
      } else {
#if DEBUG_MODE
        Serial.printf("[BLE] targetSpeed rejected (out of range): %u\n", value);
#endif
      }
    }
  }
};

class MinVoltageCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getDataLength() == sizeof(float)) {
      float value = 0.0f;
      memcpy(&value, pChar->getValue().data(), sizeof(float));
      if (value >= MIN_VOLTAGE_MIN && value <= MIN_VOLTAGE_MAX) {
        minVoltage = value;
#if DEBUG_MODE
        Serial.printf("[BLE] minVoltage updated: %.2f V\n", minVoltage);
#endif
      } else {
#if DEBUG_MODE
        Serial.printf("[BLE] minVoltage rejected (out of range): %.2f\n", value);
#endif
      }
    }
  }
};

// --- Public API ---

void initBleServer() {
  NimBLEDevice::init(BLE_DEVICE_NAME);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

  // spinUpTime characteristic - writable, uint16_t (2 bytes, little-endian)
  NimBLECharacteristic* pSpinUpTime = pService->createCharacteristic(
    BLE_CHAR_SPIN_UP_TIME,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ
  );
  pSpinUpTime->setCallbacks(new SpinUpTimeCallback());
  uint16_t defaultSpin = DEFAULT_SPIN_UP_TIME;
  pSpinUpTime->setValue(defaultSpin);

  // targetSpeed characteristic - writable, uint8_t (1 byte)
  NimBLECharacteristic* pTargetSpeed = pService->createCharacteristic(
    BLE_CHAR_TARGET_SPEED,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ
  );
  pTargetSpeed->setCallbacks(new TargetSpeedCallback());
  uint8_t defaultSpeed = DEFAULT_TARGET_SPEED;
  pTargetSpeed->setValue(defaultSpeed);

  // minVoltage characteristic - writable, float (4 bytes, little-endian)
  NimBLECharacteristic* pMinVoltage = pService->createCharacteristic(
    BLE_CHAR_MIN_VOLTAGE,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ
  );
  pMinVoltage->setCallbacks(new MinVoltageCallback());
  float defaultVoltage = DEFAULT_MIN_VOLTAGE;
  pMinVoltage->setValue(defaultVoltage);

  pService->start();
  NimBLEDevice::startAdvertising();

#if DEBUG_MODE
  Serial.println("[BLE] Server initialised, advertising as: " BLE_DEVICE_NAME);
#endif
}

bool isBleConnected() {
  return bleConnected;
}
