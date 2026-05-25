#pragma once

// =============================================================================
// OpenNerfESC - pwm_control.h
// Sterowanie silnikami: LEDC PWM (2 kanały) + odczyt triggera + ramp-up/down.
// =============================================================================

// Inicjalizuje LEDC, konfiguruje piny PWM i triggera.
void setupPwm();

// Wywoływana co loop(). Czyta trigger, obsługuje ramp i ustawia duty cycle.
void updateMotors();
