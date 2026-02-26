// Constants.h
#pragma once

// Analog pins
#define PIN_THERMISTOR A1
#define PIN_LDR A0

// Motor pins
#define MOTOR_1_PIN 3 // +X
#define MOTOR_2_PIN 2 // eteenpäi +Y
#define MOTOR_3_PIN 6 // oike ylä - 6 pin
#define MOTOR_4_PIN 5 // oike ala - 5 pn

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

// Battery / voltage divider for 2S LiPo
#define LIPO_DIV_R1 18000.0f // top resistor to battery +
#define LIPO_DIV_R2 10000.0f // bottom resistor to GND
#define LIPO_ADC_PIN 22      // analog pin used for battery measurement
#define LIPO_MIN_V 6.6f      // 0% battery
#define LIPO_MAX_V 8.4f      // 100% battery
