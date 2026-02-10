// Constants.h
#pragma once

// Analog pins
#define PIN_THERMISTOR A1
#define PIN_LDR A0

// Motor pins
#define MOTOR_1_PIN 2
#define MOTOR_2_PIN 3
#define MOTOR_3_PIN 4
#define MOTOR_4_PIN 5

// Thermistor constants (safe names)
#define THERM_VCC 3.3f
#define THERM_ADC_MAX 1023.0f
#define THERM_R2 10000.0f
#define THERM_R0 10000.0f
#define THERM_T0 298.15f
#define THERM_BETA 3950.0f

// EEPROM constants
#define EEPROM_SIZE_BYTES 1080
#define EEPROM_STATE_ADDR 0
#define EEPROM_LOG_START 1
#define EEPROM_MAX_ENTRIES 120 // <128 for wrap-safe logic
#define EEPROM_VALID_MARKER 0x42
