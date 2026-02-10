#pragma once
#include <Arduino.h>
#include "Constants.h"

class VLiPo
{
public:
  VLiPo() {}

  void begin(uint8_t pin = LIPO_ADC_PIN);

  // Read LiPo voltage (V)
  float readVoltage();

  // Read LiPo charge percentage (0–100%)
  uint8_t readPercentage();

private:
  uint8_t _pin;
};
