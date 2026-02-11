#include "VLiPo.h"
#include "Constants.h"
#include <Arduino.h>

void VLiPo::begin(uint8_t pin)
{
  _pin = pin;
}

// Read LiPo voltage at battery terminals
float VLiPo::readVoltage()
{
  float adc = analogRead(_pin);                              // raw ADC 0..1023
  float v_adc = (adc / THERM_ADC_MAX) * THERM_VCC;           // voltage at ADC pin
  float v_lipo = v_adc * (1.0f + LIPO_DIV_R1 / LIPO_DIV_R2); // scale back to battery
  return v_lipo;
}

// Convert battery voltage to 0–100% using linear approximation
uint8_t VLiPo::readPercentage()
{
  float v = readVoltage();

  if (v <= LIPO_MIN_V)
    return 0;
  if (v >= LIPO_MAX_V)
    return 100;

  // Linear mapping
  return (uint8_t)((v - LIPO_MIN_V) / (LIPO_MAX_V - LIPO_MIN_V) * 100.0f);
}
