// =============================================================================
// OpenFBC - ble_server.cpp
// BLE GATT server - NimBLE-Arduino 2.x.
// Exposes spinUpTime, spinUpRearmTime, reTriggerSpinUpTime, targetSpeed,
// minVoltage as readable/writable and batteryVoltage as read-only.
// characteristics. Parameter values live in params.cpp (NVS-backed).
// =============================================================================
#include <Arduino.h>
#include "ble_server.h"
#include "battery_monitor.h"
#include "params.h"
#include "config.h"
#include <NimBLEDevice.h>

// --- Connection state ---
static bool bleConnected = false;

// --- BLE server callbacks ---
class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    bleConnected = true;
#if DEBUG_MODE
    Serial.printf("[BLE] Client connected, handle=%d\n", connInfo.getConnHandle());
#endif
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    bleConnected = false;
#if DEBUG_MODE
    Serial.printf("[BLE] Client disconnected, reason=0x%02X - restarting advertising\n", reason);
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
        if (reTriggerSpinUpTime > spinUpTime) {
          reTriggerSpinUpTime = spinUpTime;
        }
        saveParams();
#if DEBUG_MODE
        Serial.printf("[BLE] spinUpTime updated: %u ms (reSpin=%u ms)\n", spinUpTime, reTriggerSpinUpTime);
#endif
      }
    }
  }
};

class SpinUpRearmTimeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getLength() == sizeof(uint16_t)) {
      uint16_t value = 0;
      memcpy(&value, pChar->getValue().data(), sizeof(uint16_t));
      if (value >= SPIN_UP_REARM_TIME_MIN && value <= SPIN_UP_REARM_TIME_MAX) {
        spinUpRearmTime = value;
        saveParams();
#if DEBUG_MODE
        Serial.printf("[BLE] spinUpRearmTime updated: %u ms\n", spinUpRearmTime);
#endif
      }
    }
  }
};

class ReTriggerSpinUpTimeCallback : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    if (pChar->getLength() == sizeof(uint16_t)) {
      uint16_t value = 0;
      memcpy(&value, pChar->getValue().data(), sizeof(uint16_t));
      if (value >= RETRIGGER_SPIN_UP_TIME_MIN && value <= RETRIGGER_SPIN_UP_TIME_MAX) {
        reTriggerSpinUpTime = (value <= spinUpTime) ? value : spinUpTime;
        saveParams();
#if DEBUG_MODE
        Serial.printf("[BLE] reTriggerSpinUpTime updated: %u ms (requested %u ms)\n",
                      reTriggerSpinUpTime, value);
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
        saveParams();
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
        saveParams();
#if DEBUG_MODE
        Serial.printf("[BLE] minVoltage updated: %.2f V\n", minVoltage);
#endif
      }
    }
  }
};

class BatteryVoltageCallback : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* pChar, NimBLEConnInfo& connInfo) override {
    float batteryVoltage = readBatteryVoltageNow();
    pChar->setValue(batteryVoltage);
#if DEBUG_MODE
    Serial.printf("[BLE] batteryVoltage read: %.2f V\n", batteryVoltage);
#endif
  }
};

void initBleServer() {
  // Load last saved parameters from NVS flash
  loadParams();

  // Init BLE stack
  NimBLEDevice::init(BLE_DEVICE_NAME);

  // Disable security/bonding - required for Web Bluetooth compatibility
  // Must be called AFTER NimBLEDevice::init()
  NimBLEDevice::setSecurityAuth(false, false, false);

  NimBLEServer* pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks());

  NimBLEService* pService = pServer->createService(BLE_SERVICE_UUID);

  NimBLECharacteristic* pSpinUpTime = pService->createCharacteristic(
    BLE_CHAR_SPIN_UP_TIME,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pSpinUpTime->setCallbacks(new SpinUpTimeCallback());
  pSpinUpTime->setValue(spinUpTime);

  NimBLECharacteristic* pSpinUpRearmTime = pService->createCharacteristic(
    BLE_CHAR_SPIN_UP_REARM_TIME,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pSpinUpRearmTime->setCallbacks(new SpinUpRearmTimeCallback());
  pSpinUpRearmTime->setValue(spinUpRearmTime);

  NimBLECharacteristic* pReTriggerSpinUpTime = pService->createCharacteristic(
    BLE_CHAR_RETRIGGER_SPIN_UP_TIME,
    NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::READ);
  pReTriggerSpinUpTime->setCallbacks(new ReTriggerSpinUpTimeCallback());
  pReTriggerSpinUpTime->setValue(reTriggerSpinUpTime);

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

  NimBLECharacteristic* pBatteryVoltage = pService->createCharacteristic(
    BLE_CHAR_BATTERY_VOLTAGE,
    NIMBLE_PROPERTY::READ);
  pBatteryVoltage->setCallbacks(new BatteryVoltageCallback());
  pBatteryVoltage->setValue(readBatteryVoltageNow());

  // Service MUST be started before pServer->start() in NimBLE 2.x
  
  pServer->start();

  // Advertising: name + service UUID in scan response (needed for Web BT / Android)
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
