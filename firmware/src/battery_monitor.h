#pragma once

#include <stdint.h>

void setupBatteryMonitor();
void updateBatteryMonitor(bool motorsActive, uint32_t now);
float readBatteryVoltageNow();
float getLastIdleBatteryVoltage();
bool hasIdleBatteryVoltageSample();
bool isBatteryLow();
