#pragma once

// Analog
#define PIN_THERMISTOR A1
#define PIN_LDR A0

// Motors
#define MOTOR_1_PIN 2
#define MOTOR_2_PIN 3
#define MOTOR_3_PIN 4
#define MOTOR_4_PIN 5

// Thermistor constants
#define VCC 3.3f
#define ADC_MAX 1023.0f
#define R2 10000.0f
#define R0 10000.0f
#define T0 298.15f
#define BETA 3950.0f

// EEPROM
#define EEPROM_SIZE_BYTES 1080
#define EEPROM_STATE_ADDR 0
#define EEPROM_LOG_START 1
#define EEPROM_MAX_ENTRIES 120 // <128 to satisfy wrap-safe logic