#pragma once

// =============================================================================
// OpenFBC - pwm_control.h
// DC motor control: LEDC PWM (single channel) + trigger input + spin-up/down.
// =============================================================================

// Configures LEDC, sets up PWM frequency and trigger pin.
void setupPwm();

// Called every loop(). Reads trigger, handles ramp and sets duty cycle.
void updateMotors();
